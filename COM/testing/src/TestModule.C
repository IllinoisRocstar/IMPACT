///
/// @file
/// @ingroup com_group
/// @brief C++ Test Module
/// @author JEK
/// @author MTC 
/// @date May 1, 2014
/// 
/// This file serves as both a test and a simple example for implementing
/// a service module in C/C++. 
///
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "FC.h"
#include "com.h"
#include "com_devel.hpp"
#include "TestSolver.H"

namespace COM {
  /// Simple test function sets integer to 1.
  void IntFunction(int *io_data)
  {
    if(io_data)
      *io_data = 1;
  };

  /// Simple test function writes an integer to string
  void ConstIntFunction(const int *idata,void *string_data)
  {
    std::ostringstream Ostr;
    std::string *sdptr = static_cast<std::string *>(string_data);
    Ostr << *idata;
    sdptr->assign(Ostr.str());
  };
  
  ///
  /// @brief Test function for optional arguments
  ///
  /// This function tests the functionality of optional 
  /// arguments for registered functions. It will count the
  /// number of non-null arguments and sum their values. If
  /// the number of non-null arguments passed in does not match
  /// that specified by a0 on input, then -sum is returned. Otherwise
  /// the sum will be returned in a0.
  ///
  /// @param a0 integer specifies number of optional arguments on input
  /// @returns a0 Sum of all non-null arguments, or -1 if error detected
  ///
  void OptionalArgsFunction(int *a0, int *a1,int *a2, int *a3, int *a4,
                            int *a5,int *a6, int *a7, int *a8,
                            int *a9,int *a10,int *a11, int *a12){
    int number_of_nonnull_arguments = 0;
    int nonnull_count = 0;
    if(a0)
      number_of_nonnull_arguments = *a0;
    else
      return;
    *a0 = 0;
    if(a1){
      nonnull_count++;
      *a0 += *a1;
    }
    if(a2){
      nonnull_count++;
      *a0 += *a2;
    }
    if(a3){
      nonnull_count++;
      *a0 += *a3;
    }
    if(a4){
      nonnull_count++;
      *a0 += *a4;
    }
    if(a5){
      nonnull_count++;
      *a0 += *a5;
    }
    if(a6){
      nonnull_count++;
      *a0 += *a6;
    }
    if(a7){
      nonnull_count++;
      *a0 += *a7;
    }
    if(a8){
      nonnull_count++;
      *a0 += *a8;
    }
    if(a9){
      nonnull_count++;
      *a0 += *a9;
    }
    if(a10){
      nonnull_count++;
      *a0 += *a10;
    }
    if(a11){
      nonnull_count++;
      *a0 += *a11;
    }
    if(a12){
      nonnull_count++;
      *a0 += *a12;
    }
    if(nonnull_count != number_of_nonnull_arguments)
      *a0 *= -1;
  }
  class TestModule : public TestSolver, public COM_Object {
  public:
    /// Default constructor.
    TestModule(){};
    
