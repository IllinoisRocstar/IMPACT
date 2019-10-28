///
/// \file
/// \brief TCP/IP Network Utilities Interface
/// \ingroup irad_group
///
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <iostream>
#include <vector>

namespace IRAD {
namespace Sys {

/// TCP/IP Network Utilities
namespace Net {

class Server {
 private:
  int _server_descriptor;
  struct sockaddr_in _server_addy;
  std::vector<int> _client_descriptors;
  std::vector<struct sockaddr_in> _client_addys;

 public:
  Server() : _server_descriptor(-1) {
    std::memset(&_server_addy, 0, sizeof(struct sockaddr_in));
    _client_descriptors.resize(0);
    _client_addys.resize(0);
  };
  struct sockaddr_in &Address() {
    return (_server_addy);
  };
  const struct sockaddr_in Address() const { return (_server_addy); };
  std::vector<int> &ClientDescriptors() { return (_client_descriptors); };
  const std::vector<int> &ClientDescriptors() const {
    return (_client_descriptors);
  };
  int &Descriptor() { return (_server_descriptor); };
  int Descriptor() const { return (_server_descriptor); };
  std::vector<struct sockaddr_in> &ClientAddresses() {
    return (_client_addys);
  };
  const std::vector<struct sockaddr_in> &ClientAddresses() const {
    return (_client_addys);
  };
  int SimpleInit(int port = 0) {
    _server_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_descriptor < 0) {
      perror("Sys::Net::Server::SimpleInit::socket");
      return (-1);
    }
    if (port <= 0) port = 8044;  // should not collide with much ... if anything
    _server_addy.sin_family = AF_INET;
    _server_addy.sin_addr.s_addr = htonl(INADDR_ANY);
    _server_addy.sin_port = htons(port);
    int retval = bind(_server_descriptor,
                      reinterpret_cast<const sockaddr *>(&_server_addy),
                      sizeof(struct sockaddr_in));
    if (retval < 0) {
      perror("Sys::Net::Server::SimpleInit::bind");
      return (-1);
    }
    if (listen(_server_descriptor, 1)) {
      perror("Sys::Net::Server::SimpleInit::listen");
      return (-1);
    }
    return (0);
  };
  int Accept() {
    struct sockaddr_in client_addy;
    socklen_t addysize = sizeof(struct sockaddr_in);
    int new_descriptor =
        accept(_server_descriptor, reinterpret_cast<sockaddr *>(&client_addy),
               &addysize);
    if (new_descriptor < 0) {
      if ((errno != EAGAIN) || (errno != EWOULDBLOCK))
        perror("Sys::Net::Server::accept");
    } else {
      _client_descriptors.push_back(new_descriptor);
      _client_addys.push_back(client_addy);
    }
    return (new_descriptor);
  };
  int CloseAll() {
    std::vector<int>::iterator fdi = _client_descriptors.begin();
    while (fdi != _client_descriptors.end()) close(*fdi++);
    return (0);
  };
  ~Server() { CloseAll(); };
};

class Client {
 private:
  int _descriptor;
  struct sockaddr_in _server_addy;

 public:
  Client() : _descriptor(-1) {
    std::memset(&_server_addy, 0, sizeof(struct sockaddr_in));
  };
  int &Descriptor() { return (_descriptor); };
  int Descriptor() const { return (_descriptor); };
  struct sockaddr_in &ServerAddress() {
    return (_server_addy);
  };
  const struct sockaddr_in ServerAddresss() const { return (_server_addy); };
  int Connect(const std::string &server_address, int port = 0) {
    if (port <= 0) port = 8044;
    _descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (_descriptor < 0) {
      perror("Sys::Net::Client::Connect::socket");
      return (-1);
    }
    _server_addy.sin_family = AF_INET;
    _server_addy.sin_port = htons(port);
    int retval =
        inet_pton(AF_INET, server_address.c_str(), &_server_addy.sin_addr);
    if (retval < 0) {
      perror("Sys::Net::Client::Connect::inet_pton");
      return (-1);
    } else if (retval == 0) {
      std::cerr << "Sys::Net::Client::Connect::inet_pton: Invalid address: "
                << server_address << std::endl;
      return (-1);
    }
    if (connect(_descriptor, reinterpret_cast<const sockaddr *>(&_server_addy),
                sizeof(struct sockaddr_in)) < 0) {
      perror("Sys::Net::Client::Connect::connect");
      return (-1);
    }
    return (0);
  };
  int Close() {
    if (_descriptor > 0) close(_descriptor);
    return (0);
  };
  ~Client() { Close(); };
};
}  // namespace Net
}  // namespace Sys
}  // namespace IRAD
