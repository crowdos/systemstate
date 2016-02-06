#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <vector>

class Process {
public:
  Process();
  ~Process();

  bool exec(const std::string& exe,
	    const std::vector<std::string>& args = std::vector<std::string>());

  bool read(std::string& data);
  bool write(const std::string& data);

  int fd();

  void kill();

private:
  std::string m_exe;
  int m_fd;
  pid_t m_pid;
};

#endif /* PROCESS_H */
