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

  void run(std::function<void(const Response&)> func);

private:
  void read_packet();
  void handle_read_packet(const boost::system::error_code& error);

  boost::asio::io_service m_service;
  boost::asio::local::stream_protocol::endpoint m_ep;
  boost::asio::local::stream_protocol::socket m_socket;
  uint32_t m_len;
  std::function<void(const Response&)> m_func;
};

#endif /* COMMON_H */
