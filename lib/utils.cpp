#include "utils.h"

std::string Utils::getAddress() {
  std::string path;
  path.push_back('\0');
  path.append("/systemstate.sock");

  return path;
}
