//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef MSH_READER_H
#define MSH_READER_H

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "surfbasic.h"

COM_EXTERN_MODULE(SimIN)

// This class provides an interface for reading in a surface mesh
// from the ".smf" or ".obj" file.
class MSH_Reader {
 public:
  // Constructor from an MPI communicator and a scale factor.
  MSH_Reader() {}

  // Assuming the window has been created, read in the local panes from
  // given file and return the number of local panes.
  int read_mesh(const char *fname, const std::string &wname) {
    std::ifstream is(fname);
    if (!is.is_open()) {
      std::cerr << "Error: Could not open file " << fname << std::endl;
      exit(-1);
    }

    // Create the window if not yet exist
    int h = COM_get_window_handle(wname.c_str());
    if (h <= 0) COM_new_window(wname.c_str());

    std::cout << "Reading file " << fname << std::endl;

    read_pane_coors(is, wname);
    read_pane_elems(is, wname);

    return 1;
  }

 private:
  void get_nextline(std::istream &is, char *str) {
    str[0] = '\0';
    if (is.eof()) {
      return;
    }
    is.getline(str, MAXLEN);
  }

  // Read in coordinates of a pane into a window if local
  void read_pane_coors(std::istream &is, const std::string &wname) {
    std::vector<SURF::Point_3<double> > coors;

    // First, look for number of vertices
    for (;;) {
      get_nextline(is, buf);
      std::string s(buf);

      if (s.find("Vertices", 0) != std::string::npos) break;
    }

    int n = 0;
    is >> n;
    get_nextline(is, buf);
    coors.reserve(n);

    for (int i = 0; i < n; ++i) {
      get_nextline(is, buf);

      SURF::Point_3<double> p;

      std::sscanf(buf, "%lf %lf %lf", &p[0], &p[1], &p[2]);
      coors.push_back(p);
    }

    COM_assertion_msg(n > 0, "Did not find any nodes");
    std::cout << "Read in " << n << " nodes" << std::endl;

    COM_set_size((wname + ".nc").c_str(), 1, n);
    SURF::Point_3<double> *p;
    COM_allocate_array((wname + ".nc").c_str(), 1, &(void *&)p);

    std::copy(coors.begin(), coors.end(), p);
  }

  // Read in element connectivity of a pane into a window if local
  void read_pane_elems(std::istream &is, const std::string &wname) {
    std::vector<SURF::Vector_3<int> > elems;

    // First, look for number of triangles
    for (;;) {
      get_nextline(is, buf);
      std::string s(buf);

      if (s.find("Triangles", 0) != std::string::npos) break;
    }

    int n = 0;
    is >> n;
    get_nextline(is, buf);
    elems.reserve(n);

    // When entering this routine, buf[0] should start with 'f'
    for (int i = 0; i < n; ++i) {
      get_nextline(is, buf);

      SURF::Vector_3<int> p;
      std::sscanf(buf, "%d %d %d", &p[0], &p[1], &p[2]);
      elems.push_back(p);
    }

    COM_assertion_msg(n > 0, "Did not find any faces");
    std::cout << "Read in " << n << " triangles" << std::endl;

    COM_set_size((wname + ".:t3:").c_str(), 1, n);
    SURF::Vector_3<int> *p;
    COM_allocate_array((wname + ".:t3:").c_str(), 1, &(void *&)p);

    std::copy(elems.begin(), elems.end(), p);
  }

 private:
  enum { MAXLEN = 255 };
  char buf[MAXLEN + 1];
};

#endif
