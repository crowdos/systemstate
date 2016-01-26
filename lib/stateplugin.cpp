#include "stateplugin.h"
#include <algorithm>
#include <assert.h>

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
    return true;
  }

  if (plugin()->start(this)) {
    ++m_open;
    return true;
  }

  return false;
}

void FileNode::close() {
  --m_open;

  if (m_open == 0) {
    plugin()->stop(this);
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

#if 0
bool DirNode::removeFile(FileNode *child) {
  // TODO:
}
#endif

Plugin::Plugin() {

}

Plugin::~Plugin() {

}
