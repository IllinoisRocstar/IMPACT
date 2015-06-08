
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



#include "Mesh.H"





int meshgen2d(int argc,char *argv[])
{
  //reading the input file and making sure it exists.
  if(!argv[1]){
    std::cerr << argv[0] << ":Error: input file required." << std::endl;
    return(1);
  }

  std::ifstream Inf;
  Inf.open(argv[1]);
  if(!Inf){
    std::cerr << argv[0] << ":Error: Unable to open input file, " << argv[1] << "." 
              << std::endl;
    return(1);
  }

  std::ostringstream outStream;
  std::string line;
  std::vector<double> limits;
  std::vector<unsigned int> sizes;

  //reads for triangle mesh specification
  int wantTriangles=0;
  std::getline(Inf, line);
  
  if (line.at(0)=='1') wantTriangles=1;

  if (line.at(0)=='2') wantTriangles=2;

  //reads all lines to get mesh boundaries and mesh size
  while(std::getline(Inf,line)){
    std::istringstream Istr(line);
    double limit1, limit2;
    int nx;
    Istr >> limit1 >> limit2 >> nx;
    limits.push_back(limit1);
    limits.push_back(limit2);
    sizes.push_back(nx);
  }
  Inf.close();
  assert(limits.size() == 6);
  assert(sizes.size() == 3);
 

  if((sizes[0] > 0) && (sizes[1] > 0) && (sizes[2] > 0)){
    std::cerr << "Invalid sizes specifications (only 2d supported): (" 
              << sizes[0] << "," << sizes[1] << "," 
              << sizes[2] << ")" << std::endl;
    return(1);
  }

  int nX = (sizes[0] > 0 ? sizes[0] : 1);
  int nY = (sizes[1] > 0 ? sizes[1] : 1);
  int nZ = (sizes[2] > 0 ? sizes[2] : 1);
  

  std::vector<double> coordinates;
  int numberElementsX = (nX-1);
  int numberElementsY = (nY-1);
  int numberElementsZ = (nZ-1); 


  //nDirs for providing one axis for each direction.
  int nDir1 = numberElementsX;
  int nDir2 = numberElementsY;
  

  if(nDir1 == 0){
    nDir1 = numberElementsY;
  
   
    nDir2 = numberElementsZ;
  } else if(nDir2 == 0){
    nDir2 = numberElementsZ;
    
  }
  int numberOfNodes=nX * nY * nZ;
  if (wantTriangles==2) numberOfNodes = (nX * nY * nZ)+(nDir1*nDir2);
  outStream << numberOfNodes << std::endl;

  //generating the nodal coordinates
  double xSpacing = 0;
  if(nX > 1) xSpacing = (limits[1] - limits[0])/(nX-1);
  double ySpacing = 0;
  if(nY > 1) ySpacing = (limits[3] - limits[2])/(nY-1);
  double zSpacing = 0;
  if(nZ > 1) zSpacing = (limits[5] - limits[4])/(nZ-1);
  for(int iZ = 0;iZ < nZ;iZ++){
    for(int iY = 0;iY < nY;iY++){
      for(int iX = 0; iX < nX;iX++){    
        double xCoord = limits[0] + iX*xSpacing;
        double yCoord = limits[2] + iY*ySpacing;
        double zCoord = limits[4] + iZ*zSpacing;
        coordinates.push_back(xCoord);
        coordinates.push_back(yCoord);
        coordinates.push_back(zCoord);
        outStream << xCoord << " " << yCoord << " " << zCoord << std::endl;
      }
    }
  } 
  
 


	

  std::vector<std::vector<unsigned int> > connectivityArray;

  if (wantTriangles==1) {
    int numberOfElements=(nDir1*nDir2)*2;
	
	
    int nElem = 0;
    int nCount = 0;

    while(nElem++ < numberOfElements){
      std::vector<unsigned int> element;
      element.push_back(nCount+1);
      nCount=nCount+nDir1+1;
      element.push_back(nCount+1);
      nCount++;
      element.push_back(nCount+1);
      connectivityArray.push_back(element);
      nElem++;
      element.clear();
      element.push_back(nCount+1);
      nCount=nCount-nDir1-1;
      element.push_back(nCount+1);
      nCount--;
      element.push_back(nCount+1);
      nCount++;
      if(!((nCount+1)%(nDir1+1))) nCount++;
      connectivityArray.push_back(element);
    }

  }




  if (wantTriangles==2) {
    //creates the centroid nodal coordinates for use in breaking the quad into triangles
	
    int numberOfElements=(nDir1*nDir2)*4;
    int nElem=0;	

    double centroidX=limits[0]+xSpacing/2;
    if (xSpacing==0) centroidX=limits[0];
    double centroidY=limits[2]+ySpacing/2;
    if (ySpacing==0) centroidY=limits[2];
    double centroidZ=limits[4]+zSpacing/2;
    if (zSpacing==0) centroidZ=limits[4];
	
    double startXCentroid=centroidX;
    double startYCentroid=centroidY;

    int cCount=0;
	

    while (nElem++ < (numberOfElements/4)) {
	
      coordinates.push_back(centroidX);
      coordinates.push_back(centroidY);
      coordinates.push_back(centroidZ);
      outStream << centroidX << " " << centroidY << " " << centroidZ << std::endl;
      cCount++;


      if(!((cCount)%(nDir1))){
	
	centroidZ=centroidZ+zSpacing;
	if (numberElementsX!=0) {
		
          centroidY=centroidY+ySpacing;
          centroidX=startXCentroid;		
	} else {
          centroidY=startYCentroid;          
	}	
      } else {
	centroidX=centroidX+xSpacing;
      }
      if (numberElementsX==0) {
        centroidY=centroidY+ySpacing;		
      } 
    }
	
    nElem = 0;
    int nCount = 0;
    int cIndex=(coordinates.size()/3)-(numberOfElements/4);
	
    //specifying the connectivity of nodes to actually make the triangles
    while(nElem < numberOfElements){
      std::vector<unsigned int> element;
      element.push_back(nCount+1);
      nCount=nCount+nDir1+1;
      element.push_back(nCount+1);
      element.push_back(cIndex+1);
      nElem++;
      connectivityArray.push_back(element);
      element.clear();

      element.push_back(nCount+1);
      nCount++;
      element.push_back(nCount+1);
      element.push_back(cIndex+1);
      connectivityArray.push_back(element);
      nElem++;
      element.clear();

      element.push_back(nCount+1);
      nCount=nCount-nDir1-1;
      element.push_back(nCount+1);
      element.push_back(cIndex+1);
      connectivityArray.push_back(element);
      nElem++;
      element.clear();

      element.push_back(nCount+1);
      nCount--;
      element.push_back(nCount+1);
      element.push_back(cIndex+1);
      nCount++;
      connectivityArray.push_back(element);
      nElem++;
      element.clear();
	
      cIndex++;

      if(!((nCount+1)%(nDir1+1))) nCount++;
    		
    }

  } else {
    //does connectivity for quads
    int numberOfElements = nDir1*nDir2;
    
    
    int nElem = 0;
    int nCount = 0;
    while(nElem++ < numberOfElements){
      std::vector<unsigned int> element;
      element.push_back(nCount+1);
      element.push_back((nCount++)+(nDir1+1)+1);
      element.push_back(nCount+(nDir1+1)+1);
      element.push_back(nCount+1);
      if(!((nCount+1)%(nDir1+1))) nCount++;
      connectivityArray.push_back(element);
    }
    
  }
  
  std::vector<std::vector<unsigned int> >::iterator conIt = connectivityArray.begin();
  outStream << connectivityArray.size() << std::endl;

  //writing the connectivity array to the outstream
  while(conIt != connectivityArray.end()){
    //std::cout << "Element " << (conIt - connectivityArray.begin())+1 << ": (";
    std::vector<unsigned int>::iterator elemIt = conIt->begin();
    while(elemIt != conIt->end())
      outStream << *elemIt++ << " ";
    outStream << std::endl;
    conIt++;
  }
  
  //  std::cout << outStream.str() << std::endl;
  std::istringstream inStream(outStream.str());
  SolverUtils::Mesh::UnstructuredMesh unMesh;
  SolverUtils::Mesh::ReadMeshFromStream(unMesh,inStream);
  std::ostringstream vtkOut;
  SolverUtils::Mesh::WriteVTKToStream("testMesh",unMesh,vtkOut);
  std::cout << vtkOut.str();

  return(0);
}

int main (int argc, char *argv[]) {
	return(meshgen2d(argc, argv));
}
