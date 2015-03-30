///
/// @file
/// @ingroup com_group
/// @brief Testing object for COM.
///
/// This file contains the implementation of the
/// overarching testing object for COM.
/// Every project should have one of these as
/// it allows external entities unified methodology
/// for access project or package-specific tests.
///
#ifndef __COM_TEST_H__
#define __COM_TEST_H__
#include "COMM.H"
#include "Testing.H"
//#include "ExampleHeader.H"
#include "Profiler.H"
#include <vector>
#include <cmath>
#include "com.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <time.h>
#include "Mesh.H"
#include "UnixUtils.H"

COM_EXTERN_MODULE( COMTESTMOD );
COM_EXTERN_MODULE( COMFTESTMOD );

namespace COM {

  ///
  /// Convenience typedef for CommunicatorObject
  ///
  typedef IRAD::Comm::CommunicatorObject CommType;
  ///
  /// Namespace for storing simple test utility fixtures
  ///
  namespace TestFixture {
    ///
    /// Simple test function fixture returns \f$2x\f$.
    /// 
    /// F1 is a 1st order function in x that has unity integral over [0,1].
    ///
    double F1(double x) { return (2.0*x); };
    ///
    /// Simple test function fixture returns \f$3x^2\f$.
    ///
    /// F2 is quadratic function in x and integrates to unity over [0,1].
    ///
    double F2(double x) { return (3.0*x*x); };
  };
  ///
  /// Project-specific test results type.
  ///
  /// Test results types can be custom implemented
  /// by the project developer.  The requirement
  /// is that they produce the proper test results
  /// output when used with outstreams.
  ///
  typedef IRAD::Util::TestResults TestResults;

  /// 
  /// Project-specific testing object.
  ///
  /// The project-specific testing object encapsulates
  /// all tests for the project. This is the interface
  /// to all testing available for the given project.
  /// It implements the interface given by 
  /// IRAD::Util::TestingObject.
  ///
  /// Every project should define a "TestingObject"
  /// which implements (or interfaces) all of the tests
  /// for the project.
  /// If necessary (e.g. for parallel tests through
  /// batch systems), this is the object responsible
  /// for spawning off children processes to conduct
  /// parallel tests and for gathering the results
  /// of those tests.
  ///
  template<typename ResultsType>
  class TestingObject : public IRAD::Util::TestingObject<ResultsType>
  {
    typedef IRAD::Util::TestingObject<ResultsType> TestingObjectBaseType;
  protected:
    // Example test fixtures for COM testing.
    //
    // In a real project, there would be many test fixtures
    // in the TestingObject.
    ///
    /// A sample string test fixture. 
    ///
    std::string ExampleTestFixture;
    ///
    /// A set of values for testing quadrature methods.
    /// 
    std::vector<int> N;
    ///
    /// An unstructured mesh for testing mesh-associated data
    ///
    SolverUtils::Mesh::UnstructuredMesh mesh;
  public:
    /// 
    /// Default constructor.
    ///
    TestingObject() : TestingObjectBaseType() {};

    ///
    /// Tears down the testing fixtures if needed.
    ///
    virtual void Epilogue() 
    {
      IRAD::Sys::Remove("test.mesh");
    };

    ///
    /// Sets up the data fixtures for the tests.
    /// 
    /// This function gives the object a chance to 
    /// allocate and set up any "fixtures" before the
    /// tests are run.
    ///
    virtual void  Prologue(){
      ExampleTestFixture.assign("ExampleTestData");
      for(int i = 10;i < 10000000;i*=10) N.push_back(i);

      // Generates a triangulated rectangle with 25 nodes, 32 elements
      mesh.nc.init(25);
      int nodes = 1;
      for(int j = 0;j < 5;j++){
        double y = j*1.25;
        for(int i = 0;i < 5;i++){
          double x = i*1.25;
          mesh.nc.x(nodes) = x;
          mesh.nc.y(nodes) = y;
          mesh.nc.z(nodes) = 0;
          nodes++;
        }
      }
      for(int j = 0;j < 4;j++){
        unsigned int r = j*5;
        for(int i = 1;i <= 4;i++){
          unsigned int f = i + r;
          mesh.con.AddElement(f,f+1,f+5);
          mesh.con.AddElement(f+1,f+6,f+5);
        }
      }
      // Write the test mesh to test.mesh
      std::ofstream Ouf;
      Ouf.open("test.mesh");
      Ouf << mesh.nc << std::endl << mesh.con << std::endl;
      Ouf.close();
    }
    ///
    /// Simple test function fixture returns \f$2x\f$.
    /// 
    /// F1 is a 1st order function in x that has unity integral over [0,1].
    ///
    double F1(double x) { return (2.0*x); };
    ///
    /// Simple test function fixture returns \f$3x^2\f$.
    ///
    /// F2 is quadratic function in x and integrates to unity over [0,1].
    ///
    double F2(double x) { return (3.0*x*x); };
    ///
    /// Test for COM_LOAD_MODULE_STATIC_DYNAMIC and COM_UNLOAD_MODULE_STATIC_DYNAMIC.
    ///
    /// @param result COM::TestResults object to store test results.
    ///
    /// This function implements several module management tests designed to 
    /// test dataitem management.  It uses the two testing modules COMTESTMOD and 
    /// COMFTESTMOD which are C++ and F90 respectively. 
    virtual void Test__DataItemManagement(ResultsType &result) {

      COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,"Solver1");

      int InitHandle = COM_get_function_handle("Solver1.Init");
      int DumpHandle = COM_get_function_handle("Solver1.Dump");
      int ValidateHandle = COM_get_function_handle("Solver1.ValidateAddress");
      int StepHandle = COM_get_function_handle("Solver1.Step");

      std::string meshname("test.mesh");
      std::string filename("Solver1");
      int use_timestamp = 0;
      // This init function registers a bunch of DataItems
      COM_call_function(InitHandle,&meshname);
      COM_call_function(DumpHandle,&filename,&use_timestamp);
      std::string dataitem_names;
      int number_of_dataitems = 0;
      COM_get_dataitems("Solver1",&number_of_dataitems,dataitem_names);
      std::istringstream Istr(dataitem_names);
      std::vector<std::string> expected_dataitem_names;
      expected_dataitem_names.push_back("global");
      expected_dataitem_names.push_back("displacement");
      expected_dataitem_names.push_back("temperature");
      expected_dataitem_names.push_back("nodeflags");
      expected_dataitem_names.push_back("cellflags");
      expected_dataitem_names.push_back("pressure");
      expected_dataitem_names.push_back("id");
      expected_dataitem_names.push_back("time");
      expected_dataitem_names.push_back("dt");
      std::cout << "COM::Test__DataItemManagement::DataItems:" << std::endl;
      for(int i = 0;i < number_of_dataitems;i++){
        std::string dataitem_name;
        Istr >> dataitem_name;
        std::cout << dataitem_name << std::endl;
      }
      std::vector<std::string>::iterator edit = expected_dataitem_names.begin();
      bool get_dataitems_works = true;
      bool nonlocal_data_works = true;
      while((edit != expected_dataitem_names.end())&&get_dataitems_works){
        std::string::size_type x = dataitem_names.find(*edit);
        if(x == std::string::npos)
          get_dataitems_works = false;
        else if(*edit != "global" && ValidateHandle){
          int result=0;
          int myaddress;
          std::string full_data_name("Solver1." + *edit);
          std::string data_name(*edit);
   
          char test_loc;
          COM_Type get_dataitem_type;
          int get_dataitem_size;
          std::string get_dataitem_units;
          COM_get_dataitem(full_data_name,&test_loc,&get_dataitem_type,&get_dataitem_size,&get_dataitem_units);
          void* testarray_get;
          if(test_loc == 'w')
            COM_get_array(full_data_name.c_str(),0,&testarray_get);
          else
            COM_get_array(full_data_name.c_str(),101,&testarray_get);

          COM_call_function(ValidateHandle,&data_name,testarray_get,&result);
          if(result != 1)
            nonlocal_data_works = false;
        }
        *edit++;
      }
      result.UpdateResult("COM::GetNonLocalDataItem:Works", nonlocal_data_works);
      result.UpdateResult("COM::GetDataItems:Works",get_dataitems_works);
      
