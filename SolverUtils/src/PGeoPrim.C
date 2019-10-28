//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//
#include "PGeoPrim.H"

namespace SolverUtils {
namespace GeoPrim {

int GeoPrim::PBox::Pack(void **inbuf) {
  if (_buf && _mine) {
    delete[](char *) _buf;
    _buf = NULL;
    _mine = false;
  }
  std::ostringstream Ostr;
  Ostr.write((const char *)p1.Data(), sizeof(double) * 3);
  Ostr.write((const char *)p2.Data(), sizeof(double) * 3);
  // Ostr << *this;
  if (!inbuf) {
    int nchar = IRAD::Util::String2Buf(Ostr.str(), &_buf);
    if (_buf) {
      _mine = true;
      _bsize = nchar;
      return (_bsize);
    }
  } else {
    // Caller must free inbuf, else it leaks
    int nchar = IRAD::Util::String2Buf(Ostr.str(), inbuf);
    if (*inbuf) {
      _buf = *inbuf;
      _mine = false;
      _bsize = nchar;
      return (_bsize);
    }
  }
  return 0;
}

int GeoPrim::PBox::UnPack(const void *inbuf) {
  if (!inbuf) {
    if (!_buf) return 1;
    std::istringstream Istr(std::string((const char *)_buf, 48));
    Istr.read((char *)p1.Data(), sizeof(double) * 3);
    Istr.read((char *)p2.Data(), sizeof(double) * 3);
    //	Istr >> *this;
    if (_mine) {
      delete[](char *) _buf;
      _buf = NULL;
      _mine = false;
      _bsize = 0;
    }
    return (0);
  } else {
    std::istringstream Istr(std::string((const char *)inbuf, 48));
    //	Istr >> *this;
    Istr.read((char *)p1.Data(), sizeof(double) * 3);
    Istr.read((char *)p2.Data(), sizeof(double) * 3);
    return (0);
  }
  return (0);
}

//   int GeoPrim::PBox::Pack(void **inbuf)
//   {
//     if(_buf && _mine){
//       delete [] (char *)_buf;
//       _buf = NULL;
//       _mine = false;
//     }
//     std::ostringstream Ostr;
//     Ostr << *(dynamic_cast<GeoPrim::C3Box *>(this));
//     if(!inbuf){
//       int nchar = Util::String2Buf(Ostr.str(),&_buf);
//       if(_buf){
// 	_mine = true;
// 	return(nchar);
//       }
//     }
//     else {
//       // Caller must free inbuf, else it leaks
//       int nchar = Util::String2Buf(Ostr.str(),inbuf);
//       if(*inbuf){
// 	_buf = *inbuf;
// 	_mine = false;
// 	return(nchar);
//       }
//     }
//     return 0;
//   };

//   int GeoPrim::PBox::UnPack(const void *inbuf)
//   {
//     if(!inbuf){
//       if(!_buf)
// 	return 1;
//       std::istringstream Istr(std::string((const char *)_buf));
//       Istr >> *(dynamic_cast<GeoPrim::C3Box *>(this));
//       if(_mine){
// 	delete [] (char *)_buf;
// 	_buf = NULL;
// 	_mine = false;
//       }
//       return(0);
//     }
//     else{
//       std::istringstream Istr(std::string((const char *)inbuf));
//       Istr >> *(dynamic_cast<GeoPrim::C3Box *>(this));
//       return(0);
//     }
//     return 0;
//   };

}  // namespace GeoPrim
}  // namespace SolverUtils
