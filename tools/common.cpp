#include "common.h"
#include "utils.h"
#include <iostream>

Connection::Connection() :
  m_ep(Utils::getAddress()),
  m_socket(m_service) {

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
