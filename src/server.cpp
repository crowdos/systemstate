#include <iostream>
#include <cstring>
#include "server.h"
#include "utils.h"
#include "stateplugin.h"
#include <vector>
#include <algorithm>
extern "C" {
#include <ixp.h>
};

using namespace systemstate;

static char Enoperm[] = "permission denied",
  Enofile[] = "file not found",
  Ebadvalue[] = "bad value";

#define DEFAULT_SIZE 4096

static char fillState(IxpStat& s, const Node *& node) {
  static char id[] = "root";

  s.qid.type = node->type() == Node::Dir ? P9_QTDIR : P9_QTFILE;
  s.qid.path = reinterpret_cast<uint64_t>(node);
  s.qid.version = 0;

  s.type = 0;
  s.dev = 0;
  s.mode = P9_DMREAD;
  if (node->type() == Node::Dir) {
    s.mode |= P9_DMDIR;
  } else {
    s.mode |= P9_DMTMP;
  }

  s.atime = 0;
  s.mtime = 0;
  s.length = DEFAULT_SIZE;
  s.name = const_cast<char *>(node->name().c_str());
  s.uid = id;
  s.gid = id;
  s.muid = id;
}

class Dbg {
public:
  Dbg(const char *id, Ixp9Req *r) :
  m_id(id) {
    if (r) {
      Node *n = reinterpret_cast<Node *>(r->fid->aux);
      if (n) {
	std::cerr << ">> " << m_id << " " << n->name() << std::endl;
	return;
      }
    }

    std::cerr << ">> " << m_id << std::endl;
  }

  ~Dbg() {
    std::cerr << "<< " << m_id << std::endl;
  }

private:
  std::string m_id;
};

// TODO: We do not check whether another server is already running or not.
// I am nit aware of any race-free way to do it.
Server::Server() :
  m_srv(0),
  m_fd(-1),
  m_conn(0) {

}

Server::~Server() {
  // TODO:
}

