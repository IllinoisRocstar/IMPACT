///
///
///
/// @file
/// @ingroup impact_group
/// @brief A utility program for listing information about a COM window object
/// @author Masoud Safdari (msafdari@illinoisrocstar.com)
/// @date  May 20, 2015
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
//#include "COMM.H" // contains IRAD utilities for MPI
//#include "Pane_communicator.h" // IMPACT's inter-process tools

COM_EXTERN_MODULE(SimOut)

void usage(char *exec) {
  std::cout << std::endl << " Usage: " << std::endl;
  std::cout << exec << " [--topo] [--mesh] window_file_name" << std::endl
            << "\tWhere flags are :\n"
            << std::endl
            << "--help " << std::endl
            << "\tprints these instructions\n"
            << std::endl
            << "--topo " << std::endl
            << "\tprints topological data of the window \n"
            << std::endl
            << "--mesh " << std::endl
            << "\tprints mesh data\n"
            << std::endl
            << "--item " << std::endl
            << "\tprints item names registered in window\n"
            << std::endl
            << "--specific dataitem_name " << std::endl
            << "\tlooks if specific dataitem_name is registered in window\n"
            << std::endl
            << "\tand prints out its information" << std::endl;
  // exiting
  exit(-1);
}

int main(int argc, char *argv[]) {
  const char separator = ' ';
  const int nameWidth = 12;
  const int numWidth = 12;

  // prompting usage
  if (argc < 2) usage(argv[0]);

  // procssing flags for the operation modes
  // default mode of operation, list all window members
  bool winTopoData = false;
  bool meshData = false;
  bool itmNameData = false;
  bool spcData = false;
  std::string arg, spcDataItmName;
  std::stringstream ss;
  for (int iArg = 1; iArg < argc; iArg++) {
    ss.clear();
    ss.str("");
    ss << argv[iArg];
    if (ss.str() == "--topo") {
      winTopoData = true;
    } else if (ss.str() == "--mesh") {
      meshData = true;
    } else if (ss.str() == "--item") {
      itmNameData = true;
    } else if (ss.str() == "--specific") {
      spcData = true;
      // next argument should be the name
      // of the specific dataitem that will
      // be queryed
      spcDataItmName = argv[iArg + 1];
      iArg++;
    } else if (ss.str() == "--help") {
      usage(argv[0]);
    }
  }

  // initializing COM
  COM_init(&argc, &argv);

  // loading SimIN
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN");

  // reading window preprocessing
  std::cout << "Reading window file \"" << argv[argc - 1] << '"' << std::endl;
  std::string fname(argv[argc - 1]), wname;
  std::string::size_type n0 = fname.find_last_of("/");
  if (n0 != std::string::npos) fname = fname.substr(n0 + 1, fname.size());
  std::string::size_type ni;
  ni = fname.find_first_of(".:-*[]?\\\"\'0123456789");
  COM_assertion_msg(ni, "File name must start with a letter");
  if (ni == std::string::npos) {
    wname = fname;
  } else {
    while (fname[ni - 1] == '_') --ni;  // Remove the '_' at the end.
    wname = fname.substr(0, ni);
  }
  std::cout << "Loading window \"" << wname << '"' << std::endl;

  // selecting proper reader based on the file extension
  const char *lastdot = std::strrchr(argv[argc - 1], '.');
  int IN_read;
  if (lastdot && std::strcmp(lastdot, ".txt") == 0)
    IN_read = COM_get_function_handle("IN.read_by_control_file");
  else
    IN_read = COM_get_function_handle("IN.read_window");

  // reading window from the file
  COM_call_function(IN_read, argv[argc - 1], wname.c_str());

  // pane data
  int nPane;
  int *paneList;
  COM_get_panes(wname.c_str(), &nPane, &paneList);
  COM_assertion(nPane >= 0);

  // obtaining topographic data
  if (winTopoData) {
    std::cout << "I see " << nPane << " pane(s) registered for window --> "
              << wname << std::endl;
    std::cout << "\t # \t ID# \t #Nde \n";
    std::cout << "\t---\t-----\t------\n";
    for (int iPane = 0; iPane < nPane; ++iPane) {
      // geting number of the nodes for pane
      int nNde = 0;
      COM_get_size((wname + ".nc").c_str(), paneList[iPane], &nNde);
      std::cout << "\t " << iPane + 1 << "\t " << paneList[iPane] << "\t  "
                << nNde << std::endl;
    }
    // working on panes
    std::cout << "Checking connectivity table(s) " << std::endl;
    for (int iPane = 0; iPane < nPane; iPane++) {
      int paneId = paneList[iPane];
      // get connectivity tables of the pane
      int numConn;
      std::string stringNames;
      COM_get_connectivities(wname.c_str(), paneId, &numConn, stringNames);
      std::istringstream ConnISS(stringNames);
      std::vector<std::string> connNames;

      // reporting
      std::cout << " pane --> " << paneId << std::endl;
      std::cout << "\t # \t Type \t #Elm \t #ElmNde \t Loc \n";
      std::cout << "\t---\t------\t------\t---------\t-----\n";
      for (int i = 0; i < numConn; ++i) {
        std::string name;
        ConnISS >> name;
        connNames.push_back(name);
        std::string fullConnName(wname + "." + name);
        int nElm;
        COM_get_size(fullConnName, paneId, &nElm);
        int nElmNde;
        std::string dataItemUnits;
        char dataItemLoc;
        COM_Type dataItemType;
        COM_get_dataitem(fullConnName, &dataItemLoc, &dataItemType, &nElmNde,
                         &dataItemUnits);
        std::cout << "\t " << i + 1 << "\t " << name << "\t  " << nElm
                  << "\t    " << nElmNde << "\t          " << dataItemLoc
                  << std::endl;
      }
    }
  }

  // obtaining mesh data
  if (meshData) {
    for (int iPane = 0; iPane < nPane; iPane++) {
      int paneId = paneList[iPane];
      std::cout << "\nMesh informaiton for pane --> " << paneList[iPane]
                << std::endl;
      // getting node coordinates
      double *ndeCrd;
      COM_get_array((wname + ".nc").c_str(), paneId, &ndeCrd);
      // check for expected number of nodes
      int nNde = 0;
      COM_get_size((wname + ".nc").c_str(), paneId, &nNde);
      // reporting node coordinates
      std::cout << "\tNode Coordinates" << std::endl;
      std::cout << std::left << std::setw(nameWidth) << "#";
      std::cout << std::left << std::setw(nameWidth) << "X";
      std::cout << std::left << std::setw(nameWidth) << "Y";
      std::cout << std::left << std::setw(nameWidth) << "Z";
      std::cout << "\n------------------------------------------\n";
      for (int iNde = 0; iNde < nNde; iNde++) {
        std::cout << std::left << std::setw(numWidth) << iNde;
        std::cout << std::left << std::setw(numWidth) << ndeCrd[iNde];
        std::cout << std::left << std::setw(numWidth) << ndeCrd[iNde + nNde];
        std::cout << std::left << std::setw(numWidth)
                  << ndeCrd[iNde + 2 * nNde];
        std::cout << std::endl;
      }

      // get connectivity tables of the pane
      int nConn;
      std::string stringNames;
      COM_get_connectivities(wname.c_str(), paneId, &nConn, stringNames);
      std::istringstream ConnISS(stringNames);
      // get connectivities
      for (int i = 0; i < nConn; ++i) {
        std::string name;
        ConnISS >> name;
        std::string fullConnName(wname + "." + name);
        int *elmConn;
        int nElm;
        char dataItemLoc;
        COM_Type dataItemType;
        int nElmNde;
        std::string dataItemUnits;
        COM_get_dataitem(fullConnName, &dataItemLoc, &dataItemType, &nElmNde,
                         &dataItemUnits);
        COM_get_array(fullConnName.c_str(), paneId, &elmConn);
        COM_get_size(fullConnName, paneId, &nElm);
        std::cout << "\n\t Connectivity Table\n";
        std::cout << std::left << std::setw(nameWidth) << "#Elm";
        std::cout << std::left << std::setw(nameWidth) << "Nde IDs.....\n";
        std::cout << "------------------------------------------------------";
        for (int iElm = 0; iElm < nElm; iElm++) {
          std::cout << std::endl;
          std::cout << std::left << std::setw(numWidth) << iElm;
          for (int iElmNde = 0; iElmNde < nElmNde; iElmNde++)
            std::cout << std::left << std::setw(numWidth)
                      << elmConn[iElm * nElmNde + iElmNde];
        }
        std::cout << std::endl;
      }
    }
  }

  // obtaining item names registered in window
  if (itmNameData) {
    int nItm;
    std::string itmNames;
    COM_get_dataitems(wname.c_str(), &nItm, itmNames);
    std::cout << "Number of items registered = " << nItm << std::endl;
    std::cout << "Item names :\n";
    std::cout << itmNames << std::endl;
  }

  // accessing mesh datastructure
  if (spcData) {
    int dataItemSize;
    int dataItemNComp;
    std::string dataItemUnits;
    char dataItemLoc;
    COM_Type dataItemType;
    std::cout << "Looking for data item --> " << spcDataItmName << std::endl;
    COM_get_dataitem((wname + "." + spcDataItmName).c_str(), &dataItemLoc,
                     &dataItemType, &dataItemNComp, &dataItemUnits);
    COM_get_size((wname + "." + spcDataItmName).c_str(), paneList[0],
                 &dataItemSize);
    std::cout << "\tLocation = " << dataItemLoc << std::endl;
    std::cout << "\tType = " << dataItemType << std::endl;
    std::cout << "\tnComp = " << dataItemNComp << std::endl;
    std::cout << "\tUnits = " << dataItemUnits << std::endl;
    std::cout << "\tSize = " << dataItemSize << std::endl;
  }

  // unloading modules and finalizing
  COM_UNLOAD_MODULE_STATIC_DYNAMIC(SimIN, "IN");
  COM_finalize();
  return (0);
}
