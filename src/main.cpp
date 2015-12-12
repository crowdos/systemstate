#include "server.h"

#define SOCK_PATH "/var/run/systemstate"

int
main(int argc, char *argv[]) {
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
