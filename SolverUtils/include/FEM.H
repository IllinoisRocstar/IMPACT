///
/// \file 
/// \ingroup support
/// \brief Finite Element Stuff
///
/// All mesh data structures (MDS) will follow the CGNS conventions:\n
/// \n \verbatim
/// Triangle Edges: 12 23 31    (counterclockwise)
/// Square Edges:   12 23 34 41 (ccw)
/// Tet Faces:      132 241 342 143
/// Pyr Faces:      1432 251 352 453 154
/// Prism Faces:    2541 3652 1463 132 456
/// Hex Faces:      1432 2651 3762 4873 1584 5678 
/// \endverbatim
/// \image latex elements.eps "Element Types"
/// \image html elements.jpg "Element Types" width=10cm
/*
        4                                              6   
       /|\             5       8---------7            /|\ 
      / | \         /  |       | \       |\          / | \
     /  |  \       /  /        |  \      | \        /  |  \
    /   |   \     /   |        |   5--------\6     /   3   \
   1----|----3   4----|-3      4---|-----3   |    4---------5
    \   |   /     \  /   \      \  |      \  |    |  /   \  |
     \  |  /       \ |    \      \ |       \ |    | /     \ |
      \ | /         \|     \      \|        \|    |/       \|
        2            1------2      1---------2    1---------2
 
*/
#ifndef __FEM_H__
#define __FEM_H__

#include "PMesh.H"
#include <algorithm>
namespace SolverUtils { namespace FEM {
  
  typedef IRAD::Primitive::ubyte MethodIDType;
  typedef std::string MethodNameType;
  typedef std::string FieldNameType;
  typedef IRAD::Primitive::ubyte FieldIDType;
  class MethodDescriptor : public std::vector<FEM::MethodNameType> {
  public:
    FEM::FieldNameType FieldName(FEM::MethodIDType id){ return (*this)[id];};
  };
  //  typedef std::vector<NameType> MethodDescriptor;
  //  typedef std::pair<NameType,std::vector<NameType> > MethodDescriptor;
  typedef std::pair<FEM::MethodIDType,std::vector<FEM::FieldIDType> > MethodIDDescriptor;

// Given a DOF and a container of DOF's, assemble to the
// stiffness row. Brute force searches for every ij.
//  template<typename DOFDataType,typename DOFType,typename DOFContType,typename DOFDataContType,typename KType>
//  void BruteForceAssemble(DOFType &i,DOFContType &j,DOFDataContType &dofdat,KType &k) {
//    typename DOFContType::iterator jIt = j.begin();
//    typename DOFDataContType::iterator ddIt = dofdat.begin();
//    while(jIt != j.end()) {
//      IndexType jid = *jIt++;
//      DOFDataType dd = *ddIt++;
//      if(jid > 0)
//	k.element(i,j) += dd;
//    }
//  };
  
//  template<typename DOFType,typename DOFContType,typename DOFDataType,typename KType>
//  void AssembleOrderedRow(DOFType &row,DOFContType &j,DOFDataType *dofdat,KType &k) {
//    typename DOFContType::iterator jIt = j.begin();
//    IndexType dindex = 0;
//    if(*jIt == 0){
//      jIt++;
//      dindex++;
//    }
//    IndexType kindex = 0;
//    IndexType kend = k.get_index(row+1);
    // returns a dof iterator and sets kindex at the same time
  //    typename DOFContType::iterator dcIt = k.get_dof_iterator(row,j,kindex);
  //    while(kindex < kend){
  //      IndexType jind = *jIt;
  //      if(*dcIt == jind){
  //	k[kindex] += *ddIt++;
  //	jIt++;
  //      }
  //      dcIt++;
  //      kindex++;
  //    }
  //  };

