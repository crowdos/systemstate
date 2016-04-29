#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <list>
#include <memory>

namespace boost {
  namespace asio {
    class io_service;
  };
};

namespace systemstate {
  class RootNode;
  class DirNode;
  class Plugin;
};

class PluginData;
class PluginDb;

class PluginLoader {
public:
  PluginLoader(PluginDb& db, const std::string& path);
  ~PluginLoader();

  systemstate::RootNode *rootNode();
  bool load(const std::string& path, const boost::asio::io_service& service);

private:
  void unload(systemstate::Plugin *plugin);
  void clean(systemstate::DirNode *node, systemstate::Plugin *plugin);

  systemstate::RootNode *m_root;
  PluginDb& m_db;
  std::string m_path;
  std::list<std::shared_ptr<PluginData >> m_plugins;
};

#endif /* PLUGIN_LOADER_H */
