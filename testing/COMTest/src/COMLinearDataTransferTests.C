#include <iostream>
#include <sstream>
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

// class used to store data about the Mesh Transfer
class TransferSolver : public SolverUtils::FEM::SolverAgent {
 public:
  void Init(const std::string &filename);
  ~TransferSolver() { SolverUtils::UnRegisterSolver(name); }

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
  int NNodes() { return (Mesh().nc.NNodes()); }
  int NElem() { return (Mesh().con.Nelem()); }
  std::vector<double> &NodeCoordinates1() { return (nodeCoordinates1); }
  std::vector<double> &NodeCoordinates2() { return (nodeCoordinates2); }
  std::vector<double> &CellCoordinates1() { return (cellCoordinates1); }
  std::vector<double> &CellCoordinates2() { return (cellCoordinates2); }

  std::vector<double> &NodeFunction1() { return (nodeFunction1); }
  std::vector<double> &NodeFunction2() { return (nodeFunction2); }
  std::vector<double> &CellFunction1() { return (cellFunction1); }
  std::vector<double> &CellFunction2() { return (cellFunction2); }

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
class COMLinearDataTransfer : public ::testing::Test {
 protected:
  COMLinearDataTransfer() {}
  virtual ~COMLinearDataTransfer() {}
  static void SetUpTestCase() {
    COM_init(&ARGC, &ARGV);

    testSolver1 = new TransferSolver();
    testSolver2 = new TransferSolver();
    testSolver3 = new TransferSolver();
    // set the tolerance value
    compTol = new double(1e-3);
    // Generates a triangulated mesh by bisecting rectangles
    std::ostringstream meshOut;
    meshOut << "1"
            << "\n"
            << "0 5 10"
            << "\n"
            << "0 5 10"
            << "\n"
            << "0 0 0" << std::endl;
    testSolver1->Setup(meshOut.str(), "transSolver1", "testTriangle1");

    // Generate a square element mesh
    meshOut.str("");
    meshOut.clear();
    meshOut << "0"
            << "\n"
            << "0 5 23"
            << "\n"
            << "0 5 23"
            << "\n"
            << "0 0 0" << std::endl;
    testSolver2->Setup(meshOut.str(), "transSolver2", "testSquare");

    // Generate a second triangle mesh by quartering rectangles
    meshOut.str("");
    meshOut.clear();
    meshOut << "2"
            << "\n"
            << "0 5 13"
            << "\n"
            << "0 5 13"
            << "\n"
            << "0 0 0" << std::endl;
    testSolver3->Setup(meshOut.str(), "transSolver3", "testTriangle2");

    // TransferObject Constructor loads the SurfX Module and gets several
    // function handles the parameters are the names of the windows
    t11 = new SolverUtils::TransferObject("transfer11");
    t12 = new SolverUtils::TransferObject("transfer12");
    t13 = new SolverUtils::TransferObject("transfer13");
    t23 = new SolverUtils::TransferObject("transfer23");

    ASSERT_NE(-1, t11->Overlay("transSolver1", "transSolver1"))
        << "DataTransfer11 Overlay was not created succesfully\n";
    ASSERT_NE(-1, t12->Overlay("transSolver1", "transSolver2"))
        << "DataTransfer12 Overlay was not created succesfully\n";
    ASSERT_NE(-1, t13->Overlay("transSolver1", "transSolver3"))
        << "DataTransfer13 Overlay was not created succesfully\n";
    ASSERT_NE(-1, t23->Overlay("transSolver2", "transSolver3"))
        << "DataTransfer23 Overlay was not created succesfully\n";

    // This is the "name" of the mesh coordinates array in the CI 'Window'
    std::string solver1MeshCoordinatesName("transSolver1.nc");
    std::string solver2MeshCoordinatesName("transSolver2.nc");
    std::string solver3MeshCoordinatesName("transSolver3.nc");
    std::string solver1ConnectivityName("transSolver1.:t3:");
    std::string solver2ConnectivityName("transSolver2.:q4:");
    std::string solver3ConnectivityName("transSolver3.:t3:");

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
    // make sure the coordinates were gotten correctly
    ASSERT_NE(nullptr, solver1MeshCoordinates)
        << "Mesh Coordinates were incorrectly gotten from COM" << std::endl;
    ASSERT_NE(nullptr, solver2MeshCoordinates)
        << "Mesh Coordinates were incorrectly gotten from COM" << std::endl;
    ASSERT_NE(nullptr, solver3MeshCoordinates)
        << "Mesh Coordinates were incorrectly gotten from COM" << std::endl;

    // "Get" the element connectivities for each "solver"
    int *solver1Connectivity = NULL;
    int *solver2Connectivity = NULL;
    int *solver3Connectivity = NULL;
    COM_get_array(solver1ConnectivityName.c_str(), 101, &solver1Connectivity);
    COM_get_array(solver2ConnectivityName.c_str(), 101, &solver2Connectivity);
    COM_get_array(solver3ConnectivityName.c_str(), 101, &solver3Connectivity);
    // make sure the connectivities were gotten correctly
    ASSERT_NE(nullptr, solver1Connectivity)
        << "Mesh Connectivities were incorrectly gotten from COM" << std::endl;
    ASSERT_NE(nullptr, solver2Connectivity)
        << "Mesh Connectivities were incorrectly gotten from COM" << std::endl;
    ASSERT_NE(nullptr, solver3Connectivity)
        << "Mesh Connectivities were incorrectly gotten from COM" << std::endl;

    // Copy the mesh coordinate data into the first solution array (i.e.
    // nodeCoordinates1) for each solver.
    std::vector<double>::iterator nc1It =
        testSolver1->NodeCoordinates1().begin();
    while (nc1It != testSolver1->NodeCoordinates1().end())
      *nc1It++ = *solver1MeshCoordinates++;

    std::vector<double>::iterator nc2It =
        testSolver2->NodeCoordinates1().begin();
    while (nc2It != testSolver2->NodeCoordinates1().end())
      *nc2It++ = *solver2MeshCoordinates++;

    std::vector<double>::iterator nc3It =
        testSolver3->NodeCoordinates1().begin();
    while (nc3It != testSolver3->NodeCoordinates1().end())
      *nc3It++ = *solver3MeshCoordinates++;
  }
  void SetUp() {
    // MPI_Init(&ARGC,&ARGV);
  }
  static void TearDownTestCase() {
    // transferObjects must be deleted before finalizing COM
    delete compTol;
    delete testSolver1;
    delete testSolver2;
    delete testSolver3;
    delete t11;
    delete t12;
    delete t13;
    delete t23;
    COM_finalize();
  }
  void TearDown() {
    // MPI_Finalize();
  }
  static double *compTol;
  static TransferSolver *testSolver1;
  static TransferSolver *testSolver2;
  static TransferSolver *testSolver3;
  static SolverUtils::TransferObject *t11;
  static SolverUtils::TransferObject *t12;
  static SolverUtils::TransferObject *t13;
  static SolverUtils::TransferObject *t23;
};

double *COMLinearDataTransfer::compTol = nullptr;
TransferSolver *COMLinearDataTransfer::testSolver1 = nullptr;
TransferSolver *COMLinearDataTransfer::testSolver2 = nullptr;
TransferSolver *COMLinearDataTransfer::testSolver3 = nullptr;
SolverUtils::TransferObject *COMLinearDataTransfer::t11 = nullptr;
SolverUtils::TransferObject *COMLinearDataTransfer::t12 = nullptr;
SolverUtils::TransferObject *COMLinearDataTransfer::t13 = nullptr;
SolverUtils::TransferObject *COMLinearDataTransfer::t23 = nullptr;

//
// Linear function transfers - transferring the nodal coordinates themselves
// is a linear function transfer. The following transfers do the coordinate
// transfers between the various meshes.
//

TEST_F(COMLinearDataTransfer, DirectInjection) {
  std::vector<double> &tri1Comp(testSolver1->NodeCoordinates1());
  std::vector<double> &tri1Soln(testSolver1->NodeCoordinates2());
  EXPECT_EQ(0, t11->Transfer("nodeCoordinates1", "nodeCoordinates2"))
      << "Transfer of nodal coordinates between Triangular mesh 1 and"
      << "Triangular mesh 1 failed\n";
  bool match = TwoSolutionsMatch(tri1Comp, tri1Soln, *compTol);
  EXPECT_TRUE(match)
      << "Solution for Triangular mesh does not match for tolerance:" << compTol
      << "\n";
  if (!match)
    CompareSolutionsWithDetail(tri1Comp, tri1Soln, std::cout, *compTol);
}

TEST_F(COMLinearDataTransfer, Triangle1ToQuads) {
  std::vector<double> &quadComp(testSolver2->NodeCoordinates1());
  std::vector<double> &quadSoln(testSolver2->NodeCoordinates2());
  EXPECT_EQ(0, t12->Transfer("nodeCoordinates1", "nodeCoordinates2"))
      << "Transfer of nodal coordinates between triangular mesh 1"
      << "and Quad mesh failed\n";
  bool match = TwoSolutionsMatch(quadComp, quadSoln, *compTol);
  EXPECT_TRUE(match) << "Solution for Quad mesh does not match for tolerance:"
                     << compTol << "\n";
  if (!match)
    CompareSolutionsWithDetail(quadComp, quadSoln, std::cout, *compTol);
}

TEST_F(COMLinearDataTransfer, QuadsToTriangle1) {
  std::vector<double> &tri1Comp(testSolver1->NodeCoordinates1());
  std::vector<double> &tri1Soln(testSolver1->NodeCoordinates2());
  EXPECT_EQ(0, t12->Transfer("nodeCoordinates1", "nodeCoordinates2", true))
      << "Transfer of nodal coordinates between Quad mesh"
      << "and Triangular mesh 1 failed\n";
  bool match = TwoSolutionsMatch(tri1Comp, tri1Soln, *compTol);
  EXPECT_TRUE(match)
      << "Solution for Triangular mesh1 does not match for tolerance:"
      << compTol << "\n";
  if (!match)
    CompareSolutionsWithDetail(tri1Comp, tri1Soln, std::cout, *compTol);
}

TEST_F(COMLinearDataTransfer, Triangle1ToTriangle2) {
  std::vector<double> &tri2Comp(testSolver3->NodeCoordinates1());
  std::vector<double> &tri2Soln(testSolver3->NodeCoordinates2());
  EXPECT_EQ(0, t13->Transfer("nodeCoordinates1", "nodeCoordinates2"))
      << "Transfer of nodal coordinates between Triangular mesh 1"
      << "and Triangular mesh 2 failed\n";
  bool match = TwoSolutionsMatch(tri2Comp, tri2Soln, *compTol);
  EXPECT_TRUE(match)
      << "Solutions for Triangular mesh2 do not match for tolerance:" << compTol
      << "\n";
  if (!match)
    CompareSolutionsWithDetail(tri2Comp, tri2Soln, std::cout, *compTol);
}

TEST_F(COMLinearDataTransfer, Triangle2ToTriangle1) {
  std::vector<double> &tri1Comp(testSolver1->NodeCoordinates1());
  std::vector<double> &tri1Soln(testSolver1->NodeCoordinates2());
  EXPECT_EQ(0, t13->Transfer("nodeCoordinates1", "nodeCoordinates2", true))
      << "Transfer of nodal coordinates between Triangular mesh 2"
      << "and Triangular mesh 1 failed\n";
  bool match = TwoSolutionsMatch(tri1Comp, tri1Soln, *compTol);
  EXPECT_TRUE(match)
      << "Solutions for Triangular mesh1 do not match for tolerance:" << compTol
      << "\n";
  if (!match)
    CompareSolutionsWithDetail(tri1Comp, tri1Soln, std::cout, *compTol);
}

TEST_F(COMLinearDataTransfer, QuadsToTriangle2) {
  std::vector<double> &tri2Comp(testSolver3->NodeCoordinates1());
  std::vector<double> &tri2Soln(testSolver3->NodeCoordinates2());
  EXPECT_EQ(0, t23->Transfer("nodeCoordinates1", "nodeCoordinates2"))
      << "Transfer of nodal coordinates between Quad mesh"
      << "and Triangle mesh 2 failed\n";
  bool match = TwoSolutionsMatch(tri2Comp, tri2Soln, *compTol);
  EXPECT_TRUE(match)
      << "Solution for Triangular mesh2 does not match for tolerance:"
      << compTol << "\n";
  if (!match)
    CompareSolutionsWithDetail(tri2Comp, tri2Soln, std::cout, *compTol);
}

TEST_F(COMLinearDataTransfer, Triangle2ToQuads) {
  std::vector<double> &quadComp(testSolver2->NodeCoordinates1());
  std::vector<double> &quadSoln(testSolver2->NodeCoordinates2());
  EXPECT_EQ(0, t23->Transfer("nodeCoordinates1", "nodeCoordinates2", true))
      << "Transfer of nodal coordinates between Triangular mesh 2"
      << "and Quad mesh failed\n";
  bool match = TwoSolutionsMatch(quadComp, quadSoln, *compTol);
  EXPECT_TRUE(match) << "Solution for Quad mesh does not match for tolerance:"
                     << compTol << "\n";
  if (!match)
    CompareSolutionsWithDetail(quadComp, quadSoln, std::cout, *compTol);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}