#include "plugindb.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/tokenizer.hpp>
#include <unordered_set>
#include <cstring>
#include <dirent.h>

class PluginDb::Data {
public:
  Data(const std::string& name,
       const boost::optional<std::string>& files,
       const boost::optional<std::string>& dirs) :
    m_name(name) {
    if (files) {
      m_files = parse(*files);
    }

    if (dirs) {
      m_dirs = parse(*dirs);
    }
  }

  ~Data() {

  }

  bool match(const std::string& path) const {
    if (match(path, m_files)) {
      return true;
    }

    std::size_t pos = path.find_last_of(".");
    if (pos == std::string::npos) {
      return false;
    }

    return match(path.substr(0, pos), m_dirs);
  }

  const std::string& name() const { return m_name; }

private:
  std::unordered_set<std::string> parse(const std::string& str) {
    boost::char_separator<char> sep("|");
    boost::tokenizer<boost::char_separator<char> > tok(str, sep);

    std::unordered_set<std::string> tokens;

    for (auto iter = tok.begin(); iter != tok.end(); iter++) {
      if (tokens.find(*iter) == tokens.end()) {
	tokens.insert(*iter);
      }
    }

    return std::move(tokens);
  }

  bool match(const std::string& path, const std::unordered_set<std::string>& paths) const {
    return paths.find(path) != paths.end();
  }

  const std::string m_name;
  std::unordered_set<std::string> m_files;
  std::unordered_set<std::string> m_dirs;
};

PluginDb::PluginDb() {

}

PluginDb::~PluginDb() {

}

void PluginDb::scan(const std::string& path) {
  struct dirent **namelist;
  int len = scandir(path.c_str(), &namelist, [](const struct dirent *entry) -> int {
      std::string d(entry->d_name);
      // return non zero to keep the entries.
      try {
	return d.compare(d.size() - 4, 4, ".ini") == 0 ? 1 : 0;
      } catch (...) {
	// Discard.
	return 0;
      }
    }, alphasort);

  if (len == -1) {
    std::cerr << "scandir() failed: " << std::strerror(errno) << std::endl;
    return;
  }

  for (int x = 0; x < len; x++) {
    std::stringstream s;
    s << path << "/" << namelist[x]->d_name;
    boost::property_tree::ptree pt;

    try {
      boost::property_tree::ini_parser::read_ini(s.str(), pt);
      std::string name(pt.get<std::string>("plugin.name"));
      boost::optional<std::string> files(pt.get_optional<std::string>("plugin.file"));
      boost::optional<std::string> dirs(pt.get_optional<std::string>("plugin.dir"));

      std::shared_ptr<Data> data = std::make_shared<Data>(name, files, dirs);

      m_data.push_back(data);
    } catch (const std::exception& ex) {
      std::cerr << "Error reading plugin ini file: " << ex.what() << std::endl;
      continue;
    }
  }

  while (len--) {
    free(namelist[len]);
  }

  free(namelist);
}

std::string PluginDb::lookUp(const std::string& path) {
  auto iter = std::find_if(m_data.begin(), m_data.end(),
			   [&path](const std::shared_ptr<Data>& data)->bool{
			     return data->match(path);
			   });

  return iter == m_data.end() ? std::string() : (*iter)->name();
}
