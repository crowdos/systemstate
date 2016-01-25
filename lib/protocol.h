#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PROTOCOL_VERSION         0x1

#include <string>
#include <list>

typedef enum {
  Error = -1,
  Ping = 0,
  List,
  Read,
  Write,
  Subscribe,
  Unsubscribe,
  Disconnect,
  Notify,
} Op;

class Request {
public:
  Request(const Request& other) :
    m_op(other.m_op),
    m_path(other.m_path) {}

  Request(const Op& op, const std::string& path = std::string()) :
    m_op(op),
    m_path(path) {}

  const std::string& path() const { return m_path; }
  const Op& op() const { return m_op; }

  template<class Archive> void serialize(Archive& archive);

  std::string data() const;

  static Request fromData(const std::string& data);

private:
  Request() : m_op(Error) {}

  Op m_op;
  std::string m_path;
};

class Response {
public:
  Response(const Response& other) :
    m_op(other.m_op),
    m_path(other.m_path),
    m_values(other.m_values) {}

  Response(const Op& op, const std::string& path,
	   const std::list<std::string>& values = std::list<std::string>()) :
    m_op(op),
    m_path(path),
    m_values(values) {}

  Response(const Op& op, const std::string& path,
	   const std::string& value) :
    m_op(op),
    m_path(path),
    m_values(1, value) {}

  const std::string& path() const { return m_path; }
  const Op& op() const { return m_op; }
  std::string value() const { return m_values.empty() ? std::string() : m_values.front(); }
  const std::list<std::string>& values() const { return m_values; }

  template<class Archive> void serialize(Archive& archive);

  std::string data() const;

  static Response fromData(const std::string& data);

private:
  Response() : m_op(Error) {}

  Op m_op;
  std::string m_path;
  std::list<std::string> m_values;
};

#endif /* PROTOCOL_H */
