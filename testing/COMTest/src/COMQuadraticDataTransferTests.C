#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include "COM_base.hpp"
#include "InterfaceLayer.H"
#include "MeshUtils.H"
#include "SolverAgent.H"
#include "com_basic.h"
#include "com_c++.hpp"
#include "gtest/gtest.h"

// Global variables used to pass arguments to the tests
char **ARGV;
int ARGC;

// Auxillary helper functions
void ResetSolution(std::vector<double> &solution, double value = -1) {
  std::vector<double>::iterator sIt = solution.begin();
  while (sIt != solution.end()) *sIt++ = value;
}

double GetRelativeDiff(double A, double B, double epsilon) {
  if (A == B) return 0;

  double absA = std::fabs(A);
  double absB = std::fabs(B);
  double diff = std::fabs(A - B);

  double doubleMin = std::numeric_limits<double>::min();
  if (A == 0 || B == 0 || diff < doubleMin) {
    return (diff);
  }
  return (diff / (absA + absB));
}

void GetErrors(std::vector<double> &solution1, std::vector<double> &solution2,
               std::vector<double> &errors) {
  if (solution1.size() != solution2.size()) return;
  if (solution1.empty()) return;
  int nVal = 0;
  double minVal = std::numeric_limits<double>::min();
  errors.resize(6, 0);
  errors[0] = errors[3] = std::numeric_limits<double>::max();
  std::vector<double>::iterator iT1 = solution1.begin();
  std::vector<double>::iterator iT2 = solution2.begin();
  while (iT1 != solution1.end()) {
    nVal++;
    double value1 = *iT1++;
    double value2 = *iT2++;
    double diff = std::fabs(value2 - value1);
    if (value1 == value2) {
      errors[0] = errors[3] = 0;
    } else if (value1 == 0 || value2 == 0 || diff < minVal) {
      errors[2] += diff;
      if (diff < errors[0]) errors[0] = diff;
      if (diff > errors[1]) errors[1] = diff;
      if (diff < errors[3]) errors[3] = diff;
      if (diff > errors[4]) errors[4] = diff;
      errors[5] += diff;
    } else {
      errors[2] += diff;
      if (diff < errors[0]) errors[0] = diff;
      if (diff > errors[1]) errors[1] = diff;
      double abs1 = std::fabs(value1);
      double abs2 = std::fabs(value2);
      diff = diff / (abs1 + abs2);
      if (diff < errors[3]) errors[3] = diff;
      if (diff > errors[4]) errors[4] = diff;
      errors[5] += diff;
    }
  }
  errors[2] /= static_cast<double>(nVal);
  errors[5] /= static_cast<double>(nVal);
}

bool TwoSolutionsMatch(std::vector<double> &solution1,
                       std::vector<double> &solution2, double tol = 1e-5) {
  std::vector<double>::iterator soln1It = solution1.begin();
  std::vector<double>::iterator soln2It = solution2.begin();
  while (soln1It != solution1.end())
    if (std::abs(*soln1It++ - *soln2It++) > tol) return (false);
  return (true);
}

void CompareSolutionsWithDetail(std::vector<double> &solution1,
                                std::vector<double> &solution2,
                                std::ostream &outStream, double tol = 1e-5) {
  std::vector<double>::iterator soln1It = solution1.begin();
  std::vector<double>::iterator soln2It = solution2.begin();
  int n = 0;
  double sum = 0;
  double diffmax = 0;
  double diffmin = 10000.0;
  while (soln1It != solution1.end()) {
    double value1 = *soln1It++;
    double value2 = *soln2It++;
    double diff = std::abs(*soln1It - *soln2It);
    sum += diff;
    if (diff > diffmax) diffmax = diff;
    if (diff < diffmin) diffmin = diff;
    // if(diff > tol){
    //   outStream << soln1It - solution1.begin() << "\t: " << *soln1It
    //             << ", " << *soln2It << std::endl;
    // }
    soln1It++;
    soln2It++;
    n++;
  }
  outStream << "Absolute (min,max,mean) = (" << diffmax << "," << diffmin << ","
            << sum / static_cast<double>(n) << ")" << std::endl;
}

