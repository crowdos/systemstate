#ifndef STATE_PLUGIN_H
#define STATE_PLUGIN_H

#include <string>

namespace systemstate {

class FileNode;
class DirNode;
class Plugin;

class Node {
public:
  typedef enum {
    Dir,
    File,
  } NodeType;

  Node(const std::string& name, DirNode *parent);
  virtual ~Node();

  virtual NodeType type() const = 0;

  const DirNode *parent() const { return m_parent; }

private:
  const std::string m_name;
  const DirNode *m_parent;
};

class FileNode : public Node {
public:
  FileNode(const std::string& name, DirNode *parent, Plugin *plugin);
  Node::NodeType type() const { return Node::File; }

private:
  const Plugin *m_plugin;
};

class DirNode : public Node {
public:
  DirNode(const std::string& name, DirNode *parent);
  Node::NodeType type() const { return Node::Dir; }

  DirNode *appendDir(const std::string& name);
  DirNode *appendFile(FileNode *child);
  bool removeFile(FileNode *child);

  ssize_t numberOfChildren() const;
  const Node *childAt(int x) const;
};

class Plugin {
public:
  virtual ~Plugin();

  virtual void init(DirNode *root) = 0;

  virtual bool start(const FileNode *node) = 0;
  virtual void stop(const FileNode *node) = 0;

  virtual ssize_t size(const FileNode *node) = 0;
  virtual bool read(const FileNode *node, std::string& data) = 0;

protected:
  Plugin();
};

};

#define REGISTER_STATE_PLUGIN(x)		\
  extern "C" systemstate::Plugin *__init() {	\
    return new x;                               \
  }

#endif /* STATE_PLUGIN_H */
