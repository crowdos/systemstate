#include <iostream>
#include <boost/asio.hpp>
#include "pluginloader.h"
#include "server.h"
#include "utils.h"
#include "plugindb.h"

#define PLUGINS_DIR "plugins"

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <plugins path>" << std::endl;
    return 1;
  }

  PluginDb db;

  db.scan(argv[1]);

  PluginLoader loader;

  systemstate::RootNode *root = loader.loadPlugins(argv[1]);

  boost::asio::io_service service;

  try {
    std::string path(Utils::getAddress());
    ::unlink(path.c_str());
    Server server(path, service, root);
    server.start();
    return server.loop();
  } catch (std::exception& ex) {
    std::cerr << "Failed to start server: " << ex.what() << std::endl;
    return 1;
  }
}
