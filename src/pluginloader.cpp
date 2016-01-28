#include "pluginloader.h"
#include "stateplugin.h"
#include <iostream>
#include <dirent.h>
#include <cstring>
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

PluginLoader::PluginLoader() {

}

PluginLoader::~PluginLoader() {

}

systemstate::DirNode *PluginLoader::loadPlugins(const std::string& path) {
  systemstate::DirNode *node = new systemstate::DirNode("/", nullptr);

  struct dirent **namelist;
  int len = scandir(path.c_str(), &namelist, [](const struct dirent *entry) -> int {
      std::string d(entry->d_name);
      // return non zero to keep the entries.
      try {
	return d.compare(d.size() - 3, 3, ".so") == 0 ? 1 : 0;
      } catch (...) {
	// Discard.
	return 0;
      }
    }, alphasort);

  if (len == -1) {
    std::cerr << "scandir() failed: " << std::strerror(errno) << std::endl;
    return node;
  }

  for (int x = 0; x < len; x++) {
    std::stringstream s;
    s << path << "/" << namelist[x]->d_name;
    std::shared_ptr<PluginData> data = std::make_shared<PluginData>();
    if (!data->load(s.str(), node)) {
      continue;
    }

    m_plugins.push_back(data);
  }

  while (len--) {
    free(namelist[len]);
  }

  free(namelist);

  return node;
}
