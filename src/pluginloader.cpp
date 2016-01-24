#include "pluginloader.h"
#include "stateplugin.h"
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <sstream>
#include <dlfcn.h>

#define PLUGINS_DIR "plugins"

typedef systemstate::Plugin *(*__init)();

class PluginData {
public:
  systemstate::Plugin *plugin;
  void *handle;
};

PluginLoader::PluginLoader() {
}

PluginLoader::~PluginLoader() {

}

systemstate::DirNode *PluginLoader::loadPlugins() {
  systemstate::DirNode *node = new systemstate::DirNode("/", nullptr);

  struct dirent **namelist;
  int len = scandir(PLUGINS_DIR, &namelist, [](const struct dirent *entry) -> int {
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
    s << PLUGINS_DIR << "/" << namelist[x]->d_name;
    void *handle = dlopen(s.str().c_str(), RTLD_LAZY);
    if (!handle) {
      continue;
    }

    __init init = (__init)dlsym(handle, "__init");
    if (!init) {
      dlclose(handle);
      continue;
    }

    PluginData *data = new PluginData;
    data->handle = handle;
    data->plugin = init();

    data->plugin->init(node);

    m_plugins.push_back(data);
  }

  while (len--) {
    free(namelist[len]);
  }

  free(namelist);

  return node;
}
