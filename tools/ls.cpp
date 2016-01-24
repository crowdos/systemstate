#include <iostream>
#include "protocol.h"
#include "common.h"

int
main(int argc, char *argv[]) {
  if (argc > 2) {
    std::cerr << "Usage: " << argv[0] << " [key]" << std::endl;
    return 1;
  }

  std::string path = argc == 2 ? argv[1] : std::string();

  Connection c;

  if (!c.connect()) {
    return 1;
  }

  Response r(c.send(List, path));
  if (r.op() != List) {
    c.send(Disconnect, std::string());
    c.disconnect();
    return 1;
  }

  for (const std::string& v : r.values()) {
    if (path.empty()) {
      std::cout << v << std::endl;
    } else {
      std::cout << path << "." << v << std::endl;
    }
  }

  c.send(Disconnect, std::string());
  c.disconnect();

  return 0;
}
