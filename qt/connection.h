#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <thread>
#include <functional>
#include <boost/asio.hpp>
#include "protocol.h"

class Connection {
public:
  Connection(const std::string& key);
  ~Connection();

  void connect(const std::function<void(const std::string&)>& listener);
  void disconnect();

  const std::string& key() { return m_key; }

  void setValue(const std::string& value);

private:
  void run();
  void connect();
  Response send(const Op& op, const std::string& path);
  void async_send(const Op& op, const std::string& path,
		  const std::string& value = std::string());
  void handle_read_packet(const boost::system::error_code& error);

  void read_packet();

  void close();

  uint32_t m_len;

  std::function<void(const std::string&)> m_listener;
  std::string m_key;
  std::thread m_thread;
  boost::asio::io_service m_service;
  std::unique_ptr<boost::asio::io_service::work> m_work;
  boost::asio::local::stream_protocol::endpoint m_ep;
  boost::asio::local::stream_protocol::socket m_socket;
};

#endif /* CONNECTION_H */
