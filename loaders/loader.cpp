#include "loader.h"
#include <iostream>
#include "stateplugin.h"

class Plugin {
public:
  Plugin(const std::string& path) {

  }

  ~Plugin() {

  }

  bool init() {

  }

  void unload() {

  }

private:

};

AbstractLoader::AbstractLoader() :
  m_plugin(nullptr) {

}

AbstractLoader::~AbstractLoader() {
  delete m_plugin;
  m_plugin = nullptr;
}

bool AbstractLoader::init(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <key> <plugin>" << std::endl;
    return false;
  }

  std::string key = argv[1];
  std::string path = argv[2];
  if (!load(path)) {
    return false;
  }

  // Connect to server

  return false;
}

int AbstractLoader::exec() {
  return run();
}

bool AbstractLoader::load(const std::string& path) {
  if (m_plugin) {
    std::cerr << "Plugin already loaded" << std::endl;
    return false;
  }

  m_plugin = new Plugin(path);

  return m_plugin->init();
}

void AbstractLoader::unload() {

}
