//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#ifndef OFF_READER_H
#define OFF_READER_H

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "surfbasic.h"

COM_EXTERN_MODULE(SimIN)

// This class provides an interface for reading in a surface mesh
// from the ".off"  file. It can read files with mixed meshes with
// triangles and quadrilaterals.
class OFF_Reader {
 public:
  // Constructor from an MPI communicator and a scale factor.
  OFF_Reader() {}

  // Assuming the window has been created, read in the local panes from
  // given file and return the number of local panes.
  // Return 0 if successful.
  int read_mesh(const char *fname, const std::string &wname) {
    std::ifstream is(fname);
    if (!is.is_open()) {  // use to be : "is == NULL" -> gcc-7 error: no match
                          // operator== for std::ifstream and long int
      std::cerr << "Error: Could not open file " << fname << std::endl;
      exit(-1);
    }

    // Create the window if not yet exist
    int h = COM_get_window_handle(wname.c_str());
    if (h <= 0) COM_new_window(wname.c_str());
    char cmt[MAXLEN];

    for (int pane_id = 1;; ++pane_id) {
      int nn, ne, tmp, pid = 0;
      get_nextline(is, buf);
      // First line of the file tells that it is a .off file
      if (std::string(buf, 0, 3) != "OFF") {
        if (pane_id == 1)
          std::cerr << "Input file does not have a valid OFF header."
                    << std::endl;
        else {
          if (pane_id > 2)
            std::cout << "Read in " << pane_id - 1 << " panes." << std::endl;
          return 0;
        }
      } else
        // 2nd line of the file gives number of nodes and elements
        get_nextline(is, buf);

      std::sscanf(buf, "%d %d %d %s %d", &nn, &ne, &tmp, cmt, &pid);
      if (std::strcmp(cmt, "#!") || !pid) pid = pane_id;

      std::cout << "Reading " << nn << " nodes and " << ne
                << " faces for patch " << pid << std::endl;
      read_pane_coors(is, wname, pid, nn);
      read_pane_elems(is, wname, pid, ne);
    }

    return 0;
  }

 private:
  // Read in coordinates of a pane into a window if local
  void read_pane_coors(std::istream &is, const std::string &wname, int pane_id,
                       int nn) {
    COM_set_size((wname + ".nc").c_str(), pane_id, nn);
    if (!nn) return;

    SURF::Point_3<double> *ps;
    COM_allocate_array((wname + ".nc").c_str(), pane_id, &(void *&)ps);

    for (int i = 0; i < nn; ++i) {
      get_nextline(is, buf);
      std::sscanf(buf, "%lf %lf %lf", &ps[i][0], &ps[i][1], &ps[i][2]);
    }
  }

  // Read in element connectivity of a pane into a window if local
  void read_pane_elems(std::istream &is, const std::string &wname, int pane_id,
                       int ne) {
    if (ne == 0) return;

    // Note: Supports triangular and quadrilateral meshes now.
    int egroups = 0, num_ele = 0;
    std::string str;

    int numEdges1 = 0, numEdges2 = 0, vs[9];
    get_nextline(is, buf);
    std::istringstream sin(buf);
    sin >> numEdges1;
    for (int i = 0; i < numEdges1; ++i) sin >> vs[i];
    numEdges2 = numEdges1;

    while (!is.eof() && buf[0] && num_ele < ne) {
      std::vector<int> conns[9];

      do {  // read a group
        // read  through the file for connectivity
        for (int i = 0; i < numEdges2; ++i) conns[i].push_back(vs[i]);
        num_ele++;

        if (num_ele == ne) break;
        get_nextline(is, buf);
        numEdges2 = 0;
        std::istringstream sin(buf);
        sin >> numEdges2;
        for (int i = 0; i < numEdges2; ++i) sin >> vs[i];
      } while (numEdges1 == numEdges2);

      char ext1[10];
      sprintf(ext1, "%d", ++egroups);
      switch (numEdges1) {
        case 3:
          str = wname + ".:t3:" + ext1;
          break;
        case 6:
          str = wname + ".:t6:" + ext1;
          break;
        case 4:
          str = wname + ".:q4:" + ext1;
          break;
        case 8:
          str = wname + ".:q8:" + ext1;
          break;
        case 9:
          str = wname + ".:q9:" + ext1;
          break;
        default:;
      }

      COM_set_size(str.c_str(), pane_id, conns[0].size());
      int *es;
      COM_allocate_array(str.c_str(), pane_id, &(void *&)es);

      for (int j = 0, nconn = conns[0].size(); j < nconn; j++) {
        for (int i = 0; i < numEdges1; ++i) *(es++) = conns[i][j] + 1;
      }

      numEdges1 = numEdges2;
    }

    if (num_ele < ne) {
      std::cerr << "Incomplete element connectivity table. File corrupted?"
                << std::endl;
      exit(-1);
    }
  }

  // Read in the next non-blank line from the file
  void get_nextline(std::istream &is, char *str) {
    str[0] = '\0';
    if (is.eof()) {
      return;
    }
    is.getline(str, MAXLEN);

    while ((emptyLine(str) || isComment(str)) && (!is.eof())) {
      is.getline(str, MAXLEN);
    }
  }

  /* Checks whether the line pointed to by the file handle is empty or not.
   *  return 1 : if line is empty
   *  return 0 : if line is not empty */
  int emptyLine(char *str) {
    char s;
    int len = strlen(str);

    for (int i = 0; i < len; i++) {
      s = str[i];

      if (s != ' ' && s != '\t' && s != '\r') return 0;
    }

    return 1;
  }

  /* checks whether a line is a comment or not.
   * A line is a comment if it begins with '#'.
   * returns 1 : if line is a comment
   * returns 0 : if line is not a comment */
  int isComment(char *str) {
    int i = 1;
    char s = str[0];
    while (s == ' ' || s == '\t') s = str[i++];

    return s == '#';
  }

 private:
  enum { MAXLEN = 1023 };
  char buf[MAXLEN + 1];
};

#endif
