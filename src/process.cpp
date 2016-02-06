#include "process.h"
#include <iostream>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>

Process::Process() :
  m_fd(-1),
  m_pid(-1) {

}

Process::~Process() {
  kill();
}

bool Process::exec(const std::string& exe, const std::vector<std::string>& args) {
  if (m_pid != -1) {
    std::cerr << "A process is already running with pid " << m_pid << std::endl;
    return false;
  }

  if (exe[exe.size() - 1] == '/') {
    std::cerr << "Invalid executable " << exe << std::endl;
    return false;
  }
#if 0
  m_pid = fork();
  switch (m_pid) {
  case -1:
    std::cerr << "Error forking " << std::strerror(errno) << std::endl;
    return false;

  case 0: // child
    break;

  default:
    return true;
  }
#endif

  std::vector<const char *> argv;
  int pos = exe.find_last_of("/");
  if (pos == std::string::npos) {
    m_exe = exe;
  } else {
    m_exe = exe.substr(pos+1);
  }

  argv.push_back(m_exe.c_str());

  for (const std::string& s : args) {
    argv.push_back(s.c_str());
  }

  argv.push_back(NULL);

  int sv[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
    std::cerr << "Error creating communication socket " << std::strerror(errno) << std::endl;
    return false;
  }

  m_pid = fork();

  switch (m_pid) {
  case -1:
    std::cerr << "Error forking " << std::strerror(errno) << std::endl;
    return false;

  case 0: // child
    close(sv[0]);
    close(2); // std error

    dup2(sv[1], 0); // std in
    dup2(sv[1], 1); // std out
    execvp(m_exe.c_str(), (char **)argv.data());
    std::cout << "ER" << std::endl;
    _exit(1);

  default:
    break;
  }

  close(sv[1]);
  m_fd = sv[0];

  char buff[3];
  ssize_t len = ::read(m_fd, buff, 3);

  if (len != 3 || buff != std::string("OK\n")) {
    std::cerr << "Failed to launch " << exe << std::endl;
    return false;
  }

  return true;
}

bool Process::read(std::string& data) {
  // TODO:
}

bool Process::write(const std::string& data) {
  // TODO:
}

int Process::fd() {
  return m_fd;
}

void Process::kill() {
  if (m_pid != -1) {
    ::kill(m_pid, SIGKILL);
    m_pid = -1;
  }
}