void MakeWindow(std::string name) {
  COM_new_window(name.c_str());
  std::string solnName = name + ".soln";
  COM_new_dataitem(solnName.c_str(), 'n', COM_DOUBLE, 3, "m/s");

  std::string compName = name + ".comp";
  COM_new_dataitem(compName.c_str(), 'n', COM_DOUBLE, 3, "m/s");
}

// class used to store data about the Mesh transfer
class TransferSolver : public SolverUtils::FEM::SolverAgent {
 public:
  void Init(const std::string &filename);
  ~TransferSolver() { SolverUtils::UnRegisterSolver(name); };

 protected:
  std::string name;                      // string at solver
  std::vector<double> nodeCoordinates1;  // vector at nodes
  std::vector<double> nodeCoordinates2;  // vector at nodes
  std::vector<double> cellCoordinates1;  // vector at nodes
  std::vector<double> cellCoordinates2;  // vector at nodes
  std::vector<double> nodeFunction1;     // function at nodes
  std::vector<double> nodeFunction2;     // function at nodes
  std::vector<double> cellFunction1;     // function at cells
  std::vector<double> cellFunction2;     // function at cells
  int nNodes;
  int nElem;

 public:
  int NNodes() { return (Mesh().nc.NNodes()); };
  int NElem() { return (Mesh().con.Nelem()); };
  std::vector<double> &NodeCoordinates1() { return (nodeCoordinates1); };
  std::vector<double> &NodeCoordinates2() { return (nodeCoordinates2); };
  std::vector<double> &CellCoordinates1() { return (cellCoordinates1); };
  std::vector<double> &CellCoordinates2() { return (cellCoordinates2); };

  std::vector<double> &NodeFunction1() { return (nodeFunction1); };
  std::vector<double> &NodeFunction2() { return (nodeFunction2); };
  std::vector<double> &CellFunction1() { return (cellFunction1); };
  std::vector<double> &CellFunction2() { return (cellFunction2); };

  void Setup(const std::string &meshString, const std::string &solverName,
             const std::string &meshName) {
    std::istringstream meshStream(meshString);
    SolverUtils::MeshUtils::meshgen2d(meshStream, Mesh());

    // Write the test mesh for use in later tests
    std::ofstream Ouf;
    Ouf.open((meshName + ".mesh").c_str());
    Ouf << Mesh().nc << std::endl << Mesh().con << std::endl;
    Ouf.close();
    std::ofstream vtkOut;
    vtkOut.open((meshName + ".vtk").c_str());
    SolverUtils::Mesh::WriteVTKToStream(meshName, Mesh(), vtkOut);
    vtkOut.close();

    int nNodes = Mesh().nc.NNodes();
    int nElem = Mesh().con.Nelem();

    nodeCoordinates1.resize(3 * nNodes);
    nodeCoordinates2.resize(3 * nNodes);
    cellCoordinates1.resize(3 * nElem);
    cellCoordinates2.resize(3 * nElem);
    nodeFunction1.resize(nNodes);
    cellFunction1.resize(nElem);
    nodeFunction2.resize(nNodes);
    cellFunction2.resize(nElem);

    Solution().Meta().AddField("nodeCoordinates1", 'n', 3, 8, "m");
    Solution().Meta().AddField("nodeCoordinates2", 'n', 3, 8, "m");
    Solution().Meta().AddField("cellCoordinates1", 'c', 3, 8, "m");
    Solution().Meta().AddField("cellCoordinates2", 'c', 3, 8, "m");
    Solution().Meta().AddField("cellFunction1", 'c', 1, 8, "m");
    Solution().Meta().AddField("cellFunction2", 'c', 1, 8, "m");
    Solution().Meta().AddField("nodeFunction1", 'n', 1, 8, "m");
    Solution().Meta().AddField("nodeFunction2", 'n', 1, 8, "m");

    CreateSoln();

    Solution().SetFieldBuffer("nodeCoordinates1", nodeCoordinates1);
    Solution().SetFieldBuffer("nodeCoordinates2", nodeCoordinates2);
    Solution().SetFieldBuffer("cellCoordinates1", cellCoordinates1);
    Solution().SetFieldBuffer("cellCoordinates2", cellCoordinates2);
    Solution().SetFieldBuffer("nodeFunction1", nodeFunction1);
    Solution().SetFieldBuffer("nodeFunction2", nodeFunction2);
    Solution().SetFieldBuffer("cellFunction1", cellFunction1);
    Solution().SetFieldBuffer("cellFunction2", cellFunction2);
    name = solverName;
    SolverUtils::RegisterSolver(solverName, *this);
  }
};

