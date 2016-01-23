#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <boost/asio.hpp>

namespace systemstate {
  class Node;
};

class Server {
public:
  Server(const std::string& path, boost::asio::io_service& service, systemstate::Node *root);
  ~Server();

  void start();

  int loop();

  systemstate::Node *root() { return m_root; }

private:
  void shutdown();

  systemstate::Node *m_root;
  boost::asio::io_service& m_service;
  boost::asio::local::stream_protocol::endpoint m_ep;
  boost::asio::local::stream_protocol::acceptor m_acceptor;
};

#endif /* SERVER_H */
