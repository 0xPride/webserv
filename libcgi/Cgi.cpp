#include "libcgi/Cgi.hpp"
#include "libnet/SessionState.hpp"
#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

char libcgi::Cgi::temp[] = "/tmp/webserv/webserv_cgi_in_XXXXXX";

ssize_t doesContainerHasBuff(const char *raw, size_t rLen, const char *ptr, size_t pLen) {
  for (size_t i = 0; i < rLen - pLen + 1; i++) {
    if (!strncmp(&raw[i], ptr, strlen(ptr)))
      return i;
  }
  return -1;
}

std::string asStr(int fd) {
  std::stringstream ss;

  ss << fd;
  return ss.str();
};

std::pair<libcgi::Cgi::Error, libcgi::Cgi::State> libcgi::Cgi::handleCgiBuff(char  *ptr,
                                                                             size_t len) {
  ssize_t        idx;
  const char    *del = "\r\n\r\n";
  Respons::error err;

  if (this->state == READING_HEADERS) {
    idx = doesContainerHasBuff(ptr, len, del, strlen(del));
    if (idx == -1) {
      this->res.cgiHeader.insert(this->res.cgiHeader.end(), ptr, ptr + len);
      return std::make_pair(OK, READING_HEADERS);
    }
    this->state = READING_BODY;
    this->res.cgiHeader.insert(this->res.cgiHeader.end(), ptr, ptr + idx + 2); // plus the \r\n
    err = this->res.build();
    if (err != Respons::OK)
      return std::make_pair(MALFORMED, READING_BODY);
    this->res.sockBuff->insert(this->res.sockBuff->end(), ptr + idx + 3,
                               ptr + len); // plus 2 cus \n\n
    return std::make_pair(OK, READING_BODY);
  }
  // this->res.body.insert(this->res.body.end(), ptr, ptr + len);
  this->res.sockBuff->insert(this->res.sockBuff->end(), ptr, ptr + len);
  return std::make_pair(OK, READING_BODY);
}

char *keyAndValAsStr(const std::string &key, const std::string &val) {
  char *keyAndVal;

  keyAndVal = new char[key.size() + val.size() + 2];
  std::memcpy(keyAndVal, key.c_str(), key.size());
  keyAndVal[key.size()] = '=';
  std::memcpy(keyAndVal + key.size() + 1, val.c_str(), val.size());
  keyAndVal[key.size() + val.size() + 1] = 0;
  return keyAndVal;
}

char **headersAsEnv(std::map<std::string, std::string> &headers) {
  char                                             **env;
  std::map<std::string, std::string>::const_iterator it;
  std::map<std::string, std::string>::const_iterator end;
  std::map<std::string, std::string>::size_type      i;

  env = new char *[headers.size() + 1];
  i = 0;
  it = headers.begin();
  end = headers.end();
  for (; i < headers.size(); i++, it++) {
    env[i] = keyAndValAsStr(it->first, it->second);
  }
  env[i] = NULL;
  return env;
}

char **getScriptArgs(std::string &scriptPath) {
  char **argv;

  argv = new char *[2];

  argv[0] = ::strdup(scriptPath.c_str());
  argv[1] = NULL;
  return argv;
}

void delete2d(char **env) {
  unsigned int i;

  for (i = 0; env[i]; i++)
    delete[] env[i];
  delete[] env;
}

libcgi::Cgi::Cgi(sockaddr_in *clientAddr, size_t bufferSize)
    : clientAddr(clientAddr)
    , req()
    , state(INIT)
    , pid(-1)
    , bodySize(0)
    , bufferSize(bufferSize) {
  fd[0] = -1;
  fd[1] = -1;
}

libcgi::Cgi::Error libcgi::Cgi::init(libhttp::Request *httpReq, std::string scriptPath,
                                     std::string serverName, std::string localReqPath,
                                     std::string serverPort, std::string protocol,
                                     std::string serverSoftware) {
  struct stat            s;
  std::string::size_type i;
  std::string            scriptName;

  if (::stat(scriptPath.c_str(), &s) != 0)
    return FAILED_OPEN_SCRIPT;
  if (!(s.st_mode & S_IXUSR))
    return FAILED_EXEC_PERM;

  if (::pipe(fd) < 0)
    return FAILED_OPEN_PIPE;
  this->cgiInput = ::mkstemp(temp);
  if (cgiInput < 0)
    return FAILED_OPEN_FILE;
  i = scriptPath.rfind('/');
  if (i == std::string::npos)
    scriptName = scriptPath;
  else
    scriptName = scriptPath.substr(i + 1, scriptPath.size() - i);

  this->req.init(scriptPath, serverName, scriptName, localReqPath, serverPort, protocol,
                 serverSoftware);
  this->req.build(httpReq);
  return OK;
}

libcgi::Cgi::Error libcgi::Cgi::write(std::vector<char> &body) {
  ssize_t len;

  len = ::write(this->cgiInput, &body.front(), body.size());
  if (len < 0)
    return FAILED_WRITE;
  this->bodySize += len;
  return OK;
};

libcgi::Cgi::Error libcgi::Cgi::exec() {
  char **env, **argv;

  lseek(this->cgiInput, 0, SEEK_SET);
  pid = fork();
  if (pid == 0) {
    dup2(this->cgiInput, STDIN_FILENO);
    dup2(this->fd[1], STDOUT_FILENO);
    argv = getScriptArgs(req.scriptPath);
    env = headersAsEnv(req.env);
    ::execve(req.scriptPath.c_str(), argv, env);
    delete2d(argv);
    delete2d(env);
    std::cout << "Status: 500 Internal Server Error\n\n";
    // write to the fd the error
    exit(1);
  } else if (pid > 0) {
    this->state = READING_HEADERS;
    close(fd[1]);
    return OK;
  }
  return FAILED_FORK;
}

libcgi::Cgi::Error libcgi::Cgi::read() {
  char                    buff[bufferSize];
  ssize_t                 len;
  std::pair<Error, State> newState;

  if (this->state == INIT)
    this->state = READING_HEADERS;
  len = ::read(this->fd[0], buff, sizeof buff);
  if (len == 0) {
    this->state = FIN;
    return OK;
  }
  if (len < 0)
    return FAILED_READ;

  newState = this->handleCgiBuff(buff, len);
  this->state = newState.second;
  if (newState.first != OK)
    return MALFORMED;
  return OK;
}

void libcgi::Cgi::clean() {
  close(this->fd[0]);
  close(this->cgiInput);
  this->req.clean();
  // waitpid(this->pid, 0, 0); // this will block
  kill(this->pid, SIGKILL);
  fd[0] = -1;
  fd[1] = -1;
  cgiInput = -1;
}