    ///
    /// @brief Destructor
    ///
    /// The destructor will destroy the externally loaded modules (if they exists)
    ///
    virtual ~TestModule()
    {
      if(!other_window_name.empty()){
        int other_window_handle = COM_get_window_handle(other_window_name.c_str());
        if(other_window_handle > 0)
          COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,other_window_name.c_str());
      }
      if(!fortran_window_name.empty()){
        int fortran_window_handle = COM_get_window_handle(fortran_window_name.c_str());
        if(fortran_window_handle > 0)
          COM_UNLOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD,fortran_window_name.c_str());
      }
    };  


    ///
    /// @brief Hello COM World
    ///
    /// This function just prints a message:
    /// "Hello COM from my_window_name!"
    /// to stdout and to the string passed in.
    ///
    /// @param  string_data, int.
    /// @returns.
    void Function0F(char *string_data, int size)
    {
      std::cout << "Entering Function0F: " << std::endl
		<< "Size = " << size << std::endl;
      std::ostringstream Ostr;
      Ostr << "Hello COM from " << my_window_name << "!";
      std::string::size_type x=Ostr.str().size();
      if(x > size)
        x = size;
      strncpy(string_data, Ostr.str().c_str(), x);
      std::cout << string_data << std::endl;
    };

    ///
    /// @brief Hello COM World
    ///
    /// This function just prints a message:
    /// "Hello COM from my_window_name!"
    /// to stdout and to the string passed in.
    ///
    /// @param string_data A pointer to a std::string object.
    /// @returns string_data set to the message.
    void Function0(void *string_data)
    {
      std::string *output_string = static_cast<std::string *>(string_data);
      std::ostringstream Ostr;
      Ostr << "Hello COM from " << my_window_name << "!";
      output_string->assign(Ostr.str());
      std::cout << *output_string << std::endl;
    };

    ///
    /// @brief Load external module by name
    ///
    /// This function loads another instance of *this* test
    /// module with the name passed in. 
    ///
    /// @param string_data A pointer to a std::string object containing
    /// the name of the new window.
    ///
    void Function1(void *string_data)
    {
      std::string *owindow_name = static_cast<std::string *>(string_data);
      other_window_name.assign(*owindow_name);
      COM_LOAD_MODULE_STATIC_DYNAMIC(COMTESTMOD,other_window_name.c_str());
    };

    ///
    /// @brief Invoke external test module's "Hello COM World!"
    ///
    /// This function calls the external module's "Hello COM World!"
    /// function (if it exists). The output from the call to the 
    /// external module is returned in string_data.
    ///
    /// @param string_data A pointer to a std::String object where
    /// the results of the call will be returned.
    ///
    void Function2(void *string_data)
    {
      if(!other_window_name.empty()){
        int other_window_handle = COM_get_window_handle(other_window_name.c_str());
        if(other_window_handle > 0){
          int other_function_handle = COM_get_function_handle((other_window_name+".Function0").c_str());
          if(other_function_handle > 0)
            COM_call_function(other_function_handle,string_data);
        }
      }
    };


    ///
    /// @brief Load external module by name
    ///
    /// This function loads an instance of the Fortran test
    /// module with the name passed in. 
    ///
    /// @param string_data A pointer to a std::string object containing
    /// the name of the new window.
    ///
    void Function3(void *string_data)
    {
      std::string *fwindow_name = static_cast<std::string *>(string_data);
      fortran_window_name.assign(*fwindow_name);
      COM_LOAD_MODULE_STATIC_DYNAMIC(COMFTESTMOD,fortran_window_name.c_str());
    };


    ///
    /// @brief Invoke external Fortran test module's "Hello COM World!"
    ///
    /// This function calls the external Fortran module's "Hello COM World!"
    /// function (if it exists). The output from the call to the 
    /// external module is returned in string_data.
    ///
    /// @param string_data A pointer to a std::String object where
    /// the results of the call will be returned.
    ///
    void Function4(void *string_data)
    {
      std::string *output_string = static_cast<std::string *>(string_data);
      if(!fortran_window_name.empty()){
        int fortran_window_handle = COM_get_window_handle(fortran_window_name.c_str());
        if(fortran_window_handle > 0){
          int fortran_function_handle = COM_get_function_handle((fortran_window_name+".Function0").c_str());
          if(fortran_function_handle > 0)
          {
            std::vector<char> fresult(81,'\0');
            COM_call_function(fortran_function_handle,&fresult[0]);
            output_string->assign(&fresult[0]);
          }
        }
      }
    };

    ///
    /// @brief Function that increments an integer
    ///
    /// @param an integer
    ///
    /// This function simply increments the value of
    /// the integer passed in to it.
    void Increment(int *i, int *rank)

    {
      *i+=*rank;
    };


    ///
    /// @brief Function that sleeps for the number of seconds of its process rank
    ///
    /// @param
    ///
    /// This function simply sleeps for the number of seconds of its process rank
    void SleepMultiply(int *mult)

    {
      sleep((*mult));
    };


    ///
    /// @brief Initialize stub solver from config file
    ///
    /// This function reads a configuration file and sets
    /// up some example solver-like data for use in testing
    //nt *rank/
    /// @param config_file A pointer to a std::string object where
    /// the configuration filename is specified.
    ///
    void InitSolver(void *config_file)
    {
      std::string *config_name = NULL;
      if(config_file)
        config_name = static_cast<std::string *>(config_file);
      
      std::ifstream Inf;
      Inf.open(config_name->c_str());
      std::cout << "Opening config file " << config_name->c_str() << std::endl;
      if(!Inf){
        std::cerr << "TestModule::InitModule:Error: Could not open configuration file, "
                  << config_name->c_str() << "." << std::endl;
        return;
      }
      // for now, config is just "the mesh"
      ReadMeshFromStream(Inf);
      Inf.close();

      // Step (1) sets up solution meta data
      Solution().Meta().AddField("time",'s',1,8,"s");
      Solution().Meta().AddField("dt",'s',1,8,"s");
      Solution().Meta().AddField("id",'m',1,4,"");
      Solution().Meta().AddField("displacement",'n',3,8,"m");
      Solution().Meta().AddField("pressure",'c',1,8,"Pa");
      Solution().Meta().AddField("temperature",'n',1,8,"K");
      Solution().Meta().AddField("nodeflags",'n',1,4,"");
      Solution().Meta().AddField("cellflags",'c',1,4,"");
      
      // Step (2) creates the buffers for the actual data
      CreateSoln();
      
      unsigned int nnodes = Mesh().nc.NNodes();
      unsigned int nelem  = Mesh().con.Nelem();

      // Step (3) allocate our own local buffers for the data
      step = 0;
      time.resize(1,0);
      delta_time.resize(1,.1);
      id.resize(1,1);
      displacements.resize(3*nnodes,0);
      pressures.resize(nelem,1.0);
      temperatures.resize(nnodes,1.0);
      nodeflags.resize(nnodes,2);
      cellflags.resize(nelem,1);
      
      // Step (4) reset the buffers created above to be our own local buffers
      Solution().SetFieldBuffer("time",time);
      Solution().SetFieldBuffer("dt",delta_time);
      Solution().SetFieldBuffer("temperature",temperatures);
      Solution().SetFieldBuffer("nodeflags",nodeflags);
      Solution().SetFieldBuffer("cellflags",cellflags);
      Solution().SetFieldBuffer("displacement",displacements);
      Solution().SetFieldBuffer("pressure",pressures);
      Solution().SetFieldBuffer("id",id);

      SolverUtils::RegisterSolverInto(my_window_name,*this);
    };

    void ValidateDataItemAddress(void *name_of_dataitem, void *address, int *result){
      std::string *name_string = static_cast<std::string *>(name_of_dataitem);
      if(Solution().GetFieldData(*name_string).data() == static_cast<const char *>(address)){
        *result = 1;
      }
    }

    ///
    /// @brief Step the dummy solver through time
    ///
    /// This function steps the dummy solver and sets up
    /// periodic solutions on the grid. time is advanced
    /// by delta_t.
    ///
    void Step()
    {
      step++;
      unsigned int nnodes = Mesh().nc.NNodes();
      unsigned int nelem  = Mesh().con.Nelem();
      double pi = 4.0*std::atan(1.0)/5.0;
      time[0] += delta_time[0];
      double t = time[0];
      for(int i = 0;i < nnodes;i++){
        double xcomp = std::sin(pi*Mesh().nc.x(i+1));
        double ycomp = std::sin(pi*Mesh().nc.y(i+1));
        double tcomp = std::sin(5.0*pi*t);
        double disp = xcomp*ycomp*tcomp;
        displacements[i*3+2] = xcomp*ycomp*tcomp;
        temperatures[i] = disp*disp;
      }
      SolverUtils::Mesh::NodalCoordinates nccopy;
      nccopy.init_copy(nnodes,Mesh().nc.Data());
      SolverUtils::Mesh::DisplaceNodalCoordinates(nccopy,displacements);
      std::vector<double> centroids(3*nelem,0);
      SolverUtils::Mesh::GetMeshCentroids(nccopy,Mesh().con,centroids);
      for(int i = 0;i < nelem;i++)
        pressures[i] = centroids[i*3+2];
    };

    ///
    /// @brief Output solver solution to VTK format
    ///
    /// This function writes the solver solution to a VTK file
    /// with the specified name. If indicated, a timestamp will
    /// be appended to the specified name resulting in a name 
    /// like: <name>_<time>.vtk.
    ///
    /// @param config_file A pointer to a std::string object where
    /// the filename is specified.
    /// @param use_timestamp integer indicating whether to append a timestamp
    ///
    void DumpSolverSolution(void *config_file,int *use_timestamp)
    {
      std::string *config_name = NULL;
      if(config_file)
        config_name = static_cast<std::string *>(config_file);
      else{
        std::cerr << "TestModule::DumpSolution:Error: No filename given." << std::endl;
        return;
      }
      
      std::ostringstream Ostr;
      Ostr << *config_name;
      if(use_timestamp){
        if(*use_timestamp)
          Ostr << "_" << time[0];
        else {
          Ostr << "_" << step; 
        }
      }
      Ostr << ".vtk";
      std::ofstream Ouf;
      Ouf.open(Ostr.str().c_str());
      if(!Ouf){
        std::cerr << "TestModule::DumpSolution:Error: Could not open output file, "
                  << Ostr.str() << "." << std::endl;
        return;
      }
      SolverUtils::WriteVTKToStream(my_window_name,*this,Ouf);
      Ouf.close();
    };
    ///
    /// @brief 
    /// 
    /// This function ...
    void get_communicator_module( int *commworks)
    {
      const char *comm_name = "TestParallelWin1";
      typedef IRAD::Comm::CommunicatorObject CommType;
      CommType _communicator;
      bool getcommunicator = true;
      MPI_Comm comm, default_comm;
      COM_get_communicator(comm_name, &comm);
      default_comm = COM_get_default_communicator();

      if(comm != MPI_COMM_SELF){
        _communicator.SetErr(1);
        std::cout << "COM_get_communicator does not return the communicator"
                  << " used by set_default_communicator!" << std::endl;
      }
      *commworks = 1;

      if(_communicator.Check()){
        //getcommunicator = false;
        *commworks = 0;
      }
      std::cout << "Getcommunicator output:  " << getcommunicator << std::endl;
      _communicator.ClearErr();

    };
    
    ///
    /// @brief "Loads" the C++ TestModule
    ///
    /// This function creates an instance of a COM::TestModule object,
    /// creates a new Window with the specified name and registers the
    /// address of the new COM::TestModule object as the ".global"
    /// DataItem of the Window. This registers the C++ COM::TestModule
    /// "object" with the COM runtime system.
    /// 
    /// Next, several member functions are registered. The first
    /// argument to each member function is a COM_RAWDATA type
    /// which will be the address of the object registerd to
    /// the ".global" DataItem.
    ///
    /// @param name const std::string specifying the name of the 
    /// window to load the module into.
    ///
    /// @note This function must be a *static* function in order
    /// for it to work in the COM_LOAD_MODULE interface.
    ///
    /// @note The COM_VOID type has been used here to pass
    /// in a pointer to a std::string object.  This is cool.
    ///
    static void Load(const std::string &name){
      std::cout << "Loading TestModule with name " << name 
                << "." << std::endl;

      TestModule *test_module_pointer = new TestModule();
      COM_new_window(name);
      test_module_pointer->my_window_name = name;
      std::string global_name(name+".global");
      COM_new_dataitem(global_name.c_str(),'w',COM_VOID,1,"");
      COM_set_object(global_name.c_str(),0,test_module_pointer);

      std::vector<COM_Type> types(13,COM_INT);
    
      types[0] = COM_RAWDATA;
      types[1] = COM_STRING;

      COM_set_member_function( (name+".Function0F").c_str(), 
                               (Member_func_ptr)(&TestModule::Function0F), 
                               global_name.c_str(), "bbi", &types[0]);
 
      types[1] = COM_VOID;
      COM_set_member_function( (name+".Function0").c_str(), 
                               (Member_func_ptr)(&TestModule::Function0), 
                               global_name.c_str(), "bb", &types[0]);
    
      COM_set_member_function( (name+".Function1").c_str(), 
                               (Member_func_ptr)(&TestModule::Function1), 
                               global_name.c_str(), "bi", &types[0]);
      
      COM_set_member_function( (name+".Init").c_str(), 
                               (Member_func_ptr)(&TestModule::InitSolver), 
                               global_name.c_str(), "bi", &types[0]);
    
      COM_set_member_function( (name+".ValidateAddress").c_str(), 
                               (Member_func_ptr)(&TestModule::ValidateDataItemAddress), 
                               global_name.c_str(), "biio", &types[0]);

      COM_set_member_function( (name+".Dump").c_str(), 
                               (Member_func_ptr)(&TestModule::DumpSolverSolution), 
                               global_name.c_str(), "bii", &types[0]);

      COM_set_member_function( (name+".Step").c_str(), 
                               (Member_func_ptr)(&TestModule::Step), 
                               global_name.c_str(), "b", &types[0]);
      
      COM_set_member_function( (name+".Function2").c_str(), 
                               (Member_func_ptr)(&TestModule::Function2), 
                               global_name.c_str(), "bb", &types[0]);

      COM_set_member_function( (name+".Function3").c_str(), 
                               (Member_func_ptr)(&TestModule::Function3), 
                               global_name.c_str(), "bi", &types[0]);

      COM_set_member_function( (name+".Function4").c_str(), 
                               (Member_func_ptr)(&TestModule::Function4), 
                               global_name.c_str(), "bb", &types[0]);
      
      types[1] = COM_INT;
      COM_set_member_function( (name+".get_communicator_module").c_str(),
                               (Member_func_ptr)(&TestModule::get_communicator_module),
                               global_name.c_str(), "bb", &types[0]);

      types[0] = COM_INT;     
      types[1] = COM_INT;

      COM_set_member_function( (name+".Increment").c_str(), 
                               (Member_func_ptr)(&TestModule::Increment), 
                               global_name.c_str(), "bbi", &types[0]);

      COM_set_member_function( (name+".SleepMultiply").c_str(), 
                               (Member_func_ptr)(&TestModule::SleepMultiply), 
                               global_name.c_str(), "bi", &types[0]);
 
      COM_set_function((name+".IntFunction").c_str(),
                       (Func_ptr)(&IntFunction),"b",&types[0]);
      
      COM_set_function((name+".ConstIntFunction").c_str(),
                       (Func_ptr)(&ConstIntFunction),"io",&types[0]);

      COM_set_function((name+".OptionalArgsFunction").c_str(),
                       (Func_ptr)(&OptionalArgsFunction),
                       "bIIIIIIIIIIII",&types[0]);

      COM_window_init_done(name);  
    }

    ///
    /// @brief Unloads the TestModule
    ///
    /// This function simply destroys the COM::TestModuleObject created
    /// in the call to Load and then deletes the associated window with
    /// the specified name.
    ///
    static void Unload(const std::string &name){
      std::cout << "Unloading TestModule with name " << name 
                << "." << std::endl;
      TestModule *test_module_pointer = NULL;
      std::string global_name(name+".global");
      COM_get_object(global_name.c_str(),0,&test_module_pointer);
      COM_assertion_msg( test_module_pointer->validate_object()==0, "Invalid object");
      delete test_module_pointer;
      COM_delete_window(std::string(name));
    }

  private:
    std::string my_window_name; /// Tracks *this* window name.
    std::string other_window_name; /// Tracks *this* external window's name
    std::string fortran_window_name; /// Tracks the fortran external window's name
  };
}
  
