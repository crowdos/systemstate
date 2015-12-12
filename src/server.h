#ifndef SERVER_H
#define SERVER_H

#include <string>

struct IxpServer;
struct IxpConn;
struct Ixp9Srv;

class Server {
public:
  Server();
  ~Server();

  bool start();

  int loop();

private:
  std::string m_addr;
  IxpServer *m_srv;
  IxpConn *m_conn;
  Ixp9Srv *m_table;
  int m_fd;
};

#endif /* SERVER_H */
