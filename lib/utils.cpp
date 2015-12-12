#include "utils.h"
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
extern "C" {
#include <ixp.h>
};

bool Utils::getAddress(std::string& addr) {
  const char *ns = ixp_namespace();
  if (mkdir(ns, 666) != 0 && errno != EEXIST) {
    std::cerr << "Failed to create diractory " << ns << ": " << std::strerror(errno) << std::endl;
    return false;
  } else if (errno == EEXIST) {
    // The node exists. Is it a directory?
    struct stat buf;
    if (stat(ns, &buf) != 0) {
      // I give up.
      std::cerr << "Failed to stat diractory " << ns << ": " << std::strerror(errno) << std::endl;
      return false;
    }

    if (!buf.st_mode & S_IFDIR) {
      std::cerr << ns << " is not a directory!" << std::endl;
      // TODO: Ideally we should check that it is not a symlink to a directory but enough is enough
      // for now :)
      return false;
    }
  }

  std::stringstream s;
  s << "unix!" << ns << "/systemstate";
  addr = std::move(s.str());

  return true;
}
