#include "pluginloader.h"
#include "stateplugin.h"
#include "plugindb.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <dlfcn.h>
#include <cassert>

typedef systemstate::Plugin *(*__init)(const std::function<void(systemstate::Plugin *)>&,
				       const boost::asio::io_service&);

class PluginData {
public:
  PluginData() :
    m_handle(nullptr),
    m_plugin(nullptr)
  { }

  ~PluginData() { unload(); }

  void unload() {
    if (m_plugin) {
      delete m_plugin;
      m_plugin = nullptr;
    }

    if (m_handle) {
      dlclose(m_handle);
      m_handle = nullptr;
    }
  }

  bool load(const std::string& name, systemstate::DirNode *node,
	    const std::function<void(systemstate::Plugin *)>& unload,
	    const boost::asio::io_service& service) {
    m_handle = dlopen(name.c_str(), RTLD_LAZY);
    if (!m_handle) {
      std::cerr << "Failed to load " << name << ": " << dlerror() << std::endl;
      return false;
    }

    __init init = (__init)dlsym(m_handle, "__init");
    if (!init) {
      std::cerr << "Failed to find __init symbol" << std::endl;
      return false;
    }

    m_plugin = init(unload, service);

    m_plugin->init(node);

    return true;
  }

  systemstate::Plugin *plugin() { return m_plugin; }

private:
  systemstate::Plugin *m_plugin;
  void *m_handle;
};

PluginLoader::PluginLoader(PluginDb& db, const std::string& path) :
  m_root(new systemstate::RootNode),
  m_db(db),
  m_path(path) {

}

PluginLoader::~PluginLoader() {
  delete m_root;
  m_root = nullptr;
}

systemstate::RootNode *PluginLoader::rootNode() {
  return m_root;
}

bool PluginLoader::load(const std::string& path, const boost::asio::io_service& service) {
  std::string id = m_db.lookUp(path);
  if (id.empty()) {
    return false;
  }

  std::stringstream s;
  s << m_path << "/lib" << id << ".so";
  std::shared_ptr<PluginData> data = std::make_shared<PluginData>();
  if (!data->load(s.str(), m_root, [this](systemstate::Plugin *plugin) {unload(plugin);},
		  service)) {
    return false;
  }

  m_plugins.push_back(data);

  return true;
}

void PluginLoader::unload(systemstate::Plugin *plugin) {
  clean(m_root, plugin);

  auto iter =
    std::find_if(m_plugins.begin(), m_plugins.end(),
		 [plugin] (std::shared_ptr<PluginData>& data) {return data->plugin() == plugin;});

  assert(iter != m_plugins.end());

  std::shared_ptr<PluginData> data = *iter;

  m_plugins.erase(iter);
}

void PluginLoader::clean(systemstate::DirNode *node, systemstate::Plugin *plugin) {
  std::list<systemstate::FileNode *> nodes;

  int len = node->numberOfChildren();
  while (--len >= 0) {
    const systemstate::Node *child = node->childAt(len);

    if (child->type() == systemstate::Node::File) {
      systemstate::FileNode *f
	= dynamic_cast<systemstate::FileNode *>(const_cast<systemstate::Node *>(child));

      if (f->plugin() == plugin) {
	nodes.push_back(f);
      }
    } else {
      clean(dynamic_cast<systemstate::DirNode *>(const_cast<systemstate::Node *>(child)), plugin);
    }
  }

  while (!nodes.empty()) {
    auto iter = nodes.begin();
    systemstate::FileNode *f = *iter;
    nodes.erase(iter);
    node->removeNode(f);
  }

  if (node->numberOfChildren() == 0 && node != m_root) {
    node->parent()->removeNode(node);
  }
}
