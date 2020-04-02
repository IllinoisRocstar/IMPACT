//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include "RFC_Window_base.h"
#include "writer.h"

RFC_BEGIN_NAME_SPACE

// New dataitems in the given window for output using Rocout.
void RFC_Window_base::new_sdv_dataitems(const std::string &wname) const {
  // New dataitems for v2b and b2v.
  COM_new_dataitem((wname + ".v2b").c_str(), 'p', COM_INT, 1, "");
  COM_new_dataitem((wname + ".b2v").c_str(), 'p', COM_INT, 1, "");

  // New dataitems for subnode parents
  COM_new_dataitem((wname + ".sn_parent_fcID").c_str(), 'n', COM_INT, 1, "");
  COM_new_dataitem((wname + ".sn_parent_ncs").c_str(), 'n', COM_FLOAT, 2, "");

  // New dataitems for natural coordinates
  COM_new_dataitem((wname + ".sn_permu_edID").c_str(), 'n', COM_INT, 1, "");
  COM_new_dataitem((wname + ".sn_permu_ncs").c_str(), 'n', COM_FLOAT, 2, "");

  // New dataitems for counterpart of each subnode
  COM_new_dataitem((wname + ".sn_cntpt_pnID").c_str(), 'n', COM_INT, 1, "");
  COM_new_dataitem((wname + ".sn_cntpt_ndID").c_str(), 'n', COM_INT, 1, "");

  // New dataitems for the mapping from node IDs to sub-node IDs.
  COM_new_dataitem((wname + ".sn_subID").c_str(), 'p', COM_INT, 1, "");

  // New dataitems for the parent of each subface
  COM_new_dataitem((wname + ".sf_parent").c_str(), 'e', COM_INT, 1, "");

  // Register the natural coordinates of subnodes in each subface
  COM_new_dataitem((wname + ".sf_ncs").c_str(), 'e', COM_FLOAT, 6, "");

  // New dataitems for the counterpart in the other mesh of each subface
  COM_new_dataitem((wname + ".sf_cntpt_pnID").c_str(), 'e', COM_INT, 1, "");
  COM_new_dataitem((wname + ".sf_cntpt_fcID").c_str(), 'e', COM_INT, 1, "");

  // New dataitems for the offset of the subface ID for each input facet
  COM_new_dataitem((wname + ".sf_offset").c_str(), 'p', COM_INT, 1, "");
}

