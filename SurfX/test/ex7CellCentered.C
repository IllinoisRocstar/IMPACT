/* *******************************************************************
 * Rocstar Simulation Suite                                          *
 * Copyright@2015, Illinois Rocstar LLC. All rights reserved.        *
 *                                                                   *
 * Illinois Rocstar LLC                                              *
 * Champaign, IL                                                     *
 * www.illinoisrocstar.com                                           *
 * sales@illinoisrocstar.com                                         *
 *                                                                   *
 * License: See LICENSE file in top level of distribution package or *
 * http://opensource.org/licenses/NCSA                               *
 *********************************************************************/
/* *******************************************************************
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,   *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES   *
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND          *
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE CONTRIBUTORS OR           *
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   *
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE    *
 * USE OR OTHER DEALINGS WITH THE SOFTWARE.                          *
 *********************************************************************/

#include "com.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <cassert>
#include <sstream>
#include <cmath>

COM_EXTERN_MODULE( SurfX);
COM_EXTERN_MODULE( SimOut);


void makeWindow(std::string name, int windowNumber){

  if(windowNumber == 0){
    COM_new_window(name.c_str());
    std::string solnName = name + ".soln";
    COM_new_dataitem(solnName.c_str(), 'e', COM_DOUBLE, 1, "m/s");

    std::string compName = name + ".comp";
    COM_new_dataitem(compName.c_str(), 'e', COM_DOUBLE, 1, "m/s");
  }
  else{
    COM_new_window(name.c_str());
    std::string solnName = name + ".soln";
    COM_new_dataitem(solnName.c_str(), 'e', COM_DOUBLE, 1, "m/s");

    std::string compName = name + ".comp";
    COM_new_dataitem(compName.c_str(), 'e', COM_DOUBLE, 1, "m/s");
  }
}

void initUnstructuredMesh( std::vector<double> &coors, 
                           std::vector<int> &elmts, 
                           int nrow, int ncol, 
                           int rank, int nproc, int type,
                           int &nnodes,int &nelem) {
  
  // consider the processors as a 2*(nproc/2) grid
  int proc_col=nproc;
  if (nproc%2 ==0) {
    proc_col=nproc/2;
  }
  else {
    proc_col=nproc;
  }

  int row=rank/proc_col, col=rank%proc_col;

  const double width=100., length=100.;

  if(type == 0){
    nnodes = (nrow*ncol + (nrow-1)*(ncol-1));
    nelem = 4*(nrow-1)*(ncol-1);
    elmts.resize(nelem*3);
  } else if(type == 1){
    nnodes = nrow*ncol;
    nelem = 2*(nrow-1)*(ncol-1);
    elmts.resize(nelem*3);
  } else {
    nnodes = nrow*ncol;
    nelem = (nrow-1)*(ncol-1);
    elmts.resize(nelem*4);
  }
  coors.resize(nnodes*3);

  for (int i=0; i<nrow; ++i) {
    for (int j=0; j<ncol; ++j) {
      coors[3*(i*ncol+j) + 0]=col*length+length/(ncol-1)*j;
      coors[3*(i*ncol+j) + 1]=row*width+width/(nrow-1)*i;
      coors[3*(i*ncol+j) + 2]=0;
    }
  }

  //generating the nodal coordinates
  double xSpacing = 0;
  if(ncol > 1) xSpacing =  width/(ncol-1);
  double ySpacing = 0;
  if(nrow > 1) ySpacing = length/(nrow-1);

  if (type == 0){
     for(int i=0; i < nrow-1; ++i){
        for(int j=0; j < ncol-1; ++j){
           int count = nrow*ncol;
           int elmidx = 4*(i*(ncol-1)+j);

           coors[3*(count+i*(ncol-1)+j) + 0] = xSpacing*j + xSpacing/2 + col*length; 
           coors[3*(count+i*(ncol-1)+j) + 1] = ySpacing*i + ySpacing/2 + row*width; 
           coors[3*(count+i*(ncol-1)+j) + 2] = 0;

           // Nodes per triangle element - 'bottom' triangle
           elmts[3*(elmidx) + 0]=i*ncol+j+1;
           elmts[3*(elmidx) + 1]=i*(ncol-1)+j+count+1;
           elmts[3*(elmidx) + 2]=i*ncol+j+2;

           // Nodes per triangle element - 'top' triangle
           elmts[3*(elmidx+1) + 0]=i*ncol+j+ncol+1;
           elmts[3*(elmidx+1) + 1]=i*ncol+j+ncol+2;
           elmts[3*(elmidx+1) + 2]=i*(ncol-1)+j+count+1;

           // Nodes per triangle element - 'left' triangle
           elmts[3*(elmidx+2) + 0]=i*ncol+j+1;
           elmts[3*(elmidx+2) + 1]=i*ncol+j+ncol+1;
           elmts[3*(elmidx+2) + 2]=i*(ncol-1)+j+count+1;

           // Nodes per triangle element - 'right' triangle
           elmts[3*(elmidx+3) + 0]=i*ncol+j+2;
           elmts[3*(elmidx+3) + 1]=i*(ncol-1)+j+count+1;
           elmts[3*(elmidx+3) + 2]=i*ncol+j+ncol+2;

       }
     }
  }

  if (type == 1){
     for(int i=0; i < nrow-1; ++i){
        for(int j=0; j < ncol-1; ++j){
           int elmidx = 2*(i*(ncol-1)+j);

           // Nodes per triangle element - 'bottom' triangle
           elmts[3*(elmidx) + 0]=i*ncol+j+1;
           elmts[3*(elmidx) + 1]=i*ncol+j+ncol+1;
           elmts[3*(elmidx) + 2]=i*ncol+j+2;

           // Nodes per triangle element - 'top' triangle
           elmts[3*(elmidx+1) + 0]=i*ncol+j+ncol+1;
           elmts[3*(elmidx+1) + 1]=i*ncol+j+ncol+2;
           elmts[3*(elmidx+1) + 2]=i*ncol+j+2;

       }
     }
  }

  if (type == 2){
     for (int i=0; i<nrow-1; ++i) {
       for (int j=0; j<ncol-1; ++j) {
           int elmidx = i*(ncol-1)+j;

           elmts[4*elmidx + 0]=i*ncol+j+1;
           elmts[4*elmidx + 1]=i*ncol+j+ncol+1;
           elmts[4*elmidx + 2]=i*ncol+j+ncol+2;
           elmts[4*elmidx + 3]=i*ncol+j+2;
       }
     }
  }
}

