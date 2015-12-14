#include "fidaux.h"
#include "stateplugin.h"
#include <iostream>
#include <list>
#include <algorithm>
#include <cstring> // memcpy()
#include "libixp.h"

using namespace systemstate;

static char id[] = "root";

class Stat {
public:
  Stat(const Node *node) {
    m_stat.qid.type = node->type() == Node::Dir ? P9_QTDIR : P9_QTFILE;
    m_stat.qid.path = reinterpret_cast<uint64_t>(node);
    m_stat.qid.version = 0;
    m_stat.type = 0;
    m_stat.dev = 0;
    m_stat.mode = P9_DMREAD;
    if (node->type() == Node::Dir) {
      m_stat.mode |= P9_DMDIR;
    } else {
      m_stat.mode |= P9_DMTMP;
    }

    m_stat.atime = 0;
    m_stat.mtime = 0;
    m_stat.length = 0;
    m_stat.name = const_cast<char *>(node->name().c_str());
    m_stat.uid = m_stat.gid = m_stat.muid = id;
  }

  ~Stat() {

  }

  void pstat(IxpMsg *m) {
    ixp_pstat(m, &m_stat);
  }

  int size() {
    return ixp_sizeof_stat(&m_stat);
  }

private:
  IxpStat m_stat;
};

FidAux::FidAux(Node *node) :
  m_version(0),
  m_node(node) {

}

FidAux::~FidAux() {
  m_node = nullptr;
}

bool FidAux::open() {
  if (m_node->type() == Node::Dir) {
    return true;
  }

  return dynamic_cast<FileNode *>(m_node)->open();
}

void FidAux::close() {
  if (m_node->type() == Node::File) {
    dynamic_cast<FileNode *>(m_node)->close();
  }
}

FidAux::ReadResult FidAux::read(char *& data, uint32_t& len, uint64_t offset, uint32_t count) {
  if (m_node->type() == Node::Dir) {
    DirNode *d = dynamic_cast<DirNode *>(m_node);
    if (offset == 0) {
      std::list<Stat *> stats;
      uint size = 0;
      for (int x = 0; x < d->numberOfChildren(); x++) {
	const Node *child = d->childAt(x);
	Stat *s = new Stat(child);
        size += s->size();
	stats.push_back(s);
      }

      len = size;
      data = ixp_mallocz<char>(size);

      IxpMsg m = ixp_message(data, size, MsgPack);
      while (!stats.empty()) {
	Stat *s = stats.front();
	stats.pop_front();
	s->pstat(&m);
	delete s;
      }

      data = m.data;

      return Ok;
    } else {
      // For the sake of simplification, we can only return the stat data for
      // directories in one go.
      data = 0;
      len = 0;
      return Ok;
    }
  } else {
    std::cerr << "offset " << offset << std::endl;

    FileNode *f = dynamic_cast<FileNode *>(m_node);

    // For the sake of simplicity we will push all the data too.
    // The assumption is we do not have a lot of data anyway so
    // it will always fit in one go.
    if (m_data.empty()) {
      if (!f->plugin()->read(f, m_data)) {
	return Error;
      } else {
	m_version = f->version();
      }
    }

    if (offset < m_data.size()) {
      if (m_data.empty()) {
	data = 0;
	len = 0;
	return Ok;
      }

      len = m_data.size() - offset;
      data = ixp_mallocz<char>(len);
      std::memcpy(data, &m_data.c_str()[offset], len);
      return Ok;
    }

    data = 0;
    len = 0;
    return Ok;
  }

  return Error;
}

void FidAux::stat(char *& data, uint32_t& len) {
  Stat s(m_node);

  len = s.size();
  data = ixp_mallocz<char>(len);

  IxpMsg m = ixp_message(data, len, MsgPack);
  s.pstat(&m);
  data = m.data;
}
