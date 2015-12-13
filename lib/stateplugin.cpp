#include "stateplugin.h"
#include <algorithm>

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
  m_plugin(plugin) {

}

FileNode::~FileNode() {
  m_plugin = nullptr;
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