// Testing Fixture class for testing Linear Data Transfer between
class COMQuadraticDataTransfer : public ::testing::Test {
 public:
  // Per-test-suite set-up.
  // Called before the first test in this test suite.
  // Can be omitted if not needed.
  static void SetUpTestCase() {
    COM_init(&ARGC, &ARGV);

    std::vector<std::vector<double> > meshSpacing(3);
    std::vector<std::vector<double> > absoluteError(6);
    std::vector<std::vector<double> > relativeError(6);
    std::vector<int> targetMeshIndex;
    std::vector<int> sourceMeshIndex;

    for (int iTrial = 1; iTrial < 16; iTrial *= 2) {
      int nTri1 = 9 * iTrial;
      int nQuad = 7 * iTrial;
      int nTri2 = 5 * iTrial;

      TransferSolver testSolver1;
      TransferSolver testSolver2;
      TransferSolver testSolver3;

      meshSpacing[0].push_back(1.0 / static_cast<double>(nTri1 - 1));
      meshSpacing[1].push_back(1.0 / static_cast<double>(nQuad - 1));
      meshSpacing[2].push_back(1.0 / static_cast<double>(nTri2 - 1));

      // Generates a triangulated mesh by bisecting rectangles
      std::ostringstream meshOut;
      meshOut << "1"
              << "\n"
              << "0 5 " << nTri1 << "\n"
              << "0 5 " << nTri1 << "\n"
              << "0 0 0" << std::endl;
      testSolver1.Setup(meshOut.str(), "transSolver1", "testTriangle1");

      // Generate a square element mesh
      meshOut.str("");
      meshOut.clear();
      meshOut << "0"
              << "\n"
              << "0 5 " << nQuad << "\n"
              << "0 5 " << nQuad << "\n"
              << "0 0 0" << std::endl;
      testSolver2.Setup(meshOut.str(), "transSolver2", "testSquare");

      // Generate a second triangle mesh by quartering rectangles
      meshOut.str("");
      meshOut.clear();
      meshOut << "2"
              << "\n"
              << "0 5 " << nTri2 << "\n"
              << "0 5 " << nTri2 << "\n"
              << "0 0 0" << std::endl;
      testSolver3.Setup(meshOut.str(), "transSolver3", "testTriangle2");
      SolverUtils::TransferObject transfer11("transfer11");
      SolverUtils::TransferObject transfer12("transfer12");
      SolverUtils::TransferObject transfer13("transfer13");
      SolverUtils::TransferObject transfer23("transfer23");

      transfer11.Overlay("transSolver1", "transSolver1");
      transfer12.Overlay("transSolver1", "transSolver2");
      transfer13.Overlay("transSolver1", "transSolver3");
      transfer23.Overlay("transSolver2", "transSolver3");

      // This is the "name" of the mesh coordinates array in the CI 'Window'
      std::string solver1MeshCoordinatesName("transSolver1.nc");
      std::string solver2MeshCoordinatesName("transSolver2.nc");
      std::string solver3MeshCoordinatesName("transSolver3.nc");

      // "Get" the mesh coordinates array from COM for each of the "solvers"
      double *solver1MeshCoordinates = NULL;
      double *solver2MeshCoordinates = NULL;
      double *solver3MeshCoordinates = NULL;
      COM_get_array(solver1MeshCoordinatesName.c_str(), 101,
                    &solver1MeshCoordinates);
      COM_get_array(solver2MeshCoordinatesName.c_str(), 101,
                    &solver2MeshCoordinates);
      COM_get_array(solver3MeshCoordinatesName.c_str(), 101,
                    &solver3MeshCoordinates);

      std::vector<double> &solver1Soln(testSolver1.NodeCoordinates2());
      std::vector<double> &solver1Comp(testSolver1.NodeCoordinates1());
      std::vector<double> &solver2Soln(testSolver2.NodeCoordinates2());
      std::vector<double> &solver2Comp(testSolver2.NodeCoordinates1());
      std::vector<double> &solver3Soln(testSolver3.NodeCoordinates2());
      std::vector<double> &solver3Comp(testSolver3.NodeCoordinates1());

      // Reset the receiving arrays with some conspicious value
      ResetSolution(solver1Soln, -1);
      ResetSolution(solver2Soln, -1);
      ResetSolution(solver3Soln, -1);

      // Set up quadratic values
      std::vector<double>::iterator nc1It = solver1Comp.begin();
      while (nc1It != solver1Comp.end()) {
        double squareValue1 =
            ((*solver1MeshCoordinates) * (*solver1MeshCoordinates));
        *nc1It++ = squareValue1;
        solver1MeshCoordinates++;
      }
      std::vector<double>::iterator nc2It = solver2Comp.begin();
      while (nc2It != solver2Comp.end()) {
        double squareValue2 =
            ((*solver2MeshCoordinates) * (*solver2MeshCoordinates));
        *nc2It++ = squareValue2;
        solver2MeshCoordinates++;
      }
      std::vector<double>::iterator nc3It = solver3Comp.begin();
      while (nc3It != solver3Comp.end()) {
        double squareValue3 =
            ((*solver3MeshCoordinates) * (*solver3MeshCoordinates));
        *nc3It++ = squareValue3;
        solver3MeshCoordinates++;
      }

      // Transfer coordinate data to different meshes and make sure that the
      // new coordinates match the actual coordinates!
      std::vector<double> errors;
      std::cout << "Transferring quadratic data from Triangles 1 to Quads..."
                << std::flush;
      transfer12.Transfer("nodeCoordinates1", "nodeCoordinates2");
      std::cout << "done." << std::endl;
      GetErrors(solver2Comp, solver2Soln, errors);
      absoluteError[0].push_back(errors[1]);
      relativeError[0].push_back(errors[4]);
      ResetSolution(solver2Soln, -1);
      sourceMeshIndex.push_back(0);
      targetMeshIndex.push_back(1);
      std::cout << "\"Error\" for quadratic: " << errors[1] << " " << errors[4]
                << std::endl;

      std::cout << "Transferring quadratic data from Quads to Triangles 1..."
                << std::flush;
      transfer12.Transfer("nodeCoordinates1", "nodeCoordinates2", true);
      std::cout << "done." << std::endl;
      GetErrors(solver1Comp, solver1Soln, errors);
      absoluteError[1].push_back(errors[1]);
      relativeError[1].push_back(errors[4]);
      targetMeshIndex.push_back(0);
      sourceMeshIndex.push_back(1);
      ResetSolution(solver1Soln, -1);

      std::cout
          << "Transferring quadratic data from Triangles 1 to Triangles 2..."
          << std::flush;
      transfer13.Transfer("nodeCoordinates1", "nodeCoordinates2");
      std::cout << "done. " << std::endl;
      GetErrors(solver3Comp, solver3Soln, errors);
      absoluteError[2].push_back(errors[1]);
      relativeError[2].push_back(errors[4]);
      targetMeshIndex.push_back(2);
      sourceMeshIndex.push_back(0);
      ResetSolution(solver3Soln, -1);

      std::cout
          << "Transferring quadratic data from Triangles 2 to Triangles 1..."
          << std::flush;
      transfer13.Transfer("nodeCoordinates1", "nodeCoordinates2", true);
      std::cout << "done. " << std::endl;
      GetErrors(solver1Comp, solver1Soln, errors);
      absoluteError[3].push_back(errors[1]);
      relativeError[3].push_back(errors[4]);
      targetMeshIndex.push_back(0);
      sourceMeshIndex.push_back(2);
      ResetSolution(solver1Soln, -1);

      std::cout << "Transferring quadratic data from Quads to Triangles 2..."
                << std::flush;
      transfer23.Transfer("nodeCoordinates1", "nodeCoordinates2");
      std::cout << "done." << std::endl;
      GetErrors(solver3Comp, solver3Soln, errors);
      absoluteError[4].push_back(errors[1]);
      relativeError[4].push_back(errors[4]);
      targetMeshIndex.push_back(2);
      sourceMeshIndex.push_back(1);
      ResetSolution(solver3Soln, -1);

      std::cout << "Transferring quadratic data from Triangles 2 to Quads..."
                << std::flush;
      transfer23.Transfer("nodeCoordinates1", "nodeCoordinates2", true);
      std::cout << "done." << std::endl;
      GetErrors(solver2Comp, solver2Soln, errors);
      absoluteError[5].push_back(errors[1]);
      relativeError[5].push_back(errors[4]);
      targetMeshIndex.push_back(1);
      sourceMeshIndex.push_back(2);
      ResetSolution(solver2Soln, -1);
    }

    int nMeshes = meshSpacing.size();
    int nTransferTypes = absoluteError.size();
    nTrial = new int(absoluteError[0].size());
    std::vector<double> logSpacing1(nMeshes, 0);
    std::vector<double> logError1(nTransferTypes, 0);
    for (int j = 0; j < nMeshes; j++) {
      logSpacing1[j] = std::log(meshSpacing[j][0]);
    }
    for (int j = 0; j < nTransferTypes; j++) {
      logError1[j] = std::log(absoluteError[j][0]);
    }
    // create 2D vector array which stores the errorOrder for each meshTransfer
    // test trial there are six types of transfers and 4 different mesh sizes
    errorOrder = new std::vector<std::vector<double> >(6);

    for (int i = 1; i < *nTrial; i++) {
      std::vector<double> logSpacing2(nMeshes, 0);
      std::vector<double> dlogSpacing(nMeshes, 0);
      for (int j = 0; j < nMeshes; j++) {
        double logSpacing2 = std::log(meshSpacing[j][i]);
        dlogSpacing[j] = logSpacing2 - logSpacing1[j];
        logSpacing1[j] = logSpacing2;
      }

      for (int j = 0; j < nTransferTypes; j++) {
        int targetMesh = targetMeshIndex[j] + 1;
        int sourceMesh = sourceMeshIndex[j] + 1;
        double logError2 = std::log(absoluteError[j][i]);
        double dlogError = logError2 - logError1[j];
        logError1[j] = logError2;
        (*errorOrder)[j].push_back(dlogError / dlogSpacing[sourceMesh - 1]);
        std::cout << i << ": " << sourceMesh << "," << targetMesh << ": "
                  << (*errorOrder)[j].back() << std::endl;
      }
    }
  }
  // Per-test-suite tear-down.
  // Called after the last test in this test suite.
  // Can be omitted if not needed.
  static void TearDownTestCase() { COM_finalize(); }

