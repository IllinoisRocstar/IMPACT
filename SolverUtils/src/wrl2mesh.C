#include "Mesh.H"

using namespace SolverUtils;

int main(int argc, char *argv[]) {
  Mesh::UnstructuredMesh mesh;
  Mesh::ReadWRLFromStream(mesh, std::cin);
  std::cout << mesh.nc << std::endl << mesh.con << std::endl;
  return (0);
}
