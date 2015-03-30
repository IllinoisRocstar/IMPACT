/// 
/// @file
/// @ingroup impact_group
/// @brief Main for example parallel program.
/// @author Mike Campbell (mtcampbe@illinois.edu)
/// @date 
/// 
#include "ExampleProgram.H"

typedef IMPACT::ExampleProgram::PEProgramType ProgramType;

int main(int argc,char *argv[])
{
  return(IMPACT::ExampleProgram::Driver<ProgramType>(argc,argv));
}
