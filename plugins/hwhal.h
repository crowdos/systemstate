#ifndef __HW_HAL_H__
#define __HW_HAL_H__

#include <string>
#include "stateplugin.h"

class Context;
class Display;

class ControlContainer {
public:
  virtual ~ControlContainer() {}
  virtual bool start() = 0;
  virtual void stop() = 0;
  virtual bool read(std::string& data) = 0;
  virtual bool write(const std::string& data) = 0;
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

  bool read(std::string& data);
  bool write(const std::string& data);
};

#endif /* __HW_HAL_H__ */
