//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <cstring>
#include <iostream>
#include <sstream>
#include "IM_Reader.h"
#include "com.h"

COM_EXTERN_MODULE(SimOUT)

int main(int argc, char *argv[]) {
  COM_init(&argc, &argv);

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <im_file> [hdf_prefix] " << std::endl
              << "\tWrite each pane into a separate HDF file." << std::endl;
    std::cout << "Usage: " << argv[0] << " <im_file> [hdf_name] " << std::endl
              << "\tWrite all panes into one HDF file." << std::endl;
    exit(-1);
  }

  char *firstdot = strchr(argv[1], '.');
  const std::string wname(argv[1],
                          firstdot ? firstdot - argv[1] : strlen(argv[1]));

  std::cout << "Reading surface mesh file \"" << argv[1] << '"' << std::endl;

  COM_new_window(wname.c_str());
  IM_Reader().read_mesh(argv[1], wname, NULL);
  COM_window_init_done(wname.c_str());

  // Output mesh
  COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "OUT");

  int OUT_write = COM_get_function_handle("OUT.write_dataitem");
  int mesh = COM_get_dataitem_handle_const((wname + ".mesh").c_str());

  std::string fname;
  if (argc > 2)
    fname = argv[2];
  else
    fname = wname + ".hdf";

  int separate_files = fname.find(".hdf") == std::string::npos;

  if (!separate_files) {
    std::cout << "Writing HDF file(s) " << fname << std::endl;

    COM_call_function(OUT_write, fname.c_str(), &mesh,
                      reinterpret_cast<const char *>(wname.c_str()), "000");
  } else {
    int npanes, *paneIDs;
    COM_get_panes(wname.c_str(), &npanes, &paneIDs);

    for (int i = 0; i < npanes; ++i) {
      std::ostringstream ostr;

      ostr << fname;
      ostr.fill('0');
      ostr.width(5);
      ostr << paneIDs[i] << ".hdf";

      std::cout << "Writing HDF file(s) " << ostr.str() << std::endl;

      COM_call_function(OUT_write, ostr.str().c_str(), &mesh,
                        reinterpret_cast<const char *>(wname.c_str()), "000",
                        NULL, NULL, &paneIDs[i]);
    }

    COM_free_buffer(&paneIDs);
  }

  COM_finalize();
}
