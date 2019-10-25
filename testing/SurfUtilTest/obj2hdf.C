//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

// Converting a .smf file to .hdf.

#include <cstring>
#include <iostream>
#include <sstream>
#include "OBJ_Reader.h"
#include "com.h"

using namespace std;

COM_EXTERN_MODULE(SimOUT);

int main(int argc, char *argv[]) {
  COM_init(&argc, &argv);

  if (argc < 2) {
    cout << "Usage: " << argv[0] << " <obj_file> [hdf_name] " << endl;
    return -1;
  }

  char *firstdot = strchr(argv[1], '.');
  const string wname(argv[1], firstdot ? firstdot - argv[1] : strlen(argv[1]));

  std::cout << "Reading surface mesh file \"" << argv[1] << '"' << endl;

  COM_new_window(wname.c_str());
  OBJ_Reader().read_mesh(argv[1], wname);
  COM_window_init_done(wname.c_str());

  // Output mesh
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");

  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  int mesh = COM_get_dataitem_handle_const((wname + ".mesh").c_str());

  std::string fname(argc >= 3 ? argv[2] : (wname + ".hdf").c_str());

  int separate_files = fname.find(".hdf") == string::npos;

  if (!separate_files) {
    std::cout << "Writing HDF file(s) " << fname << endl;

    COM_call_function(OUT_write, fname.c_str(), &mesh, (char *)wname.c_str(),
                      "000");
  } else {
    int npanes, *paneIDs;
    COM_get_panes(wname.c_str(), &npanes, &paneIDs);

    for (int i = 0; i < npanes; ++i) {
      std::ostringstream ostr;

      ostr << fname;
      ostr.fill('0');
      ostr.width(5);
      ostr << paneIDs[i] << ".hdf";

      std::cout << "Writing HDF file(s) " << ostr.str() << endl;

      COM_call_function(OUT_write, ostr.str().c_str(), &mesh,
                        (char *)wname.c_str(), "000", NULL, NULL, &paneIDs[i]);
    }

    COM_free_buffer(&paneIDs);
  }

  COM_finalize();
  return 0;
}
