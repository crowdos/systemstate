#include <glib.h>
#include "loader.h"

class Loader : public AbstractLoader {
public:
  Loader() :
    m_loop(g_main_loop_new(NULL, FALSE)) {
  }

  ~Loader() {
    if (m_loop) {
      g_main_loop_unref(m_loop);
      m_loop = nullptr;
    }
  }

protected:
  void quit() {
    g_main_loop_quit(m_loop);
  }

  int run() {
    g_main_loop_run(m_loop);
    return 0;
  }

private:
  GMainLoop *m_loop;
};

EXEC_LOADER(Loader)
