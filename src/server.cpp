#include <iostream>
#include "server.h"
#include "stateplugin.h"
#include "protocol.h"
#include <arpa/inet.h>

using namespace systemstate;

class Session {
public:
  Session(boost::asio::io_service& service) :
    m_socket(boost::asio::local::stream_protocol::socket(service)),
    m_len(0) {}

  boost::asio::local::stream_protocol::socket& socket() { return m_socket; }

  void start() {
    // Read protocol version:
    uint8_t ver = 0x0;

    try {
      m_socket.receive(boost::asio::buffer(&ver, 1));
    } catch (std::exception& ex) {
      std::cerr << "Error reading from socket: " << ex.what() << std::endl;
      close();
      return;
    }

    if (ver != PROTOCOL_VERSION) {
      std::cerr << "Protocol version is not supported: " << std::hex << ver << std::endl;
      close();
      return;
    }

    read_length();
  }

private:
  void read_length() {
    m_socket.async_receive(boost::asio::buffer(&m_len, 4),
			   [this] (const boost::system::error_code& error,
			       std::size_t bytes_transferred) {
			     m_len = ntohl(m_len);

			     if (error) {
			       std::cerr << "Failed to read header" << std::endl;
			       close();
			       return;
			     }

			     std::cerr << m_len << " " << error;
			   });
  }

  void close() {
    m_socket.close();
    delete this;
  }

  boost::asio::local::stream_protocol::socket m_socket;
  uint32_t m_len;
};

// TODO: We do not check whether another server is already running or not.
// I am nit aware of any race-free way to do it.
Server::Server(const std::string& path, boost::asio::io_service& service,
	       systemstate::Node *root) :
  m_service(service),
  m_root(root),
  m_ep(boost::asio::local::stream_protocol::endpoint(path)),
  m_acceptor(boost::asio::local::stream_protocol::acceptor(m_service, m_ep))
{

}

Server::~Server() {
  shutdown();
}

void Server::start() {
  Session *sess = new Session(m_service);

  m_acceptor.async_accept(sess->socket(), [this, sess] (const boost::system::error_code& error) {
      if (error) {
	std::cerr << "Failed to accept connection: " << error << std::endl;
	delete sess;
      } else {
	sess->start();
      }
      start();
    });
}

int Server::loop() {
  m_service.run();

  return 0;
}

void Server::shutdown() {
  // TODO:
  m_service.stop();
}
