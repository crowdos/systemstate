#ifndef STATE_PLUGIN_H
#define STATE_PLUGIN_H

#include <string>
#include <deque>
#include <list>
#include <functional>

namespace systemstate {

class FileNode;
class DirNode;
class Plugin;

class Listener {
public:
  virtual ~Listener() {}
  virtual void dataChanged(const std::string& data) = 0;
};

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
  const std::string& name() const { return m_name; }

private:
  const std::string m_name;
  const DirNode *m_parent;
};

class FileNode : public Node {
public:
  FileNode(const std::string& name, DirNode *parent, Plugin *plugin);
  virtual ~FileNode();

  Node::NodeType type() const { return Node::File; }
  systemstate::Plugin *plugin() const { return m_plugin; }
  uint64_t version() const { return m_version; }
  void addListener(Listener *listener);
  void removeListener(Listener *listener);
  bool open();
  void close();

protected:
  void dataChanged(const std::string& data);

private:
  std::list<Listener *> m_listeners;
  int m_open;
  uint64_t m_version;
  Plugin *m_plugin;
};

class DirNode : public Node {
public:
  DirNode(const std::string& name, DirNode *parent);
  virtual ~DirNode();

  Node::NodeType type() const { return Node::Dir; }

  DirNode *appendDir(const std::string& name);
  FileNode *appendFile(FileNode *child);
  FileNode *appendFile(const std::string& name, Plugin *plugin);

#if 0
  bool removeFile(FileNode *child);
#endif

  ssize_t numberOfChildren() const { return m_children.size(); }
  const Node *childAt(int x) const { return m_children.at(x); }

  std::deque<Node *>::iterator begin() { return m_children.begin(); }
  std::deque<Node *>::iterator end() { return m_children.end(); }

private:
  std::deque<Node *> m_children;
};

class Plugin {
public:
  virtual ~Plugin();

  virtual void init(DirNode *root) = 0;

  virtual bool start(const FileNode *node) = 0;
  virtual void stop(const FileNode *node) = 0;

  virtual ssize_t size(const FileNode *node) = 0;
  virtual bool read(const FileNode *node, std::string& data) = 0;
  virtual bool write(FileNode *node, const std::string& data) = 0;

protected:
  Plugin();
};

};

#define REGISTER_STATE_PLUGIN(x)		\
  extern "C" systemstate::Plugin *__init() {	\
    return new x;                               \
  }

#endif /* STATE_PLUGIN_H */
