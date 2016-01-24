#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <list>

namespace systemstate {
  class DirNode;
};

class PluginData;

class PluginLoader {
public:
  PluginLoader();
  ~PluginLoader();

  systemstate::DirNode *loadPlugins();

private:
  std::list<PluginData *> m_plugins;
};

#endif /* PLUGIN_LOADER_H */
