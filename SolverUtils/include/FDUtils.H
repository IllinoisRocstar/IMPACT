///
/// \file
/// \brief Implementation of stream object for Unix file descriptors
/// \ingroup irad_group
///
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <istream>
#include <ostream>
#include <sstream>
#include <streambuf>

//#include "primitive_utilities.H"

namespace IRAD {

namespace Sys {

// Josuttis' fdstream classes (slightly modified)

/************************************************************
 * fdostream
 * - a stream that writes on a file descriptor
 ************************************************************/

class fdoutbuf : public std::streambuf {
 protected:
  int fd;  // file descriptor
 public:
  fdoutbuf() { fd = -1; };
  // constructor
  fdoutbuf(int _fd) : fd(_fd) {}
  void Init(int _fd) { fd = _fd; };
  bool Ready() { return (fd >= 0); };
  virtual int FD() { return fd; };

 protected:
  // write one character
  virtual int_type overflow(int_type c) {
    if (c != EOF) {
      char z = c;
      if (write(fd, &z, 1) != 1) {
        return EOF;
      }
    }
    return c;
  }
  // write multiple characters
  virtual std::streamsize xsputn(const char *s, std::streamsize num) {
    return write(fd, s, num);
  }
};

class fdostream : public std::ostream {
 protected:
  Sys::fdoutbuf buf;

 public:
  fdostream() : std::ostream(0){};
  fdostream(int fd) : std::ostream(0), buf(fd) { rdbuf(&buf); }
  int FD() { return (buf.FD()); };
  int Init(int _fd) {
    buf.Init(_fd);
    rdbuf(&buf);
    return (buf.FD());
  };
};

/************************************************************
 * fdistream
 * - a stream that reads on a file descriptor
 ************************************************************/

class fdinbuf : public std::streambuf {
 protected:
  int fd;  // file descriptor
 protected:
  /* data buffer:
   * - at most, pbSize characters in putback area plus
   * - at most, bufSize characters in ordinary read buffer
   */
  static const int pbSize = 4;      // size of putback area
  static const int bufSize = 1024;  // size of the data buffer
  char buffer[bufSize + pbSize];    // data buffer

 public:
  fdinbuf() { fd = -1; };
  /* constructor
   * - initialize file descriptor
   * - initialize empty data buffer
   * - no putback area
   * => force underflow()
   */
  fdinbuf(int _fd) : fd(_fd) {
    setg(buffer + pbSize,   // beginning of putback area
         buffer + pbSize,   // read position
         buffer + pbSize);  // end position
  }

  void Init(int _fd) { fd = _fd; };
  bool Ready() { return (fd >= 0); };
  virtual int FD() { return fd; };

 protected:
  // insert new characters into the buffer
  virtual int_type underflow() {
    using std::memcpy;

    // is read position before end of buffer?
    if (gptr() < egptr()) {
      return *gptr();
    }

    /* process size of putback area
     * - use number of characters read
     * - but at most size of putback area
     */
    int numPutback;
    numPutback = gptr() - eback();
    if (numPutback > pbSize) {
      numPutback = pbSize;
    }

    /* copy up to pbSize characters previously read into
     * the putback area
     */
    memcpy(buffer + (pbSize - numPutback), gptr() - numPutback, numPutback);

    // read at most bufSize new characters
    int num;
    num = read(fd, buffer + pbSize, bufSize);
    if (num <= 0) {
      // ERROR or EOF
      return EOF;
    }

    // reset buffer pointers
    setg(buffer + (pbSize - numPutback),  // beginning of putback area
         buffer + pbSize,                 // read position
         buffer + pbSize + num);          // end of buffer

    // return next character
    return *gptr();
  }
};

class fdistream : public std::istream {
 protected:
  Sys::fdinbuf buf;

 public:
  fdistream() : std::istream(0){};
  fdistream(int fd) : std::istream(0), buf(fd) { rdbuf(&buf); };
  int FD() { return (buf.FD()); };
  int Init(int _fd) {
    buf.Init(_fd);
    rdbuf(&buf);
    return (buf.FD());
  };
};

class InProcess : public fdistream {
 private:
  std::string _comline;
  FILE *_file_object;