// Register the dataitems for the pane for output using Rocout.
void RFC_Pane_base::register_sdv_dataitems(const std::string &wname) const {
  // Register the mesh
  const int pid = id();

  // Pack coordinates into double precision buffers
  int nnodes = size_of_subnodes();
  COM_set_size((wname + ".nc").c_str(), pid, nnodes);

  Point_3 *coor;
  COM_allocate_array((wname + ".nc").c_str(), pid, &(void *&)coor);
  for (int i = 0; i < nnodes; ++i) coor[i] = get_point_of_subnode(i + 1);

  // Register subface connectivity, parents, and counterparts
  COM_set_size((wname + ".:t3").c_str(), pid, _subfaces.size());
  COM_set_array_const((wname + ".:t3").c_str(), pid, &_subfaces[0]);

  // Pack pane connectivity v2b of the pane into integer buffers
  //      allocated in COM.
  int *v2b_buf;
  COM_set_size((wname + ".v2b").c_str(), pid, _v2b_table.size() * 3);
  COM_allocate_array((wname + ".v2b").c_str(), pid, &(void *&)v2b_buf);

  for (V2b_table::const_iterator it = _v2b_table.begin(),
                                 iend = _v2b_table.end();
       it != iend; ++it) {
    *(v2b_buf++) = it->first;
    *(v2b_buf++) = it->second.first;
    *(v2b_buf++) = it->second.second;
  }

  // Pack b2v into an integer buffer.
  std::string b2v_name = wname + ".b2v";
  for (B2v_table::const_iterator it = _b2v_table.begin(),
                                 iend = _b2v_table.end();
       it != iend; ++it) {
    const B2v &b2v = it->second;
    int n = b2v.size();

    COM_append_array(b2v_name.c_str(), pid, &it->first, 0, 1);
    COM_append_array(b2v_name.c_str(), pid, &n, 0, 1);
    COM_append_array(b2v_name.c_str(), pid, &b2v[0], 0, b2v.size());
  }

  COM_set_array_const((wname + ".sn_parent_fcID").c_str(), pid,
                      &_subnode_parents[0].face_id, 2);
  COM_set_array_const((wname + ".sn_parent_ncs").c_str(), pid,
                      &_subnode_normalized_nc[0], 2);

  // Permuted local coordinates within parent face
  COM_set_array_const((wname + ".sn_permu_edID").c_str(), pid,
                      &_subnode_parents[0].edge_id, 2);
  COM_set_array_const((wname + ".sn_permu_ncs").c_str(), pid,
                      &_subnode_nat_coors[0][0], 2);

  // Register the counterpart of each subnode
  COM_set_array_const((wname + ".sn_cntpt_pnID").c_str(), pid,
                      &_subnode_counterparts[0].pane_id, 2);
  COM_set_array_const((wname + ".sn_cntpt_ndID").c_str(), pid,
                      &_subnode_counterparts[0].node_id, 2);

  // Register the parent of each subface
  COM_set_array_const((wname + ".sf_parent").c_str(), pid,
                      &_subface_parents[0]);

  // Register the natural coordinates of subnodes in each subface
  COM_set_array_const((wname + ".sf_ncs").c_str(), pid, &_subface_nat_coors[0]);

  // Register the counterpart in the other mesh of each subface
  COM_set_array_const((wname + ".sf_cntpt_pnID").c_str(), pid,
                      &_subface_counterparts[0].pane_id, 2);
  COM_set_array_const((wname + ".sf_cntpt_fcID").c_str(), pid,
                      &_subface_counterparts[0].face_id, 2);

  // Register the offset of the subface ID for each input facet
  COM_set_size((wname + ".sn_subID").c_str(), pid, _subnode_subID.size());
  COM_set_array_const((wname + ".sn_subID").c_str(), pid, &_subnode_subID[0]);

  // Register the offset of the subface ID for each input facet
  COM_set_size((wname + ".sf_offset").c_str(), pid, size_of_faces());
  COM_set_array_const((wname + ".sf_offset").c_str(), pid,
                      &_subface_offsets[0]);

  COM_window_init_done(wname.c_str());
}

class Pane_friend : public COM::Pane {
  explicit Pane_friend(COM::Pane &);

 public:
  using Pane::inherit;
};

void RFC_Pane_base::write_off(std::FILE *fhandle) const {
  SD_set_option("start_vertexID", 1);
  SD_set_option("start_faceID", 1);
  SD_set_option("start_subvertexID", 1);
  SD_set_option("start_subfaceID", 1);

  // Pack coordinates into double precision buffers
  int nnodes = size_of_subnodes();
  int nsf = _subfaces.size();

  std::vector<float> coor(nnodes * 3);
  std::vector<int> sn_parents(nnodes);
  std::vector<int> sn_cntpt_pnID(nnodes), sn_cntpt_ndID(nnodes);

  for (int i = 0; i < nnodes; ++i) {
    Point_3 p = get_point_of_subnode(i + 1);
    coor[3 * i] = p[0];
    coor[3 * i + 1] = p[1];
    coor[3 * i + 2] = p[2];

    sn_parents[i] = _subnode_parents[i].face_id;
    sn_cntpt_pnID[i] = _subnode_counterparts[i].pane_id;
    sn_cntpt_ndID[i] = _subnode_counterparts[i].node_id;
  }

  std::vector<int> sf_cntpt_pnID(nsf), sf_cntpt_fcID(nsf);
  for (int i = 0; i < nsf; ++i) {
    sf_cntpt_pnID[i] = _subface_counterparts[i].pane_id;
    sf_cntpt_fcID[i] = _subface_counterparts[i].face_id;
  }

  // Construct a mapping from subnodes to real nodes
  std::vector<int> sn_parent_vID(nnodes, -1);
  for (int i = 0, ni = _subnode_subID.size(); i < ni; ++i) {
    if (_subnode_subID[i] > 0) sn_parent_vID[_subnode_subID[i] - 1] = i;
  }

  SD_write_off_comref(fhandle, id(), nnodes, _subfaces.size(), &coor[0],
                      &sn_parents[0], &_subnode_normalized_nc[0][0],
                      &sn_cntpt_ndID[0], &_subfaces[0][0], &_subface_parents[0],
                      &_subface_nat_coors[0][0][0], &sf_cntpt_fcID[0],
                      &sn_parent_vID[0], &sn_cntpt_pnID[0], &sf_cntpt_pnID[0]);
}

