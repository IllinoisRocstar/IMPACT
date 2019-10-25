
//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
///
/// @file
/// @ingroup impact_group
/// @brief A utility program for testing parallel transfer between COM windows
/// @author Masoud Safdari (msafdari@illinoisrocstar.com)
/// @date  June 10, 2016
///
///
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "com.h"
#include "com_devel.hpp"
//#include "primitive_utilities.H"
//#include "InterfaceLayer.H"

int myRank = 0;
int nProc = 1;

COM_EXTERN_MODULE(SimOut)
COM_EXTERN_MODULE(SurfX)

// methods for splitting a string
static std::vector<std::string> &split(const std::string &s, char delim,
                                       std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

static std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

// read IMPACT windows
static void read_file_rocin(const char *fname, const std::string &wname,
                            std::ostream &Inf, MPI_Comm comm) {
  char *lastdot = strrchr(const_cast<char *>(fname), '.');
  //  const char *lastdot=strrchr( fname, '.');
  COM_new_window(wname.c_str(), comm);

  // Read in HDF files or a Rocin control file
  Inf << "\nRank #" << myRank << " Reading file " << fname << "...";

  // Read in HDF or CGNS format
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN");
  int IN_read;
  // Read in HDF format using Rocin::read_window or ::read_by_control_file
  if (strcmp(lastdot, ".hdf") == 0 || strcmp(lastdot, ".cgns") == 0) {
    IN_read = COM_get_function_handle("IN.read_window");
  } else if (strcmp(lastdot, ".txt") == 0) {
    IN_read = COM_get_function_handle("IN.read_by_control_file");
  }

  std::string bufwin("bufwin");
  // Pass MPI_COMM_NULL to Rocin so that the rank becomes a wildcard.
  COM_call_function(IN_read, fname, bufwin.c_str(), &comm);

  int IN_obtain = COM_get_function_handle("IN.obtain_dataitem");

  // copying contents of the window
  int nItm;
  std::string itmName;
  COM_get_dataitems(bufwin.c_str(), &nItm, itmName);
  Inf << "\nRank #" << myRank << " Number of items in this window " << nItm
      << "\nCopying... ";
  std::vector<std::string> itmNameVec;
  // itmNameVec = split( itmName, ' ');
  itmNameVec.push_back("mesh");
  for (std::vector<std::string>::iterator it = itmNameVec.begin();
       it != itmNameVec.end(); ++it) {
    std::string tmpItmName = "." + *it;
    Inf << "\n\tRank #" << myRank << " " << tmpItmName;
    int buf_mesh = COM_get_dataitem_handle((bufwin + tmpItmName).c_str());
    COM_call_function(IN_obtain, &buf_mesh, &buf_mesh);
    // change the memory layout to contiguous.
    COM_clone_dataitem((wname + tmpItmName).c_str(),
                       (bufwin + tmpItmName).c_str(), 0);
  }

  COM_delete_window(bufwin.c_str());
  Inf << "\nRank #" << myRank << " Obtained window " << wname << " from file "
      << fname << std::endl;
  COM_UNLOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN");
}

static void read_file(const char *fname, const std::string &wname,
                      std::ostream &Inf, MPI_Comm comm) {
  const char *lastdot = strrchr(fname, '.');

  read_file_rocin(fname, wname, Inf, comm);
}

static void showPaneIds(int myRank, std::string winName) {
  // shows registered pane ids for a given window for
  // specific rank
  int nPane;
  int *paneList;
  COM_get_panes(winName.c_str(), &nPane, &paneList);
  std::cout << "\n ****** Rank " << myRank << " window " << winName
            << " Number of panes = " << nPane << " Pane List = ";
  for (int i = 0; i < nPane; ++i) std::cout << paneList[i] << " ";
  std::cout << std::endl;
}

void usage(char *exec, std::ostream &Inf) {
  Inf << std::endl << " Usage: " << std::endl;
  Inf << exec
      << " [--help] [--test] window_hdf_file_name1 window_hdf_file_name2"
      << " window_rocin_file_name1 window_rocin_file_name" << std::endl
      << "\tWhere flags are :\n"
      << std::endl
      << "--help " << std::endl
      << "\tprints these instructions\n"
      << std::endl
      << "--overlay " << std::endl
      << "\tcreates an overlay from input windows \n"
      << std::endl
      << "--panemap " << std::endl
      << "\tshows windows and their panes to process map \n"
      << std::endl
      << "--tansfer " << std::endl
      << "\ttransfers an example dataset between windows manually\n"
      << std::endl
      << std::endl
      << "Example : " << std::endl
      << " mpirun -np 2 " << exec
      << " --transfer \"ELMModule_window_proc_*.hdf\""
      << " \"OFModule_window_proc_*.hdf\" \"ELMModule_in_0.txt\" "
         "\"OFModule_in_0.txt\" \n"
      << std::endl;

  exit(-1);
}

// figure out window name from file name
static void findWinName(std::string fnameIn, std::string &wname) {
  std::string::size_type n0 = fnameIn.find_last_of("/");
  std::string fname;
  if (n0 == std::string::npos)
    fname = fnameIn;
  else
    fname = fnameIn.substr(n0 + 1, fnameIn.size());
  std::string::size_type ni;
  ni = fname.find_first_of("_.:-*[]?\\\"\'0123456789");
  COM_assertion_msg(ni, "File name must start with a letter");
  if (ni == std::string::npos) {
    wname = fname;
  } else {
    wname = fname.substr(0, ni);
  }
}

// console applications
int main(int argc, char *argv[]) {
  // constants
  const char separator = ' ';
  const int nameWidth = 12;
  const int numWidth = 12;
  std::ostream Inf(NULL);

  // initializing the MPI communicator
  MPI_Init(&argc, &argv);
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Comm commNull = MPI_COMM_NULL;
  MPI_Comm transComm = MPI_COMM_SELF;  // will be used for transfer
  MPI_Comm_rank(comm, &myRank);
  MPI_Comm_size(comm, &nProc);
  bool isTrans = myRank == 0;

  // defining rank which outputs to screen
  if (myRank == 0) Inf.rdbuf(std::cout.rdbuf());

  // prompting usage if needed
  if (argc < 3) usage(argv[0], Inf);

  // preprocessing flags for the operation modes
  std::string arg;
  std::stringstream ss;
  std::vector<std::string> attLst;
  bool testManOverlay = false;
  bool testManTransfer = false;
  bool testObjTransfer = false;
  bool showPaneProcMap = false;
  for (int iArg = 1; iArg < argc; iArg++) {
    ss.clear();
    ss.str("");
    ss << argv[iArg];
    if (ss.str() == "--overlay") {
      testManOverlay = true;
    } else if (ss.str() == "--panemap") {
      // compute, write and load overlay needed
      testManOverlay = true;
      showPaneProcMap = true;
    } else if (ss.str() == "--transfer") {
      // compute, write and load overlay needed
      testManOverlay = true;
      testManTransfer = true;
      attLst.push_back("solidDisp");
      attLst.push_back("fluidDisp");
    } else if (ss.str() == "--transferObj") {
      // overlay should be created for this test
      testObjTransfer = true;
      attLst.push_back("solidDisp");
      attLst.push_back("fluidDisp");
    } else if (ss.str() == "--help") {
      usage(argv[0], Inf);
    }
  }

  // initializing COM
  COM_init(&argc, &argv);
  COM_set_default_communicator(comm);

  // loading modules needed
  COM_LOAD_MODULE_STATIC_DYNAMIC(SurfX, "RFC");
  int RFC_verbosity =
      COM_get_function_handle(("RFC" + std::string(".set_verbose")).c_str());
  int RFC_overlay =
      COM_get_function_handle(("RFC" + std::string(".overlay")).c_str());
  int RFC_write =
      COM_get_function_handle(("RFC" + std::string(".write_overlay")).c_str());
  int RFC_read =
      COM_get_function_handle(("RFC" + std::string(".read_overlay")).c_str());
  int RFC_load =
      COM_get_function_handle(("RFC" + std::string(".load_transfer")).c_str());
  int RFC_clear =
      COM_get_function_handle(("RFC" + std::string(".clear_overlay")).c_str());
  int RFC_transfer = COM_get_function_handle(
      ("RFC" + std::string(".least_squares_transfer")).c_str());
  int RFC_interpolate =
      COM_get_function_handle(("RFC" + std::string(".interpolate")).c_str());

  // set profiling
  // COM_set_profiling_barrier( RFC_transfer, comm);
  // COM_set_profiling_barrier( RFC_interpolate, comm);

  // setting verbosity level
  int rfcVerb = 1;
  COM_call_function(RFC_verbosity, &rfcVerb);

  // find window names from file names
  std::string fnames[2] = {std::string(argv[argc - 2]),
                           std::string(argv[argc - 1])};
  std::string fnameHdf[2] = {std::string(argv[argc - 4]),
                             std::string(argv[argc - 3])};
  std::string wnames[2];

  // first read windows and perform a manual overlay
  if (testManOverlay) {
    if (isTrans) {
      // computing the overlay (common refinement)
      COM_set_default_communicator(commNull);
      Inf << "\nRank #" << myRank << " Computing overlay...";

      std::string tmp_wname[2];
      findWinName(fnameHdf[0], tmp_wname[0]);
      findWinName(fnameHdf[1], tmp_wname[1]);
      std::cout << "\n tmp_wnames are " << tmp_wname[0] << " " << tmp_wname[1];

      read_file(fnameHdf[0].c_str(), tmp_wname[0].c_str(), Inf, MPI_COMM_SELF);
      COM_window_init_done(tmp_wname[0]);
      read_file(fnameHdf[1].c_str(), tmp_wname[1].c_str(), Inf, MPI_COMM_SELF);
      COM_window_init_done(tmp_wname[1]);
      int solid_mesh =
          COM_get_dataitem_handle((tmp_wname[0] + ".mesh").c_str());
      int fluid_mesh =
          COM_get_dataitem_handle((tmp_wname[1] + ".mesh").c_str());
      COM_call_function(RFC_overlay, &solid_mesh, &fluid_mesh);
      Inf << "\nRank #" << myRank
          << " Computing overlay finished successfully.";

      // write the overlay
      Inf << "\nRank #" << myRank << " Writing overlay...";
      const char *format = "HDF";
      COM_call_function(RFC_write, &solid_mesh, &fluid_mesh,
                        tmp_wname[0].c_str(), tmp_wname[1].c_str(), format);
      Inf << "\nRank #" << myRank << " Writing overlay finished successfully.";

      // destroy temporary windows
      COM_delete_window(tmp_wname[0]);
      COM_delete_window(tmp_wname[1]);
      COM_set_default_communicator(comm);
    }
  }
  MPI_Barrier(comm);

  // loading windows from files
  int nPane[2];
  std::vector<std::vector<int> > paneList;
  paneList.resize(2);
  int *tmpPaneList;
  int mesh0, mesh1;
  for (int k = 0; k < 2; k++) {
    Inf << "\nRank #" << myRank << " Reading window file \"" << fnames[k]
        << '"';

    // find window name from file name
    findWinName(fnames[k], wnames[k]);

    COM_assertion_msg(
        k == 0 || wnames[0] != wnames[1],
        "Two input files must have different alphanumeric prefix");

    // loading the window
    Inf << "\nRank #" << myRank << " Loading window \"" << wnames[k] << '"';
    read_file(fnames[k].c_str(), wnames[k], Inf, comm);
    COM_window_init_done(wnames[k].c_str());

    // pane data
    COM_get_panes(wnames[k].c_str(), &nPane[k], &tmpPaneList);
    COM_assertion(nPane[k] >= 0);
    Inf << "\tPane list = ";
    for (int iPane = 0; iPane < nPane[k]; iPane++) {
      paneList[k].push_back(tmpPaneList[iPane]);
      Inf << paneList[k][iPane] << "  ";
      if (iPane % 10 == 0 && iPane > 0) Inf << "\n\t";
    }

    // check window contents
    int nItm;
    std::string itmName;
    COM_get_dataitems(wnames[k].c_str(), &nItm, itmName);
    Inf << "\n\tRank #" << myRank << " # Item = " << nItm;
    Inf << "\n\tRank #" << myRank << " Contents = " << itmName;
  }
  // make sure mesh is coppied
  mesh0 = COM_get_dataitem_handle((wnames[0] + ".mesh"));
  mesh1 = COM_get_dataitem_handle((wnames[1] + ".mesh"));
  COM_assertion(mesh0 > 0 && mesh1 > 0);

  // creating data items for testing
  std::vector<std::vector<std::vector<double> > > dataVec;
  std::vector<std::string> spcDataItmName;
  spcDataItmName.push_back("nc");
  dataVec.resize(2);
  int dataItemSize;
  int dataItemNComp = 3;
  std::string dataItemUnits;
  COM_Type dataItemType;
  char dataItemLoc;
  int stride = 0;
  int cap = 0;
  double *dataAry = NULL;
  if (testManTransfer || testObjTransfer) {
    Inf << "\nRank #" << myRank
        << " Registering data on the windows for testing.";

    // registering new dataitem
    for (int iWin = 0; iWin < 2; iWin++) {
      dataVec[iWin].resize(nPane[iWin]);
      COM_new_dataitem((wnames[iWin] + "." + attLst[iWin]).c_str(), 'n',
                       COM_DOUBLE, 3, "m/s");
      for (int iPane = 0; iPane < nPane[iWin]; iPane++) {
        // using COM_resize_array: sets to zero
        COM_resize_array((wnames[iWin] + "." + attLst[iWin]).c_str(),
                         paneList[iWin][iPane]);
        // setting values
        COM_get_size((wnames[iWin] + "." + attLst[iWin]).c_str(),
                     paneList[iWin][iPane], &dataItemSize);
        dataVec[iWin][iPane].resize(3 * dataItemSize, 1 - iWin);
        COM_set_array((wnames[iWin] + "." + attLst[iWin]).c_str(),
                      paneList[iWin][iPane], &dataVec[iWin][iPane][0]);

        // accessing values to  make sure registered properly
        COM_get_array((wnames[iWin] + "." + attLst[iWin]).c_str(),
                      paneList[iWin][iPane], &dataAry, &stride, &cap);
        COM_get_size((wnames[iWin] + "." + attLst[iWin]).c_str(),
                     paneList[iWin][iPane], &dataItemSize);
        int isize = cap * stride;
        Inf << "\n\tRank #" << myRank << " size = " << dataItemSize;
        Inf << "\n\tRank #" << myRank << " cap = " << cap;
        Inf << "\n\tRank #" << myRank << " stride = " << stride;
        Inf << "\n\tRank #" << myRank << " dataItem[0] = " << dataAry[0];
      }
      COM_window_init_done(wnames[iWin].c_str());
    }

    // make sure dataitems are registered
    int data0 = COM_get_dataitem_handle(wnames[0] + "." + attLst[0].c_str());
    int data1 = COM_get_dataitem_handle(wnames[1] + "." + attLst[1].c_str());
    COM_assertion(data0 > 0 && data1 > 0);
  }

  // read the overlay
  if (testManTransfer || testObjTransfer) {
    Inf << "\nRank #" << myRank << " Loading the overlay...";

    mesh0 = COM_get_dataitem_handle((wnames[0] + ".mesh"));
    mesh1 = COM_get_dataitem_handle((wnames[1] + ".mesh"));

    COM_call_function(RFC_read, &mesh0, &mesh1, &comm, wnames[0].c_str(),
                      wnames[1].c_str(), "HDF");

    Inf << "\nRank #" << myRank
        << " Loading the overlay finished successfully.";
  }
  MPI_Barrier(comm);

  // manual attribute transfer
  if (showPaneProcMap) {
    showPaneIds(myRank, wnames[0].c_str());
    showPaneIds(myRank, wnames[1].c_str());
  }
  MPI_Barrier(comm);

  if (testManTransfer) {
    Inf << "\nRank #" << myRank
        << " Attempting to transfer an attribute manually...";

    std::string srcatt = wnames[0] + "." + attLst[0];
    std::string trgatt = wnames[1] + "." + attLst[1];

    Inf << "\tRank #" << myRank << "  " << srcatt << " --> " << trgatt
        << std::endl;

    int src_handle = COM_get_dataitem_handle(srcatt.c_str());
    int trg_handle = COM_get_dataitem_handle(trgatt.c_str());
    COM_assertion(src_handle > 0 && trg_handle > 0);
    COM_call_function(RFC_transfer, &src_handle, &trg_handle, &comm);
    // COM_call_function( RFC_interpolate, &src_handle, &trg_handle);

    Inf << "\nRank #" << myRank << " Transfer finished successfully...";
  }

  /*
  // create overlay using transfer Object
  SolverUtils::TransferObject* transObj;
  if (testObjOverlay) {
     if (isTrans) {
        COM_set_default_communicator(transComm);
        Inf << "\nRank #" << myRank << " Performing the overlay using transfer
  object\n"; transObj = new SolverUtils::TransferObject("trans"); int opRes =
  transObj->Overlay( wnames[0].c_str(), wnames[1].c_str()); if (opRes) Inf <<
  "\nRank #" << myRank
                     << " Error in the creation of overlay " << std::endl;
        else
           Inf << "\nRank #"
                     << myRank << " Successfully created the overlay";
        //setting verbosity
        //transObj->SetVerbosity(5);
        COM_set_default_communicator(comm);
    }
  }

  // transfer attribute by tansfer Object
  if (testObjTransfer) {
     Inf << "\nRank #" << myRank << " Transfering : ";
     Inf << "\n\t" << wnames[0] << "." << attLst[0]
         << " --> " << wnames[1] << "." << attLst[1];
     int opRes = transObj->Transfer( attLst[0].c_str(), attLst[1].c_str());
     if (opRes)
        Inf << "\nRank #"
            << myRank << " Error in transfering the attributes " << std::endl;
     else
        Inf << "\nRank #"
            << myRank << " Successfully transfered the attributes";
  }
  */

  // unloading modules and finalizing
  Inf << "\n\nRank #" << myRank << " Unloading modules and finishing ...\n";
  COM_UNLOAD_MODULE_STATIC_DYNAMIC(SurfX, "RFC");
  COM_finalize();
  MPI_Finalize();
  return (0);
}
