#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <list>
#include <memory>

namespace systemstate {
  class RootNode;
};

class PluginData;
class PluginDb;

class PluginLoader {
public:
  PluginLoader(PluginDb& db, const std::string& path);
  ~PluginLoader();

  systemstate::RootNode *rootNode();
  bool load(const std::string& path);

private:
  systemstate::RootNode *m_root;
  PluginDb& m_db;
  std::string m_path;
  std::list<std::shared_ptr<PluginData >> m_plugins;
};

#endif /* PLUGIN_LOADER_H */
