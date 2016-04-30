#include "hwhal.h"
#include "stateplugin.h"
#include <sstream>

#include <hwhal/loopintegration/asio.h>
#include <hwhal/context.h>
#include <hwhal/control.h>
#include <hwhal/display.h>
#include <hwhal/lights.h>
#include <hwhal/info.h>
#include <hwhal/usb.h>

bool ControlContainer::read(std::string& data) {
  std::stringstream s;

  if (read(s)) {
    data = s.str();
    return true;
  }

  return false;
}

template <class T> ControlNode<T>::ControlNode(const std::string& name, systemstate::DirNode *dir,
					       systemstate::Plugin *plugin,
					       Context *ctx, const ControlId& id) :
  FileNode(name, dir, plugin),
  m_id(id),
  m_ctx(ctx),
  m_ctl(nullptr) {
}

template <class T> bool ControlNode<T>::start() {
  if (!m_ctx) {
    return false;
  }

  if (m_ctl) {
    return false;
  }

  m_ctl = m_ctx->control<T>(m_id);

  return m_ctl != nullptr;
}

template <class T> void ControlNode<T>::stop() {
  if (m_ctx && m_ctl) {
    Control *c = dynamic_cast<Control *>(m_ctl);
    if (c) {
      m_ctx->put(c);
    }
  }
}

bool ScreenBrightness::read(std::stringstream& data) {
  data << control()->backlightBrightness();
  return true;
}

bool ScreenBrightness::write(const std::string& data) {
  if (data.empty()) {
    return false;
  }

  int level;

  try {
    level = std::stoi(data);
  } catch (...) {
    // Oops!
    return false;
  }

  if (level < control()->minBacklightBrightness() || level > control()->maxBacklightBrightness()) {
    return false;
  }

  control()->setBacklightBrightness(level);

  return true;
}

bool ScreenBrightnessMin::read(std::stringstream& data) {
  data << control()->minBacklightBrightness();
  return true;
}

bool ScreenBrightnessMax::read(std::stringstream& data) {
  data << control()->maxBacklightBrightness();
  return true;
}

bool DeviceMaker::read(std::stringstream& data) {
  data << control()->maker();
  return true;
}

bool DeviceModel::read(std::stringstream& data) {
  data << control()->model();
  return true;
}

bool DeviceCodeName::read(std::stringstream& data) {
  data << control()->codeName();
  return true;
}

bool UsbConnected::read(std::stringstream& data) {
  data << m_connected ? "1" : "0";
  return true;
}

bool UsbConnected::start() {
  if (ControlNode::start()) {

    control()->addListener([this](bool connected) {
	if (connected != m_connected) {
	  m_connected = connected;
	  std::stringstream data;
	  read(data);
	  dataChanged(data.str());
	}
      });
    m_connected = control()->isCableConnected();
    return true;
  }

  return false;
}

void UsbConnected::stop() {
  if (control()) {
    control()->addListener(nullptr);
  }

  ControlNode::stop();
}

// Now the plugin
class HwHalPlugin : public systemstate::Plugin {
public:
  HwHalPlugin(boost::asio::io_service& service);
  ~HwHalPlugin();

  void init(systemstate::DirNode *root);
  bool start(systemstate::FileNode *node);
  void stop(systemstate::FileNode *node);
  bool read(systemstate::FileNode *node, std::string& data);
  bool write(systemstate::FileNode *node, const std::string& data);

private:
  LoopIntegration *m_loop;
  Context *m_ctx;
};

HwHalPlugin::HwHalPlugin(boost::asio::io_service& service) :
  m_loop(new LoopIntegrationAsio(service)),
  m_ctx(nullptr) {

}

HwHalPlugin::~HwHalPlugin() {
  delete m_loop;
  m_loop = nullptr;

  delete m_ctx;
  m_ctx = nullptr;
}

void HwHalPlugin::init(systemstate::DirNode *root) {
  m_ctx = Context::create(m_loop);
  if (!m_ctx) {
    return;
  }

  systemstate::DirNode *screen = root->appendDir("Screen");

  systemstate::DirNode *brightness = screen->appendDir("Brightness");
  brightness->appendFile(new ScreenBrightness(brightness, this, m_ctx));
  brightness->appendFile(new ScreenBrightnessMin(brightness, this, m_ctx));
  brightness->appendFile(new ScreenBrightnessMax(brightness, this, m_ctx));

  systemstate::DirNode *device = root->appendDir("Device");
  device->appendFile(new DeviceMaker(device, this, m_ctx));
  device->appendFile(new DeviceModel(device, this, m_ctx));
  device->appendFile(new DeviceCodeName(device, this, m_ctx));

  systemstate::DirNode *usb = root->appendDir("USB");
  usb->appendFile(new UsbConnected(usb, this, m_ctx));
}

bool HwHalPlugin::start(systemstate::FileNode *node) {
  if (!m_ctx) {
    return false;
  }

  return dynamic_cast<ControlContainer *>(node)->start();
}

void HwHalPlugin::stop(systemstate::FileNode *node) {
  dynamic_cast<ControlContainer *>(node)->stop();
}

bool HwHalPlugin::read(systemstate::FileNode *node, std::string& data) {
  return dynamic_cast<ControlContainer *>(node)->read(data);
}

bool HwHalPlugin::write(systemstate::FileNode *node, const std::string& data) {
  return dynamic_cast<ControlContainer *>(node)->write(data);
}

REGISTER_STATE_PLUGIN(HwHalPlugin);