      bool get_dataitem_handle_works = true;
      while((edit != expected_dataitem_names.end()) && get_dataitem_handle_works){ 
        std::string datastring = *edit;
        int dataitem_handle = COM_get_dataitem_handle(datastring.c_str());
        if(dataitem_handle <= 0)
          get_dataitem_handle_works = false; 
      }
      result.UpdateResult("COM::GetDataItemHandle:Works",get_dataitem_handle_works);

      for(int i = 0;i < 100;i++){
        COM_call_function(StepHandle);
        COM_call_function(DumpHandle,&filename,&use_timestamp);
      }
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,"Solver1");
      result.UpdateResult("COM::DataItemManagement:Runs",true);


      //Test for COM_get_size, COM_get_array, and COM_get_dataitem
      COM_new_window("testwindow");
      int testarray_size = 20;       
      std::vector<int> testarray(testarray_size);
      for(int i=0; i < testarray_size; i++)
        testarray[i] = i*i*3 - 2*i;  
      COM_new_dataitem("testwindow.array",'w',COM_DOUBLE,testarray_size,"");
      COM_set_size("testwindow.array",0,testarray_size);
      COM_set_array("testwindow.array",0,&testarray[0]);

      char test_loc = 'w';
      COM_Type get_dataitem_type;
      int get_dataitem_size;
      std::string get_dataitem_units;
      COM_get_dataitem("testwindow.array",&test_loc,&get_dataitem_type,&get_dataitem_size,&get_dataitem_units);
      


      bool get_dataitem_works = true;
      if(get_dataitem_type != COM_DOUBLE)
        get_dataitem_works = false;
      if(get_dataitem_size != testarray_size)
        get_dataitem_works = false;
      if(get_dataitem_units != "")
        get_dataitem_works = false;
      result.UpdateResult("COM::GetDataItem:Works",get_dataitem_works);
 
      int testarray_get_size;
      bool get_array_works = true;
      COM_get_size("testwindow.array",0,&testarray_get_size);
      if(testarray_get_size == testarray_size)
        result.UpdateResult("COM::GetSize:Works",true);
     
