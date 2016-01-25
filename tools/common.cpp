#include "common.h"
#include "utils.h"
#include <iostream>
#include <boost/bind.hpp>

Connection::Connection() :
  m_ep(Utils::getAddress()),
  m_socket(m_service),
  m_len(0) {

}

Connection::~Connection() {
  disconnect();
}

bool Connection::connect() {
  try {
    m_socket.connect(m_ep);
    uint32_t ver = PROTOCOL_VERSION;
    m_socket.send(boost::asio::buffer(&ver, 1));
  } catch (std::exception& ex) {
    std::cerr << "Failed to connect: " << ex.what() << std::endl;
    return false;
  }

  return true;
}

Response Connection::send(const Op& op, const std::string& path, const std::string& value) {
  try {
    Request req(op, path, value);
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
    return Response(Error, path);;
  }
}

void Connection::disconnect() {
  try {
    m_socket.close();
  } catch (...) {
    // We don't care.
  }
}

void Connection::run(std::function<void(const Response&)> func) {
  m_func = func;

  read_packet();

  m_service.run();
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
    m_service.stop();
    return;
  }

  Response r = Response::fromData(std::string(buff, len));

  m_func(r);

  read_packet();
}
