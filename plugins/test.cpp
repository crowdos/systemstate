#include "stateplugin.h"
#include <iostream>
#include <sstream>
#include <boost/asio.hpp>

using namespace systemstate;

class Counter {
public:
  Counter(boost::asio::io_service& service) :
    m_timer(service),
    m_value(0) {

  }

  bool start(const FileNode *node) {
    m_value = 0;
    m_timer.expires_from_now(boost::posix_time::seconds(1));
    m_timer.async_wait([this, node](const boost::system::error_code& error) {
	if (!error) {
	  ++m_value;
	  const_cast<FileNode *>(node)->dataChanged(value());
	}
      });

    return true;
  }

  void stop() {
    m_timer.cancel();
    m_value = 0;
  }

  std::string value() {
    std::stringstream s;
    s << m_value;
    return s.str();
  }

private:
  boost::asio::deadline_timer m_timer;
  int m_value;
};

class TestPlugin : public Plugin {
public:
  TestPlugin(boost::asio::io_service& service) :
    m_counter(service) {
  }

  void init(DirNode *root);
  bool start(FileNode *node);
  void stop(FileNode *node);
  bool read(FileNode *node, std::string& data);
  bool write(FileNode *node, const std::string& data);

private:
  Counter m_counter;
  std::string m_data;
};

void TestPlugin::init(DirNode *root) {
  std::cerr << "init" << std::endl;
  DirNode *d = root->appendDir("Test");
  d->appendFile("test", this);
  d->appendFile("counter", this);
  m_data = "test_data";
}

bool TestPlugin::start(FileNode *node) {
  if (node->name() == "counter") {
    return m_counter.start(node);
  }

  return true;
}

void TestPlugin::stop(FileNode *node) {
  if (node->name() == "counter") {
    m_counter.stop();
  }
}

bool TestPlugin::read(FileNode *node, std::string& data) {
  if (node->name() == "counter") {
    data = m_counter.value();
  } else {
    data = m_data;
  }

  return true;
}

bool TestPlugin::write(FileNode *node, const std::string& data) {
  if (node->name() == "counter") {
    return false;
  }

  m_data = data;
  return true;
}

REGISTER_STATE_PLUGIN(TestPlugin);
