#ifndef SERVER_H
#define SERVER_H

#include <string>

struct IxpServer;
struct IxpConn;
struct Ixp9Srv;

namespace systemstate {
  class Node;
};

class Server {
public:
  Server();
  ~Server();

  bool start();

  int loop(systemstate::Node *root);

  systemstate::Node *root() { return m_root; }

private:
  std::string m_addr;
  IxpServer *m_srv;
  IxpConn *m_conn;
  Ixp9Srv *m_table;
  int m_fd;

  systemstate::Node *m_root;
};

#endif /* SERVER_H */