template <class TYPE> void ReportDataVector(
           std::vector< std::vector< std::vector<TYPE> > > data, int stride){
  for(int i=0; i < data.size(); i++){
    std::cout << "Window " << i << ":" << std::endl;
    for(int j=0; j < data[i].size(); j++){
      std::cout << "Pane " << j << ":" << std::endl;
      for(int k=0; k < data[i][j].size()/stride; k++){
        for(int l=0; l < stride; l++){
          std::cout << data[i][j][stride*k + l] << " ";
        }
        std::cout << std::endl;
      }
    }
  } 
}

void ReportUnstructuredMesh(std::vector<std::vector<double> > &coordinates,
                            std::vector<std::vector<int> >    &elements,
                            int element_size,
                            std::ostream &outStream)
{
  int npanes = coordinates.size();
  outStream << "Number of panes: " << npanes << std::endl;
  std::vector<std::vector<double> >::iterator paneCoordIt = coordinates.begin();
  std::vector<std::vector<int> >::iterator paneElemIt   = elements.begin();
  while(paneCoordIt != coordinates.end()){
    outStream << "Pane " << paneCoordIt - coordinates.begin() + 1 
              << ":" << std::endl;
    std::vector<double> &paneCoordinates(*paneCoordIt++);
    std::vector<int> &paneElements(*paneElemIt++);
    int nnodes = paneCoordinates.size()/3;
    int nelem  = paneElements.size()/element_size;
    outStream << "Number of nodes: " << nnodes << std::endl
              << "Nodes: " << std::endl;
    for(int i = 0;i < nnodes;i++)
      outStream << i+1 << ": (" << paneCoordinates[i*3] << ","
                << paneCoordinates[i*3+1] << "," 
                << paneCoordinates[i*3+2] << ")" << std::endl;
    outStream << "Number of elements: " << nelem << std::endl
              << "Elements: " << std::endl;
    for(int i = 0;i < nelem;i++){
      outStream << i+1 << ": (";
      for(int j = 0;j < element_size;j++){
        outStream << paneElements[i*element_size+j];
        if(j < (element_size-1))
          outStream << ",";
      }
      outStream << ")" << std::endl;
    }
  }
}