 public:
  InProcess() : _file_object(NULL){};
  InProcess(const std::string &command)
      : _comline(command), _file_object(NULL) {
    _file_object = popen(_comline.c_str(), "r");
    if (_file_object) Init(fileno(_file_object));
  }
  virtual int Execute(const std::string &command) {
    if (_file_object) pclose(_file_object);
    _comline.assign(command);
    _file_object = popen(_comline.c_str(), "r");
    if (_file_object) return (Init(fileno(_file_object)));
    return (-1);
  }
  virtual int Finalize() {
    if (_file_object) return (pclose(_file_object));
    return (0);
  };
  virtual FILE *GetFile() { return (_file_object); };
  virtual std::string Command() { return (_comline); };
  virtual ~InProcess() { pclose(_file_object); };
};

// Class for managing the FD sets
class FDSetMan {
 public:
  typedef std::vector<int> FDContainerType;

 private:
  fd_set readset;
  fd_set writeset;
  fd_set exset;
  int max_descriptor;
  FDContainerType o_descriptors;
  FDContainerType i_descriptors;
  FDContainerType io_descriptors;
  //    std::vector<int> file_descriptors;
  //    std::map<int,Sys::fdostream *> outstreams;
  //    std::map<int,Sys::fdistream *> instreams;
 public:
  FDSetMan() : max_descriptor(-1) {
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exset);
    o_descriptors.resize(0);
    i_descriptors.resize(0);
    io_descriptors.resize(0);
    //      instreams[0]  = &std::cin;
    //      outstreams[1] = &std::out;
    //      outstreams[2] = &std::cerr;
  };
  fd_set &ReadSet() { return (readset); };
  const fd_set ReadSet() const { return (readset); };
  fd_set &WriteSet() { return (writeset); };
  const fd_set WriteSet() const { return (writeset); };
  fd_set &ExceptionSet() { return (exset); };
  const fd_set ExceptionSet() const { return (exset); };
  void SetOutDescriptor(int infd) {
    if (infd >= 0) FD_SET(infd, &writeset);
    if (infd > max_descriptor) max_descriptor = infd;
  };
  void SetInDescriptor(int infd) {
    if (infd >= 0) FD_SET(infd, &readset);
    if (infd > max_descriptor) max_descriptor = infd;
  };
  void ClearOutDescriptor(int infd) {
    if (infd >= 0) FD_CLR(infd, &writeset);
  };
  void ClearInDescriptor(int infd) {
    if (infd >= 0) FD_CLR(infd, &readset);
  };
  int AddOutDescriptor(int infd) {
    if (infd < 0) return (-1);
    o_descriptors.push_back(infd);
    SetOutDescriptor(infd);
    //      if(infd > max_descriptor)
    //	max_descriptor = infd;
    return (0);
  };
  int AddInDescriptor(int infd) {
    if (infd < 0) return (-1);
    i_descriptors.push_back(infd);
    SetInDescriptor(infd);
    return (0);
  };
  int AddIODescriptor(int infd) {
    if (infd < 0) return (-1);
    io_descriptors.push_back(infd);
    SetInDescriptor(infd);
    SetOutDescriptor(infd);
    return (0);
  };
  void Zero() {
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exset);
  };
  void Clear(int infd) {
    if (infd >= 0) {
      FD_CLR(infd, &readset);
      FD_CLR(infd, &writeset);
      FD_CLR(infd, &exset);
    }
  };
  void SetForRead(int infd = -1) {
    if (infd >= 0)
      FD_SET(infd, &readset);
    else {
      Set(i_descriptors, &readset);
      Set(io_descriptors, &readset, false);
    }
  };
  template <class FDContainer>
  void SetForRead(const FDContainer &fds, bool reset = true) {
    Set(fds, &readset, reset);
  }
  void SetForWrite(int infd = -1) {
    if (infd >= 0)
      FD_SET(infd, &writeset);
    else {
      Set(o_descriptors, &writeset);
      Set(io_descriptors, &writeset, false);
    }
  };
  template <class FDContainer>
  void SetForWrite(const FDContainer &fds) {
    Set(fds, &writeset);
  }
  void SetForIO(int infd = -1) {
    if (infd >= 0) {
      FD_SET(infd, &readset);
      FD_SET(infd, &writeset);
    } else {
      Set(i_descriptors, &readset);
      Set(o_descriptors, &writeset);
      Set(io_descriptors, &readset, false);
      Set(io_descriptors, &writeset, false);
    }
  };
  template <class FDContainer>
  void SetForIO(const FDContainer &fds, bool reset = true) {
    Set(i_descriptors, &readset, reset);
    Set(o_descriptors, &writeset, reset);
    Set(io_descriptors, &readset, false);
    Set(io_descriptors, &writeset, false);
  }
  int ReadyForOutput(int infd = -1) {
    int retval = 0;
    if (infd >= 0)
      retval = FD_ISSET(infd, &writeset);
    else {
      retval += NSet(o_descriptors, &writeset);
      retval += NSet(io_descriptors, &writeset);
    }
    return (retval);
  };
  int ReadyForInput(int infd = -1) {
    int retval = 0;
    if (infd >= 0) {
      retval = FD_ISSET(infd, &readset);
    } else {
      retval += NSet(i_descriptors, &readset);
      retval += NSet(io_descriptors, &readset);
    }
    return (retval);
  };
  template <class FDContainer>
  int ReadyForOutput(const FDContainer &fds) {
    return (NSet(fds, &writeset));
  }
  template <class FDContainer>
  int ReadyForInput(const FDContainer &fds) {
    return (NSet(fds, &readset));
  }
  int Select(float seconds = 0.0, bool reset = true) {
    float usec = 1.0E-6;
    struct timeval tv;
    struct timeval *tv_ptr = NULL;
    if (seconds >= 0.0) {
      tv_ptr = &tv;
      tv.tv_sec = static_cast<long>(seconds);
      tv.tv_usec = static_cast<long>((seconds - tv.tv_sec) / usec);
    }
    if (reset) {
      this->Zero();
      this->SetForRead();
      this->SetForWrite();
    }
    int retval =
        select(max_descriptor + 1, &readset, &writeset, &exset, tv_ptr);
    if (retval < 0) perror("Sys::FDStreamSet::Select::select");
    return (retval);
  };
  int ListenForInput(float seconds = 0.0, bool reset = true) {
    float usec = 1.0E-6;
    struct timeval tv;
    struct timeval *tv_ptr = NULL;
    if (seconds >= 0.0) {
      tv_ptr = &tv;
      tv.tv_sec = static_cast<long>(seconds);
      tv.tv_usec = static_cast<long>((seconds - tv.tv_sec) / usec);
    }
    if (reset) {
      this->Zero();
      this->SetForRead();
    }
    int retval = select(max_descriptor + 1, &readset, NULL, &exset, tv_ptr);
    if (retval < 0) perror("Sys::FDStreamSet::Select::select");
    return (retval);
  };
  int ListenForOutput(float seconds = 0.0, bool reset = true) {
    float usec = 1.0E-6;
    struct timeval tv;
    struct timeval *tv_ptr = NULL;
    if (seconds >= 0.0) {
      tv_ptr = &tv;
      tv.tv_sec = static_cast<long>(seconds);
      tv.tv_usec = static_cast<long>((seconds - tv.tv_sec) / usec);
    }
    if (reset) {
      this->Zero();
      this->SetForWrite();
    }
    int retval = select(max_descriptor + 1, NULL, &writeset, &exset, tv_ptr);
    if (retval < 0) perror("Sys::FDStreamSet::Select::select");
    return (retval);
  };
  template <class FDContainer>
  int NSet(const FDContainer &fds, fd_set *fdset) {
    int retval = 0;
    typename FDContainer::const_iterator fdi = fds.begin();
    while (fdi != fds.end()) retval += FD_ISSET(*fdi++, fdset);
    return (retval);
  }
  template <class FDContainer>
  void Set(const FDContainer &fds, fd_set *fdset, bool reset = true) {
    if (reset) FD_ZERO(fdset);
    typename FDContainer::const_iterator fdi = fds.begin();
    while (fdi != fds.end()) {
      int descriptor = *fdi++;
      FD_SET(descriptor, fdset);
    }
  }
};
}  // namespace Sys
}  // namespace IRAD
