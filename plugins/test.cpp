#include "stateplugin.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>

using namespace systemstate;

class Counter {
public:
  Counter() : m_running(false) {}

  bool start(const FileNode *node) {
    m_value = 0;
    m_running = true;
    m_thread = std::thread([this, node]() {
	while (m_running) {
	  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	  m_mutex.lock();
	  ++m_value;
	  m_mutex.unlock();
	  const_cast<FileNode *>(node)->dataChanged(value(m_value));
	}
      });

    return true;
  }

  void stop() {
    if (m_running) {
      m_running = false;
      m_thread.join();
      m_value = -1;
    }
  }

  std::string value() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return value(m_value);
  }

  std::string value(int val) {
    std::stringstream s;
    s << val;
    return s.str();
  }

private:
  std::mutex m_mutex;
  std::thread m_thread;
  int m_value;
  bool m_running;
};

class TestPlugin : public Plugin {
  void init(DirNode *root);
  bool start(const FileNode *node);
  void stop(const FileNode *node);
  bool read(const FileNode *node, std::string& data);
  bool write(FileNode *node, const std::string& data);

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

bool TestPlugin::start(const FileNode *node) {
  if (node->name() == "counter") {
    return m_counter.start(node);
  }

  return true;
}

void TestPlugin::stop(const FileNode *node) {
  if (node->name() == "counter") {
    m_counter.stop();
  }
}

bool TestPlugin::read(const FileNode *node, std::string& data) {
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
