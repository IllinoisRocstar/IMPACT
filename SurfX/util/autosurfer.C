//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#include <iostream>
#include <cstring>
#include <cstdlib>

#include "com.h"
#include "../test/IM_Reader.h" // surfutil

COM_EXTERN_MODULE( SurfX);
COM_EXTERN_MODULE( SimOUT);
COM_EXTERN_MODULE( SimIN);
COM_EXTERN_MODULE( Simpal);
COM_EXTERN_MODULE( SurfUtil);

using namespace std;

void read_file( const char *fname, const string &wname, double alpha) {
  char *lastdot=strrchr( const_cast<char *>(fname), '.');
  //  const char *lastdot=strrchr( fname, '.');

  COM_new_window( wname.c_str());
  // Read in HDF files or a Rocin control file

  // Read in HDF format
  COM_LOAD_MODULE_STATIC_DYNAMIC( SimIN, "IN");
    
  int IN_read;
  // Read in HDF format using Rocin::read_window or ::read_by_control_file 
  if ( strcmp( lastdot, ".hdf")==0)
    IN_read = COM_get_function_handle( "IN.read_window");
  else
    IN_read = COM_get_function_handle( "IN.read_by_control_file");

  // Pass MPI_COMM_NULL to Rocin so that the rank becomes a wildcard.
  MPI_Comm comm_null = MPI_COMM_NULL;
  std::string bufwin("bufwin");
  std::cout << "Reading file " << fname << "..." << std::endl;
  COM_call_function( IN_read, fname, bufwin.c_str(), &comm_null);
  std::cout << "Done reading file " << fname << "." << std::endl;
  int IN_obtain = COM_get_function_handle( "IN.obtain_dataitem");


  // Remove all dataitems except for the mesh
  COM_delete_dataitem(  (bufwin+".atts").c_str());

  // Read in the mesh.
  int buf_mesh = COM_get_dataitem_handle((bufwin+".mesh").c_str());
  COM_call_function( IN_obtain, &buf_mesh, &buf_mesh);
  COM_UNLOAD_MODULE_STATIC_DYNAMIC( SimIN, "IN");
  
  std::cout << "Obtained window " << wname
	    << " from file " << fname << std::endl;

  // Change the memory layout to contiguous.
  COM_clone_dataitem( (wname+".mesh").c_str(), (bufwin+".mesh").c_str(), 0);
  COM_delete_window( bufwin.c_str());
}

int main(int argc, char *argv[]) {
  COM_init( &argc, &argv);
  
  if ( argc < 3) {
    std::cerr << "Usage: " << argv[0]
	      << " <HDF|RocinControlFile1> <HDF|RocinControlFile2> [<out_prefix>] [<RocfaceControlFile>]\n\n"
              << "\t<HDF|RocinControl File1> specifies the files for the first window\n"
              << "\t<HDF|RocinControl File2> specifies the files for the second window\n"
              << "\t<out_prefix> specifies a prefix for output files. \n\t\tDefault is the current directory\n"
	      << "\t<RocfaceControlFile> specifies a file name for Rocface control parameters. \n"
              << "\nExample:\t" 
              << argv[0] << " \"ifluid*.hdf\" \"isolid*.hdf\" " << "\n\t\t"
              << argv[0] << " Rocflo/Rocin/\"ifluid*.txt\" Rocfrac/Rocin/\"isolid*.txt\" Rocman/RocfloRocfrac/" << "\n\t\t"
              << std::endl;
    exit(-1);
  }

  COM_set_profiling( 1);

  string     fnames[2] = {string(argv[1]), string(argv[2])};
  string     pre = (argc>3)?argv[3]:"";
  // Append '/' to pre if not there
  if ( !pre.empty() && pre[pre.size()-1] != '/') pre.append("/");
  
  COM_LOAD_MODULE_STATIC_DYNAMIC( SurfX, "RFC");

  int RFC_readcntr = COM_get_function_handle( "RFC.read_control_file");
  int RFC_overlay = COM_get_function_handle( "RFC.overlay");
  int RFC_write = COM_get_function_handle( "RFC.write_overlay");

  if ( argc>4) {
    std::cout << "Reading Rocface control file..." << std::endl;
    COM_call_function( RFC_readcntr, argv[4]);
    std::cout << "Finished reading Rocface control file." << std::endl;
  }

  string     wnames[2];
  for ( int k=0; k<2; ++k) {
    // Discard the directory name and suffix to obtain a window name.
    string::size_type n0 = fnames[k].find_last_of( "/");

    std::string fname;
    if ( n0 == std::string::npos) fname=fnames[k]; 
    else fname = fnames[k].substr( n0+1, fnames[k].size());

    string::size_type ni;
    ni = fname.find_first_of( ".:_-*[]?\\\"\'0123456789");
    COM_assertion_msg(ni, "File name must start with a letter");

    if ( ni == std::string::npos) {
      wnames[k] = fname;
      fnames[k].append(".hdf"); // Append the .hdf suffix to the file name.
    }
    else {
      if ( fname[ni] == '_' && (fname[ni+1] == 's' || fname[ni+1] == 'f'))
	ni += 2;
      wnames[k] = pre+fname.substr( 0, ni);
    }
    COM_assertion_msg( k==0 || wnames[0]!=wnames[1],
		       "Two input files must have different alphabetic prefix");

    read_file( fnames[k].c_str(), wnames[k], 1.);

    COM_window_init_done( wnames[k].c_str());
  }

  int tri1_mesh = COM_get_dataitem_handle( (wnames[0]+".mesh").c_str());
  int tri2_mesh = COM_get_dataitem_handle( (wnames[1]+".mesh").c_str());

  const char *format = "HDF";
  
  std::cout << "Starting mesh overlay..." << std::endl;
  COM_call_function( RFC_overlay, &tri1_mesh, &tri2_mesh);
  COM_call_function( RFC_write, &tri1_mesh, &tri2_mesh, 
		     wnames[0].c_str(), wnames[1].c_str(), format);

  COM_print_profile( "", "");

  COM_finalize();
}


