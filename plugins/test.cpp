#include "stateplugin.h"
#include <iostream>
#include <sstream>

using namespace systemstate;

class TestPlugin : public Plugin {
  void init(DirNode *root);
  bool start(const FileNode *node);
  void stop(const FileNode *node);
  ssize_t size(const FileNode *node);
  bool read(const FileNode *node, std::string& data);
};

void TestPlugin::init(DirNode *root) {
  std::cerr << "init" << std::endl;
  root->appendDir("Test")->appendFile("test", this);
}

bool TestPlugin::start(const FileNode *node) {
  return true;
}

void TestPlugin::stop(const FileNode *node) {

}

ssize_t TestPlugin::size(const FileNode *node) {
  return 0;
}

bool TestPlugin::read(const FileNode *node, std::string& data) {
  data = "t";
  return true;
}

REGISTER_STATE_PLUGIN(TestPlugin);
