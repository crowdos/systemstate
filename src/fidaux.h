#ifndef FID_AUX_H
#define FID_AUX_H

#include <string>

namespace systemstate {
  class Node;
};

struct IxpStat;

class FidAux {
public:
  typedef enum {
    Ok,
    Error,
    NoReply,
  } ReadResult;

  FidAux(systemstate::Node *node);
  ~FidAux();

  systemstate::Node *node() const { return m_node; }

  bool open();
  void close();
  ReadResult read(char *& data, uint32_t& len, uint64_t offset, uint32_t count);
  void stat(char *& data, uint32_t& len);

private:
  uint64_t m_version;
  systemstate::Node *m_node;
  std::string m_data;
};

#endif /* FID_AUX_H */
