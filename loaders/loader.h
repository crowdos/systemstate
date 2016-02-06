#ifndef LOADER_H
#define LOADER_H

#include <string>

class Plugin;

class AbstractLoader {
public:
  AbstractLoader();
  virtual ~AbstractLoader();

  bool init(int argc, char *argv[]);

  int exec();

protected:
  virtual void quit() = 0;
  virtual int run() = 0;

private:
  bool load(const std::string& path);
  void unload();

  Plugin *m_plugin;
};

#define EXEC_LOADER(l)				\
  int main(int argc, char *argv[]) {		\
    l loader;					\
						\
    if (!loader.init(argc, argv)) {		\
      return 1;					\
    }						\
    						\
    return loader.exec();			\
  }

#endif /* LOADER_H */
