/**
 \file
 \ingroup support
 \brief Mesh Stuff
 \author MTC
 \author WooHyun Kim
 \author Brian Weisberg
*/
#ifndef __MESH_UTIL_H__
#define __MESH_UTIL_H__

#include "Mesh.H"

namespace SolverUtils {
namespace MeshUtils {
int meshgen2d(std::istream &inStream,
              SolverUtils::Mesh::UnstructuredMesh &unMesh);
int meshgen2d(int argc, char *argv[]);
}  // namespace MeshUtils
}  // namespace SolverUtils

#endif
