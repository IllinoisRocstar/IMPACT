#ifndef __TEST_SOLVER_H__
#define __TEST_SOLVER_H__

#include <string>
#include <vector>
#include "InterfaceLayer.H"
#include "SolverAgent.H"

namespace COM {

class TestSolver : public SolverUtils::FEM::SolverAgent {
 public:
  void Init(const std::string &filename);

 protected:
  int step;
  std::string name;                   // string at solver
  std::vector<double> time;           // scalar at solver
  std::vector<double> delta_time;     // scalar at solver
  std::vector<int> id;                // scalar at mesh
  std::vector<double> displacements;  // vector at nodes
  std::vector<double> pressures;      // scalar at cells
  std::vector<double> temperatures;   // scalar at nodes
  std::vector<int> nodeflags;         // vector at nodes
  std::vector<int> cellflags;         // vector at cells
};

}  // namespace COM
#endif