      int* testarray_get;
      COM_get_array("testwindow.array",0,&testarray_get);
      if(testarray_get != &testarray[0])
        get_array_works = false;
      else{
        for(int i=0; i < testarray_get_size; i++){
          if(testarray_get[i] != i*i*3 - 2*i){
            get_array_works = false;
            break;
          }
        }
      }
      result.UpdateResult("COM::GetArray:Works",get_array_works); 
    }

    virtual void Test__LoadUnloadMemTest(ResultsType &result) {
      // Unload/Load a bunch of times for memory test purposes
      for(int i = 0;i < 1000;i++){
        COM_LOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestWin1");
        COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestWin1");
      }
      result.UpdateResult("CModuleLoadingMemTest:Runs", true);
      
      for(int i = 0;i < 1000;i++){
        COM_LOAD_MODULE_STATIC_DYNAMIC( COMFTESTMOD, "TestFWin1");
        COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin1");
      }
      result.UpdateResult("FModuleLoadingMemTest:Runs", true);

    }

    ///
    /// @brief Module management tests.
    ///
    /// @param result COM::TestResults object to store test results.
    ///
    /// This function implements several module management tests designed to 
    /// put module loading, unloading, and management through their paces.  It uses
    /// the two testing modules COMTESTMOD and COMFTESTMOD which are C++ and F90 
    /// respectively. Creating, obtaining and destroying modules is tested in both
    /// languages including recursive tests wherein modules load modules.
    virtual void Test__ModuleLoadingTest(ResultsType &result) {
      std::cout << "Running COM::TestingObject::Test__ModuleLoadingTest" << std::endl;
      // This is an actual test of the functions called 
      // COM_LOAD_MODULE_STATIC_DYNAMIC and COM_UNLOAD_MODULE_STATIC_DYNAMIC.   
      // The name Test__XXXXX will
      // eventually help automated utilities with 
      // running tests by name
      COM_LOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestWin1");
      COM_LOAD_MODULE_STATIC_DYNAMIC( COMFTESTMOD, "TestFWin1");

      bool c_window_exists=true;
      bool f_window_exists=true;

      int h=COM_get_window_handle("TestWin1");
      if(h <= 0) {
        std::cout << "COM_get_window_handle(\"TestWin1\") returns " 
                  << h << std::endl;
        c_window_exists=false;
      }

      int k=COM_get_window_handle("TestFWin1");
      if(k <= 0) {
        std::cout << "COM_get_window_handle(\"TestFWin1\") returns " 
                  << k << std::endl;
        f_window_exists=false;
      }
      
      result.UpdateResult("LoadModule:WorksForCModules", c_window_exists);
      result.UpdateResult("LoadModule:WorksForFortranModules", f_window_exists);
      
      // This is an actual test of the function called 
      // COM_UNLOAD_MODULE_STATIC_DYNAMIC for the C Test Module. 
      if(c_window_exists){
        COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestWin1");
        
        h = COM_get_window_handle("TestWin1");
        if(h > 0) {
          std::cout << "COM_get_window_handle(\"TestWin1\") returns " << h 
                    << " for unloaded module." << std::endl;
          c_window_exists=false;
        }
        result.UpdateResult("UnLoadModule:WorksForCModules", c_window_exists);
        c_window_exists = !c_window_exists;
      }
      if(!c_window_exists)
        COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,"TestWin1");
      h = COM_get_window_handle("TestWin1");
      if(h > 0)
        c_window_exists = true;

      if(f_window_exists){
        // This is an actual test of the function called 
        // COM_UNLOAD_MODULE_STATIC_DYNAMIC for the Fortran Test Module.   
        COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMFTESTMOD, "TestFWin1");
        k = COM_get_window_handle("TestFWin1");
        if(k > 0) {
          std::cout << "COM_get_window_handle(\"TestFWin1\") returns " << k 
                    << " for unloaded module." << std::endl;
          f_window_exists=false;
        }
      
        result.UpdateResult("UnLoadModule:WorksForFortranModules", f_window_exists);
        f_window_exists = !f_window_exists;
      }
      if(!f_window_exists)
        COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD,"TestFWin1");
      k = COM_get_window_handle("TestFWin1");
      if(k > 0)
        f_window_exists = true;
    
      if(c_window_exists){
        // This is a test of calling CModule function1.
        // CModule function1 recursively loads the CModule.
        // This tests verifies both the loading of the second
        // CModule and the unloading of it upon unloading the
        // original CModule. 
        int CModFunction1_handle = COM_get_function_handle("TestWin1.Function1");
        if(CModFunction1_handle > 0){
          std::string win2("TestWin2");
          COM_call_function(CModFunction1_handle,&win2);
          int win2_handle = COM_get_window_handle(win2.c_str());
          if(win2_handle > 0) {
            result.UpdateResult("CModuleLoadsCModule",true);
            COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD,"TestWin1");
            win2_handle = COM_get_window_handle(win2.c_str());
            if(win2_handle <= 0)
              result.UpdateResult("CModuleDestroysCModuleOnUnload",true);
            COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,"TestWin1");
            h = COM_get_window_handle("TestWin1");
            if(h < 0)
              c_window_exists = false;
          }
        }
      }

      if(f_window_exists){
        // This is a test of calling the Fortran Test Module function1.
        // FModule function1 recursively loads the FModule.
        // This tests verifies both the loading of the second
        // FModule and the unloading of it upon unloading the
        // original FModule. 
         int FModFunction1_handle = COM_get_function_handle("TestFWin1.Function1");
        if(FModFunction1_handle > 0){
          std::string win2("TestFWin2");
          const char *win2ptr = win2.c_str();
          COM_call_function(FModFunction1_handle,win2ptr);
          int win2_handle = COM_get_window_handle(win2.c_str());
          if(win2_handle > 0) {
            result.UpdateResult("FModuleLoadsFModule",true);
            COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMFTESTMOD,"TestFWin1");
            win2_handle = COM_get_window_handle(win2.c_str());
            if(win2_handle <= 0)
              result.UpdateResult("FModuleDestroysFModuleOnUnload",true);
            COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD,"TestFWin1");
            h = COM_get_window_handle("TestFWin1");
            if(h < 0)
              f_window_exists = false;
          }
        }
      }
  
      if(c_window_exists){
        // This is a test of calling CModule function3.
        // CModule function3 loads the Fortran Test Module.
        // This tests verifies both the loading of the
        // Fortran Test Module and the unloading of it upon unloading the
        // CModule. 
        int CModFunction3_handle = COM_get_function_handle("TestWin1.Function3");
        if(CModFunction3_handle > 0){
          std::string win3("TestFWin3");
          COM_call_function(CModFunction3_handle,&win3);
          int win3_handle = COM_get_window_handle(win3.c_str());
          if(win3_handle > 0) {
            result.UpdateResult("CModuleLoadsFModule",true);
            COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD,"TestWin1");
            win3_handle = COM_get_window_handle(win3.c_str());
            if(win3_handle <= 0)
              result.UpdateResult("CModuleDestroysFModuleOnUnload",true);
            COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,"TestWin1");
            h = COM_get_window_handle("TestWin1");
            if(h < 0)
              c_window_exists = false;
          }
        }
      }

      if(f_window_exists){
        // This is a test of calling Fortran Test Module function3.
        // Fortran Module function3 loads the CModule.
        // This tests verifies both the loading of the
        // CModule and the unloading of it upon unloading the
        // Fortran Module. 
         int FModFunction3_handle = COM_get_function_handle("TestFWin1.Function3");
        if(FModFunction3_handle > 0){
          std::string win3("TestFCWin");
          const char *win3ptr = win3.c_str();
          COM_call_function(FModFunction3_handle,win3ptr);
          int win3_handle = COM_get_window_handle(win3.c_str());
          if(win3_handle > 0) {
            result.UpdateResult("FModuleLoadsCModule",true);
            COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMFTESTMOD,"TestFWin1");
            win3_handle = COM_get_window_handle(win3.c_str());
            if(win3_handle <= 0)
              result.UpdateResult("FModuleDestroysCModuleOnUnload",true);
            COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD,"TestFWin1");
            h = COM_get_window_handle("TestFWin1");
            if(h < 0)
              f_window_exists = false;
          }
        }
      }
 
      result.UpdateResult("ModuleLoadingTest:Runs", true);
      std::cout << "Exiting COM::TestingObject::Test__ModuleLoadingTest" << std::endl;
    }
    /// 
    /// Test for COM_get_modules
    /// 
    /// @param result COM::TestResults object to store test results
    ///
    /// This function implements a simple test of the COM
    /// facility to return a list of all loaded modules.
    virtual void Test__GetModules(ResultsType &results) {
      std::cout << "Enter COM::TestingObject::Test_GetModules" << std::endl;
      std::vector<std::string> module_names;
      std::vector<std::string> window_names; 
      std::vector<std::string> imods;
      std::vector<std::string> iwins;
      std::vector<std::string> smods;
      std::vector<std::string> swins;
      COM_get_modules(module_names);
      COM_get_windows(window_names);
      bool module_names_works = true;
      bool window_names_works = true;
      std::vector<std::string>::iterator ni = module_names.begin();
      if(!module_names.empty()){
        std::cout << "Initial module list:" << std::endl;
        while(ni != module_names.end()){
          std::cout << "\"" << *ni << "\"" << std::endl;
          imods.push_back(*ni++);
        }
      }
      if(!window_names.empty()){
        ni = window_names.begin();
        std::cout << "Initial window list:" << std::endl;
        while(ni != window_names.end()){
          std::cout << "\"" << *ni << "\"" << std::endl;
          iwins.push_back(*ni++);
        }
      }
      ni = imods.begin();
      while(ni != imods.end())
        smods.push_back(*ni++);
      ni = iwins.begin();
      while(ni != iwins.end())
        swins.push_back(*ni++);
      smods.push_back("COMTESTMOD");
      smods.push_back("COMFTESTMOD");
      swins.push_back("win1");
      swins.push_back("win2");
      swins.push_back("win3");
      swins.push_back("win4");
      module_names.resize(0);
      window_names.resize(0);
      COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,"win1");
      COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,"win2");
      COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD,"win3");
      COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD,"win4");
      COM_get_modules(module_names);
      COM_get_windows(window_names);
      ni = smods.begin();
      while(ni != smods.end()){
        std::vector<std::string>::iterator mni = std::find(module_names.begin(),module_names.end(),*ni);
        if(mni == module_names.end()){
          module_names_works = false;
          std::cout << "Couldn't find expected module: " << "\"" << *ni << "\"" << std::endl;
        }
        ni++;
      }
      ni = swins.begin();
      while(ni != swins.end()){
        std::vector<std::string>::iterator mni = std::find(window_names.begin(),window_names.end(),*ni);
        if(mni == module_names.end()){
          window_names_works = false;
          std::cout << "Couldn't find expected window: " << "\"" << *ni << "\"" << std::endl;
        }
        ni++;
      }
      ni = module_names.begin();
      std::cout << "Module list:" << std::endl;
      while(ni != module_names.end())
        std::cout << "\"" << *ni++ << "\"" << std::endl;
      ni = window_names.begin();
      std::cout << "Window list:" << std::endl;
      while(ni != window_names.end())
        std::cout << "\"" << *ni++ << "\"" << std::endl;      
      module_names.resize(0);
      window_names.resize(0);
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,"win1");
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,"win2");
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD,"win3");
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD,"win4");
      COM_get_modules(module_names);
      COM_get_windows(window_names);
      if(!module_names.empty() && !(module_names == imods)){
        module_names_works = false;
        ni = module_names.begin();
        std::cout << "Final module list:" << std::endl;
        while(ni != module_names.end())
          std::cout << *ni++ << std::endl;
      }
      if(!window_names.empty() && !(window_names == iwins)){
        window_names_works = false;
        ni = window_names.begin();
        std::cout << "Final window list:" << std::endl;
        while(ni != window_names.end())
          std::cout << *ni++ << std::endl;      
      }
      results.UpdateResult("COM::GetModules:Works",module_names_works);
      results.UpdateResult("COM::GetWindows:Works",window_names_works);
    }
    ///
    /// Test for Module function registration and access
    ///
    /// @param result COM::TestResults object to store test results.
    ///
    /// This function implements a simple test of the ability to 
    /// access functions which have been registered by modules.
    /// Once the test module is loaded this unit test calls the test modules
    /// functions to ensure that they have (1) been successfully registered
    /// and (2) that they can successfully be called.
    virtual void Test__ModuleFunctionTest(ResultsType &result) {
      std::cout << "Running COM::TestingObject::Test__ModuleFunctionTest" << std::endl;

      COM_LOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestCWin");
  
      int CModFunction0_handle = COM_get_function_handle("TestCWin.Function0");
      bool cfunc0 = (CModFunction0_handle > 0); 
      result.UpdateResult("CModuleFunction0:RegistrationSuccessful",cfunc0);

      int CModFunction1_handle = COM_get_function_handle("TestCWin.Function1");
      bool cfunc1 = (CModFunction1_handle > 0);
      result.UpdateResult("CModuleFunction1:RegistrationSuccessful",cfunc1);

      int CModFunction2_handle = COM_get_function_handle("TestCWin.Function2");
      bool cfunc2 = (CModFunction2_handle > 0);
      result.UpdateResult("CModuleFunction2:RegistrationSuccessful",cfunc2);

      int CModFunction3_handle = COM_get_function_handle("TestCWin.Function3");
      bool cfunc3 = (CModFunction3_handle > 0);
      result.UpdateResult("CModuleFunction3:RegistrationSuccessful",cfunc3);
     
      int CModFunction4_handle = COM_get_function_handle("TestCWin.Function4");
      bool cfunc4 = (CModFunction4_handle > 0);
      result.UpdateResult("CModuleFunction4:RegistrationSuccessful",cfunc4);
 
      int CModIntegerFunction_handle = COM_get_function_handle("TestCWin.IntFunction");
      bool cintfunc = (CModIntegerFunction_handle > 0);
      result.UpdateResult("CModuleIntFunction:RegistrationSuccessful",cintfunc);

      int CModConstIntegerFunction_handle = COM_get_function_handle("TestCWin.ConstIntFunction");
      bool cconstintfunc = (CModConstIntegerFunction_handle > 0);
      result.UpdateResult("CModuleConstIntFunction:RegistrationSuccessful",cconstintfunc);

      int CModOptionalArgsFunction_handle = COM_get_function_handle("TestCWin.OptionalArgsFunction");
      bool copargsfunc = (CModOptionalArgsFunction_handle > 0);
      result.UpdateResult("CModuleOptionalArgsFunction:RegistrationSuccessful",copargsfunc);

      if(cintfunc){
        int j = 0;
        COM_call_function(CModIntegerFunction_handle,&j);
        if(j == 1)
          result.UpdateResult("CModuleIntFunction:Works",true);
      }

      if(cconstintfunc){
        int j = 10;
        std::string output;
        COM_call_function(CModConstIntegerFunction_handle,&j,&output);
        if(output=="10")
          result.UpdateResult("CModuleConstIntFunction:Works",true);
        else
          std::cout << "CModuleConstIntFunction: returned " << output << "." << std::endl;
      }

      if(copargsfunc){
        std::vector<int> args1(12,0);
        std::vector<int>::iterator ai = args1.begin();
        while(ai != args1.end()){
          *ai = (ai - args1.begin());
          ai++;
        }
        int nargs = 0;
        int testit = 6;
        bool optionalargs_works = true;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs);
        if(nargs != 0){
          optionalargs_works = false;
          std::cout << "Optional Arguments Test(0) nargs = " << nargs << "." << std::endl;
        }
        nargs = 1;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0]);
        if(nargs != 0){
          optionalargs_works = false;
          std::cout << "Optional Arguments Test(1) nargs = " << nargs << "." << std::endl;
        }
        nargs = 2;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1]);
        if(nargs != 1){
          optionalargs_works = false;
          std::cout << "Optional Arguments Test(2) nargs = " << nargs << "." << std::endl;
        }
        nargs = 3;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2]);
        if(nargs != 3){
          optionalargs_works = false;
          std::cout << "Optional Arguments Test(3) nargs = " << nargs << "." << std::endl;
        }
        nargs = 4;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2],
                          &args1[3]);
        if(nargs != 6){
          optionalargs_works = false;
          std::cout << "Optional Arguments Test(4) nargs = " << nargs << "." << std::endl;
        }
        nargs = 5;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2],
                          &args1[3],&args1[4]);
        if(nargs != 10){
          optionalargs_works = false;
          std::cout << "Optional Arguments Test(5) nargs = " << nargs << "." << std::endl;
        }
        nargs = 6;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2],
                          &args1[3],&args1[4],&args1[5]);
        if(nargs != 15){
          optionalargs_works = false;
          std::cout << "Optional Arguments Test(6) nargs = " << nargs << "." << std::endl;
        }
        nargs = 7;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2],
                          &args1[3],&args1[4],&args1[5],&args1[6]);
        if(nargs != 21)
          optionalargs_works = false;
        nargs = 8;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2],
                          &args1[3],&args1[4],&args1[5],&args1[6],&args1[7]);
        if(nargs != 28)
          optionalargs_works = false;
        nargs = 9;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2],
                          &args1[3],&args1[4],&args1[5],&args1[6],&args1[7],&args1[8]);
        if(nargs != 36)
          optionalargs_works = false;
        nargs = 10;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2],
                          &args1[3],&args1[4],&args1[5],&args1[6],&args1[7],&args1[8],&args1[9]);
        if(nargs != 45)
          optionalargs_works = false;
        nargs = 11;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2],
                          &args1[3],&args1[4],&args1[5],&args1[6],&args1[7],&args1[8],&args1[9],
                          &args1[10]);
        if(nargs != 55)
          optionalargs_works = false;
        nargs = 12;
        COM_call_function(CModOptionalArgsFunction_handle,&nargs,&args1[0],&args1[1],&args1[2],
                          &args1[3],&args1[4],&args1[5],&args1[6],&args1[7],&args1[8],&args1[9],
                          &args1[10],&args1[11]);
        if(nargs != 66)
          optionalargs_works = false;
        result.UpdateResult("CModuleOptionalArgsFunction:Works",optionalargs_works);
      }
      
      std::string result0;
      if(cfunc0){
        COM_call_function(CModFunction0_handle,&result0);
        if(result0=="Hello COM from TestCWin!")
          result.UpdateResult("CModuleFunction0:CallFromDriver",true);
        result0.erase();
      }
      
      std::string win2("TestCWin2");
      int win2_func_handle = 0;
      int win2_handle = 0;
      if(cfunc1){
        COM_call_function(CModFunction1_handle,&win2);
        win2_handle = COM_get_window_handle(win2.c_str());
        if(win2_handle > 0){
          win2_func_handle = COM_get_function_handle("TestCWin2.Function0");
          if(win2_func_handle > 0)
            COM_call_function(win2_func_handle,&result0);
          if(result0=="Hello COM from TestCWin2!")
            result.UpdateResult("ExternalCCModuleFunction:CallFromDriver",true);
          result0.erase();
          if(cfunc2)
            COM_call_function(CModFunction2_handle,&result0);
          if(result0=="Hello COM from TestCWin2!")
            result.UpdateResult("ExternalCCModuleFunction:CallFromCModule",true);
          result0.erase();
        }

      }

      std::string fwin3("TestCFWin");
      int fwin3_func_handle = 0;
      int fwin3_handle = 0;

      if(cfunc3){
        COM_call_function(CModFunction3_handle,&fwin3);
        fwin3_handle = COM_get_window_handle(fwin3.c_str());
        if(fwin3_handle > 0){
          fwin3_func_handle = COM_get_function_handle("TestCFWin.Function0");
          if(fwin3_func_handle > 0){
            std::vector<char> fresult(81,'\0');
            COM_call_function(fwin3_func_handle,&fresult[0]);
            std::string result_string(&fresult[0]);
            IRAD::Util::Trim(result_string);
            if(result_string=="Hello COM from TestCFWin!")
              result.UpdateResult("ExternalCFModuleFunction:CallFromDriver",true);
            else
              std::cout << "Test of Fortran (loaded by C Module) Hello COM World returns: "
                        << result_string << std::endl;
          }
          if(cfunc4){
            std::string result_string;
            COM_call_function(CModFunction4_handle, &result_string);
            IRAD::Util::Trim(result_string);
            if(result_string=="Hello COM from TestCFWin!")
              result.UpdateResult("ExternalCFModuleFunction:CallFromCModule",true);
            else
              std::cout << "Test of Fortran (called by C Module) Hello COM World returns: "
                        << result_string << std::endl;
          }
        }
      }


      COM_LOAD_MODULE_STATIC_DYNAMIC( COMFTESTMOD, "TestFWin");

      int FModFunction0_handle = COM_get_function_handle("TestFWin.Function0");
      bool ffunc0 = (FModFunction0_handle > 0); 
      result.UpdateResult("FModuleFunction0:RegistrationSuccessful",ffunc0);

      int FModFunction1_handle = COM_get_function_handle("TestFWin.Function1");
      bool ffunc1 = (FModFunction1_handle > 0);
      result.UpdateResult("FModuleFunction1:RegistrationSuccessful",ffunc1);

      int FModFunction2_handle = COM_get_function_handle("TestFWin.Function2");
      bool ffunc2 = (FModFunction2_handle > 0);
      result.UpdateResult("FModuleFunction2:RegistrationSuccessful",ffunc2);

      int FModFunction3_handle = COM_get_function_handle("TestFWin.Function3");
      bool ffunc3 = (FModFunction3_handle > 0);
      result.UpdateResult("FModuleFunction3:RegistrationSuccessful",ffunc3);
     
      int FModFunction4_handle = COM_get_function_handle("TestFWin.Function4");
      bool ffunc4 = (FModFunction4_handle > 0);
      result.UpdateResult("FModuleFunction4:RegistrationSuccessful",ffunc4);
 
      int FModIntegerFunction_handle = COM_get_function_handle("TestFWin.IntFunction");
      bool fintfunc = (FModIntegerFunction_handle > 0);
      result.UpdateResult("FModuleIntFunction:RegistrationSuccessful",fintfunc);

      int FModConstIntegerFunction_handle = COM_get_function_handle("TestFWin.ConstIntFunction");
      bool fconstintfunc = (FModConstIntegerFunction_handle > 0);
      result.UpdateResult("FModuleConstIntFunction:RegistrationSuccessful",fconstintfunc);

      if(ffunc0){
        std::vector<char> fresult(81,'\0');
        COM_call_function(FModFunction0_handle,&fresult[0]);
        std::string result_string(&fresult[0]);
        IRAD::Util::Trim(result_string);
        if(result_string=="Hello COM from TestFWin!")
          result.UpdateResult("FModuleMemberFunction:CallFromDriver",true);
        else
          std::cout << "Test of Fortran Hello COM World returns: " 
                    << result_string << std::endl;
      }
     
      if(fintfunc){
        int j = 0;
        COM_call_function(FModIntegerFunction_handle,&j);
        if(j == 1)
          result.UpdateResult("FModuleIntFunction:Works",true);
        else
          result.UpdateResult("FModuleIntFunction:Works",false);
      }


      if(fconstintfunc){
        int j = 10;
        std::string output = "original";
        const char *outputptr = output.c_str();
        COM_call_function(FModConstIntegerFunction_handle,&j,outputptr);
        output = outputptr;
        if(output.compare("10"))
          result.UpdateResult("FModuleConstIntFunction:Works",true);
        else
          result.UpdateResult("FModuleConstIntFunction:Works",false);
      }

      std::string fwin2("TestFWin2");
      const char *fwin2ptr = fwin2.c_str();
      int fwin2_func_handle = 0;
      int fwin2_handle = 0;
      if(ffunc1){
        std::cout << "ffunc1 true" << std::endl;
        COM_call_function(FModFunction1_handle,fwin2ptr);
        fwin2_handle = COM_get_window_handle(fwin2.c_str());
        if(fwin2_handle > 0){
          std::cout << "fwin2_handle true" << std::endl;
          fwin2_func_handle = COM_get_function_handle("TestFWin2.Function0");
          if(fwin2_func_handle > 0){
            std::cout << "fwin2_func_handle true" << std::endl;
            std::vector<char> fresult(81,'\0');
            COM_call_function(fwin2_func_handle,&fresult[0]);
            std::string result_string(&fresult[0]);
           IRAD::Util::Trim(result_string);
            if(result_string=="Hello COM from TestFWin2!")
              result.UpdateResult("ExternalFFModuleFunction:CallFromDriver",true);
          }
          if(ffunc2){
            std::cout << "ffunc2 true" << std::endl;
            std::vector<char> fresult(81,'\0');
            COM_call_function(FModFunction2_handle,&fresult[0]);
            std::string result_string(&fresult[0]);
            IRAD::Util::Trim(result_string);
            if(result_string=="Hello COM from TestFWin2!")
              result.UpdateResult("ExternalFFModuleFunction:CallFromFModule",true);
          }
        }
      }

