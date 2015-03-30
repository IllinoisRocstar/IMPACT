// MPACT LICENSE 

/** \file Rocsurf.h
 */
#ifndef __ROCSURF_H_
#define __ROCSURF_H_

#include "surfbasic.h"
#include "com_devel.hpp"

SURF_BEGIN_NAMESPACE

class Window_manifold_2;

class Rocsurf : public COM_Object {
public:
  // protected:
  Rocsurf() : _wm(NULL), _cookie(SURF_COOKIE) {}
  virtual ~Rocsurf();

  /// Loads Rocsurf onto Roccom with a given module name.
  static void load( const std::string &mname);
  /// Unloads Rocsurf from Roccom.
  static void unload( const std::string &mname);

  /// Interpolates nodal coordinates to element centers.
  static void interpolate_to_centers( const COM::DataItem *x, 
				      COM::DataItem *z);

  /// Integrate a function given by an elemental dataitem over surface
  /// z is an array of size equal to number of components of x
  static void integrate( const COM::DataItem *x, double *z);

  /** Computes the area of each face of the surface mesh of window
   *  areas->window and saves the results in dataitem areas.
   *  If pnts is present, then use it in place of the nodal coordinates
   */
  static void compute_element_areas( COM::DataItem *areas, 
				     const COM::DataItem *pnts=NULL);

  /// Computes elemental normals of a given window. Normalize the 
  /// normals if to_normalize is NULL (default) or its value is nonzero.
  /// If pnts is present, then use it in place of the nodal coordinates
  static void compute_element_normals( COM::DataItem *nrm,
				       const int *to_normalize=NULL, 
				       const COM::DataItem *pnts=NULL);

  /** Computes the volume bounded between two different locations of
   *  each face of the surface mesh of window volumes->window. Typically, 
   *  the two locations of the surface correspond to the surface at two
   *  different snapshots of a simulation.
   *
   *  \param old_location stores the old nodal coordinates.
   *  \param new_location stores the new nodal coordinates.
   *  \param volumes stores the bounded volume for each element.
   *  \param If flag is present, then only compute volume for elements whose 
   *    corresponding value of the volume dataitem was set to nonzero value. 
   */
  static void compute_bounded_volumes( const COM::DataItem *old_location,
				       const COM::DataItem *new_location,
				       COM::DataItem *volumes,
				       void *flag=NULL);

  /** Computes the swept volume by a given displacement of each face of 
   *  the surface mesh of window volumes->window. 
   *
   *  \param old_location stores the nodal coordinates.
   *  \param disps stores the nodal displacement.
   *  \param volumes stores the swept volume for each element.
   *  \param If flag is present, then only compute volume for elements whose 
   *    corresponding value of the volume dataitem was set to nonzero value. 
   */
  static void compute_swept_volumes( const COM::DataItem *location,
				     const COM::DataItem *disps,
				     COM::DataItem *volumes,
				     void *flag=NULL);

  /** Computes the center of a body.
   */
  static void compute_center( const COM::DataItem *mesh,
			      Vector_3<double> &cnt);

  /** Computes the signed volume of a body.
   */
  static void compute_signed_volumes( const COM::DataItem *mesh,
				      double *vol);

  /// Obtain a reference to the manifold.
  virtual Window_manifold_2 *manifold() { return _wm; }

  /// Constructs the communication patterns of a distributed mesh.
  /// If the input is pmesh, then use the given pconn.
  /// Otherwise, compute pconn.
  void initialize( const COM::DataItem *pmesh);

  /// Computes nodal or elemental normals of a given window
  void compute_normals( const COM::DataItem *mesh,
			COM::DataItem *nrm,
			const int *scheme=NULL);

  /// Computes nodal or elemental normals of a given window
  void compute_mcn( COM::DataItem *mcn, COM::DataItem *lbmcn);

  /// Serialize the mesh of a given window.
  void serialize_mesh( const COM::DataItem *inmesh, COM::DataItem *outmesh);

  /// Computes edge lengths of a given window.
  void compute_edge_lengths( double *lave, double *lmin, double *lmax);
  
  /// Computes nodal or elemental normals of a given window
  void elements_to_nodes( const COM::DataItem *elem_vals,
			  COM::DataItem *nodal_vals,
			  const COM::DataItem *mesh=NULL,
			  const int *scheme=NULL,
			  const COM::DataItem *elem_weights=NULL,
			  COM::DataItem *nodal_weights=NULL);

protected:

  template <class T>
  static void normalize( T *a, int size) throw(int) {
    T tmp(0);
    for (int i=0; i<size; ++i) tmp += a[i]*a[i]; 
    
    if ( tmp == 0) return;
    tmp = std::sqrt(tmp);
    for (int i=0; i<size; ++i) a[i] /= tmp;
  }

  int validate_object() const {
    if ( _cookie != SURF_COOKIE) return -1;
    else return COM_Object::validate_object();
  }
  
protected:
  enum { SURF_COOKIE=7627873};
  Window_manifold_2   *_wm;
  static const int scheme_vals[];
  int _cookie;
};

SURF_END_NAMESPACE

#endif