// Read in the subdivision using Rocin.
void RFC_Pane_base::read_rocin(const std::string &sdv_wname,
                               const std::string &parent_wname,
                               COM::Pane *pn_base) {
  const int pid = id();

  // If pn_base is not NULL, then inherit from parent pane.
  if (pn_base) {
    // Obtain the mesh object in the corresponding pane in the parent window
    COM::COM_base *rcom = COM_get_com();
    int win_handle = rcom->get_window_handle(parent_wname.c_str());
    COM::Window *parent_win = rcom->get_window_object(win_handle);

    COM::DataItem *mesh = parent_win->pane(pid).dataitem(COM::COM_PMESH);

    // Clone the mesh from parent pane onto pn_base.
    ((Pane_friend *)pn_base)->inherit(mesh, "", COM::Pane::INHERIT_CLONE, 0);
  }

  // Obtain v2b_table
  int nv2b;
  const int *v2b_ptr;
  COM_get_size((sdv_wname + ".v2b").c_str(), pid, &nv2b);
  COM_get_array_const((sdv_wname + ".v2b").c_str(), pid, &v2b_ptr);

  for (int i = 0; i < nv2b; i += 3)
    _v2b_table[v2b_ptr[i]] = std::make_pair(v2b_ptr[i + 1], v2b_ptr[i + 2]);

  // Obtain b2v mapping
  int nb2v;
  const int *b2v_ptr;
  COM_get_size((sdv_wname + ".b2v").c_str(), pid, &nb2v);

  COM_get_array_const((sdv_wname + ".b2v").c_str(), pid, &b2v_ptr);

  int nsubn, nsubf;
  COM_get_size((sdv_wname + ".nc").c_str(), pid, &nsubn);
  COM_get_size((sdv_wname + ".:t3").c_str(), pid, &nsubf);

  _subfaces.resize(nsubf);
  COM_copy_array((sdv_wname + ".:t3").c_str(), pid, &_subfaces[0]);

  if (COM_get_dataitem_handle((sdv_wname + ".sn_parent_fcID").c_str()) > 0) {
    // This is the new HDF file format!

    // Obtain subnode_parents, subnode_natcoor, and subnode_counterparts
    _subnode_parents.resize(nsubn);
    COM_copy_array((sdv_wname + ".sn_parent_fcID").c_str(), pid,
                   &_subnode_parents[0].face_id, 2);
    _subnode_normalized_nc.resize(nsubn);
    COM_copy_array((sdv_wname + ".sn_parent_ncs").c_str(), pid,
                   &_subnode_normalized_nc[0], 2);
    _subnode_subID.resize(size_of_nodes());
    COM_copy_array((sdv_wname + ".sn_subID").c_str(), pid, &_subnode_subID[0],
                   1);

    COM_copy_array((sdv_wname + ".sn_permu_edID").c_str(), pid,
                   &_subnode_parents[0].edge_id, 2);
    _subnode_nat_coors.resize(nsubn);
    COM_copy_array((sdv_wname + ".sn_permu_ncs").c_str(), pid,
                   &_subnode_nat_coors[0][0], 2);

    _subnode_counterparts.resize(nsubn);
    COM_copy_array((sdv_wname + ".sn_cntpt_pnID").c_str(), pid,
                   &_subnode_counterparts[0].pane_id, 2);
    COM_copy_array((sdv_wname + ".sn_cntpt_ndID").c_str(), pid,
                   &_subnode_counterparts[0].node_id, 2);

    _subface_parents.resize(nsubf);
    COM_copy_array((sdv_wname + ".sf_parent").c_str(), pid,
                   &_subface_parents[0]);

    _subface_counterparts.resize(nsubf);
    COM_copy_array((sdv_wname + ".sf_cntpt_pnID").c_str(), pid,
                   &_subface_counterparts[0].pane_id, 2);
    COM_copy_array((sdv_wname + ".sf_cntpt_fcID").c_str(), pid,
                   &_subface_counterparts[0].face_id, 2);

    _subface_nat_coors.resize(nsubf);
    COM_copy_array((sdv_wname + ".sf_ncs").c_str(), pid, &_subface_nat_coors[0],
                   6);

    // Obtain subface_offsets
    _subface_offsets.resize(size_of_faces() + 1);
    COM_copy_array((sdv_wname + ".sf_offset").c_str(), pid,
                   &_subface_offsets[0]);
    _subface_offsets[size_of_faces()] = nsubf;
  } else {  // This is old file format
    // Obtain subnode_parents, subnode_natcoor, and subnode_counterparts
    _subnode_parents.resize(nsubn);
    COM_copy_array((sdv_wname + ".sn_parents_fcID").c_str(), pid,
                   &_subnode_parents[0].face_id, 2);
    COM_copy_array((sdv_wname + ".sn_parents_edID").c_str(), pid,
                   &_subnode_parents[0].edge_id, 2);

    _subnode_nat_coors.resize(nsubn);
    COM_copy_array((sdv_wname + ".sn_xi").c_str(), pid,
                   &_subnode_nat_coors[0][0], 2);
    COM_copy_array((sdv_wname + ".sn_eta").c_str(), pid,
                   &_subnode_nat_coors[0][1], 2);

    _subnode_counterparts.resize(nsubn);
    COM_copy_array((sdv_wname + ".sn_cntpt_pnID").c_str(), pid,
                   &_subnode_counterparts[0].pane_id, 2);
    COM_copy_array((sdv_wname + ".sn_cntpt_ndID").c_str(), pid,
                   &_subnode_counterparts[0].node_id, 2);

    _subface_parents.resize(nsubf);
    COM_copy_array((sdv_wname + ".sf_parents").c_str(), pid,
                   &_subface_parents[0]);

    _subface_counterparts.resize(nsubf);
    COM_copy_array((sdv_wname + ".sf_cntpt_pnID").c_str(), pid,
                   &_subface_counterparts[0].pane_id, 2);
    COM_copy_array((sdv_wname + ".sf_cntpt_fcID").c_str(), pid,
                   &_subface_counterparts[0].face_id, 2);

    // Obtain subface_offsets
    _subface_offsets.resize(size_of_faces() + 1);
    COM_copy_array((sdv_wname + ".sf_offsets").c_str(), pid,
                   &_subface_offsets[0]);

    // Compute the natural coordinates.
    comp_nat_coors();
  }
}

