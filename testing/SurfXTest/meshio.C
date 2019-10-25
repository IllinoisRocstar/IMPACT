//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Read in an unstructed triangular mesh in obj format
int read_obj(istream &is, vector<double> &coors, vector<int> &elems) {
  const int MAXLEN = 255;
  char buf[MAXLEN];
  int face_type = 3;

  do {
    is.getline(buf, MAXLEN);
  } while (buf[0] == '#' && !is.eof());
  is.getline(buf, MAXLEN);
  if (strncmp(buf, "#OBJ", 4) == 0 && !is.eof()) {
    face_type = buf[4] - '0';
    assert(face_type == 3 || face_type == 4 || face_type == 6);
    do {
      is.getline(buf, MAXLEN);
    } while (buf[0] == '#' && !is.eof());
  }
  int np = 0, nf = 0;
  if (strcmp(buf, "off") == 0 || strcmp(buf, "OFF") == 0) do {
      is.getline(buf, MAXLEN);
    } while (buf[0] == '#' && !is.eof());
  sscanf(buf, "%d %d", &np, &nf);

  // Allocate space for points and faces.
  coors.reserve(np);
  elems.reserve(nf);

  // Read in the coordinates
  for (int i = 0; i < np; ++i) {
    do {
      is.getline(buf, MAXLEN);
    } while (buf[0] == '#' && !is.eof());  // skip over comments
    double p0, p1, p2;
    char type[2];
    sscanf(buf, "%s %lf %lf %lf", type, &p0, &p1, &p2);
    // std::cout << i << endl;
    coors.push_back(p0);
    coors.push_back(p1);
    coors.push_back(p2);
  }

  // skip over vertex normals
  do {
    is.getline(buf, MAXLEN);
  } while ((buf[0] == '#' || (buf[0] == 'v' && buf[1] == 'n')) && !is.eof());

  // Read in faces
  vector<int> f(face_type);
  char type;
  while (is.peek() == '#' && !is.eof()) is.getline(buf, MAXLEN);
  for (int i = 0; i < nf; ++i) {
    // is.getline(buf, MAXLEN);
    is >> type >> f[0] >> f[1] >>
        f[2];  // should read in the leading f on each line
    for (int j = 0; j < face_type; ++j) {
      assert(f[j] >= 1 && f[j] <= np);
      elems.push_back(f[j]);
    }
  }
  /*  for ( int i=0; i<nf; ++i) {
    do { is.getline( buf, MAXLEN); } while ( buf[0]=='#' && !is.eof()); //skip
  over comments int p0, p1, p2, NI0, NI1, NI2; // NI stands for normal Index,
  which in this case is equal to p... char throwAway0[2], throwAway1[2],
  throwAway2[2], type[2]; sscanf( buf, "%s %d%s%d %d%s%d %d%s%d", type, &p0,
  throwAway0, &NI0, &p1, throwAway1, &NI1, &p2, throwAway2, &NI2);
    coors.push_back( p0); coors.push_back( p1); coors.push_back( p2);
  } */

  return face_type;
}

// Read in a nstructed mesh
void read_ij(istream &is, vector<double> &coors, int dims[2]) {
  const int MAXLEN = 255;
  char buf[MAXLEN];

  do {
    is.getline(buf, MAXLEN);
  } while (buf[0] == '#' && !is.eof());
  sscanf(buf, "%d %d", &dims[0], &dims[1]);

  int np = dims[0] * dims[1];
  // Allocate space for points and faces.

  // Read in the coordinates
  for (int i = 0; i < np; ++i) {
    do {
      is.getline(buf, MAXLEN);
    } while (buf[0] == '#' && !is.eof());
    double p0, p1, p2;
    char type[2];  // must read in string at beginning of line
    sscanf(buf, "%s %lf %lf %lf", type, &p0, &p1, &p2);
    coors.push_back(p0);
    coors.push_back(p1);
    coors.push_back(p2);
  }
}

// Read in an unstructed triangular mesh in obj format
void write_ij(ostream &os, const vector<double> &coors, int dims[]) {
  os << dims[0] << " " << dims[1] << endl;

  char buf[100];
  // Write out the coordinates
  for (unsigned int i = 0; i < coors.size(); i += 3) {
    sprintf(buf, "%.10E\t%.10E\t%.10E", coors[i], coors[i + 1], coors[i + 2]);
    os << buf << endl;
  }
}
