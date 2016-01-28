#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <list>
#include <memory>

namespace systemstate {
  class RootNode;
};

class PluginData;

class PluginLoader {
public:
  PluginLoader();
  ~PluginLoader();

  systemstate::RootNode *loadPlugins(const std::string& path);

private:
  std::list<std::shared_ptr<PluginData >> m_plugins;
};

#endif /* PLUGIN_LOADER_H */