  // Given a DOF and a container of DOF's, assemble to the
  // stiffness row. Brute force searches for every ij.
  template<typename DOFType,typename DOFContType,typename DOFDataType,typename KType>
  void BruteForceAssemble(DOFType &i,DOFContType &j,DOFDataType *dofdat,KType &k) {
    typename DOFContType::iterator jIt = j.begin();
    Mesh::IndexType dofdatindex = 0;
    while(jIt != j.end()) {
      Mesh::IndexType jid = *jIt++;
      if(jid > 0){
	// symbolic assembly
	k.find_index(i,jid);
	// real assembly
	//	k.element(i,jid) += dofdat[dofdatindex];
      }
      dofdatindex++;
    }
  }
  // Given a DOF and a container of DOF's, assemble to the
  // stiffness row. Brute force searches for every ij.
  template<typename DOFType,typename DOFDataType,typename KType>
  void AssembleIJ(DOFType &i,DOFType &j,DOFDataType *dofdat,KType &k,unsigned int N=1) {
    k.find_index(i,j);
    // commented for symbolic assembly
    //    DOFDataType *dest = &(k.element(i,j));
    DOFDataType *dest = &(k[0]);
    memcpy(dofdat,dest,N*sizeof(DOFDataType));
  }

  // original one
  // Given a DOF and a container of DOF's, assemble to the
  // stiffness row. Brute force searches for every ij.
  template<typename DOFType,typename DOFDataType,typename KType>
  inline void AssembleIJMN(DOFType &i,DOFType &j,std::vector<DOFDataType> &dofdat,
			   KType &k,Mesh::IndexType datind,unsigned int dskip,
			   unsigned int M=1,unsigned int N=1) {
    Mesh::IndexType index = k.fast_find_index(i,j);
    // commented for symbolic assembly
    for(unsigned int l = 0;l < M;l++){
      DOFDataType *dest = &(k[index-index]);
      memcpy(&dofdat[datind],dest,N*sizeof(DOFDataType));
      index+=k.RowSize(i);
      datind+=dskip;
    }
  }


  // dofdatatype = double
  // doftype = Mesh::IndexType
  // dofconttype = Connectivity
  // InnerContType = std::vector<Mesh::IndexType>
  template <typename DOFDataType,typename DOFType,typename DOFContType,typename InnerContType>
  class DummyStiffness 
  {
  public:
    std::vector<DOFDataType> _data;
    DOFContType             *_dofs; // dofs for each dof row
    //    DOFContType              _dofmap;
    std::vector<Mesh::IndexType>   _sizes; // running tally style as in CSR
    Mesh::IndexType                _ndof;
    unsigned int rank;
  public:
    inline DOFDataType &operator[](const Mesh::IndexType &index){ return(_data[index]); };
    inline Mesh::IndexType RowSize(const Mesh::IndexType &row){return(_sizes[row]-_sizes[row-1]);}; 
    Mesh::IndexType find_index(const Mesh::IndexType &row,const Mesh::IndexType &col){
      typename InnerContType::iterator iIt  = (*_dofs)[row-1].begin();
      typename InnerContType::iterator iEnd = (*_dofs)[row-1].end();
      Mesh::IndexType rval = 0;
      while(iIt != iEnd)
	if(*iIt ==  col){
	  return(rval+_sizes[row-1]);
	}
	else{
	  iIt++;
	  rval++;
	}
      assert(false); // didn't find it, error out
      return(0);
    }
    inline Mesh::IndexType fast_find_index(const Mesh::IndexType &row,const Mesh::IndexType &col){
      //      std::cerr << "fast searching (" << row << "," << col << ")" << std::endl;
      //      assert(row <= _ndof);
      //      assert(col > 0);
      //      assert(row > 0);
      typename InnerContType::iterator iIt  = (*_dofs)[row-1].begin();
      typename InnerContType::iterator iEnd = (*_dofs)[row-1].end();
      return(std::lower_bound(iIt,iEnd,col) - iIt + _sizes[row-1]);
    }
    Mesh::IndexType find_index(const Mesh::IndexType &row){
      assert(row <= _ndof);
      return(_sizes[row-1]);
    };
    DOFDataType &element(const Mesh::IndexType &row,const Mesh::IndexType &col){
      // real assembly
      return(_data[fast_find_index(row,col)]);
      // symbolic assembly
      //      return(static_cast<DOFDataType>(find_index(row,col)));
    };
    void Dump(std::ostream &Out)
    {
      Out << _ndof << std::endl;
      for(Mesh::IndexType i = 0;i <= _ndof;i++)
	Out << _sizes[i] << " ";
      Out << std::endl;
      Out << *_dofs << std::endl;
    };
  };

  class FieldData {
  protected:
    int order;
    int ncomp;
  public:
    int Order() const { return(order); };
    int Order(int o) { 
      order = o;
      return(order);
    };
    int Components() const { return(ncomp); };
    int Components(int c) {
      ncomp = c;
      return(ncomp);
    };
  };

