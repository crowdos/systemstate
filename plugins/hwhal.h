#ifndef __HW_HAL_H__
#define __HW_HAL_H__

#include <string>
#include <sstream>
#include "stateplugin.h"

class Context;
class Display;
class Lights;
class Info;

class ControlContainer {
public:
  virtual ~ControlContainer() {}
  virtual bool start() = 0;
  virtual void stop() = 0;
  bool read(std::string& data);
  virtual bool write(const std::string& data) { return false; }

protected:
  virtual bool read(std::stringstream& data) = 0;
};

template <class T> class ControlNode : public systemstate::FileNode, public ControlContainer {
public:
  ControlNode(const std::string& name, systemstate::DirNode *dir, systemstate::Plugin *plugin,
	      Context *ctx, const std::string& id);

  virtual ~ControlNode() {}
  bool start();
  void stop();

  T *control() { return m_ctl; }

private:
  std::string m_id;
  Context *m_ctx;
  T *m_ctl;
};

class ScreenBlanked : public ControlNode<Display> {
public:
  ScreenBlanked(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Display>("Blanked", dir, plugin, ctx, "display") {

  }

  bool read(std::stringstream& data);
  bool write(const std::string& data);
};

class ScreenBrightness : public ControlNode<Lights> {
public:
  ScreenBrightness(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Lights>("Current", dir, plugin, ctx, "light") {

  }

  bool read(std::stringstream& data);
  bool write(const std::string& data);
};

class ScreenBrightnessMin : public ControlNode<Lights> {
public:
  ScreenBrightnessMin(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Lights>("Minimum", dir, plugin, ctx, "light") {

  }

  bool read(std::stringstream& data);
};

class ScreenBrightnessMax : public ControlNode<Lights> {
public:
  ScreenBrightnessMax(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Lights>("Maximum", dir, plugin, ctx, "light") {

  }

  bool read(std::stringstream& data);
};

class DeviceMaker : public ControlNode<Info> {
public:
  DeviceMaker(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Info>("Maker", dir, plugin, ctx, "info") {

  }

  bool read(std::stringstream& data);
};

class DeviceModel : public ControlNode<Info> {
public:
  DeviceModel(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Info>("Model", dir, plugin, ctx, "info") {

  }

  bool read(std::stringstream& data);
};

class DeviceCodeName : public ControlNode<Info> {
public:
  DeviceCodeName(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Info>("CodeName", dir, plugin, ctx, "info") {

  }

  bool read(std::stringstream& data);
};

#endif /* __HW_HAL_H__ */