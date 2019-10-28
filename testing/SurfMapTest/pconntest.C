//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//
#include <iostream>
#include "com.h"
#include "mapbasic.h"
//#include <sstream>
#include "gtest/gtest.h"

COM_EXTERN_MODULE(SimIN)
COM_EXTERN_MODULE(SimOUT)
COM_EXTERN_MODULE(SurfMap)

using namespace std;

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

// This test doesn't test SurfMap, it simply tosses numbers around in different
// vectors
/* static int abs_val( int a ) {
        if (a < 0) {return (-a);}
        else {return a;}
}

static void add_ghost_nodes2D( int *pconn , int *new_pconn) {
        int ni = 8;
        int nj = 7;
        int nk = 5;
        int ghost_layers = 2;
        int num_com_panes = pconn[0];

        int indx = 0;

        indx++;

        std::vector< std::vector<int> > real2send(num_com_panes);
        std::vector< std::vector<int> > ghost2recv(num_com_panes);
        std::vector< std::vector<int> > shared_nodes(num_com_panes);
        std::vector< int > pane_info(num_com_panes);

        std::vector< std::vector<int> >::iterator s = shared_nodes.begin();
        std::vector< std::vector<int> >::iterator s_end = shared_nodes.end();

        for (int q=0; q< num_com_panes; q++){
                int id_num = pconn[indx];
                pane_info[q] = id_num;
                indx++;
                int count = pconn[indx];
                indx++;
                int val = indx + count;
                for ( ; indx < val; indx++){
                    (shared_nodes[q]).push_back(pconn[indx]);
                }
        }

        std::vector< std::vector<int> >::iterator p2 = real2send.begin();
        std::vector< std::vector<int> >::iterator g2 = ghost2recv.begin();
        std::vector< std::vector<int> >::iterator s2 = shared_nodes.begin();

        for ( ; s2 != s_end; p2++,g2++,s2++){
                int node1 = (*s2)[0];
                int node2 = (*s2)[1];
                int diff = abs_val(node1-node2);
                if ( diff == 1) {
                  if ( (node1 - ni*ghost_layers) < ni ) { //top
                        //newnode = node1+ni
                        int size = (*s2).size();
                        for (int k=0; k < size; k++){
                          for (int m=0; m< ghost_layers; m++){
                            (*p2).push_back( (*s2)[k] + (m+1)*ni);
                            (*g2).push_back( (*s2)[k] - (m+1)*ni);
                          }
                        }
                  } else if ( ni*(nj-ghost_layers) - node1 < ni){  //bottom
                        //newnode = node1-ni
                        int size = (*s2).size();
                        for (int k=0; k < size; k++){
                          for (int m=0; m< ghost_layers; m++){
                            (*p2).push_back( (*s2)[k] - (m+1)*ni);
                            (*g2).push_back( (*s2)[k] + (m+1)*ni);
                          }
                        }
                } else {
                          //error state
                        COM_assertion_msg(false, "Should be top or bottom.");
                }

                } else if ( diff == ni ) {
                  if ( (node1 % ni) == (ni - ghost_layers)){ //right
                        //newnode = node1-1
                        int size = (*s2).size();
                        for (int k=0; k < size; k++){
                          for (int m=0; m< ghost_layers; m++){
                            (*p2).push_back( (*s2)[k] - (m+1));
                            (*g2).push_back( (*s2)[k] + (m+1));
                          }
                        }

                  } else if ( (node1 % ni) == (ghost_layers+1) ) { //left
                        //newnode = node1+1
                        int size = (*s2).size();
                        for (int k=0; k < size; k++){
                          for (int m=0; m< ghost_layers; m++){
                            (*p2).push_back( (*s2)[k] + (m+1));
                            (*g2).push_back( (*s2)[k] - (m+1));
                          }
                        }

                  } else {
                        //error state
                        COM_assertion_msg(false, "Should be right or left.");
                  }
                  }

                }

        int size = indx;

        //block 1
        for (int j=0; j<size; j++){
            new_pconn[j] = pconn[j];
        }

        //block 2
        new_pconn[indx] = num_com_panes;
        indx++;
        for (int k=0; k< num_com_panes; k++){
            new_pconn[indx] = pane_info[k]; //comm. pane ID
            indx++;
            new_pconn[indx] = real2send[k].size(); //num values to follow
            indx++;
            for (int m=0; m< real2send[k].size(); m++){
                    new_pconn[indx] = real2send[k][m];
                    indx++;
            }
        }

        //block 3
        new_pconn[indx] = num_com_panes;
        indx++;
        for (int k=0; k< num_com_panes; k++){
            new_pconn[indx] = pane_info[k]; //comm. pane ID
            indx++;
            new_pconn[indx] = ghost2recv[k].size(); //num values to follow
            indx++;
            for (int m=0; m< ghost2recv[k].size(); m++){
                    new_pconn[indx] = ghost2recv[k][m];
                    indx++;
            }
        }
        //block 3
        return;
}

TEST(PConnTest, ConstructedData) {

  int size = 11;

  int myPconn[size];
  myPconn[0] = 2;
  myPconn[1] = 2;
  myPconn[2] = 3;
  myPconn[3] = 36;
  myPconn[4] = 37;
  myPconn[5] = 38;
  myPconn[6] = 3;
  myPconn[7] = 3;
  myPconn[8] = 19;
  myPconn[9] = 27;
  myPconn[10] = 35;

  //int *pconn;
  int new_pconn[45];

  add_ghost_nodes2D( myPconn, new_pconn);
  for (int v=0; v < 45; v++){
          std::cerr << new_pconn[v] << std::endl;
  }

  //COM_finalize();
} */

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}

