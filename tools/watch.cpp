#include <iostream>
#include "protocol.h"
#include "common.h"

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <key>" << std::endl;
    return 1;
  }

  Connection c;

  if (!c.connect()) {
    return 1;
  }

  Response r(c.send(Subscribe, argv[1]));
  if (r.op() != Subscribe) {
    c.send(Disconnect, std::string());
    c.disconnect();
    return 1;
  }

  // TODO:
  // Read

  // follow and run loop


  c.send(Unsubscribe, argv[1]);

  c.send(Disconnect, std::string());
  c.disconnect();

  return 0;
}
