#include "stateplugin.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>

using namespace systemstate;

class Test2Plugin : public Plugin {
  void init(DirNode *root);
  bool start(FileNode *node);
  void stop(FileNode *node);
  bool read(FileNode *node, std::string& data);
  bool write(FileNode *node, const std::string& data);
};

void Test2Plugin::init(DirNode *root) {
  DirNode *d = root->appendDir("Test");
  d = d->appendDir("Test2");
  d->appendFile("test1", this);
  d->appendFile("test2", this);
  d = root->appendDir("Test3");
  d->appendFile("test33", this);
  d->appendFile("test333", this);
}

bool Test2Plugin::start(FileNode *node) {
  return true;
}

void Test2Plugin::stop(FileNode *node) {

}

bool Test2Plugin::read(FileNode *node, std::string& data) {
  data = node->name();
  return true;
}

bool Test2Plugin::write(FileNode *node, const std::string& data) {
  return false;
}

REGISTER_STATE_PLUGIN(Test2Plugin);
