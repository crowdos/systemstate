#ifndef PLUGIN_DB_H
#define PLUGIN_DB_H

#include <string>
#include <memory>
#include <list>

class PluginDb {
  class Data;

public:
  PluginDb();
  ~PluginDb();

  void scan(const std::string& path);
  std::string lookUp(const std::string& path);

private:
  std::list<std::shared_ptr<Data>> m_data;
};

#endif /* PLUGIN_DB_H */