COM_EXTERN_MODULE(SimIN)

/*! Read in panes in native or HDF format. In native format, each pane
 *  has a separate file with file name \p {prefix}paneID.sdv.
 *  In HDF format, each pane also has a separate file with file name
 *  \p {prefix}_paneID_sdv.hdf.
 * \sa write_sdv
 */
void RFC_Window_base::read_sdv(const char *prefix, const char *format) {
  // Determine the format
  int IO_format = get_sdv_format(format);

  if (IO_format == SDV_BINARY) {
    // Read in native format binary format.
    for (Pane_set::iterator it = _pane_set.begin(), iend = _pane_set.end();
         it != iend; ++it) {
      std::string fname = get_sdv_fname(prefix, it->first, IO_format);

      std::ifstream is(fname.c_str());
      if (!is) {
        std::cerr << "Rocface: Could not open input file " << fname
                  << "\nAbortting..." << std::endl;
        RFC_assertion(is);
        exit(-1);
      }

      RFC_Pane_base &pane = *it->second;
      pane.read_binary(is);
    }
  } else {
    // Read in using Rocin.
    COM_LOAD_MODULE_STATIC_DYNAMIC(SimIN, "RFC_IN");
    int hdl_read = COM_get_function_handle("RFC_IN.read_windows");
    int hdl_obtain = COM_get_function_handle("RFC_IN.obtain_dataitem");

    // Define the base-window and sdv-window names
    std::string base_material = get_prefix_base(prefix);
    std::string sdv_material = base_material + "_sdv";

    std::string buf_wname(_bufwin_prefix);
    buf_wname.append(base_material);
    std::string sdv_wname = buf_wname + "_sdv";
    MPI_Comm comm_self = MPI_COMM_SELF;

    // Loop through the panes to obtain the dataitems.
    for (Pane_set::iterator it = _pane_set.begin(), iend = _pane_set.end();
         it != iend; ++it) {
      int pane_id = it->first;
      std::string fname = get_sdv_fname(prefix, pane_id, IO_format);

      // Read the pane from the given file. Note that the file contains both
      // the parent and the subdivided windows. Read only the subdivided one.
      COM_call_function(hdl_read, fname.c_str(), _bufwin_prefix,
                        sdv_material.c_str(), &comm_self);

      int hdl_all = COM_get_dataitem_handle((sdv_wname + ".all").c_str());
      COM_call_function(hdl_obtain, &hdl_all, &hdl_all, &pane_id);
      it->second->read_rocin(sdv_wname);

      // Delete the window created by Rocin.
      COM_delete_window(sdv_wname.c_str());
    }

    // Unload Rocin
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(SimIN, "RFC_IN");
  }
}

