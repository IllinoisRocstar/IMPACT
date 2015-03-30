// This produces a PS file for visualization of sparse matrices
// Build with g++ -O3 -o s2ps s2ps.C (or equivalent)

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <cassert>
#include <algorithm>

int main(int argc,char *argv[])
{

  size_t nglobal_dof = 0;
  if(argc < 2) {
    std::cout << argv[0] << " [-o] [-d] <input file list>" << std::endl;
    exit(1);
  }

  int offset = 1;
  bool do_skip = true;
  bool do_order = false;
  std::string test(argv[1]);
  while(test[0] == '-'){
    if(test == "-d"){
      do_skip = false;
      offset++;
    }
    else if(test == "-o"){
      do_order = true;
      offset++;
    }
    test.assign(argv[offset]);
  }
  if(do_skip)
    std::cout << "Skipping extra entry of ndof enabled." << std::endl;
  if(do_order)
    std::cout << "Dof mapping read enabled." << std::endl;

  std::vector<std::string> infiles;
  for(int i = 0;i < (argc-offset);i++)
    infiles.push_back(argv[i+offset]);

  size_t nproc = infiles.size();

  std::ofstream Ouf;
  std::ostringstream Ostr;
  std::string file_base(argv[offset]);
  file_base.erase(file_base.find_first_of("._"));
  Ostr << file_base << "_" << nproc << ".ps";
  
  std::cout << "Creating image of sparsity pattern for " << file_base 
	    << " on " << nproc << " processors." << std::endl
	    << "Output in " << Ostr.str() << "." << std::endl;


  std::vector<std::string>::iterator ai = infiles.begin();
  //  std::vector<size_t> Ap;
  std::vector<size_t> nnz;
  //  std::vector< size_t > Ai;
  std::vector< std::vector<size_t> > stiffness_rows;
  unsigned int proc = 0;
  size_t doffset = 0;
  size_t total_number_of_dofs = 0;
  size_t total_nz = 0;
  // The first entry of Ap is 0, set it here, and 
  // then skip the first entry of Ap for every 
  // processor
  //  Ap.push_back(0);

  while(ai != infiles.end()){
    std::string sname(*ai++);
    std::cout << "Reading " << sname << std::endl;
    std::ifstream Infile;
    int dum;
    
    Infile.open(sname.c_str());
    if(!Infile){
      std::cerr << "Could not open " << sname << std::endl;
      exit(1);
    }
    size_t ndofd;
    Infile >> ndofd;
    std::cout << "Total number of dofs = " << ndofd << std::endl;
    if(total_number_of_dofs == 0){
      //      std::cout << "***WARNING*****" << std::endl;
      total_number_of_dofs = ndofd;
      nnz.resize(total_number_of_dofs,0);
      stiffness_rows.resize(total_number_of_dofs);
      //      std::cout << "Resizing stiffness_rows to " << total_number_of_dofs << std::endl;
    }
    assert(ndofd == total_number_of_dofs);
    Infile >> ndofd;
    std::cout << "Number of local dofs = " << ndofd << std::endl;
    std::vector<size_t> local_dof_to_global(ndofd,0);
    if(do_order){
      //      std::cout << "Reading ordering." << std::endl;
      std::vector<size_t>::iterator ldtgIt = local_dof_to_global.begin();
      while(ldtgIt != local_dof_to_global.end())
	Infile >> *ldtgIt++;
    }
    Infile >> dum;
    //    std::cout << "Read extra entry of " << dum << std::endl;
    assert(dum == ndofd);
    nglobal_dof+=ndofd; // nglobal_dof is a running tally for later sanity check
    //    std::cout << "Nlocal dofs: " << ndofd << std::endl;
    //    std::cout << "doffset = " << doffset << std::endl;

    // Reads first 0 from Ap
    Infile >> dum;
    assert(dum == 0);
    //    size_t min_Ap = std::numeric_limits<size_t>::max();
    //    size_t max_Ap = std::numeric_limits<size_t>::min();
    size_t previous_value = 0;
    for(unsigned int i = 0;i < ndofd;i++){
      Infile >> dum;
      if(!do_order)
	local_dof_to_global[i] = i + doffset;
      nnz[local_dof_to_global[i]] = dum - previous_value;
      //      std::cout << "Global DOF ID for LocalDof[" << i << "]: " 
      //		<< local_dof_to_global[i] << std::endl;
      //      std::cout << "Read " << dum << " from Ap." << std::endl;
      //      std::cout << "Size read: " << dum - previous_value << std::endl;
      //assert(nnz[local_dof_to_global[i]] == (dum - previous_value));
      total_nz += nnz[local_dof_to_global[i]];
      previous_value = dum;
    }
    //      dum += doffset;
    //      Ap.push_back(dum);

    // Report and update the DOF offset
    //    size_t local_nz_entries = *Ap.rbegin() - doffset;
    size_t local_nz_entries = dum; // the last dum value read from loop above
    doffset += ndofd;
    //    std::cout << "doffset = " << doffset << std::endl;
    //    std::cout << "Ap read for processor " << proc << " (min/max): (" << min_Ap 
    //	      << "/" << max_Ap << ")  Total NZ entries:" << local_nz_entries 
    //	      << std::endl;
    std::cout << "Ap read for processor " << proc << " Total NZ entries:" << local_nz_entries 
	      << std::endl;
    // Skip the next value from the file if do_skip is set (default)
    if(do_skip)
      Infile >> dum;


    // Read nnz dofs for each dof row 
    for(size_t i = 0;i < ndofd;i++){
      size_t idof = 0;
      size_t current_dof_id = local_dof_to_global[i] + 1;
      //      std::cout << "current_dof_id = " << current_dof_id << std::endl;
      //      std::cout << "dof row size = " << nnz[current_dof_id-1] << std::endl;
      stiffness_rows[current_dof_id-1].resize(nnz[current_dof_id-1],0);
      //      std::cout << "dof row(" << nnz[current_dof_id-1] << "): ";
      std::vector<size_t>::iterator srIt = stiffness_rows[current_dof_id-1].begin();
      while(idof < nnz[current_dof_id-1]){
	Infile >> dum;
	//	std::cout << dum  << " ";
	*srIt++ = dum;
	idof++;
      }
      //      std::cout << std::endl;
    }

    proc++;
    Infile.close();

  }
  std::cout << "Total NDOF = " << nglobal_dof << std::endl
	    << "Total NZ = " << total_nz << std::endl;
  //  std::cout << "Last Ap entry: " << *Ap.rbegin() << std::endl;
  //  std::cout << "Total NZ: " << Ap[nglobal_dof] << std::endl;
  //  std::cout << "Verify: Ap.size()/Ai.size() = " << Ap.size() 
  //	    << "/" << Ai.size() << std::endl;
  assert(nglobal_dof == total_number_of_dofs);
  size_t skyline = 0;
  for(size_t ii = 1;ii < nglobal_dof;ii++){
    std::vector<size_t>::iterator scIt = stiffness_rows[ii].begin();
    if(*scIt < (ii+1))
      skyline += ((ii+1) - *scIt);
  }
  
  std::ostringstream Ostr2;
  Ostr2 << file_base << " (Np/Ndof/NZ/Skyline): (" << nproc << "/" 
	<< nglobal_dof << "/" << total_nz << "/" << skyline << ")";
  std::string name = Ostr2.str();
  std::cout << name << std::endl;
  std::cout << "Now writing PS file." << std::endl;
  
  double haf  = 0.5;
  double zero = 0.0;
  double conv = 2.54;
  double siz  = 13.0;
  int nr = nglobal_dof;
  int nc = nglobal_dof;
  int maxdim = nglobal_dof;
  int n = nglobal_dof;
  nc++;
  nr++;
  int m = nglobal_dof;
  m++;
  double u2dot = 72.0/conv;
  double paperx = 21.0;
  double lrmrgn = (paperx-siz)/2.0;
  double botmrgn = 2.0;
  double scfct = siz*u2dot/(double)m;
  double frlw = 0.25;
  double fnstit = 0.5;
  double ytitof = 1.0;
  double xtit = paperx/2.0;
  double ytit = botmrgn+siz*nr/(double)m + ytitof;
  double xl = lrmrgn*u2dot - scfct*frlw/2.0;
  double xr = (lrmrgn+siz)*u2dot + scfct*frlw/2.0;
  double yb = botmrgn*u2dot - scfct*frlw/2.0;
  double yt = (botmrgn+siz*nr/m)*u2dot + scfct*frlw/2.0;
  yt = yt + (ytitof+fnstit*0.70)*u2dot;
  double delt = 10.0;
  xl = xl-delt;
  xr = xr+delt;
  yb = yb-delt;
  yt = yt+delt;

  Ouf.open(Ostr.str().c_str());
  Ouf << "%!" << std::endl
      << "%%Creator: Anonymous" << std::endl
      << "%%BoundingBox: " << xl << " " << yb << " " << xr << " " << yt << std::endl 
      << "%%EndComments" << std::endl
      << "/cm {72 mul 2.54 div} def" << std::endl
      << "/mc {72 div 2.54 mul} def" << std::endl
      << "/pnum { 72 div 2.54 mul 20 string" << std::endl
      << "cvs print ( ) print} def" <<  std::endl
      <<  "/Cshow {dup stringwidth pop -2 div 0 rmoveto show} def" << std::endl    
      <<  "gsave" << std::endl
      <<  "/Helvetica findfont " << fnstit << " cm scalefont setfont " << std::endl
      <<  xtit << " cm " << ytit << " cm moveto " << std::endl
      << "(" << name << ") Cshow" << std::endl
      <<  lrmrgn << " cm " << botmrgn << " cm translate" << std::endl
      <<  siz << " cm " << m << " div dup scale " << std::endl
      <<  frlw << " setlinewidth" << std::endl
      <<  "newpath" << std::endl
      <<  0 << " " <<  0 << " moveto" << std::endl
      <<  nc << " " << 0 << " lineto" << std::endl
      <<  nc << " " << nr << " lineto" << std::endl
      <<  0 << " " << nr  << " lineto" << std::endl
      <<  "closepath stroke" << std::endl 
      <<   " 0.2 setlinewidth" << std::endl
      <<  "1 1 translate" << std::endl
      <<  "0.8 setlinewidth" << std::endl
      <<  "/p {moveto 0 -.40 rmoveto " << std::endl
      <<  "           0  .80 rlineto stroke} def" << std::endl;
  for(size_t ii = 0;ii < nglobal_dof;ii++){
    //    size_t istart = Ap[ii];
    //    size_t ilast  = Ap[ii+1];
    //    for(size_t k = istart;k < ilast;k++)
    std::vector<std::vector< size_t > >::iterator srIt = stiffness_rows.begin();
    std::vector<size_t>::iterator scIt = stiffness_rows[ii].begin();
    size_t row_id = ii+1;
    while(scIt != stiffness_rows[ii].end()){
      size_t column_index = *scIt - 1;
      if(!std::binary_search(srIt[column_index].begin(),srIt[column_index].end(),row_id))
	std::cout << *scIt << ":" << row_id << std::endl;
      Ouf << column_index << " " << nglobal_dof-ii-1 << " p" << std::endl;
      scIt++;
    }
  }
  Ouf  <<  "showpage" << std::endl;
  Ouf.close();
  
  return 0;
}
