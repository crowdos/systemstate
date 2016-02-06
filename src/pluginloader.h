#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <list>
#include <memory>

namespace systemstate {
  class RootNode;
  class DirNode;
  class Plugin;
};

class PluginData;
class PluginDb;

class PluginLoader {
public:
  PluginLoader(PluginDb& db, const std::string& pluginsPath, const std::string& loadersPath);
  ~PluginLoader();

  systemstate::RootNode *rootNode();
  bool load(const std::string& path);

private:
  void unload(systemstate::Plugin *plugin);
  void clean(systemstate::DirNode *node, systemstate::Plugin *plugin);

  systemstate::RootNode *m_root;
  PluginDb& m_db;
  std::string m_pluginsPath;
  std::string m_loadersPath;
  std::list<std::shared_ptr<PluginData >> m_plugins;
};

#endif /* PLUGIN_LOADER_H */