TEST(PConnTest, FromFile) {
  COM_init(&ARGC, &ARGV);

  /* if ( argc < 3) {
    std::cout << "Usage: " << argv[0]
              << " <in_hdf_file> <out_hdf_file> " << endl;
    exit(-1);
  } */

  std::cout << "Reading mesh file \"" << ARGV[1] << '"' << endl;

  std::string fname(ARGV[1]), wname;
  string::size_type n0 = fname.find_last_of("/");

  if (n0 != std::string::npos) fname = fname.substr(n0 + 1, fname.size());

  string::size_type ni;
  ni = fname.find_first_of(".:-*[]?\\\"\'0123456789");
  ASSERT_NE(0, ni) << "File name must start with a letter!\n";
  COM_assertion_msg(ni, "File name must start with a letter");

  if (ni == std::string::npos) {
    wname = fname;
  } else {
    while (fname[ni - 1] == '_') --ni;  // Remove the '_' at the end.
    wname = fname.substr(0, ni);
  }

  cout << "Creating window \"" << wname << '"' << endl;

  // Read in CGNS format
  EXPECT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN"));
  cout << "Reading window " << endl;
  int IN_read = COM_get_function_handle("IN.read_window");
  ASSERT_NE(-1, IN_read) << "Function IN.read_window not found!\n";
  ASSERT_NO_THROW(COM_call_function(IN_read, ARGV[1], wname.c_str()))
      << "Unable to call IN.read_window";

  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");
  ASSERT_NE(-1, IN_obtain) << "Function IN.obtain_dataitem not found!\n";
  int mesh_hdl = COM_get_dataitem_handle((wname + ".mesh").c_str());
  ASSERT_NE(-1, mesh_hdl) << "Dataitem " << wname << ".mesh was not found!\n";
  cout << "Obtaining the mesh " << endl;
  EXPECT_NO_THROW(COM_call_function(IN_obtain, &mesh_hdl, &mesh_hdl));

  // Change the memory layout to contiguous.
  cout << "Resizing the array " << endl;
  EXPECT_NO_THROW(COM_resize_array((wname + ".mesh").c_str(), 0, NULL, 0));
  // Delete all dataitems and leave alone the mesh.
  cout << "deleting the dataitem" << endl;
  EXPECT_NO_THROW(COM_delete_dataitem((wname + ".data").c_str()));

  int npanes;
  COM_get_panes(wname.c_str(), &npanes, NULL);
  EXPECT_NE(0, npanes) << "There are 0 panes! Either the input file"
                       << " is incorrect or it is not being read properly!\n";

  EXPECT_NO_THROW(COM_assertion(npanes >= 0));

  cout << "finishing up window initialization" << endl;
  EXPECT_NO_THROW(COM_window_init_done(wname.c_str()));

  cout << "loading Rocout" << endl;
  EXPECT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT"));

  cout << "Computing connectivity map... " << endl;

  EXPECT_NO_THROW(COM_LOAD_MODULE_STATIC_DYNAMIC(SurfMap, "MAP"));

  // Invoke compute_pconn to obtain pane connectivity.
  int MAP_compute_pconn = COM_get_function_handle("MAP.compute_pconn");
  ASSERT_NE(-1, MAP_compute_pconn)
      << "Function MAP.compute_pconn was not found!\n";
  const string pconn = wname + ".pconn";
  int pconn_hdl = COM_get_dataitem_handle(pconn.c_str());
  ASSERT_NE(-1, pconn_hdl) << "Dataitem " << pconn << " was not found!\n";
  EXPECT_NO_THROW(COM_call_function(MAP_compute_pconn, &mesh_hdl, &pconn_hdl));

  cout << "Output window into file..." << endl;

  // Output pconn
  int OUT_set = COM_get_function_handle("OUT.set_option");
  ASSERT_NE(-1, OUT_set) << "Function OUT.set_option was not found!\n";
  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  ASSERT_NE(-1, OUT_write) << "Function OUT.write_dataitem was not found!\n";

  EXPECT_NO_THROW(COM_call_function(OUT_set, "mode", "w"));
  int all_hdl = COM_get_dataitem_handle((wname + ".all").c_str());
  ASSERT_NE(-1, all_hdl) << "Dataitem " << wname << ".all was not found!\n";
  EXPECT_NO_THROW(COM_call_function(OUT_write, ARGV[2], &all_hdl,
                                    (char*)wname.c_str(), "000"));

  COM_finalize();
}
