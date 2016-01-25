#include <iostream>
#include "protocol.h"
#include "common.h"

int
main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <key> <value>" << std::endl;
    return 1;
  }

  Connection c;

  if (!c.connect()) {
    return 1;
  }

  Response r(c.send(Write, argv[1], argv[2]));
  if (r.op() != Write) {
    c.send(Disconnect, std::string());
    c.disconnect();
    return 1;
  }

  // Read
  r = c.send(Read, argv[1]);
  if (r.op() != Read) {
    c.send(Unsubscribe, argv[1]);
    c.send(Disconnect, std::string());
    c.disconnect();
    return 1;
  }

  std::cout << r.value() << std::endl;

  c.send(Disconnect, std::string());
  c.disconnect();

  return 0;
}
