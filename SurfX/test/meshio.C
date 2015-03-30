//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cmath>
#include <cassert>

using namespace std;

// Read in an unstructed triangular mesh in obj format
int read_obj( istream &is, vector<double> &coors,
	      vector<int> &elems) {
  const int MAXLEN=255;
  char  buf[MAXLEN];
  int   face_type = 3;

  is.getline( buf, MAXLEN);
  if ( strncmp( buf, "#OBJ", 4)==0 && !is.eof()) { 
    face_type = buf[4] - '0';
    assert( face_type == 3 || face_type == 4 || face_type == 6);
    do { is.getline( buf, MAXLEN); } while ( buf[0]=='#' && !is.eof());
  }
  int np=0, nf=0;
  if ( strcmp( buf, "off")==0 || strcmp( buf, "OFF")==0 )
    do { is.getline( buf, MAXLEN); } while ( buf[0]=='#' && !is.eof());
  sscanf( buf, "%d %d", &np, &nf);

  // Allocate space for points and faces.
  coors.reserve( np*3); elems.reserve( nf*face_type); 

  // Read in the coordinates
  for ( int i=0; i<np; ++i) {
    do { is.getline( buf, MAXLEN); } while ( buf[0]=='#' && !is.eof());
    double p0, p1, p2;
    sscanf( buf, "%lf %lf %lf", &p0, &p1, &p2);
    coors.push_back( p0); coors.push_back( p1); coors.push_back( p2);
  }

  // Read in faces
  vector<int> f( face_type);
  while( is.peek() == '#' && !is.eof()) is.getline( buf, MAXLEN);
  for ( int i=0; i<nf; ++i) {
    for ( int j=0; j<face_type; ++j) {
      is >> f[j];  assert( f[j] >= 1 && f[j] <= np);
      elems.push_back( f[j]); 
    }
  }

  return face_type;
}

// Read in a nstructed mesh
void read_ij( istream &is, vector<double> &coors, int dims[2]) {
  const int MAXLEN=255;
  char buf[MAXLEN];

  do { is.getline( buf, MAXLEN); } while ( buf[0]=='#' && !is.eof());
  sscanf( buf, "%d %d", &dims[0], &dims[1]);

  int np=dims[0]*dims[1];
  // Allocate space for points and faces.

  // Read in the coordinates
  for ( int i=0; i<np; ++i) {
    do { is.getline( buf, MAXLEN); } while ( buf[0]=='#' && !is.eof());
    double p0, p1, p2;
    sscanf( buf, "%lf %lf %lf", &p0, &p1, &p2);
    coors.push_back( p0); coors.push_back( p1); coors.push_back( p2);
  }
}

// Read in an unstructed triangular mesh in obj format
void write_ij( ostream &os, const vector<double> &coors, int dims[]) {
  os << dims[0] << " " << dims[1] << endl;

  char buf[100];
  // Write out the coordinates
  for ( unsigned int i=0; i<coors.size(); i+=3) {
    sprintf( buf, "%.10E\t%.10E\t%.10E", coors[i],
	     coors[i+1], coors[i+2]);
    os << buf << endl;
  }
}


