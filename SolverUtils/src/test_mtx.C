#include "Mesh.H"

using namespace SolverUtils;

int main(int argc,char *argv[])
{
  Mesh::Connectivity     con;
  std::cin >> con;
  Mesh::Connectivity::iterator ci = con.begin();
  while(ci != con.end()){
    std::vector<Mesh::IndexType>::iterator ni = ci->begin();
    while(ni != ci->end()){
      *ni = *ni + 1;
      ni++;
    }
    ci++;
  }
  con.Sync();
  con.SyncSizes();
  std::vector<Mesh::IndexType> remap;
  con.BreadthFirstRenumber(remap);
  std::vector<Mesh::IndexType>::iterator rmi = remap.begin();
  while(rmi != remap.end()){
    std::cout << *rmi++ << " ";
  }
  std::cout << std::endl;
  return(0);
}