COM_EXTERN_MODULE(SimOUT)

/*! Write panes in native format or using Rocout's format.
 *  In native format, each pane has a separate file with file name
 *  \p {prefix}paneID.sdv. In Rocout format, each pane also has a
 *  separate file with file name \p {prefix}_paneID_sdv.<suf>.
 * \sa read_sdv
 */
void RFC_Window_base::write_sdv(const char *prefix, const char *format) const {
  // Determine the format
  int IO_format = get_sdv_format(format);

  if (IO_format == SDV_OFF) {
    std::string fname = get_sdv_fname(prefix, 0, IO_format);
    std::FILE *fhandle = std::fopen(fname.c_str(), "w");
    if (!fhandle) {
      std::cerr << "Rocface: Could not open output file " << fname
                << "\nAbortting..." << std::endl;
      RFC_assertion(fhandle);
      exit(-1);
    }

    // Output using OFF file format
    for (Pane_set::const_iterator it = _pane_set.begin(),
                                  iend = _pane_set.end();
         it != iend; ++it) {
      RFC_Pane_base &pane = *it->second;
      pane.write_off(fhandle);
    }

    std::fclose(fhandle);
  } else if (IO_format == SDV_BINARY) {
    // Output using native file format
    for (Pane_set::const_iterator it = _pane_set.begin(),
                                  iend = _pane_set.end();
         it != iend; ++it) {
      std::string fname = get_sdv_fname(prefix, it->first, IO_format);

      std::ofstream os(fname.c_str());
      if (!os) {
        std::cerr << "Rocface: Could not open output file " << fname
                  << "\nAbortting..." << std::endl;
        RFC_assertion(os);
        exit(-1);
      }

      RFC_Pane_base &pane = *it->second;
      pane.write_binary(os);
    }
  } else {
    // Output using Rocout's format
    COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT, "RFC_OUT");
    int hdl_set_option = COM_get_function_handle("RFC_OUT.set_option");
    int hdl_write = COM_get_function_handle("RFC_OUT.write_dataitem");

    COM_call_function(hdl_set_option, "rankwidth", "0");
    //COM_call_function(hdl_set_option, "format", format);

    int parent_mesh = COM_get_dataitem_handle((name() + ".pmesh").c_str());

    // Define the base-window and sdv-window names
    std::string base_material = get_prefix_base(prefix);
    std::string sdv_material = base_material + "_sdv";

    std::string buf_wname(_bufwin_prefix);
    buf_wname.append(base_material);
    std::string sdv_wname = buf_wname + "_sdv";

    // Create a window for calling Rocout.
    COM_new_window(sdv_wname.c_str());
    new_sdv_dataitems(sdv_wname);
    int win_all = COM_get_dataitem_handle((sdv_wname + ".all").c_str());

    const char *time_level = "000";
    for (Pane_set::const_iterator it = _pane_set.begin(),
                                  iend = _pane_set.end();
         it != iend; ++it) {
      int pane_id = it->first;

      // Define a file name name
      std::string fname = get_sdv_fname(prefix, pane_id, IO_format);

      // Initialize the dataitems for the subdivision pane
      it->second->register_sdv_dataitems(sdv_wname);

      // write parent mesh of the given pane into the file
      COM_call_function(hdl_set_option, "mode", "w");
      COM_call_function(hdl_write, fname.c_str(), &parent_mesh,
                        base_material.c_str(), time_level, NULL, NULL,
                        &pane_id);

      // append the subdivision pane into the file.
      COM_call_function(hdl_set_option, "mode", "a");
      COM_call_function(hdl_write, fname.c_str(), &win_all,
                        sdv_material.c_str(), time_level, NULL, NULL, &pane_id);

      // remove the subdivision pane after output it.
      COM_delete_pane(sdv_wname.c_str(), pane_id);
      COM_window_init_done(sdv_wname.c_str());
    }

    // Unload Rocout
    COM_delete_window(sdv_wname.c_str());
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(SimOUT, "RFC_OUT");
  }
}

