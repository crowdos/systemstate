#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <unordered_set>
#include <boost/asio.hpp>
#include "protocol.h"

namespace systemstate {
  class RootNode;
  class Node;
};

class Session;
class PluginLoader;

class Server {
public:
  Server(const std::string& path, PluginLoader& loader);
  ~Server();

  void start();

  int loop();

  boost::asio::io_service& service() { return m_service; }

  Response handleRequest(Session *session, const Request& request);

  void removeSession(Session *session);

private:
  void shutdown();
  const systemstate::Node *findNode(const std::string& path);

  Response read(const Request& req);
  Response write(const Request& req);
  Response list(const Request& req);
  Response error(const Request& req);
  Response subscribe(Session *session, const Request& req);
  Response unsubscribe(Session *session, const Request& req);

  PluginLoader& m_loader;
  boost::asio::io_service m_service;
  boost::asio::local::stream_protocol::endpoint m_ep;
  boost::asio::local::stream_protocol::acceptor m_acceptor;

  std::unordered_set<Session *> m_sessions;
};

#endif /* SERVER_H */