 protected:
  COMQuadraticDataTransfer() {}
  virtual ~COMQuadraticDataTransfer() {}
  virtual void SetUp() {
    // MPI_Init(&ARGC,&ARGV);
  }
  virtual void
  TearDown() {  // transferObjects must be deleted before finalizing COM
    // MPI_Finalize();
  }
  // shared resources for the test fixture
  static int *nTrial;
  static std::vector<std::vector<double> > *errorOrder;
};

int *COMQuadraticDataTransfer::nTrial = nullptr;
std::vector<std::vector<double> > *COMQuadraticDataTransfer::errorOrder =
    nullptr;

// Tests which assess the errorOrder magnitude, the transfers themselves happen
// in SetUpTestSuite()

TEST_F(COMQuadraticDataTransfer, Triangle1ToQuads) {
  for (short i = 0; i < *nTrial - 1; i++) {
    EXPECT_GE((*errorOrder)[0][i], 1.9)
        << "ErrorOrder smaller than 1.9 indicates a computational error, "
           "iteration:"
        << i << "\n";
    ;
    EXPECT_LE((*errorOrder)[0][i], 2.3)
        << "ErrorOrder larger than 2.3 indicates a computational error, "
           "iteration:"
        << i << "\n";
  }
}

TEST_F(COMQuadraticDataTransfer, QuadsToTriangle1) {
  for (short i = 0; i < *nTrial - 1; i++) {
    EXPECT_GE((*errorOrder)[1][i], 1.9)
        << "ErrorOrder smaller than 1.9 indicates a computational error, "
           "iteration:"
        << i << "\n";
    EXPECT_LE((*errorOrder)[1][i], 2.3)
        << "ErrorOrder larger than 2.3 indicates a computational error, "
           "iteration:"
        << i << "\n";
  }
}