  Mesh::IndexType AssignLocalDofsByElement(Mesh::Connectivity &ec, // input
					   Mesh::Connectivity &dc, // input
					   Mesh::Connectivity &nbr, // input element nbr list
					   Mesh::Connectivity &efields, // input (sorted)
					   Mesh::Connectivity &nfields, // output
					   Mesh::Connectivity &NodalDofs, // output
					   Mesh::Connectivity &ElementDofs, // output
					   const std::vector<FieldData> &fields,
					   Mesh::PartInfo &info,
					   std::list<Mesh::IndexType> &border_element_list,
					   std::vector<Mesh::Border> &borders,
					   Mesh::IndexType &nlocaldofs,
					   int rank);
  Mesh::IndexType AssignLocalDofsByElementII(Mesh::Connectivity &ec, // input
					     Mesh::Connectivity &dc, // input
					     Mesh::Connectivity &nbr, // input element nbr list
					     Mesh::Connectivity &efields, // input (sorted)
					     Mesh::Connectivity &nfields, // output
					     Mesh::Connectivity &NodalDofs, // output
					     Mesh::Connectivity &ElementDofs, // output
					     const std::vector<FieldData> &fields,
					     Mesh::PartInfo &info,
					     std::list<Mesh::IndexType> &border_element_list,
					     std::vector<Mesh::Border> &borders,
					     Mesh::IndexType &nlocaldofs,
					     int rank);
  
  Mesh::IndexType AssignUniformDofs(Mesh::Connectivity &ec,
				    Mesh::Connectivity &dc,
				    Mesh::Connectivity &efields,
				    Mesh::Connectivity &nfields,
				    Mesh::Connectivity &NodalDofs,
				    Mesh::Connectivity &ElementDofs,
				    const std::vector<FieldData> &fields);
  Mesh::IndexType AssignDofsByElement(Mesh::Connectivity &ec, // input
				      Mesh::Connectivity &dc, // input
				      Mesh::Connectivity &efields, // input (sorted)
				      Mesh::Connectivity &nfields, // output
				      Mesh::Connectivity &NodalDofs, // output
				      Mesh::Connectivity &ElementDofs, // output
				      const std::vector<FieldData> &fields); // input
  
  void NumberElementDofsLocal(Mesh::Connectivity &ec,
			      Mesh::Connectivity &ElementDofs,
			      Mesh::Connectivity &NodalDofs,
			      Mesh::Connectivity &efields,
			      Mesh::Connectivity &nfields,
			      const std::vector<FieldData> &fields,
			      Mesh::IndexType &dofnum,
			      Mesh::IndexType el_index,
			      Mesh::PartInfo &info,
			      std::vector<bool> &processed_node,
			      std::vector<bool> &processed_element,
			      std::list<Mesh::IndexType> &borderelements,
			      bool remote,
			      int rank);

  void NumberElementDofs(Mesh::Connectivity &ec,
			 Mesh::Connectivity &edofs,
			 Mesh::Connectivity &ndofs,
			 Mesh::Connectivity &efields,
			 Mesh::Connectivity &nfields,
			 const std::vector<FieldData> &fields,
			 Mesh::IndexType &dofnum,
			 Mesh::IndexType el_index,
			 std::vector<bool> &processed_node);
  
  Mesh::IndexType AssignDofsByElementII(Mesh::Connectivity &ec,             // input
					Mesh::Connectivity &dc,             // input
					Mesh::Connectivity &nbr,            // input element nbr list
					Mesh::Connectivity &efields,        // input (sorted)
					Mesh::Connectivity &nfields,        // output
					Mesh::Connectivity &NodalDofs,      // output
					Mesh::Connectivity &ElementDofs,    // output
					const std::vector<FieldData> &fields); // input
  //  Mesh::IndexType BuildDofCon(Mesh::Connectivity &DofCon,Mesh::Connectivity &NodalDofs,
  //		     Mesh::Connectivity &ElementDofs,Mesh::Connectivity &ec,
  //		     IRAD::Primitive::IndexVecList &adjnodes);
  //  Mesh::IndexType BuildStiffness(Mesh::Connectivity &Stiffness,Mesh::Connectivity &ElementDofs);
  Mesh::IndexType BuildSymbolicStiffness(IRAD::Primitive::IndexVecList &SymbStiff,Mesh::Connectivity &ElementDofs);
  
