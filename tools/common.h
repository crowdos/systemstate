#ifndef COMMON_H
#define COMMON_H

#include <boost/asio.hpp>
#include "protocol.h"

class Connection {
public:
  Connection();
  ~Connection();

  bool connect();

  Response send(const Op& op, const std::string& path);

  void disconnect();

private:
  boost::asio::io_service m_service;
  boost::asio::local::stream_protocol::endpoint m_ep;
  boost::asio::local::stream_protocol::socket m_socket;
};

#endif /* COMMON_H */
