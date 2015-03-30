//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

class Directory : public std::vector<std::string>
{
protected:
  std::string _path;
  bool   _good;
  DIR    *_dir;
public:
  Directory(const std::string &s = "");
  ~Directory();
  int open(const std::string &s = "");
  void close();
  operator void* ();
  bool operator ! ();
};

#endif