  void AssembleSpecialDofList(Mesh::Connectivity &econ,
			      Mesh::IndexType elindex,
			      Mesh::Connectivity &NodalDofs,
			      Mesh::Connectivity &ElementDofs);
  
  
  int AssembleFullDofList(Mesh::Connectivity &econ,
			  Mesh::IndexType elindex,
			  Mesh::Connectivity &NodalDofs,
			  Mesh::Connectivity &ElementDofs,
			  std::vector<Mesh::IndexType> &dofs);
  int FastAssembleLocalElements(std::list<Mesh::IndexType> element_queue,
				Mesh::Connectivity &econ,
				FEM::DummyStiffness<double,Mesh::IndexType,
				Mesh::Connectivity,std::vector<Mesh::IndexType> > &k,
				Mesh::Connectivity &NodalDofs,
				Mesh::Connectivity &ElementDofs,
				std::vector<Mesh::IndexType> &NDofE,
				Mesh::PartInfo &info);
  Mesh::IndexType AssembleLocalElement(std::vector<Mesh::IndexType> &con,
				       std::vector<Mesh::IndexType> &edofs,
				       Mesh::IndexType &endof,
				       Mesh::IndexType &nedofs,
				       Mesh::Connectivity &NodalDofs,
				       FEM::DummyStiffness<double,Mesh::IndexType,
				       Mesh::Connectivity,std::vector<Mesh::IndexType> > &k,
				       std::vector<Mesh::IndexType> &local_dof_to_global,
				       Mesh::IndexType &nglobal_dofs,
				       Mesh::IndexType &doffset,
				       Mesh::PartInfo &info);
  
  Mesh::IndexType AssembleLocalElementII(std::vector<Mesh::IndexType> &con,
					 std::vector<Mesh::IndexType> &edofs,
					 Mesh::IndexType &endof,
					 Mesh::IndexType &nedofs,
					 Mesh::Connectivity &NodalDofs,
					 FEM::DummyStiffness<double,Mesh::IndexType,
					 Mesh::Connectivity,std::vector<Mesh::IndexType> > &k,
					 std::vector<double> &dofdata,
					 Mesh::PartInfo &info);

  Mesh::IndexType AssembleBorderElementII(std::vector<Mesh::IndexType> &con,
					  std::vector<Mesh::IndexType> &edofs,
					  Mesh::IndexType &endof, // number of dofs to assemble
					  Mesh::IndexType &nedofs, // ndofs on element only (not nodes)
					  Mesh::Connectivity &NodalDofs,
					  Mesh::Connectivity &RemoteNodalDofs,
					  FEM::DummyStiffness<double,Mesh::IndexType,
					  Mesh::Connectivity,std::vector<Mesh::IndexType> > &k,
					  std::vector<Mesh::Border> &borders,
					  std::vector<std::pair<unsigned int,unsigned int> > &bnode_map,
					  std::vector<double> &dofdata,
					  Mesh::PartInfo &info,
					  std::ostream *Out,
					  bool verbose);
  Mesh::IndexType AssembleBorderElement(std::vector<Mesh::IndexType> &con,
					std::vector<Mesh::IndexType> &edofs,
					Mesh::IndexType &endof,
					Mesh::IndexType &nedofs,
					Mesh::Connectivity &NodalDofs,
					FEM::DummyStiffness<double,Mesh::IndexType,
					Mesh::Connectivity,std::vector<Mesh::IndexType> > &k,
					std::vector<Mesh::IndexType> &local_dof_to_global,
					Mesh::IndexType &nglobal_dofs,
					Mesh::IndexType &doffset,
					Mesh::PartInfo &info);

  int RecvBufAssembly(Mesh::Border &border,Mesh::Connectivity &NodalDofs,
		      FEM::DummyStiffness<double,Mesh::IndexType,
		      Mesh::Connectivity,std::vector<Mesh::IndexType> > &k,
		      std::vector<double> &dofdat);

  void AssembleToCommBuffer(Mesh::IndexType &i,Mesh::IndexType &j,std::vector<double> &dofdat,
			    Mesh::BorderData &data,Mesh::IndexType datind,unsigned int dskip,
			    unsigned int N,unsigned int M);
    
  } }
#endif



