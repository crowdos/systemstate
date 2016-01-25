#include <iostream>
#include "server.h"
#include "stateplugin.h"
#include <arpa/inet.h>
#include<boost/tokenizer.hpp>
#include <boost/bind.hpp>

using namespace systemstate;
class FileNodeListener;

class Session {
public:
  Session(Server *server) :
    m_server(server),
    m_socket(boost::asio::local::stream_protocol::socket(m_server->service())),
    m_len(0) {}

  ~Session();

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

    read_packet();
  }

  bool addNode(FileNode *node, const std::string& path);
  void removeNode(FileNode *node, const std::string& path);

  bool sendResponse(const Response& s) {
    // TODO: cleanup, clarify when we delete ourselves and use this function
    std::string data = s.data();
    uint32_t size = htonl(data.size());
    try {
      m_socket.send(boost::asio::buffer(&size, 4));
      m_socket.send(boost::asio::buffer(data));
    } catch (std::exception& ex) {
      if (s.op() != Disconnect) {
	std::cerr << "Error sending response: " << ex.what();
      } else {
	close();
      }

      return false;
    }

    return true;
  }

private:
  void read_packet() {
    m_socket.async_receive(boost::asio::buffer(&m_len, 4),
			   boost::bind(&Session::handle_read_packet, this,
				       boost::asio::placeholders::error
				       ));
  }

  void handle_read_packet(const boost::system::error_code& error) {
    uint32_t len = ntohl(m_len);

    if (error) {
      std::cerr << "Failed to read header" << std::endl;
      close();
      return;
    }

    char buff[len];
    m_socket.receive(boost::asio::buffer(&buff, len));
    Request r = Request::fromData(std::string(buff, len));
    Response s = m_server->handleRequest(this, r);
    std::string data = s.data();
    uint32_t size = htonl(data.size());
    try {
      m_socket.send(boost::asio::buffer(&size, 4));
      m_socket.send(boost::asio::buffer(data));
    } catch (std::exception& ex) {
      if (r.op() != Disconnect) {
	std::cerr << "Error sending response: " << ex.what();
      } else {
	close();
	return;
      }
    }

    if (r.op() == Disconnect) {
      close();
      return;
    } else {
      read_packet();
    }
  }

  void close() {
    m_socket.cancel();
    m_socket.close();
    m_server->removeSession(this);
  }

  Server *m_server;
  boost::asio::local::stream_protocol::socket m_socket;
  uint32_t m_len;
  std::string m_data;
  std::map<std::string, FileNodeListener *> m_listeners;
};

class FileNodeListener : public Listener {
public:
  FileNodeListener(Session *session, const std::string& path, FileNode *node) :
    m_session(session),
    m_path(path),
    m_node(node) {

    if (!m_node->open()) {
      throw std::logic_error("Failed to open node");
    }

    m_node->addListener(this);
  }

  ~FileNodeListener() {
    m_node->removeListener(this);
    m_node->close();
    m_node = nullptr;
  }

private:
  void dataChanged(const std::string& data) {
    Response s(Notify, m_path, data);
    m_session->sendResponse(s);
  }

  Session *m_session;
  std::string m_path;
  FileNode *m_node;
};

Session::~Session() {
  // Remove all listeners

  while (!m_listeners.empty()) {
    auto iter = m_listeners.begin();

    FileNodeListener *listener = iter->second;
    m_listeners.erase(iter);

    delete listener;
  }
}

bool Session::addNode(FileNode *node, const std::string& path) {
  if (m_listeners.find(path) != m_listeners.end()) {
    return false;
  }

  try {
    FileNodeListener *listener = new FileNodeListener(this, path, node);
    m_listeners.insert(std::make_pair(path, listener));
  } catch (...) {
    return false;
  }

  return true;
}

void Session::removeNode(FileNode *node, const std::string& path) {
  auto iter = m_listeners.find(path);
  if (iter == m_listeners.end()) {
    return;
  }

  FileNodeListener *listener = iter->second;
  m_listeners.erase(iter);

  delete listener;
}