//area function assumes that elements are two dimensional (ignores z dimension)
double area2d(std::vector<int> &elements, std::vector<double> &coords, int index, 
          int meshType, std::vector<double> &centroid){
 
  double area=0;
  std::vector<double> x, y;
  int node, stride = (meshType < 2 ? 3 : 4);
  double xc=0, yc=0;

  x.resize(stride);
  y.resize(stride);

  for(int i=0; i < stride; i++){
   node = elements[stride*index + i] - 1;
   x[i] = coords[3*node + 0];
   y[i] = coords[3*node + 1];
   xc+=x[i];
   yc+=y[i];
  }

  xc/=stride;
  yc/=stride;
  centroid[0] = xc;
  centroid[1] = yc;

  for(int i=0; i < stride-1; i++){
    area += (x[i]*y[i+1] - x[i+1]*y[i]);
  }
  area += (x[stride-1]*y[0] - x[0]*y[stride-1]);
  area = fabs(area);
  area /= 2.0;

  return area; 

}

int main(int argc, char *argv[]){
  
  int comm_rank=0;
  int comm_size=1;
  int npanes=4;
  int nwindows=3;

  int verbLevel = 1;

  std::vector<std::string> windowNames(nwindows);
  std::vector< std::vector< std::vector<double> > >  coords(nwindows);
  std::vector< std::vector< std::vector<int> > >     elements(nwindows);
  std::vector< std::vector< std::vector<double> > >  soln(nwindows);
  std::vector< std::vector< std::vector<double> > >  comp(nwindows);
  std::vector< std::vector<double> >  sums(nwindows);
  std::vector<double> centroid(2);

  if ( comm_rank == 0)
    std::cout << "Initializing COM and SurfX" << std::endl;
  COM_init( &argc, &argv);

  COM_set_profiling( 1);

  COM_LOAD_MODULE_STATIC_DYNAMIC( SurfX, "RFC");
  int RFC_clear    = COM_get_function_handle("RFC.clear_overlay");
  int RFC_read     = COM_get_function_handle( "RFC.read_overlay");
  int RFC_write    = COM_get_function_handle("RFC.write_overlay");
  int RFC_overlay  = COM_get_function_handle("RFC.overlay");
  int RFC_transfer = COM_get_function_handle( "RFC.least_squares_transfer");
  int RFC_interp   = COM_get_function_handle( "RFC.interpolate");
  int RFC_extrap   = COM_get_function_handle( "RFC.extrapolate");

  for(int i=0; i < nwindows; i++)
    {
      std::stringstream ss;
      ss << "Window" << i;
      ss >> windowNames[i];
      if ( comm_rank == 0 && verbLevel > 1)
        std::cout << "Creating window " << ss.str() << std::endl;
      makeWindow(windowNames[i], i);
    }

  for(int i=0; i < nwindows; i++){
    std::string &windowName(windowNames[i]);

    int nrow, ncol;
    nrow = i+3;
    ncol = i+2;

    int meshType = i%3;
    int nnodes = 0;
    int nelems = 0;
    int element_size = (meshType < 2 ? 3 : 4);

    if(verbLevel > 1){
      std::cout << windowName << ":" << std::endl;
      std::cout << nrow << " by " << ncol << std::endl;
    }

    coords[i].resize(npanes);
    elements[i].resize(npanes);
    soln[i].resize(npanes);
    comp[i].resize(npanes);
    sums[i].resize(npanes);

    for(int j=0; j < npanes; j++){

      int pane_id = j+1;

      initUnstructuredMesh(coords[i][j],elements[i][j],
                           nrow,ncol,j,npanes,meshType,
                           nnodes,nelems);

      soln[i][j].resize( nelems );
      if(i == 0)
        comp[i][j].resize( nnodes );
      else
        comp[i][j].resize( nelems );

      sums[i][j] = 0.0;
      
      for(int k=0; k < nelems; k++){
        double area = area2d(elements[i][j], coords[i][j], k, meshType, centroid);
        //std::cout << "element " << k << " centroid: " << centroid[0]
        //          << " " << centroid[1] << std::endl;
        //soln[i][j][3*k+0] = centroid[0];
        //soln[i][j][3*k+1] = centroid[1];
        //soln[i][j][3*k+2] = 0.0;
        //comp[i][j][3*k+0] = -1.0;
        //comp[i][j][3*k+1] = -1.0;
        //comp[i][j][3*k+2] = -1.0;
        soln[i][j][k] = double(k)*double(k)+1.0;
        comp[i][j][k] = -1.0;
        sums[i][j] += soln[i][j][k]*area;
        if(verbLevel > 1){
         std::cout << "soln[" << i << "][" << j << "][" << k 
                   << "] = " << coords[i][j][k] + 3.0 << std::endl;
        }
      }
      COM_set_size( (windowName+".nc"), pane_id, nnodes);
      COM_set_array( (windowName+".nc"), pane_id, &coords[i][j][0]); 

      std::string connectivityName;
      connectivityName = ((meshType < 2) ? (windowName+".:t3:") : 
                          (windowName+".:q4:"));
      COM_set_size(connectivityName, pane_id, nelems);
      COM_set_array(connectivityName, pane_id, &elements[i][j][0]);

      COM_set_array((windowName+".soln"), pane_id, &soln[i][j][0]); 
      COM_set_array((windowName+".comp"), pane_id, &comp[i][j][0]); 


    }//j Loop over panes
    COM_window_init_done(windowName.c_str());
    if(verbLevel > 1)
      ReportUnstructuredMesh(coords[i],elements[i],element_size,std::cout);
  }//i Loop over windows
  if(verbLevel > 1){
    std::cout << "soln: " << std::endl;
    ReportDataVector(soln,1);
    std::cout << "sums: " << std::endl;
    for(int i=0; i < nwindows; i++){
      std::cout << "Window " << i << std::endl;
      for(int j=0; j < npanes; j++){
        std::cout << sums[i][j] << std::endl;
      }
    }
  }

  int tri0_mesh  = COM_get_dataitem_handle("Window0.mesh");
  int tri1_mesh  = COM_get_dataitem_handle("Window1.mesh");
  int quad_mesh  = COM_get_dataitem_handle("Window2.mesh");

  // Perform mesh overlay
  const char *format = "HDF";

  if( comm_size == 1){

    std::cout << "Overlaying meshes..." << std::endl;

    COM_call_function( RFC_overlay, &tri0_mesh, &tri1_mesh);
    std::cout << "Overlay 01 done." << std::endl;
    COM_call_function( RFC_write,   &tri0_mesh, &tri1_mesh, "tri01", "tri10", format);
    COM_call_function( RFC_clear,   "Window0", "Window1");

    COM_call_function( RFC_overlay, &tri0_mesh, &quad_mesh);
    COM_call_function( RFC_write,   &tri0_mesh, &quad_mesh, "tri02", "quad20", format);
    COM_call_function( RFC_clear,   "Window0", "Window2");

    COM_call_function( RFC_overlay, &tri1_mesh, &quad_mesh);
    COM_call_function( RFC_write,   &tri1_mesh, &quad_mesh, "tri12", "quad21", format);
    COM_call_function( RFC_clear,   "Window1", "Window2");
    
  }

  int tri0_soln = COM_get_dataitem_handle( "Window0.soln");
  int tri1_soln = COM_get_dataitem_handle( "Window1.soln");
  int quad_soln = COM_get_dataitem_handle( "Window2.soln");
  
  int tri0_comp = COM_get_dataitem_handle( "Window0.comp");
  int tri1_comp = COM_get_dataitem_handle( "Window1.comp");
  int quad_comp = COM_get_dataitem_handle( "Window2.comp");

  ////////////////////////////////////////////////////////////////  
  COM_call_function( RFC_read, &tri0_mesh, &tri1_mesh, NULL, "tri01", "tri10", format);
  ///////////////////////////////////////////////////////////////// 
  std::cout << "Transfer Window 1 to Window 0" << std::endl;
  COM_call_function( RFC_transfer, &tri1_soln, &tri0_comp);
  if(verbLevel > 1){
    std::cout << "comp: " << std::setprecision(12) << std::endl;
    ReportDataVector(comp,1);
  }

  int check_from=1, check_to=0;
  double check_sum;
  bool pass = true;
  double tolerance= 1.0e-2;
  if(verbLevel > 1)
    std::cout << "check_sums: (sums comparison)" << std::endl;
  for(int j=0; j < npanes; j++){
    check_sum=0.0;
    for(int k=0; k < comp[check_to][j].size(); k++){
      double area = area2d(elements[check_to][j], coords[check_to][j], k, check_to, centroid);
      check_sum += area*comp[check_to][j][k];
    }
    if(verbLevel > 1){
      std::cout << std::setprecision(12) << check_sum << " ("  << sums[check_from][j]  
                << ")" << std::endl;
      std::cout << "diff =  " << fabs(check_sum - sums[check_from][j]) << std::endl;
    }
    if(fabs(check_sum - sums[check_from][j]) > tolerance)
      pass = false;
  }
  std::cout << "Cell centered solution from Window " << check_from 
            << " to Window " << check_to << " " 
            << (pass ? "passed" : "failed") << "." << std::endl;
  ///////////////////////////////////////////////////////////////// 
  std::cout << "Transfer Window 0 to Window 1" << std::endl;
  COM_call_function( RFC_transfer, &tri0_soln, &tri1_comp);
  if(verbLevel > 1){
    std::cout << "comp: " << std::setprecision(12) << std::endl;
    ReportDataVector(comp,1);
  }

  check_from=0;
  check_to=1;
  pass = true;
  if(verbLevel > 1)
    std::cout << "check_sums: (sums comparison)" << std::endl;
  for(int j=0; j < npanes; j++){
    check_sum=0.0;
    for(int k=0; k < comp[check_to][j].size(); k++){
      double area = area2d(elements[check_to][j], coords[check_to][j], k, check_to, centroid);
      check_sum += area*comp[check_to][j][k];
    }
    if(verbLevel > 1){
      std::cout << std::setprecision(12) << check_sum << " ("  << sums[check_from][j]  
                << ")" << std::endl;
      std::cout << "diff =  " << fabs(check_sum - sums[check_from][j]) << std::endl;
    }
    if(fabs(check_sum - sums[check_from][j]) > tolerance){
      pass = false;
    }
  }
  std::cout << "Cell centered solution from Window " << check_from 
            << " to Window " << check_to << " " 
            << (pass ? "passed" : "failed") << "." << std::endl;
  ////////////////////////////////////////////////////////////////
  COM_call_function( RFC_clear,   "Window0", "Window1");  
  COM_call_function( RFC_read, &tri0_mesh, &quad_mesh, NULL, "tri02", "quad20", format);
  ///////////////////////////////////////////////////////////////// 
  std::cout << "Transfer Window 0 to Window 2" << std::endl;
  COM_call_function( RFC_transfer, &tri0_soln, &quad_comp);
  if(verbLevel > 1){
    std::cout << "comp: " << std::setprecision(12) << std::endl;
    ReportDataVector(comp,1);
  }

  check_from=0;
  check_to=2;
  pass = true;
  if(verbLevel > 1)
  std::cout << "check_sums: (sums comparison)" << std::endl;
  for(int j=0; j < npanes; j++){
    check_sum=0.0;
    for(int k=0; k < comp[check_to][j].size(); k++){
      double area = area2d(elements[check_to][j], coords[check_to][j], k, check_to, centroid);
      check_sum += area*comp[check_to][j][k];
    }
    if(verbLevel > 1){
      std::cout << std::setprecision(12) << check_sum << " ("  << sums[check_from][j]  
                << ")" << std::endl;
      std::cout << "diff =  " << fabs(check_sum - sums[check_from][j]) << std::endl;
    }
    if(fabs(check_sum - sums[check_from][j]) > tolerance)
      pass = false;
  }
  std::cout << "Cell centered solution from Window " << check_from 
            << " to Window " << check_to << " " 
            << (pass ? "passed" : "failed") << "." << std::endl;
  ///////////////////////////////////////////////////////////////// 
  std::cout << "Transfer Window 2 to Window 0" << std::endl;
  COM_call_function( RFC_transfer, &quad_soln, &tri0_comp);
  if(verbLevel > 1){
    std::cout << "comp: " << std::setprecision(12) << std::endl;
    ReportDataVector(comp,1);
  }

  check_from=2;
  check_to=0;
  pass = true;
  if(verbLevel > 1)
    std::cout << "check_sums: (sums comparison)" << std::endl;
  for(int j=0; j < npanes; j++){
    check_sum=0.0;
    for(int k=0; k < comp[check_to][j].size(); k++){
      double area = area2d(elements[check_to][j], coords[check_to][j], k, check_to, centroid);
      check_sum += area*comp[check_to][j][k];
    }
    if(verbLevel > 1){
      std::cout << std::setprecision(12) << check_sum << " ("  << sums[check_from][j]  
                << ")" << std::endl;
      std::cout << "diff =  " << fabs(check_sum - sums[check_from][j]) << std::endl;
    }
    if(fabs(check_sum - sums[check_from][j]) > tolerance)
      pass = false;
  }
  std::cout << "Cell centered solution from Window " << check_from 
            << " to Window " << check_to << " " 
            << (pass ? "passed" : "failed") << "." << std::endl;
  ///////////////////////////////////////////////////////////////// 
  COM_call_function( RFC_clear,   "Window0", "Window2");  
  COM_call_function( RFC_read, &tri1_mesh, &quad_mesh, NULL, "tri12", "quad21", format);
  ///////////////////////////////////////////////////////////////// 
  std::cout << "Transfer Window 1 to Window 2" << std::endl;
  COM_call_function( RFC_transfer, &tri1_soln, &quad_comp);
  if(verbLevel > 1){
    std::cout << "comp: " << std::setprecision(12) << std::endl;
    ReportDataVector(comp,1);
  }

  check_from=1;
  check_to=2;
  pass = true;
  if(verbLevel > 1)
  std::cout << "check_sums: (sums comparison)" << std::endl;
  for(int j=0; j < npanes; j++){
    check_sum=0.0;
    for(int k=0; k < comp[check_to][j].size(); k++){
      double area = area2d(elements[check_to][j], coords[check_to][j], k, check_to, centroid);
      check_sum += area*comp[check_to][j][k];
    }
    if(verbLevel > 1){
      std::cout << std::setprecision(12) << check_sum << " ("  << sums[check_from][j]  
                << ")" << std::endl;
      std::cout << "diff =  " << fabs(check_sum - sums[check_from][j]) << std::endl;
    }
    if(fabs(check_sum - sums[check_from][j]) > tolerance)
      pass = false;
  }
  std::cout << "Cell centered solution from Window " << check_from 
            << " to Window " << check_to << " " 
            << (pass ? "passed" : "failed") << "." << std::endl;
  ///////////////////////////////////////////////////////////////// 
  std::cout << "Transfer Window 2 to Window 1" << std::endl;
  COM_call_function( RFC_transfer, &quad_soln, &tri1_comp);
  if(verbLevel > 1){
    std::cout << "comp: " << std::setprecision(12) << std::endl;
    ReportDataVector(comp,1);
  }

  check_from=2;
  check_to=1;
  pass = true;
  if(verbLevel > 1)
    std::cout << "check_sums: (sums comparison)" << std::endl;
  for(int j=0; j < npanes; j++){
    check_sum=0.0;
    for(int k=0; k < comp[check_to][j].size(); k++){
      double area = area2d(elements[check_to][j], coords[check_to][j], k, check_to, centroid);
      check_sum += area*comp[check_to][j][k];
    }
    if(verbLevel > 1){
      std::cout << std::setprecision(12) << check_sum << " ("  << sums[check_from][j]  
                << ")" << std::endl;
      std::cout << "diff =  " << fabs(check_sum - sums[check_from][j]) << std::endl;
    }
    if(fabs(check_sum - sums[check_from][j]) > tolerance)
      pass = false;
  }
  std::cout << "Cell centered solution from Window " << check_from 
            << " to Window " << check_to << " " 
            << (pass ? "passed" : "failed") << "." << std::endl;
  ///////////////////////////////////////////////////////////////// 
  COM_call_function( RFC_clear,   "Window1", "Window2");  
  ///////////////////////////////////////////////////////////////// 

  for(int i=0; i < nwindows; i++){
    COM_delete_window(windowNames[i]);
  }
  COM_finalize();

  return 0;
}
