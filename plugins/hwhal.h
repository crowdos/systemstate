#ifndef __HW_HAL_H__
#define __HW_HAL_H__

#include <string>
#include <sstream>
#include "stateplugin.h"
#include <hwhal/control.h>

class Context;
class Display;
class Lights;
class Info;
class Usb;

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
	      Context *ctx, const ControlId& id);

  virtual ~ControlNode() {}
  bool start();
  void stop();

  T *control() { return m_ctl; }

private:
  ControlId m_id;
  Context *m_ctx;
  T *m_ctl;
};

class ScreenBrightness : public ControlNode<Lights> {
public:
  ScreenBrightness(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Lights>("Current", dir, plugin, ctx, ControlId::Lights) {

  }

  bool read(std::stringstream& data);
  bool write(const std::string& data);
};

class ScreenBrightnessMin : public ControlNode<Lights> {
public:
  ScreenBrightnessMin(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Lights>("Minimum", dir, plugin, ctx, ControlId::Lights) {

  }

  bool read(std::stringstream& data);
};

class ScreenBrightnessMax : public ControlNode<Lights> {
public:
  ScreenBrightnessMax(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Lights>("Maximum", dir, plugin, ctx, ControlId::Lights) {

  }

  bool read(std::stringstream& data);
};

class DeviceMaker : public ControlNode<Info> {
public:
  DeviceMaker(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Info>("Maker", dir, plugin, ctx, ControlId::Info) {

  }

  bool read(std::stringstream& data);
};

class DeviceModel : public ControlNode<Info> {
public:
  DeviceModel(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Info>("Model", dir, plugin, ctx, ControlId::Info) {

  }

  bool read(std::stringstream& data);
};

class DeviceCodeName : public ControlNode<Info> {
public:
  DeviceCodeName(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Info>("CodeName", dir, plugin, ctx, ControlId::Info) {

  }

  bool read(std::stringstream& data);
};

class UsbConnected : public ControlNode<Usb> {
public:
  UsbConnected(systemstate::DirNode *dir, systemstate::Plugin *plugin, Context *ctx) :
    ControlNode<Usb>("Connected", dir, plugin, ctx, ControlId::Usb),
    m_connected(false) {

  }

  bool read(std::stringstream& data);

  bool start();
  void stop();

private:
  bool m_connected;
};

#endif /* __HW_HAL_H__ */
