
/** @file meshgen2d.C
 *  @brief For dynamically generating a 2d (vtk) mesh based on an input file
 *
 *   Usage
 *  ------------------------
 *  First make a text file. 
 *  In the text file you will have a series of lines
 *  - The first line will specify the type of mesh.
 *    + Put a 0 for quads, 1 for right triangles, and 2 for isosceles triangles
 *  - The next lines are for x y and z, one axis per line.
 *    + For these, there are 3 numbers per line, with a space in between numbers.
 *    + The first two numbers are the limits of the mesh in coordinate space.
 *    + The last number is the number of points to be drawn on that axis.
 *   Make sure the program is built, and you know where the executable is. 
 *   To run the program, you need to specify where the input text file is in the command line.
 *   Example: ./meshgen2d "path to input file"
 *   Replace "path to input file" with a path to the text file you made.
 *   The default output is to write the VTK to stdout, but you can easily redirect this to a .vtk file
 *   By doing > blah.vtk
 */
 
/// @author Mike Campbell
/// @author Brian Weisberg
/// @author Woohyun Kim 
/// @author Illinois Rocstar LLC \n\n



#include "MeshUtils.H"




int main (int argc, char *argv[]) {
  return(SolverUtils::MeshUtils::meshgen2d(argc, argv));
}
