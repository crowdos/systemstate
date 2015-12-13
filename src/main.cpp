#include "pluginloader.h"
#include "server.h"

int
main(int argc, char *argv[]) {
  PluginLoader loader;

  systemstate::Node *root = loader.loadPlugins();

  Server server;

  if (!server.start()) {
    return 1;
  }

  return server.loop(root);
}
