#ifndef UTILS_H
#define UTILS_H

#include <string>

#define POLL_TIMEOUT         5000 // milliseconds
#define LISTEN_BACKLOG       5

namespace Utils {
  std::string getAddress();
};

#endif /* UTILS_H */
