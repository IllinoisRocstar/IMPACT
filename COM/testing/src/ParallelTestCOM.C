///
/// @file 
/// @ingroup impact_group
/// @brief Implements a command-line interface for the COM tests.
///
/// Note that in this file, the "main" function is separated off from 
/// the program implementation.  This is done to make the documentation
/// usable and should be done for every new program in %IMPACT. 
///
#include "ComLine.H"
#include "TestCOM.H"
#include "COMM.H"


namespace COM{
  
  ///
  /// Convenience typedef for CommunicatorObject 
  ///
  typedef IRAD::Comm::CommunicatorObject CommType;
  ///
  /// Drives the COM::TestObject. 
  /// 
  /// @param argc number of string command line tokens
  /// @param argv string command line tokens
  /// @returns 0 if successful, 1 otherwise
  ///
  /// Drives the COM::TestObject, which should encapsulate
  /// all the tests for the COM namespace.
  /// 
  /// Command line documentation:
  ///
  ///           impact_test [-h] [-v [level] -o <filename> -l <filename> -n <TestName> ] 
  ///
  ///           -h,--help
  ///              Print out long version of help and exit.
  ///
  ///           -v,--verblevel [level]
  ///              Set the verbosity level. (default = 0)
  ///
  ///           -o,--output <filename>
  ///              Set the output file to <filename>. (default = stdout)
  ///
  ///           -l,--list <filename>
  ///              Set the list file name to <filename>. (no default). The list file should be a text file with one test name per line.
  ///
  ///           -n,--name <TestName>
  ///              Run test by name. (no default)
  ///
  int ParallelTest(int argc,char *argv[])
  {
    // This sets everything up with MPI
    COM::CommType communicator(&argc,&argv);
    int rank  = communicator.Rank();
    int nproc = communicator.Size();
    bool do_stdout = !rank;
    
    int com_initialized = COM_initialized();
    bool com_initialized_pass = (com_initialized <= 0);

    COM_init(&argc, &argv);

    if(com_initialized_pass)
      com_initialized = (COM_initialized() > 0);
  
    if(do_stdout)
      std::cout << "Beginning COM::ParallelTest" << std::endl;

    // The default verbosity is 0
    int verblevel = 0;

    // This line creates the COM::TestComLine object and passes in 
    // the command line arguments as a (const char **).
    TestComLine comline((const char **)(argv));
    // The call to comline.Initialize() reads the command line arguments
    // from the array passed in the previous line.
    comline.Initialize();
    
    
    // The ProcessOptions() call does detailed examination of the command
    // line arguments to check for user errors or other problems. This call
    // will return non-zero if there were errors on the commandline.
    int clerr = comline.ProcessOptions();
    // Check if the user just wanted to get the help and exit
    if(!comline.GetOption("help").empty()){
      // Print out the "long usage" (i.e. help) message to stdout
      if(do_stdout){
        std::cout << comline.LongUsage() << std::endl;
        if(verblevel > 1)
          std::cout << "COM::ParallelTest: Exiting test function (success)" 
                    << std::endl;
      }
      communicator.SetExit(1);
    }
    if(communicator.Check())
      return(0);
    if(clerr){
      if(do_stdout){
        std::cout << comline.ErrorReport() << std::endl
                  << std::endl << comline.ShortUsage() << std::endl;
        if(verblevel > 2)
          std::cout << "COM::ParallelTest: Exiting test function (fail)" << std::endl;
      }
      communicator.SetExit(1);
    }
    if(communicator.Check())
      return(1);

    // These outstreams allow the output to file to be set up and separated
    // from the stdout.
    std::ofstream Ouf;
    std::ostream *Out = NULL;
    if(do_stdout)
      Out = &std::cout;

    // The next few lines populate some strings based on the 
    // users input from the commandline.
    std::string OutFileName(comline.GetOption("output"));
    std::string TestName(comline.GetOption("name"));
    std::string ListName(comline.GetOption("list"));
    std::string sverb(comline.GetOption("verblevel"));
    
    // The following block parses and sets the verbosity level
    if(!sverb.empty()){
      verblevel = 1;
      if(sverb != ".true."){
        std::istringstream Istr(sverb);
        Istr >> verblevel;
        if(verblevel < 0)
          verblevel = 1;
      }
    }
    
    // This block sets up the output file if the user specified one
    if(!OutFileName.empty()){
      if(Out){
        Ouf.open(OutFileName.c_str());
        if(!Ouf){
          std::cout << "COM::ParallelTest> Error: Could not open output file, " 
                    <<  OutFileName << " for test output. Exiting (fail)." << std::endl;
          communicator.SetExit(1);
        }
        Out = &Ouf;
      }
      if(communicator.Check())
        return(1);
    }
    
    if(verblevel > 1 && Out)
      *Out << "COM::ParallelTest: Entering test function" << std::endl;
    
    // Make an instance of the COM testing object, COM::ParallelTestingObject
    COM::ParallelTestingObject<COM::CommType,COM::TestResults> test(communicator);
    // Make an instance of the COM results object, COM::TestResults
    COM::TestResults results;
    
    // If the user specified a name, then run only the named test
    if(!TestName.empty()){
      // This call runs a test by name
      test.RunTest(TestName,results);
    }
    // Otherwise, if the user specified a list, then read the list and
    // run the listed tests.
    else if(!ListName.empty()){
      std::ifstream ListInf;
      ListInf.open(ListName.c_str());
      if(!ListInf){
        if(Out)
          *Out << "COM::ParallelTest> Error: Could not open list of tests in file " 
               << ListName << ". Exiting (fail)." << std::endl;
        communicator.SetExit(1);
      }
      if(communicator.Check())
        return(1);
      std::string testname;
      while(std::getline(ListInf,testname))
        test.RunTest(testname,results);
      ListInf.close();
    }
    else {
      // This call runs all the tests for the COM namespace.
      test.Process(results);
    }

    COM_finalize();
    results.UpdateResult("COM::ParallelFinalize:Works",true);
    if(com_initialized_pass)
      com_initialized_pass = (COM_initialized() <= 0);
    results.UpdateResult("COM::ParallelInitialized:Works",com_initialized_pass);

    if(Out)
      *Out << results << std::endl;
    
    if(Out && Ouf)
      Ouf.close();

    if((verblevel > 1) && Out)
      *Out << "COM::ParallelTest: Exiting test function (success)" << std::endl;
    
    return(0);
  }
};

int main(int argc,char *argv[])
{
  return(COM::ParallelTest(argc,argv));
}