//         int FModFunction3_handle = COM_get_function_handle("TestFWin1.Function3");
//        if(FModFunction3_handle > 0){
//          std::string win3("TestFWin3");
//          const char *win3ptr = win3.c_str();
//          COM_call_function(FModFunction3_handle,win3ptr);
//          int win3_handle = COM_get_window_handle(win3.c_str());

      std::string fcwin1("TestFCWin1");
      const char *fcwin1ptr = fcwin1.c_str();
      int fcwin1_func_handle = 0;
      int fcwin1_handle = 0;

      int fwin_handle = COM_get_window_handle("TestFWin");
      if(fwin_handle > 0)
        std::cout << "TestFWin exists" << std::endl;
 
      if(ffunc3){
//        std::cout << "Going to call TESTROUTINE3 now!" << std::endl;
        COM_call_function(FModFunction3_handle,fcwin1ptr);
        fcwin1_handle = COM_get_window_handle(fcwin1.c_str());
        if(fcwin1_handle > 0){
          std::cout << "fcwin1_handle true" << std::endl;
          fcwin1_func_handle = COM_get_function_handle("TestFCWin1.Function0");
          if(fcwin1_func_handle > 0){
            std::cout << "fcwin1_func_handle true" << std::endl;
            std::string result_string;
            COM_call_function(fcwin1_func_handle,&result_string);
            if(result_string=="Hello COM from TestFCWin1!")
              result.UpdateResult("ExternalFCModuleFunction:CallFromDriver",true);
          }
//          if(ffunc4){
//            std::vector<char> fresult(81,'\0');
//            COM_call_function(FModFunction4_handle,&fresult[0]);
//            std::string result_string(&fresult[0]);
//            IRAD::Util::Trim(result_string);
//            if(result_string=="Hello COM from TestFCWin1!")
//              result.UpdateResult("ExternalFCModuleFunction:CallFromFModule",true);
//          }
        }
      }
   
      COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestCWin");
      COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD, "TestFWin");
      result.UpdateResult("ModuleFunctionTest:Runs", true);
      std::cout << "Running COM::TestingObject::Test__ModuleFunctionTest" << std::endl;
    }
    ///
    /// Runs all tests implemented by the COM::TestingObject.
    ///
    /// @param result COM::TestResults object to store test results.
    ///
    virtual void Process(ResultsType &result){
      Prologue();
      Test__ModuleLoadingTest(result);
      Test__ModuleFunctionTest(result);
      Test__GetModules(result);
      Test__DataItemManagement(result);
      Epilogue();
    }
    
    ///
    /// Runs a test specified by name
    ///
    /// @param name String name of the test to run.
    /// @param result COM::TestResults object to store test results.
    ///
    virtual void RunTest(const std::string &name,ResultsType &result)
    {
      Prologue();
      if(name == "ModuleLoadingTest")
        Test__ModuleLoadingTest(result);
      if(name == "ModuleFunctionTest")
        Test__ModuleFunctionTest(result);
      if(name == "GetModules")
        Test__GetModules(result);
      if(name == "LoadUnloadMemTest")
        Test__LoadUnloadMemTest(result);
      Epilogue();
    }

    ///
    /// Runs a list of tests specified by name
    /// 
    /// @param test_names list of string test names
    /// @param result COM::TestResults object to store test results.
    ///
    virtual void ProcessTests(std::list<std::string> &test_names,ResultsType &result){
      Prologue();
      std::list<std::string>::iterator tni = test_names.begin();
      while(tni != test_names.end())
        RunTest(*tni++,result);
      Epilogue();
    }


  };

  /// 
  /// Project-specific parallel testing object.
  ///
  /// The project-specific parallel testing object encapsulates
  /// all parallel tests for the project. This is the interface
  /// to all "in-core" parallel testing available for the given project.
  /// It implements the interface given by 
  /// COM::TestingObject and has an 
  /// IRAD::Comm::CommunicatorObject for parallel capabilities.
  ///
  template<typename CommType,typename ResultsType>
  class ParallelTestingObject : public TestingObject<ResultsType>
  {
  protected:
    // Example test fixtures for parallel COM testing.
    //
    /// 
    /// The CommunicatorObject for the parallel testing object.
    ///
    CommType _communicator;
    ///
    /// A set of values for testing quadrature methods.
    /// 
    std::vector<int> N;
  public:
    ///
    /// Constructor with incoming communicator.
    ///
    ParallelTestingObject(CommType &incomm) :
      TestingObject<ResultsType>(), _communicator(incomm) {};
    CommType &GetCommunicator() { return(_communicator); };

    ///
    /// Tears down the testing fixtures if needed.
    ///
    virtual void Epilogue() {};

    ///
    /// Sets up the data fixtures for the tests.
    /// 
    /// This function gives the object a chance to 
    /// allocate and set up any "fixtures" before the
    /// tests are run.
    ///
    virtual void  Prologue(){
      for(int i = _communicator.Size();i <= 1000000;i*=2) N.push_back(i);
    };

    
    ///
    /// Test for get and set communicator functions
    ///
    /// @param result COM::TestResults object to store test results.
    ///
    /// This function tests calling the functions
    /// COM_get_communicator, COM_get_default_communicator,
    /// COM_set_communicator, and COM_set_default_communicator
    /// with different contexts and window settings.
    /// It is only run when an MPI compiler is present.
    ///

    virtual void Test__WindowGetSetCommunicator(ResultsType &result) {
      MPI_Comm comm, default_comm;
      int rank, size, h;
      MPI_Comm fail_comm;
      COM::COM_base * rbase = COM_get_com();
      COM::Window *param_window = NULL;    
 
      bool getcommunicator = true;
      bool getdefaultcommunicator = true;
      bool setdefaultcommunicator = true;
      bool getwindowcommunicator = true;
      bool getsplitcommunicator = true; 
      bool getsplitwindowcommunicator = true;
      
      std::string outfile_name;
      std::ofstream outfile;
      std::stringstream ss;
      std::string str_rank;

      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      ss << rank;
      ss >> str_rank;
      outfile_name = "testout" + str_rank + ".dat";
      outfile.open(outfile_name.c_str());
   
 
      ///////////////////////////
      // Test for COM_get_default_communicator
      // when no default has been set
      ///////////////////////////
      default_comm = COM_get_default_communicator();
 
      if(default_comm != MPI_COMM_WORLD){
        _communicator.SetErr(1);
        std::cout << "COM_get_default_communicator fails on initial calling line 1123" << std::endl;
      }
      if(_communicator.Check()){
        getdefaultcommunicator = false;
      }
      _communicator.ClearErr();
    
      ///////////////////////////
      // Test for window:get_communicator  
      // when the window was initialized with no communicator
      ///////////////////////////
      COM_new_window("testparallelwindow");
      
      h = COM_get_window_handle("testparallelwindow");
      if(h > 0)
        param_window = rbase->get_window_object(h); 

      comm = param_window->get_communicator();
      
      if(comm != default_comm){
        _communicator.SetErr(1);
        std::cout << "window::get_communicator does not return default communicator!"
                  << " (window initialized with no communicator)" << std::endl;
      }
      if(_communicator.Check()){
        getwindowcommunicator = false;
      }
      _communicator.ClearErr();

      COM_delete_window("testparallelwindow"); 
      ///////////////////////////

      ///////////////////////////
      // Test for window:get_communicator
      // when the window was initialized with MPI_COMM_SELF
      ///////////////////////////
      COM_new_window("testparallelwindow",MPI_COMM_SELF);
      h = COM_get_window_handle("testparallelwindow");
      if(h > 0)
        param_window = rbase->get_window_object(h);

      comm = param_window->get_communicator();
      if(comm != MPI_COMM_SELF){
        _communicator.SetErr(1);
        std::cout << "window::get_communicator does not return communicator used"
                  << " in window initialization!" << std::endl;
      }
      if(_communicator.Check()){
        getwindowcommunicator = false;
      }
      _communicator.ClearErr();
     
      COM_delete_window("testparallelwindow"); 
      ///////////////////////////

       ///////////////////////////
      // Test for COM_set_default_communicator
      // and COM_get_default_communicator
      // when the default has been set to MPI_COMM_SELF
      // also checking get_communicator after the default is set
      ///////////////////////////
      COM_set_default_communicator(MPI_COMM_SELF);
 
      default_comm = COM_get_default_communicator();
 
      if(default_comm != MPI_COMM_SELF){
        _communicator.SetErr(1);
        std::cout << "COM_get_default_communicator does not return the communicator"
                  << " used by set_default_communicator!" << std::endl;
      }
 
      COM_new_window("testparallelwindow");
      h = COM_get_window_handle("testparallelwindow");
      if(h > 0)
        param_window = rbase->get_window_object(h);

      comm = param_window->get_communicator(); 

      if(comm != default_comm){
        _communicator.SetErr(1);
        std::cout << "window::get_communicator does not return the communicator"
                  << " used by set_default_communicator! (a window intialized"
                  << " with no communicator should have the default)" << std::endl;
      }
      if(_communicator.Check()){
        getwindowcommunicator = false;
      }
      _communicator.ClearErr();

      COM_delete_window("testparallelwindow"); 
      /////////////////////////// 

      //////////////////////////
      // Test for get and set communicator functions
      // after using the Split function
      //////////////////////////
      MPI_Comm comm_check;
      comm = MPI_COMM_WORLD;
      CommType communicator(comm);
      CommType newcommunicator;
      size = communicator.Size();
      outfile << "communicator.Size() = " << communicator.Size() << std::endl;

      int color;
      if(rank < size/2) color=0;
      else color=1;
      communicator.Split(color, rank, newcommunicator);
      outfile << "communicator.Size() = " << communicator.Size() << std::endl;
      outfile << "newcommunicator.Size() = " << newcommunicator.Size() << std::endl;
      MPI_Comm newcomm = newcommunicator.GetCommunicator();
 
      COM_set_default_communicator(newcomm);

      if(rank == 0 || rank == 1){ 
        COM_new_window("testparallelwindow");
        h = COM_get_window_handle("testparallelwindow");
        if(h > 0)
        param_window = rbase->get_window_object(h);
        comm = param_window->get_communicator();

        if(comm_check != newcomm){
          std::cout << "window::get_communicator does not return the newly"
                    << " split communicator that was set as the default!" << std::endl;
          _communicator.SetErr(1);
        }

        COM_delete_window("testparallelwindow");
      }
 
      if(_communicator.Check()){
        getsplitcommunicator = false;
      }
      _communicator.ClearErr(); 
      //////////////////////////

      result.UpdateResult("COM::WindowGetCommunicator:Runs", true);
      result.UpdateResult("COM::WindowGetCommunicator:Works", getwindowcommunicator);
      result.UpdateResult("COM::WindowGetSplitCommunicator:Works", getsplitwindowcommunicator);
      COM_set_default_communicator(MPI_COMM_WORLD);

      outfile.close();

    };

    ///
    /// Test for get and set communicator functions
    ///
    /// @param result COM::TestResults object to store test results.
    ///
    /// This function tests calling the functions
    /// COM_get_communicator, COM_get_default_communicator,
    /// COM_set_communicator, and COM_set_default_communicator
    /// with different contexts and window settings.
    /// It is only run when an MPI compiler is present.
    ///

    virtual void Test__GetSetCommunicator(ResultsType &result) {
      MPI_Comm comm, default_comm;
      int rank, size;
      MPI_Comm fail_comm;    

      bool getdefaultcommunicator = true;
      bool setdefaultcommunicator = true;
      bool getcommunicator = true;
      bool getwindowcommunicator = true;

      std::string outfile_name;
      std::ofstream outfile;
      std::stringstream ss;
      std::string str_rank;

      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      ss << rank;
      ss >> str_rank;
      outfile_name = "testout" + str_rank + ".dat";
      outfile.open(outfile_name.c_str());
   
 
      ///////////////////////////
      // Test for COM_get_default_communicator
      // when no default has been set
      ///////////////////////////
      default_comm = COM_get_default_communicator();
 
      if(default_comm != MPI_COMM_WORLD){
        _communicator.SetErr(1);
        std::cout << "COM_get_default_communicator fails on initial calling line 1308" << std::endl;
      }
      if(_communicator.Check()){
        getdefaultcommunicator = false;
      }
      _communicator.ClearErr();
    

      /////////////////////////// 

      ///////////////////////////
      // Test for COM_get_communicator  
      // when the window was initialized with no communicator
      ///////////////////////////
      COM_new_window("testparallelwindow");
      
      COM_get_communicator("testparallelwindow", &comm);

      if(comm != default_comm){
        _communicator.SetErr(1);
        std::cout << "COM_get_communicator does not return default communicator!"
                  << " (window initialized with no communicator)" << std::endl;
      }
      if(_communicator.Check()){
        getcommunicator = false;
      }
      _communicator.ClearErr();

      COM_delete_window("testparallelwindow"); 
      ///////////////////////////



      ///////////////////////////
      // Test for COM_get_communicator
      // when the window was initialized with MPI_COMM_SELF
      ///////////////////////////
      COM_new_window("testparallelwindow",MPI_COMM_SELF);
    
      COM_get_communicator("testparallelwindow", &comm);

      if(comm != MPI_COMM_SELF){
        _communicator.SetErr(1);
        std::cout << "COM_get_communicator does not return communicator used"
                  << " in window initialization!" << std::endl;
      }
      if(_communicator.Check()){
        getcommunicator = false;
      }
      _communicator.ClearErr();
     
      COM_delete_window("testparallelwindow"); 
      ///////////////////////////

       ///////////////////////////
      // Test for COM_set_default_communicator
      // and COM_get_default_communicator
      // when the default has been set to MPI_COMM_SELF
      // also checking COM_get_communicator after the default is set
      ///////////////////////////
      COM_set_default_communicator(MPI_COMM_SELF);
 
      default_comm = COM_get_default_communicator();
 
      if(default_comm != MPI_COMM_SELF){
        _communicator.SetErr(1);
        std::cout << "COM_get_default_communicator does not return the communicator"
                  << " used by set_default_communicator!" << std::endl;
      }
 
      COM_new_window("testparallelwindow");
      
      COM_get_communicator("testparallelwindow", &comm);

      if(comm != default_comm){
        _communicator.SetErr(1);
        std::cout << "COM_get_communicator does not return the communicator"
                  << " used by set_default_communicator! (a window intialized"
                  << " with no communicator should have the default)" << std::endl;
      }
      if(_communicator.Check()){
        setdefaultcommunicator = false;
      }
      _communicator.ClearErr();

      COM_delete_window("testparallelwindow"); 
      /////////////////////////// 

      //////////////////////////
      // Test for get and set communicator functions
      // after using the Split function
      //////////////////////////
      bool getsplitcommunicator = true;
      MPI_Comm comm_check;
      comm = MPI_COMM_WORLD;
      CommType communicator(comm);
      CommType newcommunicator;
      size = communicator.Size();
      outfile << "communicator.Size() = " << communicator.Size() << std::endl;

      int color;
      if(rank < size/2) color=0;
      else color=1;
      communicator.Split(color, rank, newcommunicator);
      outfile << "communicator.Size() = " << communicator.Size() << std::endl;
      outfile << "newcommunicator.Size() = " << newcommunicator.Size() << std::endl;
      MPI_Comm newcomm = newcommunicator.GetCommunicator();
 
      COM_set_default_communicator(newcomm);

      if(rank == 0 || rank == 1){ 
        COM_new_window("testparallelwindow");
      
        COM_get_communicator("testparallelwindow", &comm_check);
        if(comm_check != newcomm){
          std::cout << "COM_get_communicator does not return the newly"
                    << " split communicator that was set as the default!" << std::endl;
          _communicator.SetErr(1);
        }

        COM_delete_window("testparallelwindow");
      }
 
      if(_communicator.Check()){
        getsplitcommunicator = false;
      }
      _communicator.ClearErr(); 
      //////////////////////////
      std::cout << "getdefaultcommunicator = " << getdefaultcommunicator << std::endl;

      result.UpdateResult("COM::GetDefaultCommunicator:Runs", true);
      result.UpdateResult("COM::SetDefaultCommunicator:Runs", true);
      result.UpdateResult("COM::GetCommunicator:Runs", true);
      result.UpdateResult("COM::GetDefaultCommunicator:Works", getdefaultcommunicator);
      result.UpdateResult("COM::SetDefaultCommunicator:Works", setdefaultcommunicator);
      result.UpdateResult("COM::GetCommunicator:Works", getcommunicator);
      result.UpdateResult("COM::GetSplitCommunicator:Works", getsplitcommunicator);
      COM_set_default_communicator(MPI_COMM_WORLD);

      outfile.close();

    };

    virtual void Test__ModuleGetSetCommunicator(ResultsType &result){
      int commworksnew = 1;
      MPI_Comm comm, default_comm;
      int rank, size;
      MPI_Comm fail_comm;
      int funchand, h;
      const char *name = "TestParallelWin1";
      bool getcommunicator = true;
 
      std::string outfile_name;
      std::ofstream outfile;
      std::stringstream ss;
      std::string str_rank;

      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      ss << rank;
      ss >> str_rank;
      outfile_name = "Getsetcommout" + str_rank + ".dat";
      outfile.open(outfile_name.c_str());

      COM_LOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestParallelWin1");
      h=COM_get_window_handle("TestParallelWin1");
 
      if(h > 0)
         funchand= COM_get_function_handle("TestParallelWin1.get_communicator_module");

      if( funchand > 0)
        COM_call_function(funchand, &commworksnew);
      if (commworksnew == 0)
         getcommunicator = false;
  
       result.UpdateResult("COM::GetCommunicatorModule:Works", getcommunicator);

       COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestParallelWin1");

    };


    ///
    /// @brief Parallel Module management tests.
    ///
    /// @param result COM::TestResults object to store test results.
    ///
    /// This function implements several paralle module management tests designed to 
    /// put module loading, unloading, and management through their paces.  It uses
    /// the testing module COMTESTMOD which is written in C++.
    virtual void Test__ParallelModuleLoadingTest(ResultsType &result) {
      MPI_Comm comm, default_comm;
      int rank;
      MPI_Comm comm_check;
      comm = MPI_COMM_WORLD;
      CommType communicator(comm);
      CommType newcommunicator;
      int size = communicator.Size();

      bool c_window_gone=true;
      bool c_load_module_works=true;
      bool getsplitcommunicator=true;
      int h;

      std::string outfile_name;
      std::ofstream outfile;
      std::stringstream ss;
      std::string str_rank;

      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      ss << rank;
      ss >> str_rank;
      outfile_name = "testloading" + str_rank + ".dat";
      outfile.open(outfile_name.c_str());

      if(rank == 0)
        std::cout << "Running Test__ParallelModuleLoadingTest" << std::endl;

      outfile << "Running COM::ParallelTestingObject::Test__ParallelModuleLoadingTest" << std::endl;

      // Split the communicator so we can test which communicator the
      // module is loaded on
      int color;
      if(rank < size/2) color=0;
      else color=1;
      communicator.Split(color, rank, newcommunicator);
      outfile << "communicator.Size() = " << communicator.Size() << std::endl;
      outfile << "newcommunicator.Size() = " << newcommunicator.Size() << std::endl;
      MPI_Comm newcomm = newcommunicator.GetCommunicator();
      COM_set_default_communicator(newcomm);

      if(color == 0){
        // Load the module
        COM_LOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestParallelWin1");

        // See if the module window exists
        h=COM_get_window_handle("TestParallelWin1");
        if(h <= 0) {
          outfile << "COM_get_window_handle(\"TestParallelWin1\") returns " << h << std::endl;
          _communicator.SetErr(1);
        }
      }
      if(_communicator.Check()){
        c_load_module_works=false;
      }
      _communicator.ClearErr();

      if(color == 0){
        COM_get_communicator("TestParallelWin1", &comm_check);
        if(comm_check != newcomm){
          std::cout << "COM_get_communicator does not return the newly"
                    << " split communicator that was set as the default!" << std::endl;
          _communicator.SetErr(1);
        }
      }
      if(_communicator.Check())
        getsplitcommunicator = false;
      _communicator.ClearErr();

      if(color == 0){
        // Unolad the module
        COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD,"TestParallelWin1");

        // Check that the window no longer exists
        h=COM_get_window_handle("TestParallelWin1");
        if(h > 0) {
          outfile << "COM_get_window_handle(\"TestParallelWin1\") returns " << h << std::endl;
          _communicator.SetErr(1);
        }
      }
      if(_communicator.Check()){
        c_window_gone=false;
      }
      _communicator.ClearErr();

      ////////////////////////////////
      ////Load the module on all processes
      /// call Increment on root & then broadcast
      ////////////////////////////////
      int IncrementHandle;
      bool parallel_function_works = true;
      int number=0;
      
      // Load the module
      COM_LOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestParallelWin1");

      // See if the module window exists
      h=COM_get_window_handle("TestParallelWin1");
      if(h > 0) 
        IncrementHandle = COM_get_function_handle("TestParallelWin1.Increment");
   
      if(IncrementHandle > 0 && rank == 1)
        COM_call_function(IncrementHandle,&number,&rank);

      communicator.BroadCast(number,1); 

      if(number != 1){
        std::cout << "Incrementing of number and then broadcasting did not work!" << std::endl;
        _communicator.SetErr(1);
      }
      if(_communicator.Check())
         parallel_function_works = false;
      _communicator.ClearErr();

      if(IncrementHandle > 0)
        COM_call_function(IncrementHandle,&number,&rank);

      int nproc = _communicator.Size();
      std::vector<int> numbers(nproc,0);

      int commstat = _communicator.AllGather(number,numbers);
    
      for(int i=0; i < nproc; i++){
        if(numbers[i] != i+1){
          std::cout << "AllGather of number into numbers did not get the correct values!" << std::endl;
          _communicator.SetErr(1);
        }
      }
      if(_communicator.Check())
        parallel_function_works = false;
      _communicator.ClearErr();

      // Unload the module
      COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestParallelWin1");

      ///////////////////////////////
 

      result.UpdateResult("COM::ParallelLoadModule:Works", c_load_module_works);
      result.UpdateResult("COM::ParallelUnloadModule:Works", c_window_gone);
      result.UpdateResult("COM::ParallelFunctionCall:Works", parallel_function_works);

      outfile.close();
    }


    ///
    /// @brief Parallel function call with barrier test.
    ///
    /// @param result COM::TestResults object to store test results.
    ///
    /// This function implements a test designed to call a function
    /// in parallel and verify that the barrier after the function call
    /// worked. It uses the testing module COMTESTMOD which is written in C++.
    virtual void Test__FunctionBarrierTest(ResultsType &result) {
      bool barrier_works = false;

      MPI_Comm comm;
      int rank;
      comm = MPI_COMM_WORLD;
      CommType communicator(comm);
      int size = communicator.Size();
      int h, sleep_handle, b;
      time_t t1, t2;

      std::string outfile_name;
      std::ofstream outfile;
      std::stringstream ss;
      std::string str_rank;

      //making output files for each processor for debugging
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      ss << rank;
      ss >> str_rank;
      outfile_name = "testbarrier" + str_rank + ".dat";
      outfile.open(outfile_name.c_str());
 
      // Load the module
      COM_LOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestParallelWin1");

      // See if the module window exists
      h=COM_get_window_handle("TestParallelWin1");
      // If the window exists see if the function exists
      if(h > 0) 
        sleep_handle = COM_get_function_handle("TestParallelWin1.SleepMultiply");

      // get the current time
      t1 = time(0);
 
      // if the function exists, call it
      if(sleep_handle > 0)
        COM_call_function(sleep_handle, &rank);
 
      //get current time
      t2 = time(0);
      outfile << "Time to call function and return = " << t2 - t1 << std::endl;

      // barrier function to force all processes to wait
      b = communicator.Barrier();      

      //get current time
      t2 = time(0);
      outfile << "Time to call function and return and wait  = " << t2 - t1 << std::endl;

      // check that it took all the procceses the same amount of time
      double difft = (double)(t2 - t1);
      if(((size - 1) - difft) < 1.0e-3)
        barrier_works = true;

      // Unload the module
      COM_UNLOAD_MODULE_STATIC_DYNAMIC( COMTESTMOD, "TestParallelWin1");
    
      result.UpdateResult("COM::FunctionBarrierTest:Runs", true);
      result.UpdateResult("COM::FunctionBarrierTest:Works", barrier_works);
    }
    ///
    /// Runs all tests implemented by the COM::ParallelTestingObject.
    ///
    /// @param result COM::TestResults object to store test results.
    ///
    virtual void Process(ResultsType &result){
      Prologue();
      Test__GetSetCommunicator(result);
      Test__ParallelModuleLoadingTest(result);
      Test__FunctionBarrierTest(result);
      Test__WindowGetSetCommunicator(result);
      Test__ModuleGetSetCommunicator(result);
      Epilogue();
    }
    
    ///
    /// Runs a test specified by name
    ///
    /// @param name String name of the test to run.
    /// @param result COM::TestResults object to store test results.
    ///
    virtual void RunTest(const std::string &name,ResultsType &result)
    {
      if(name == "GetSetCommunicatorTest")
        Test__GetSetCommunicator(result);
      if(name == "ParallelModuleLoadingTest")
        Test__ParallelModuleLoadingTest(result);
      if(name == "FunctionBarrierTest")
        Test__FunctionBarrierTest(result);
      if(name == "WindowGetSetCommunicator")
         Test__WindowGetSetCommunicator(result);
      if(name == "ModuleGetSetCommunicator")
         Test__ModuleGetSetCommunicator(result);
    }

    ///
    /// Runs a list of tests specified by name
    /// 
    /// @param test_names list of string test names
    /// @param result COM::TestResults object to store test results.
    ///
    virtual void ProcessTests(std::list<std::string> &test_names,ResultsType &result){
      Prologue();
      std::list<std::string>::iterator tni = test_names.begin();
      while(tni != test_names.end())
        RunTest(*tni++,result);
      Epilogue();
    }

  };
};
#endif
