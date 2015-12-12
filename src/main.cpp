#include "pluginloader.h"
#include "server.h"

int
main(int argc, char *argv[]) {
  PluginLoader loader;

  systemstate::Node *root = loader.loadPlugins();

  // First we scan the info for all plugins

  // Next we 

  Server server;

  //  PluginScanner scanner;
  //  scanner.scan();

  if (!server.start()) {
    return 1;
  }

  //  EventLoop loop;
  //  loop->add
  return server.loop();
}
