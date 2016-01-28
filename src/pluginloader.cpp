#include "pluginloader.h"
#include "stateplugin.h"
#include "plugindb.h"
#include <iostream>
#include <sstream>
#include <dlfcn.h>

typedef systemstate::Plugin *(*__init)();

class PluginData {
public:
  PluginData() : m_handle(nullptr) { }

  ~PluginData() { unload(); }

  void unload() {
    if (m_handle) {
      dlclose(m_handle);
      m_handle = nullptr;
    }
  }

  bool load(const std::string& name, systemstate::DirNode *node) {
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

    m_plugin = init();

    m_plugin->init(node);

    return true;
  }

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

bool PluginLoader::load(const std::string& path) {
  std::string id = m_db.lookUp(path);
  if (id.empty()) {
    return false;
  }

  std::stringstream s;
  s << m_path << "/lib" << id << ".so";
  std::shared_ptr<PluginData> data = std::make_shared<PluginData>();
  if (!data->load(s.str(), m_root)) {
    return false;
  }

  m_plugins.push_back(data);

  return true;
}
