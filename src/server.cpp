#include <iostream>
#include <cstring>
#include "server.h"
#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
extern "C" {
#include <ixp.h>
};

class Path {
public:
  Path(const char *name) :
  m_name(name) {}
  std::string& name() { return m_name; }

private:
  std::string m_name;
};

static char Enoperm[] = "permission denied",
  Enofile[] = "file not found",
  Ebadvalue[] = "bad value";

class Dbg {
public:
  Dbg(const char *id) :
  m_id(id) {
    std::cerr << ">> " << m_id << std::endl;
  }

  ~Dbg() {
    std::cerr << "<< " << m_id << std::endl;
  }

private:
  std::string m_id;
};

static void
dostat(IxpStat *s, const char *name, struct stat *buf) {
  static char *user = "root";

  s->type = 0;
  s->dev = 0;
  s->qid.type = buf->st_mode & S_IFMT;
  s->qid.path = buf->st_ino;
  s->qid.version = 0;
  s->mode = buf->st_mode & 0777;
  if (S_ISDIR(buf->st_mode)) {
    s->mode |= P9_DMDIR;
    s->qid.type |= P9_QTDIR;
  }

  s->atime = buf->st_atime;
  s->mtime = buf->st_mtime;
  s->length = buf->st_size;
  s->name = const_cast<char *>(name);
  s->uid = user;
  s->gid = user;
  s->muid = user;
}

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
    Dbg("Attach");
    r->fid->qid.type = P9_QTDIR;
    r->fid->qid.path = (uintptr_t)r->fid;
    r->fid->aux = reinterpret_cast<void *>(new Path("/"));
    r->ofcall.rattach.qid = r->fid->qid;
    ixp_respond(r, 0);
  };

  m_table->clunk = [] (Ixp9Req *r) {
    Dbg("clunk");
  };

  m_table->create = [] (Ixp9Req *r) {
    Dbg("create");
  };

  m_table->flush = [] (Ixp9Req *r) {
    Dbg("flush");
  };

  m_table->open = [] (Ixp9Req *r) {
    Dbg("open");
  };

  m_table->read = [] (Ixp9Req *r) {
    Dbg("read");
  };

  m_table->remove = [] (Ixp9Req *r) {
    Dbg("remove");
  };

  m_table->stat = [] (Ixp9Req *r) {
    Dbg("stat");
    Path *p = reinterpret_cast<Path *>(r->fid->aux);

    struct stat buf;
    if (stat(p->name().c_str(), &buf) != 0) {
      ixp_respond(r, Enofile);
      return;
    }

    IxpStat s;
    uint16_t size;
    IxpMsg m;
    dostat(&s, p->name().c_str(), &buf);

    r->fid->qid = s.qid;
    r->ofcall.rstat.nstat = size = ixp_sizeof_stat(&s);
    char *buff = reinterpret_cast<char *>(ixp_emallocz(size));
    m = ixp_message(buff, size, MsgPack);
    ixp_pstat(&m, &s);
    r->ofcall.rstat.stat = reinterpret_cast<uint8_t *>(m.data);
    ixp_respond(r, 0);
  };

  m_table->walk = [] (Ixp9Req *r) {
    Dbg("walk");
    Path *p = reinterpret_cast<Path *>(r->fid->aux);

    std::cout << p->name() << std::endl;
  };

  m_table->write = [] (Ixp9Req *r) {
    Dbg("write");
  };

  m_table->wstat = [] (Ixp9Req *r) {
    Dbg("wstat");
  };

  m_table->freefid = [] (IxpFid *f) {
    Dbg("freefid");
    delete reinterpret_cast<Path *>(f->aux);
  };

  m_conn = ixp_listen(m_srv, m_fd, m_table, ixp_serve9conn, NULL);

  return true;
}

int Server::loop() {
  if (!m_srv || !m_conn) {
    return 1;
  }

  std::cout << "Listening on address: " << m_addr << std::endl;

  int rc = ixp_serverloop(m_srv);

  if (rc != 0) {
    std::cerr << "Server exited: " << ixp_errbuf() << std::endl;
  }

  return rc;
}
