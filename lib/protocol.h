#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PROTOCOL_VERSION         0x1
#define HEADER_SIZE              4

#include <string>

typedef enum {
  Error = 0,
  List,
  Read,
  Write,
  Subscribe,
  Unsubscribe,
  Notify,
} Op;

class Request {
public:
  Request(const Request& other) :
    m_op(other.m_op),
    m_path(other.m_path) {}

  Request(const Op& op, const std::string& path) :
    m_op(op),
    m_path(path) {}

  const std::string& path() const { return m_path; }
  const Op& op() const { return m_op; }

private:
  const Op m_op;
  const std::string m_path;
};

class Response {
public:
  Response(const Response& other) :
    m_op(other.m_op),
    m_path(other.m_path),
    m_value(other.m_value) {}

  Response(const Op& op, const std::string& path, const std::string& value) :
    m_op(op),
    m_path(path),
    m_value(value) {}

  const std::string& path() const { return m_path; }
  const Op& op() const { return m_op; }
  const std::string& value() const { return m_value; }

private:
  const Op m_op;
  const std::string m_path;
  const std::string m_value;
};

#endif /* PROTOCOL_H */