// Get the basename of the prefix without the directory part and no suffix.
const char *RFC_Window_base::get_prefix_base(const char *prefix) {
  const char *last_slash = std::strrchr(prefix, '/');

  if (last_slash == NULL)
    return prefix;
  else
    return last_slash + 1;
}

int RFC_Window_base::get_sdv_format(const char *format) {
  if (!format) return SDV_SIMIO;
  if (std::strcmp(format, "OFF") == 0) return SDV_OFF;
  if (std::strcmp(format, "BIN") == 0) return SDV_BINARY;
  if (std::strcmp(format, "HDF") == 0) return SDV_HDF;
  if (std::strcmp(format, "CGNS") == 0) return SDV_CGNS;

  RFC_assertion_msg(false, (std::string("Unknown format ") + format).c_str());
  return SDV_SIMIO;  // Default is SDV_SIMIO.
}

std::string RFC_Window_base::get_sdv_fname(const char *prefix, int pane_id,
                                           const int format) {
  // Define a file name name
  std::ostringstream fname;

  switch (format) {
    case SDV_BINARY:
      fname << prefix << pane_id << ".sdv";
      break;
    case SDV_OFF:
      fname << prefix << "_sdv.off";
      break;
    case SDV_HDF:
      fname << prefix << '_' << pane_id << "_sdv.hdf";
      break;
    case SDV_CGNS:
      fname << prefix << '_' << pane_id << "_sdv.cgns";
      break;
  case SDV_SIMIO:
      fname << prefix << '_' << pane_id << "_sdv";
      break;
    default:
      COM_assertion_msg(false, "Unknown file format");
  }

  return fname.str();
}

const char *RFC_Window_base::_bufwin_prefix = "buf_";

RFC_END_NAME_SPACE
