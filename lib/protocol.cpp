#include "protocol.h"
#include <sstream>
#include <cereal/types/string.hpp>
#include <cereal/types/list.hpp>
#include <cereal/archives/binary.hpp>

template<class Archive> void Request::serialize(Archive& archive) {
  archive(m_op, m_path, m_value);
}

std::string Request::data() const {
  std::ostringstream ss;
  cereal::BinaryOutputArchive oarchive(ss);

  oarchive(*this);

  return std::move(ss.str());
}

Request Request::fromData(const std::string& data) {
  std::istringstream ss(data);
  cereal::BinaryInputArchive iarchive(ss);

  Request r;
  iarchive(r);

  return std::move(r);
}

template<class Archive> void Response::serialize(Archive& archive) {
  archive(m_op, m_path, m_values);
}

std::string Response::data() const {
  std::ostringstream ss;
  cereal::BinaryOutputArchive oarchive(ss);

  oarchive(*this);

  return std::move(ss.str());
}

Response Response::fromData(const std::string& data) {
  std::istringstream ss(data);
  cereal::BinaryInputArchive iarchive(ss);

  Response r;
  iarchive(r);

  return std::move(r);
}