// TODO: We do not check whether another server is already running or not.
// I am nit aware of any race-free way to do it.
Server::Server(const std::string& path, boost::asio::io_service& service,
	       systemstate::DirNode *root) :
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
  Session *sess = new Session(this);
  m_sessions.insert(sess);

  m_acceptor.async_accept(sess->socket(), [this, sess] (const boost::system::error_code& error) {
      if (error) {
	std::cerr << "Failed to accept connection: " << error << std::endl;
	removeSession(sess);
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
  m_service.stop();

  while (!m_sessions.empty()) {
    auto iter = m_sessions.begin();
    removeSession(*iter);
  }
}

const systemstate::Node *Server::findNode(const std::string& path) {
  boost::char_separator<char> sep(".");
  boost::tokenizer<boost::char_separator<char> > tok(path, sep);

  std::list<std::string> tokens;

  for (auto iter = tok.begin(); iter != tok.end(); iter++) {
    tokens.push_back(*iter);
  }

  const Node *node = m_root;
  while (!tokens.empty()) {
    std::string token = tokens.front();
    tokens.pop_front();
    node = findNode(node, token);
    if (!node) {
      return nullptr;
    }
  }

  return node;
}

const systemstate::Node *Server::findNode(const systemstate::Node *node, const std::string& name) {
  if (node->type() != Node::Dir) {
    if (node->name() == name) {
      return node;
    }

    return nullptr;
  }

  const DirNode *d = dynamic_cast<const DirNode *>(node);

  for (int x = 0; x < d->numberOfChildren(); x++) {
    const Node *n = d->childAt(x);
    if (n->name() == name) {
      return n;
    }
  }

  return nullptr;
}

Response Server::handleRequest(Session *session, const Request& request) {
  switch (request.op()) {
  case Error:
  case Notify:
    return error(request);

  case List:
    return list(request);
    break;

  case Read:
    return read(request);
    break;

  case Write:
    return error(request); // TODO:

  case Subscribe:
    return subscribe(session, request);

  case Unsubscribe:
    return unsubscribe(session, request);

  case Disconnect:
    return Response(Disconnect, request.path());
  }

  return error(request);
}

Response Server::read(const Request& req) {
  const Node *node = findNode(req.path());

  if (!node || node->type() != Node::File) {
    return error(req);
  }

  FileNode *f = dynamic_cast<FileNode *>(const_cast<Node *>(node));
  if (!f->open()) {
    return error(req);
  }

  std::string data;
  if (!f->plugin()->read(f, data)) {
    f->close();
    return error(req);
  }

  Response r(Read, req.path(), data);
  f->close();
  return r;
}

Response Server::list(const Request& req) {
  const Node *node = findNode(req.path());

  if (!node || node->type() != Node::Dir) {
    return error(req);
  }

  const DirNode *d = dynamic_cast<const DirNode *>(node);
  std::list<std::string> data;
  for (int x = 0; x < d->numberOfChildren(); x++) {
    data.push_back(d->childAt(x)->name());
  }

  Response r(List, req.path(), data);
  return r;
}

Response Server::error(const Request& req) {
  return Response(Error, req.path());
}

Response Server::subscribe(Session *session, const Request& req) {
  const Node *n = findNode(req.path());
  if (!n || n->type() != Node::File) {
    return error(req);
  }

  FileNode *f = dynamic_cast<FileNode *>(const_cast<Node *>(n));
  if (!session->addNode(f, req.path())) {
    return error(req);
  }

  return Response(req.op(), req.path());
}

Response Server::unsubscribe(Session *session, const Request& req) {
  const Node *n = findNode(req.path());
  if (!n || n->type() != Node::File) {
    return error(req);
  }

  FileNode *f = dynamic_cast<FileNode *>(const_cast<Node *>(n));

  session->removeNode(f, req.path());

  return Response(req.op(), req.path());
}

void Server::removeSession(Session *session) {
  auto iter = m_sessions.find(session);
  if (iter != m_sessions.end()) {
    Session *s = *iter;
    m_sessions.erase(iter);
    delete s;
  }
}
