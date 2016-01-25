#include <iostream>
#include "protocol.h"
#include "common.h"

int
main(int argc, char *argv[]) {
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << std::endl;
    return 1;
  }

  Connection c;

  if (!c.connect()) {
    return 1;
  }

  Response r(c.send(Ping, std::string()));
  if (r.op() != Ping) {
    c.send(Disconnect, std::string());
    c.disconnect();
    return 1;
  }

  c.send(Disconnect, std::string());
  c.disconnect();

  return 0;
}
