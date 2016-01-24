#include <boost/asio.hpp>
#include "pluginloader.h"
#include "server.h"
#include "utils.h"
#include <unistd.h>

int
main(int argc, char *argv[]) {
  PluginLoader loader;

  systemstate::DirNode *root = loader.loadPlugins();

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
