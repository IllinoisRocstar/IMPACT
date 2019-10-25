//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <dirent.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <vector>

#include "Directory.H"

Directory::Directory(const std::string &path) {
  _good = false;
  _dir = NULL;
  _path.assign(path);
  if (open(path)) {
    std::cerr << "Directory::Error: Could not open " << path
              << " as a directory." << std::endl;
    _good = false;
  } else
    close();
}

Directory::~Directory() {
  // Take the default
}

Directory::operator void *() { return (_good ? this : NULL); }

bool Directory::operator!() { return (!_good); }

void Directory::close() {
  if (_good) closedir(_dir);
}

int Directory::open(const std::string &path) {
  //  if(_good){
  //    this->close();
  _path = path;
  //  }
  if (path.empty()) return (1);
  if (!(_dir = opendir(path.c_str()))) return (1);
  _path = path;
  _good = true;
  struct dirent *entry;
  // Skip . and ..
  entry = readdir(_dir);
  entry = readdir(_dir);
  while ((entry = readdir(_dir)) != NULL) this->push_back(entry->d_name);
  return (0);
}