// COM_Type types[5];

// types[0] = COM_RAWDATA;
// types[2] = COM_INT;
// types[1] = types[3] = types[4] = COM_METADATA;
// COM_set_member_function( (mname+".initialize").c_str(), 
// 			   (Member_func_ptr)(&Rocon::initialize), 
// 			   glb.c_str(), "bii", types);

// types[1] = COM_STRING;
// COM_set_member_function( (mname+".init_from_file").c_str(),
//       		   (Member_func_ptr)(&Rocon::init_from_file),
//       		   glb.c_str(),"bii", types);
// types[1] = COM_METADATA;
// types[2] = COM_METADATA;
// COM_set_member_function( (mname+".find_intersections").c_str(), 
// 			   (Member_func_ptr)(&Rocon::find_intersections), 
// 			   glb.c_str(), "biiob", types);
// COM_set_member_function( (mname+".constrain_displacements").c_str(), 
// 			   (Member_func_ptr)(&Rocon::constrain_displacements), 
// 			   glb.c_str(), "bibii", types);

// COM_set_member_function( (mname+".burnout").c_str(), 
// 			   (Member_func_ptr)(&Rocon::burnout), 
// 			   glb.c_str(), "biib", types);
  
// COM_set_member_function( (mname+".burnout_filter").c_str(), 
// 			   (Member_func_ptr)(&Rocon::burnout_filter), 
// 			   glb.c_str(), "bib", types);

/// 
/// @brief C/C++ bindings to load COMTESTMOD
///
extern "C" void COMTESTMOD_load_module( const char *name) {
  COM::TestModule::Load(name);
}
///
/// @brief C/C++ bindings to unload COMTESTMOD
///
extern "C" void COMTESTMOD_unload_module( const char *name){
  COM::TestModule::Unload(name);
}
