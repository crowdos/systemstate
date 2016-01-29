#include "stateplugin.h"
#include <algorithm>
#include <boost/tokenizer.hpp>
#include <cassert>

using namespace systemstate;

Node::Node(const std::string& name, DirNode *parent) :
  m_name(name),
  m_parent(parent) {

}

Node::~Node() {
  m_parent = nullptr;
}

FileNode::FileNode(const std::string& name, DirNode *parent, Plugin *plugin) :
  Node(name, parent),
  m_open(0),
  m_version(0),
  m_plugin(plugin) {

}

FileNode::~FileNode() {
  m_plugin = nullptr;

  assert(m_listeners.size() == 0);
}

void FileNode::addListener(Listener *listener) {
  m_listeners.push_back(listener);
}

void FileNode::removeListener(Listener *listener) {
  auto iter = std::find(m_listeners.begin(), m_listeners.end(), listener);
  if (iter != m_listeners.end()) {
    m_listeners.erase(iter);
  }
}

bool FileNode::open() {
  if (m_open > 0) {
    ++m_open;
    plugin()->ref();
    return true;
  }

  if (plugin()->start(this)) {
    ++m_open;
    plugin()->ref();
    return true;
  }

  return false;
}

void FileNode::close() {
  --m_open;
  plugin()->unref();

  if (m_open == 0) {
    plugin()->stop(this);
  }

  if (plugin()->refCount() == 0) {
    plugin()->notify();
  }
}

void FileNode::dataChanged(const std::string& data) {
  ++m_version;

  // Inform listeners:
  for (Listener *listener : m_listeners) {
    listener->dataChanged(data);
  }
}

DirNode::DirNode(const std::string& name, DirNode *parent) :
  Node(name, parent) {

}

DirNode::~DirNode() {
  std::for_each(m_children.begin(), m_children.end(), [](Node *node) {
      if (node->type() == Node::File) {
	FileNode *file = dynamic_cast<FileNode *>(node);
	file->plugin()->stop(file);
      }

      delete node;
    });

  m_children.clear();
}

DirNode *DirNode::appendDir(const std::string& name) {
  for (int x = 0; x < numberOfChildren(); x++) {
    const Node *n = childAt(x);
    if (n->name() == name && n->type() == Dir) {
      return dynamic_cast<DirNode *>(const_cast<Node *>(n));
    }
  }

  DirNode *node = new DirNode(name, this);

  m_children.push_back(node);

  return node;
}

FileNode *DirNode::appendFile(FileNode *child) {
  m_children.push_back(child);
  return child;
}

FileNode *DirNode::appendFile(const std::string& name, Plugin *plugin) {
  FileNode *node = new FileNode(name, this, plugin);

  m_children.push_back(node);

  return node;
}

void DirNode::removeNode(Node *node) {
  auto iter = std::find(m_children.begin(), m_children.end(), node);
  if (iter != m_children.end()) {
    m_children.erase(iter);
    delete node;
    node = nullptr;
  }

  assert(node == nullptr);
}

RootNode::RootNode() :
  DirNode(std::string(), nullptr) {

}

const FileNode *RootNode::findNode(const std::string& path) {
  boost::char_separator<char> sep(".");
  boost::tokenizer<boost::char_separator<char> > tok(path, sep);

  std::list<std::string> tokens;

  for (auto iter = tok.begin(); iter != tok.end(); iter++) {
    tokens.push_back(*iter);
  }

  const Node *node = this;
  while (!tokens.empty()) {
    std::string token = tokens.front();
    tokens.pop_front();
    node = findNode(node, token);
    if (!node) {
      return nullptr;
    }
  }

  assert(node != this);

  if (node == this) {
    return nullptr;
  }

  return node->type() == File ? dynamic_cast<const FileNode *>(node) : nullptr;
}

const Node *RootNode::findNode(const Node *node, const std::string& name) {
  if (node->type() != Node::Dir) {
    if (node->name() == name) {
      return node;
    }

    return nullptr;
  }

  const DirNode *d = dynamic_cast<const DirNode *>(node);

  for (int x = 0; x < d->numberOfChildren(); x++) {
    const Node *n = d->childAt(x);
    if (n->name() == name) {
      return n;
    }
  }

  return nullptr;
}

Plugin::Plugin() :
  m_ref(0) {

}

Plugin::~Plugin() {

}
