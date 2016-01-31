#include <iostream>
#include <boost/bind.hpp>
#include "pluginloader.h"
#include "server.h"
#include "utils.h"
#include "plugindb.h"
#include <sys/signalfd.h>

class SignalHandler {
public:
  SignalHandler(Server& server) :
    m_server(server),
    m_fd(-1),
    m_desc(boost::asio::posix::stream_descriptor(m_server.service())) {

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
      throw std::logic_error(std::strerror(errno));
    }

    m_fd = signalfd(-1, &mask, 0);
    if (m_fd == -1) {
      throw std::logic_error(std::strerror(errno));
    }

    m_desc.assign(m_fd);
  }

  ~SignalHandler() {
    m_desc.cancel();
    m_desc.close();
  }

  void start() {
    m_desc.async_read_some(boost::asio::null_buffers(),
			   boost::bind(&SignalHandler::read, this));

  }

private:
  void read() {
    struct signalfd_siginfo fdsi;
    memset(&fdsi, 0, sizeof(fdsi));
    ssize_t s = ::read(m_fd, &fdsi, sizeof(struct signalfd_siginfo));
    if (s != sizeof(struct signalfd_siginfo)){
      start();
      return;
    }

    m_server.shutdown();
  }

  Server& m_server;
  boost::asio::posix::stream_descriptor m_desc;
  int m_fd;
};

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <plugins path>" << std::endl;
    return 1;
  }

  PluginDb db;

  db.scan(argv[1]);

  PluginLoader loader(db, argv[1]);

  try {
    std::string path(Utils::getAddress());
    ::unlink(path.c_str());
    Server server(path, loader);

    SignalHandler handler(server);
    handler.start();
    server.start();
    return server.loop();
  } catch (std::exception& ex) {
    std::cerr << "Failed to start server: " << ex.what() << std::endl;
    return 1;
  }
}