bool Server::start() {
  if (m_srv) {
    std::cerr << "Server already running" << std::endl;
    return false;
  }

  if (!Utils::getAddress(m_addr)) {
    // getAddress() will print an error.
    return false;
  }

  m_fd = ixp_announce(m_addr.c_str());

  if (m_fd == -1) {
    std::cerr << "Failed to listen on address " << m_addr << ": " << ixp_errbuf() << std::endl;
    return false;
  }

  m_srv = new IxpServer;

  memset(m_srv, 0x0, sizeof(IxpServer));

  m_table = new Ixp9Srv;
  m_table->aux = this;

  m_table->attach = [] (Ixp9Req *r) {
    Dbg("Attach", r);
    Server *server = reinterpret_cast<Server *>(r->srv->aux);
    r->fid->qid.type = P9_QTDIR;
    r->fid->qid.path = reinterpret_cast<uint64_t>(server->root());
    r->fid->aux = reinterpret_cast<void *>(server->root());
    r->ofcall.rattach.qid = r->fid->qid;
    ixp_respond(r, 0);
  };

  m_table->clunk = [] (Ixp9Req *r) {
    Dbg("clunk", r);
    Node *n = reinterpret_cast<Node *>(r->fid->aux);
    if (n->type() == Node::File) {
      // TODO:
    }

    // Nothing for directories.

    ixp_respond(r, 0);
  };

  m_table->create = [] (Ixp9Req *r) {
    Dbg("create", r);
    // TODO:
  };

  m_table->flush = [] (Ixp9Req *r) {
    Dbg("flush", r);
    // TODO:
  };

  m_table->open = [] (Ixp9Req *r) {
    Dbg("open", r);

    Node *n = reinterpret_cast<Node *>(r->fid->aux);

    if (n->type() == Node::File) {
      FileNode *f = dynamic_cast<FileNode *>(n);
      if (!f->plugin()->start(f)) {
	ixp_respond(r, Enofile);
      }
    }

    ixp_respond(r, 0);
  };

  m_table->read = [] (Ixp9Req *r) {
    Dbg("read", r);

    Node *n = reinterpret_cast<Node *>(r->fid->aux);

    if (n->type() == Node::File) {
      if (r->ifcall.tread.offset == 0) {
	FileNode *file = dynamic_cast<FileNode *>(n);
	std::string data;
	if (file->plugin()->read(file, data)) {
	  r->ofcall.rread.count = data.size();
	  r->ofcall.rread.data = reinterpret_cast<char *>(ixp_emallocz(r->ofcall.rread.count));
	  memcpy(r->ofcall.rread.data, data.c_str(), r->ofcall.rread.count);
	} else {
	  ixp_respond(r, Ebadvalue);
	  return;
	}
      }
    } else if (r->ifcall.tread.offset == 0) {
      DirNode *d = dynamic_cast<DirNode *>(n);
      std::vector<IxpStat> stats;
      for (int x = 0; x < d->numberOfChildren(); x++) {
	const Node *child = d->childAt(x);
	IxpStat s;
	fillState(s, child);
	stats.push_back(std::move(s));
      }

      int size = 0;
      std::for_each(stats.begin(), stats.end(), [&size](const IxpStat& s) mutable {
	  size += ixp_sizeof_stat(const_cast<IxpStat *>(&s));
	});

      char *buff = reinterpret_cast<char *>(ixp_emallocz(size));
      IxpMsg m = ixp_message(buff, size, MsgPack);
      std::for_each(stats.begin(), stats.end(), [&m](const IxpStat& s) {
	  ixp_pstat(&m, const_cast<IxpStat *>(&s));
	});

      r->ofcall.rread.count = size;
      r->ofcall.rread.data = m.data;
    } else {
      r->ofcall.rread.count = 0;
    }
    ixp_respond(r, 0);
  };

  m_table->remove = [] (Ixp9Req *r) {
    Dbg("remove", r);
    ixp_respond(r, Enoperm);
  };

  m_table->stat = [] (Ixp9Req *r) {
    Dbg("stat", r);
    const Node *n = reinterpret_cast<Node *>(r->fid->aux);

    IxpStat s;
    fillState(s, n);

    uint16_t size;
    r->fid->qid = s.qid;
    r->ofcall.rstat.nstat = size = ixp_sizeof_stat(&s);
    char *buff = reinterpret_cast<char *>(ixp_emallocz(size));
    IxpMsg m = ixp_message(buff, size, MsgPack);
    ixp_pstat(&m, &s);
    r->ofcall.rstat.stat = reinterpret_cast<uint8_t *>(m.data);
    ixp_respond(r, 0);
  };

  m_table->walk = [] (Ixp9Req *r) {
    Dbg("walk", r);
    Node *n = reinterpret_cast<Node *>(r->fid->aux);
    DirNode *d = dynamic_cast<DirNode *>(n);
    if (!n) {
      ixp_respond(r, Ebadvalue);
      return;
    }

    for (int x = 0; x < r->ifcall.twalk.nwname; x++) {
      char *name = r->ifcall.twalk.wname[x];
      std::deque<Node *>::iterator iter =
      std::find_if(d->begin(), d->end(), [name](Node * node) -> bool {
	  return node->name() == name;
	});

      if (iter == d->end()) {
	std::cerr << "Cannot find node " << name << std::endl;
	ixp_respond(r, Ebadvalue);
	return;
      }

      n = *iter;
      r->ofcall.rwalk.wqid[x].type = n->type() == Node::Dir ? P9_QTDIR : P9_QTTMP;
      r->ofcall.rwalk.wqid[x].path = reinterpret_cast<uint64_t>(n);
    }

    if (!n) {
      ixp_respond(r, Ebadvalue);
      return;
    }

    r->newfid->aux = n;
    r->ofcall.rwalk.nwqid = r->ifcall.twalk.nwname; // TODO: ??
    ixp_respond(r, 0);
  };

  m_table->write = [] (Ixp9Req *r) {
    Dbg("write", r);
    // TODO:
  };

  m_table->wstat = [] (Ixp9Req *r) {
    Dbg("wstat", r);
    // TODO:
  };

  m_table->freefid = [] (IxpFid *f) {
    Dbg("freefid", 0);
    // Nothing.
  };

  m_conn = ixp_listen(m_srv, m_fd, m_table, ixp_serve9conn, NULL);

  return true;
}

int Server::loop(systemstate::Node *root) {
  if (!m_srv || !m_conn) {
    return 1;
  }

  std::cout << "Listening on address: " << m_addr << std::endl;

  m_root = root;
  int rc = ixp_serverloop(m_srv);
  m_root = nullptr;

  if (rc != 0) {
    std::cerr << "Server exited: " << ixp_errbuf() << std::endl;
  }

  return rc;
}
