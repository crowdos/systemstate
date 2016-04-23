#include "connection.h"
#include "utils.h"
#include "protocol.h"
#include <iostream>
#include <boost/bind.hpp>

#define RECONNECT_INTERVAL 1000 // 1 sec (in ms).

Connection::Connection(const std::string& key) :
  m_key(key),
  m_ep(Utils::getAddress()),
  m_work(new boost::asio::io_service::work(m_service)),
  m_socket(m_service) {

}

Connection::~Connection() {
  disconnect();
}

void Connection::connect(const std::function<void(const std::string&)>& listener) {
  if (!m_thread.joinable()) {
    m_listener = listener;
    m_thread = std::thread(&Connection::run, this);
  }
}

void Connection::close() {
  try {
    send(Unsubscribe, m_key);
    send(Disconnect, m_key);
  } catch (...) {}

  try {
    m_socket.cancel();
    m_socket.close();
  } catch (...) {}

  try {
    m_service.stop();
  } catch (std::exception& e) {
    // We don't care.
  }
}

void Connection::disconnect() {
  m_service.post([this]() {close();});

  try {
    m_thread.join();
  } catch (std::exception& e) {
    // We don't care.
  }
}

void Connection::run() {
  do {
    close();
    try {
      connect();
    } catch (std::exception& ex) {
      std::cerr << "Failed to connect: " << ex.what() << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_INTERVAL));
      continue;
    }

    try {
      async_send(Read, m_key);

      read_packet();

      m_service.reset();
      m_service.run();

      // Normal exit
      return;
    } catch (std::exception& ex) {
      std::cerr << "Exception while running: " << ex.what() << std::endl;
    }

    // Sleep for a while
    std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_INTERVAL));
  } while (true);
}

void Connection::connect() {
  m_socket.connect(m_ep);
  uint32_t ver = PROTOCOL_VERSION;
  m_socket.send(boost::asio::buffer(&ver, 1));

  Response r(send(Subscribe, m_key));
  if (r.op() != Subscribe) {
    // I don't think this can happen:
    throw(std::runtime_error("Failed to subscribe"));
  }
}

Response Connection::send(const Op& op, const std::string& path) {
  try {
    Request req(op, path);
    std::string data = req.data();
    uint32_t size = htonl(data.size());
    m_socket.send(boost::asio::buffer(&size, 4));
    m_socket.send(boost::asio::buffer(data));

    uint32_t len;
    m_socket.receive(boost::asio::buffer(&len, 4));
    len = ntohl(len);
    char buff[len];
    m_socket.receive(boost::asio::buffer(&buff, len));
    Response r = Response::fromData(std::string(buff, len));
    return std::move(r);
  } catch (std::exception& ex) {
    std::cerr << "Failed to send: " << ex.what() << std::endl;
    throw;
  }
}

void Connection::async_send(const Op& op, const std::string& path, const std::string& value) {
  try {
    Request req(op, path, value);
    std::string data = req.data();
    uint32_t size = htonl(data.size());
    m_socket.send(boost::asio::buffer(&size, 4));
    m_socket.send(boost::asio::buffer(data));
  } catch (std::exception& ex) {
    std::cerr << "Failed to async send: " << ex.what() << std::endl;
    throw;
  }
}

void Connection::read_packet() {
  m_socket.async_receive(boost::asio::buffer(&m_len, 4),
			 boost::bind(&Connection::handle_read_packet, this,
				     boost::asio::placeholders::error));
}

void Connection::handle_read_packet(const boost::system::error_code& error) {
  uint32_t len = ntohl(m_len);

  char buff[len];
  try {
    m_socket.receive(boost::asio::buffer(&buff, len));
  } catch (std::exception& ex) {
    std::cerr << "Failed to read: " << ex.what() << std::endl;
    throw;
    return;
  }

  Response r = Response::fromData(std::string(buff, len));

  if (r.op() != Error) {
    m_listener(r.value());
  }

  read_packet();
}

void Connection::setValue(const std::string& value) {
  m_service.post([this, value]() {
      try {
	async_send(Write, m_key, value);
      } catch (std::exception& ex) {
	std::cerr << "Exception while setting value: " << ex.what() << std::endl;
      }
    });
}