TEST_F(COMQuadraticDataTransfer, Triangle1ToTriangle2) {
  for (short i = 0; i < *nTrial - 1; i++) {
    EXPECT_GE((*errorOrder)[2][i], 1.9)
        << "ErrorOrder smaller than 1.9 indicates a computational error, "
           "iteration:"
        << i << "\n";
    EXPECT_LE((*errorOrder)[2][i], 2.3)
        << "ErrorOrder larger than 2.3 indicates a computational error, "
           "iteration:"
        << i << "\n";
  }
}

TEST_F(COMQuadraticDataTransfer, Triangle2ToTriangle1) {
  for (short i = 0; i < *nTrial - 1; i++) {
    EXPECT_GE((*errorOrder)[3][i], 1.9)
        << "ErrorOrder smaller than 1.9 indicates a computational error, "
           "iteration:"
        << i << "\n";
    EXPECT_LE((*errorOrder)[3][i], 2.3)
        << "ErrorOrder larger than 2.3 indicates a computational error, "
           "iteration:"
        << i << "\n";
  }
}

TEST_F(COMQuadraticDataTransfer, QuadsToTriangle2) {
  for (short i = 0; i < *nTrial - 1; i++) {
    EXPECT_GE((*errorOrder)[4][i], 1.9)
        << "ErrorOrder smaller than 1.9 indicates a computational error, "
           "iteration:"
        << i << "\n";
    EXPECT_LE((*errorOrder)[4][i], 2.3)
        << "ErrorOrder larger than 2.3 indicates a computational error, "
           "iteration:"
        << i << "\n";
  }
}

TEST_F(COMQuadraticDataTransfer, Triangle2ToQuads) {
  for (short i = 0; i < *nTrial - 1; i++) {
    EXPECT_GE((*errorOrder)[5][i], 1.9)
        << "ErrorOrder smaller than 1.9 indicates a computational error, "
           "iteration:"
        << i << "\n";
    EXPECT_LE((*errorOrder)[5][i], 2.3)
        << "ErrorOrder larger than 2.3 indicates a computational error, "
           "iteration:"
        << i << "\n";
  }
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}