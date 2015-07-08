#ifndef __INTERFACE_LAYER_H__
#define __INTERFACE_LAYER_H__
#include <iomanip>

#include "SolverAgent.H"
#include "com_basic.h"
#include "com.h"


COM_EXTERN_MODULE( SurfX );
COM_EXTERN_MODULE( SimIN );
COM_EXTERN_MODULE( SimOUT );
COM_EXTERN_MODULE( Simpal );

namespace SolverUtils {

  int InitializeInterface(int *argc_ptr,char ***argv_ptr)
  {
    COM_init(argc_ptr,argv_ptr);
    return(0);
  };
  void FinalizeInterface()
  {
    COM_finalize();
  };
  int PaneToUnstructuredMesh(const std::string &windowName,int paneID,
                             Mesh::UnstructuredMesh &uMesh)
  {

    std::vector< std::pair<int,std::string> > connectivityNames;
    connectivityNames.push_back(std::make_pair<int,std::string>(2,":b2")); // bar
    connectivityNames.push_back(std::make_pair<int,std::string>(3,":t3")); // triangle
    connectivityNames.push_back(std::make_pair<int,std::string>(4,":q4")); // quad
    connectivityNames.push_back(std::make_pair<int,std::string>(4,":T4")); // tet
    connectivityNames.push_back(std::make_pair<int,std::string>(8,":H8")); // hex
    connectivityNames.push_back(std::make_pair<int,std::string>(5,":P5")); // pyramid
    connectivityNames.push_back(std::make_pair<int,std::string>(6,":P6")); // prism

    double *windowNodeCoords = NULL;
    int dataStride = 0;
    int dataCap = 0;
    COM_get_array((windowName+".nc").c_str(),paneID,&windowNodeCoords,&dataStride,&dataCap);
    if(!windowNodeCoords)
      return(1);
    int numberOfNodes = (dataStride*dataCap)/3;
    uMesh.nc.init_copy(numberOfNodes,windowNodeCoords,dataStride);
    std::string paneConnectivityNames;
    int numberOfConnectivities = 0;
    COM_get_connectivities(windowName.c_str(),paneID,&numberOfConnectivities,paneConnectivityNames);
    std::istringstream Istr(paneConnectivityNames);
    std::string nameToken;
    std::vector<std::string> paneConnTableNames;
    while(Istr >> nameToken)
      paneConnTableNames.push_back(nameToken);
    std::vector<std::string>::iterator pctnIt = paneConnTableNames.begin();
    while(pctnIt != paneConnTableNames.end()){
      std::string tableName(*pctnIt++);
      std::vector< std::pair<int,std::string> >::iterator eti = connectivityNames.begin();
      bool match = false;
      while(eti != connectivityNames.end() && !match){
        std::string::size_type x = tableName.find(eti->second);
        if(x != std::string::npos)
          match = true;
        else 
          eti++;
      }
      if(!match){
        std::cerr << "SolverUtils::PaneToUnstructuredMesh:Error: Non-standard connectivity name: " << tableName << std::endl;
        return(1);
      }
      int elementSize = eti->first;
      std::string elementName(tableName);
      int *connectivityArray = NULL;
      int connStride = 0;
      int connCap = 0;
      COM_get_array((windowName+"."+elementName).c_str(),paneID,&connectivityArray,
                    &connStride,&connCap);
      if(connectivityArray){
        std::vector<Mesh::IndexType> elementConnectivity;
        int numberOfElements = (connStride*connCap)/elementSize;
        for(int i = 0;i < numberOfElements;i++){
          for(int j = 0;j < elementSize;j++){
            elementConnectivity.push_back(connectivityArray[i+j*connStride]);
          }
          uMesh.con.AddElement(elementConnectivity);
        }
      }
    }
    uMesh.con.ShrinkWrap();
    return(0);
  };
  /// Creates a window pane from a Mesh object. (copy mode or use mode)
  int SurfaceMeshToPane(const std::string &wname,int pane_id,
			Mesh::UnstructuredMesh &mesh,bool copy = true)
  {
    unsigned int number_of_nodes = mesh.nc.Size();
    COM_set_size((wname+".nc").c_str(),pane_id,number_of_nodes);
    if(copy){
      COM_resize_array((wname+".nc").c_str(),pane_id);
      double *nc = NULL;
      COM_get_array((wname+".nc").c_str(),pane_id,&nc);
      std::memcpy(nc,mesh.nc[1],number_of_nodes*3*sizeof(double));
    }
    else {
      COM_set_array((wname+".nc"),pane_id,mesh.nc[1]);
    }

    // Connectivities are *always* copied - this is because
    // the native format does not sort the elements into their
    // respective types.  It is also not a contiguous data
    // structure as in the Window.
    Mesh::Connectivity tris;
    Mesh::Connectivity quads;
    Mesh::Connectivity::iterator ci = mesh.con.begin();
    bool known_element_type = false;
    while(ci != mesh.con.end()){
      if(ci->size() == 3)
	tris.AddElement(*ci++);
      else if (ci->size() == 4)
	quads.AddElement(*ci++);
      else
	assert(known_element_type);
    }
    unsigned int number_of_tris  = tris.Nelem();
    unsigned int number_of_quads = quads.Nelem();
    int *con_array = NULL;
    if(number_of_tris > 0){
      COM_set_size((wname+".:t3:").c_str(),pane_id,number_of_tris);
      COM_resize_array((wname+".:t3:").c_str(),pane_id);
      COM_get_array((wname+".:t3:").c_str(),pane_id,&con_array);
      ci = tris.begin();
      while(ci != tris.end()){
	unsigned int element_id = ci - tris.begin() + 1;
	unsigned int element_index = element_id - 1;
	con_array[3*element_index]   = (*ci)[0];
	con_array[3*element_index+1] = (*ci)[1];
	con_array[3*element_index+2] = (*ci)[2];
	ci++;
      }
    }
    if(number_of_quads > 0){
      COM_set_size((wname+".:q4:").c_str(),pane_id,number_of_quads);
      COM_resize_array((wname+".:q4:").c_str(),pane_id);
      COM_get_array((wname+".:q4:").c_str(),pane_id,&con_array);
      ci = quads.begin();
      while(ci != quads.end()){
	unsigned int element_id = ci - quads.begin() + 1;
	unsigned int element_index = element_id - 1;
	con_array[4*element_index]   = (*ci)[0];
	con_array[4*element_index+1] = (*ci)[1];
	con_array[4*element_index+2] = (*ci)[2];
	con_array[4*element_index+3] = (*ci)[3];
	ci++;
      }
    }
    //  COM_window_init_done(wname.c_str());
    return(0);
  }
  
  /// Creates a window from a Mesh object. (copies data)
  int VolumeMeshToPane(const std::string &wname,int pane_id,
		       Mesh::UnstructuredMesh &mesh, bool copy = true)
  //			    SolnMetaData &smdv,
  //			    std::vector<std::vector<double> > &soln_data,
  //			    int verblevel)
  {
    unsigned int number_of_nodes = mesh.nc.Size();
    COM_set_size((wname+".nc").c_str(),pane_id,number_of_nodes);
    if(copy){
      COM_resize_array((wname+".nc").c_str(),pane_id);
      double *nc = NULL;
      COM_get_array((wname+".nc").c_str(),pane_id,&nc);
      std::memcpy(nc,mesh.nc[1],number_of_nodes*3*sizeof(double));
    }
    else {
      COM_set_array((wname+".nc"),pane_id,mesh.nc[1]);
    }
    Mesh::Connectivity tets;
    Mesh::Connectivity bricks;
    Mesh::Connectivity pyrs;
    Mesh::Connectivity pris;
    Mesh::Connectivity::iterator ci = mesh.con.begin();
    bool known_element_type = false;
    while(ci != mesh.con.end()){
      if(ci->size() == 4)
	tets.AddElement(*ci++);
      else if (ci->size() == 5)
	pyrs.AddElement(*ci++);
      else if (ci->size() == 6)
	pris.AddElement(*ci++);
      else if (ci->size() == 8)
	bricks.AddElement(*ci++);
      else
	assert(known_element_type);
    }
    unsigned int number_of_tets   = tets.Nelem();
    unsigned int number_of_bricks = bricks.Nelem();
    unsigned int number_of_pyrs   = pyrs.Nelem();
    unsigned int number_of_pris   = pris.Nelem();
    int *con_array = NULL;
    if(number_of_tets > 0){
      COM_set_size((wname+".:T4:").c_str(),pane_id,number_of_tets);
      COM_resize_array((wname+".:T4:").c_str(),pane_id);
      COM_get_array((wname+".:T4:").c_str(),pane_id,&con_array);
      ci = tets.begin();
      while(ci != tets.end()){
	unsigned int element_id = ci - tets.begin() + 1;
	unsigned int element_index = element_id - 1;
	con_array[4*element_index]   = (*ci)[0];
	con_array[4*element_index+1] = (*ci)[1];
	con_array[4*element_index+2] = (*ci)[2];
	con_array[4*element_index+3] = (*ci)[3];
	ci++;
      }
    }
    if(number_of_pyrs > 0){
      COM_set_size((wname+".:P5:").c_str(),pane_id,number_of_pyrs);
      COM_resize_array((wname+".:P5:").c_str(),pane_id);
      COM_get_array((wname+".:P5:").c_str(),pane_id,&con_array);
      ci = pyrs.begin();
      while(ci != pyrs.end()){
	unsigned int element_id = ci - pyrs.begin() + 1;
	unsigned int element_index = element_id - 1;
	con_array[5*element_index]   = (*ci)[0];
	con_array[5*element_index+1] = (*ci)[1];
	con_array[5*element_index+2] = (*ci)[2];
	con_array[5*element_index+3] = (*ci)[3];
	con_array[5*element_index+4] = (*ci)[4];
	ci++;
      }
    }
    if(number_of_pris > 0){
      COM_set_size((wname+".:P6:").c_str(),pane_id,number_of_pris);
      COM_resize_array((wname+".:P6:").c_str(),pane_id);
      COM_get_array((wname+".:P6:").c_str(),pane_id,&con_array);
      ci = pris.begin();
      while(ci != pris.end()){
	unsigned int element_id = ci - pris.begin() + 1;
	unsigned int element_index = element_id - 1;
	con_array[6*element_index]   = (*ci)[0];
	con_array[6*element_index+1] = (*ci)[1];
	con_array[6*element_index+2] = (*ci)[2];
	con_array[6*element_index+3] = (*ci)[3];
	con_array[6*element_index+4] = (*ci)[4];
	con_array[6*element_index+4] = (*ci)[5];
	ci++;
      }
    }
    if(number_of_bricks > 0){
      COM_set_size((wname+".:B8:").c_str(),pane_id,number_of_bricks);
      COM_resize_array((wname+".:B8:").c_str(),pane_id);
      COM_get_array((wname+".:B8:").c_str(),pane_id,&con_array);
      ci = bricks.begin();
      while(ci != bricks.end()){
	unsigned int element_id = ci - bricks.begin() + 1;
	unsigned int element_index = element_id - 1;
	con_array[8*element_index]   = (*ci)[0];
	con_array[8*element_index+1] = (*ci)[1];
	con_array[8*element_index+2] = (*ci)[2];
	con_array[8*element_index+3] = (*ci)[3];
	con_array[8*element_index+4] = (*ci)[4];
	con_array[8*element_index+5] = (*ci)[5];
	con_array[8*element_index+6] = (*ci)[6];
	con_array[8*element_index+7] = (*ci)[7];
	ci++;
      }
    }
    return(0);
  }
  
  class TransferObject {
  private:
    std::string myname;
    std::string win1;
    std::string win2;
    int readhandle;
    int overlayhandle;
    int transferhandle;
    int interphandle;
    int loadtranshandle;
  public:
    TransferObject(const std::string &name) :
      myname(name)
    {
      COM_LOAD_MODULE_STATIC_DYNAMIC(SurfX,myname.c_str());
      readhandle      = COM_get_function_handle((myname+std::string(".read_overlay")).c_str());
      overlayhandle   = COM_get_function_handle((myname+std::string(".overlay")).c_str());
      interphandle    = COM_get_function_handle((myname+std::string(".interpolate")).c_str());
      transferhandle  = COM_get_function_handle((myname+std::string(".least_squares_transfer")).c_str());
      loadtranshandle = COM_get_function_handle((myname+std::string(".load_transfer")).c_str());
    };
    int Overlay(const std::string &w1,const std::string &w2)
    {
      win1 = w1;
      win2 = w2;
      int mesh1 = COM_get_dataitem_handle((w1+".mesh"));
      int mesh2 = COM_get_dataitem_handle((w2+".mesh"));
      if(mesh1 <= 0 || mesh2 <= 0)
	return(-1);
      COM_call_function(overlayhandle,&mesh1,&mesh2);
      return(0);
    };
    void SetVerbosity(int level)
    {
      COM_call_function(COM_get_function_handle((myname+".set_verbose").c_str()),&level);
    };
    int Transfer(const std::string &src_att,const std::string &trg_att,bool reverse = false)
    {
      std::string srcwin((reverse ? win2 : win1));
      std::string trgwin((reverse ? win1 : win2));
      std::string srcatt = srcwin + "." + src_att;
      std::string trgatt = trgwin + "." + trg_att;
      int src_handle = COM_get_dataitem_handle(srcatt.c_str());
      int trg_handle = COM_get_dataitem_handle(trgatt.c_str());
      if((src_handle <= 0) || (trg_handle <= 0))
	return(-1);
      COM_call_function(transferhandle,&src_handle,&trg_handle);
      return(0);
    };
    int Interpolate(const std::string &src_att,const std::string &trg_att,bool reverse = false)
    {
      std::string srcwin((reverse ? win2 : win1));
      std::string trgwin((reverse ? win1 : win2));
      std::string srcatt = srcwin + "." + src_att;
      std::string trgatt = trgwin + "." + trg_att;
      int src_handle = COM_get_dataitem_handle(srcatt.c_str());
      int trg_handle = COM_get_dataitem_handle(trgatt.c_str());
      if((src_handle <= 0) || (trg_handle <= 0))
	return(-1);
      COM_call_function(interphandle,&src_handle,&trg_handle);
      return(0);
    };
    int TransferLoad(const std::string &src_att,const std::string &trg_att,bool reverse = false)
    {
      std::string srcwin((reverse ? win2 : win1));
      std::string trgwin((reverse ? win1 : win2));
      std::string srcatt = srcwin + "." + src_att;
      std::string trgatt = trgwin + "." + trg_att;
      int src_handle = COM_get_dataitem_handle(srcatt.c_str());
      int trg_handle = COM_get_dataitem_handle(trgatt.c_str());
      if((src_handle <= 0) || (trg_handle <= 0))
	return(-1);
      COM_call_function(loadtranshandle,&src_handle,&trg_handle);
      return(0);
    };
    ~TransferObject(){ COM_UNLOAD_MODULE_STATIC_DYNAMIC(SurfX,myname.c_str()); };
  };
    
  int CreateDataItemsFromSolution(const std::string &name,FEM::SolutionData &soln)
  {
    FEM::SolutionData::DataContainer &data = soln.Data();
    FEM::SolutionMetaData &meta = soln.Meta();
    FEM::SolutionMetaData::iterator mdi = meta.begin();
    while(mdi != meta.end()){
      Mesh::IndexType data_index = mdi - meta.begin();
      FEM::FieldMetaData &md = *mdi++;
      std::string dataitem_name(name+"."+md.name);
      COM_Type datatype = (md.dsize == 1 ? COM_CHAR :
			   md.dsize == 4 ? COM_INT  :
			   md.dsize == 8 ? COM_DOUBLE :
			   COM_RAWDATA);
      assert(datatype >= 0);
      COM::DataItem::Shorter_size wloc = ( md.loc == 's' ? 'w' :
					   md.loc == 'm' ? 'p' :
					   md.loc == 'c' ? 'e' :
					   md.loc == 'n' ? 'n' : '0');
      assert(wloc != '0');
      COM_new_dataitem(dataitem_name,wloc,datatype,md.ncomp,md.unit.c_str());
      // Can go ahead and set the arrays for the solver globals
      if(md.loc == 's') { 
	Mesh::IndexType bufsize = md.dsize * md.ncomp;
	if(md.dsize == 1) bufsize = data[data_index].size();
	assert(data[data_index].size() == bufsize);
	bufsize = 1;
	if(md.dsize == 1) bufsize = data[data_index].size();
	COM_set_size(dataitem_name,0,bufsize); // all global data get a size of 1, except strings
	COM_set_array(dataitem_name,0,const_cast<char *>(reinterpret_cast<const char *>
							  (data[data_index].data())));
      }
    }
    return(0);
  };
  inline std::vector<std::string> GetDataItemNames(const std::string &windowName){
    std::vector<std::string> dataItemNames;
    std::string nameString;
    int nnames = 0;
    COM_get_dataitems(windowName.c_str(),&nnames,nameString);
    std::istringstream Istr(nameString);
    std::string token;
    while(Istr >> token)
      dataItemNames.push_back(token);
    return(dataItemNames);
  }

  int PopulateSolutionMetaDataFromPane(const std::string &windowName,int paneID,FEM::SolutionData &soln)
  {
    FEM::SolutionData::DataContainer &data(soln.Data());
    FEM::SolutionMetaData &meta(soln.Meta());
    std::vector<std::string> dataItemNames(GetDataItemNames(windowName));
    std::vector<std::string>::iterator itemNameIt = dataItemNames.begin();
    while(itemNameIt != dataItemNames.end()){
      std::string iName(*itemNameIt++);
      std::string aName(windowName+"."+iName);
      char loc = 0;
      COM_Type dataType = COM_RAWDATA;
      FEM::FieldMetaData metaData;
      metaData.name = iName;
      int dataNComp = 0;
      COM_get_dataitem(aName.c_str(),&loc,&dataType,&dataNComp,&(metaData.unit));
      metaData.dsize = (dataType == COM_CHAR   ? 1 :
                        dataType == COM_INT    ? 4 :
                        dataType == COM_DOUBLE ? 8 : 
                        dataType == COM_CHARACTER ? 1 :
                        dataType == COM_INTEGER   ? 4 :
                        dataType == COM_REAL      ? 8 :
                        dataType == COM_DOUBLE_PRECISION ? 9 : 0);
      if(metaData.dsize == 0)
        continue;
      metaData.ncomp = dataNComp;
      metaData.loc = (loc == 'w' ? 's' :
                      loc == 'p' ? 'm' :
                      loc == 'e' ? 'c' :
                      loc == 'n' ? 'n' : '0');
      if(metaData.loc == '0')
        continue;
      meta.push_back(metaData);
    }
    return(0);
  };

  int AgentToPane(const std::string &name,int pane_id,FEM::SolverAgent &agent,bool copy = false)
  {
    FEM::SolutionData &soln = agent.Solution();
    Mesh::UnstructuredMesh &mesh = agent.Mesh();

    // First create a new pane with the mesh
    // Since we *know* that this is to be used for surfaces,
    // we'll just use it for one surface in this proof-of-concept
    // implementation.  It *should* be set up so that each solver
    // can register multiple meshes, some 2d, and some 3d.
    SurfaceMeshToPane(name,pane_id,agent.Mesh(),copy);

    FEM::SolutionData::DataContainer &data = soln.Data();
    FEM::SolutionMetaData &meta = soln.Meta();
    FEM::SolutionMetaData::iterator mdi = meta.begin();
    while(mdi != meta.end()){
      Mesh::IndexType data_index = mdi - meta.begin();
      FEM::FieldMetaData &md = *mdi++;
      std::string dataitem_name(name+"."+md.name);
      COM_Type datatype = (md.dsize == 1 ? COM_CHAR :
			   md.dsize == 4 ? COM_INT  :
			   md.dsize == 8 ? COM_DOUBLE :
			   COM_RAWDATA);
      assert(datatype >= 0);
      COM::DataItem::Shorter_size wloc = (md.loc == 's' ? 'w' :
					   md.loc == 'm' ? 'p' :
					   md.loc == 'c' ? 'e' :
					   md.loc == 'n' ? 'n' : '0');
      assert(wloc != '0');
      //      COM_new_dataitem(dataitem_name,wloc,datatype,md.ncomp,md.unit.c_str());
      // Can go ahead and set the arrays for the solver globals
      if(md.loc != 's') {
	Mesh::IndexType orig_bufsize = data[data_index].size();
	Mesh::IndexType datasize = md.dsize * md.ncomp;
	Mesh::IndexType wanted_bufsize = datasize * ((md.loc == 'm') ? 1 : 
						     (md.loc == 'c') ? mesh.con.Nelem() :
						     (md.loc == 'n') ? mesh.nc.Size() : 0);
	if(md.dsize != 1)
	  assert(wanted_bufsize == orig_bufsize);
	else
	  wanted_bufsize = data[data_index].size();
	if(md.loc == 'm')  // all global data get a size of 1, except strings
	  COM_set_size(dataitem_name,pane_id,wanted_bufsize);
	if(copy){
	  COM_resize_array(dataitem_name,pane_id);
	  void *the_array = NULL;
	  COM_get_array(dataitem_name.c_str(),pane_id,&the_array);
	  std::memcpy(the_array,
                      const_cast<char *>(reinterpret_cast<const char *>(data[data_index].data())),
		      wanted_bufsize);
	}
	else {
	  COM_set_array(dataitem_name,pane_id,
                        const_cast<char *>(reinterpret_cast<const char *>
                                           (data[data_index].data())));
	}
      }
    }
    return(0);
  };

  int WriteVTKToStream(const std::string &name,FEM::SolverAgent &agent,
                       std::ostream &Ostr)
  {
    FEM::SolutionData &soln = agent.Solution();
    Mesh::UnstructuredMesh &mesh = agent.Mesh();

    Mesh::WriteVTKToStream(name,mesh,Ostr);

    FEM::SolutionData::DataContainer &data = soln.Data();
    FEM::SolutionMetaData &meta = soln.Meta();
    FEM::SolutionMetaData::iterator mdi = meta.begin();
    Mesh::IndexType nnodes = mesh.nc.NNodes();
    Mesh::IndexType ncells = mesh.con.Nelem();
    // Write Point Data:
    bool there_is_point_data = false;
    while(mdi != meta.end()){
      Mesh::IndexType data_index = mdi - meta.begin();
      FEM::FieldMetaData &md = *mdi++;
      // It's a scalar or a double at nodes (i.e. its nodal field or flag data)
      if(md.loc == 'n' && ((md.dsize == 4) || (md.dsize == 8))){
	if(!there_is_point_data){
	  Ostr << "POINT_DATA " << nnodes << std::endl;
	  there_is_point_data = true;
	}
	std::string dataname(md.name);
	std::string datatype = (md.dsize == 4 ? "integer" : "double");
	if(md.ncomp == 1)
	  Ostr << "SCALARS " << dataname << " " << datatype << " 1" << std::endl
	       << "LOOKUP_TABLE default" << std::endl;
	else
	  Ostr << "VECTORS " << dataname << " " << datatype << std::endl;
	if(md.dsize == 4){
	  int *the_array = agent.Solution().GetFieldData(dataname).Data<int>();
	  for(Mesh::IndexType i = 0;i < nnodes;i++){
	    for(Mesh::IndexType j = 0;j < md.ncomp;j++)
	      Ostr << the_array[i*md.ncomp+j] << " ";
	    Ostr << std::endl;
	  }
	}
	else {
	  double *the_array   = agent.Solution().GetFieldData(dataname).Data<double>();
	  for(Mesh::IndexType i = 0;i < nnodes;i++){
	    for(Mesh::IndexType j = 0;j < md.ncomp;j++)
	      Ostr << the_array[i*md.ncomp+j] << " ";
	    Ostr << std::endl;
	  }

	}
      }
    }
    // Next pass for Cell data
    mdi = meta.begin(); 
    bool there_is_cell_data = false;
    while(mdi != meta.end()){
      Mesh::IndexType data_index = mdi - meta.begin();
      FEM::FieldMetaData &md = *mdi++;
      // It's a scalar or a double at cells (i.e. its nodal field or flag data)
      if(((md.loc == 'c') || (md.loc == 'e')) && ((md.dsize == 4) || (md.dsize == 8))){
	if(!there_is_cell_data){
	  Ostr << "CELL_DATA " << ncells << std::endl;
	  there_is_cell_data = true;
	}
	std::string dataname(md.name);
	std::string datatype = (md.dsize == 4 ? "integer" : "double");
	if(md.ncomp == 1)
	  Ostr << "SCALARS " << dataname << " " << datatype << " 1" << std::endl
	       << "LOOKUP_TABLE default" << std::endl;
	else
	  Ostr << "VECTORS " << dataname << " " << datatype << std::endl;
	if(md.dsize == 4){
	  int *the_array   = agent.Solution().GetFieldData(dataname).Data<int>();
	  for(Mesh::IndexType i = 0;i < ncells;i++){
	    for(Mesh::IndexType j = 0;j < md.ncomp;j++)
	      Ostr << the_array[i*md.ncomp+j] << " ";
	    Ostr << std::endl;
	  }
	}
	else {
	  double *the_array   = agent.Solution().GetFieldData(dataname).Data<double>();
	  for(Mesh::IndexType i = 0;i < ncells;i++){
	    for(Mesh::IndexType j = 0;j < md.ncomp;j++)
	      Ostr << the_array[i*md.ncomp+j] << " ";
	    Ostr << std::endl;
	  }

	}
      }

    }
    return(0);
  };
  int DestroySolver(const std::string &name)
  {
    COM_delete_window(name);
    return(0);
  };
  int UnRegisterSolver(const std::string &name){
    return(DestroySolver(name));
  };
  // int RegisterSolver(const std::string &name,FEM::SolverAgent &agent)
  // {

    
  //   // First, pass through the metadata and create new dataitems.  For
  //   // solver-global dataitems, go ahead and set the size and array.
  //   // For other, non solver-global dataitems, delay processing until
  //   // "pane" setup.
  //   COM_new_window(name);
  //   CreateDataItemsFromSolution(name,agent.Solution());
  //   // Now that the solver global data is handled, we work on 
  //   // the mesh-specific or Window Pane-specific data.
  //   //
  //   int pane_id = 101; // default - arbitrary, shouldnt be zero
  //   AgentToPane(name,pane_id,agent);
  //   COM_window_init_done(name);
  //   return(0);
  // };
  int RegisterSolver(const std::string &name,FEM::SolverAgent &agent,int pane_id=101)
  {

    
    // First, pass through the metadata and create new dataitems.  For
    // solver-global dataitems, go ahead and set the size and array.
    // For other, non solver-global dataitems, delay processing until
    // "pane" setup.
    COM_new_window(name);
    CreateDataItemsFromSolution(name,agent.Solution());
    // Now that the solver global data is handled, we work on 
    // the mesh-specific or Window Pane-specific data.
    //
    AgentToPane(name,pane_id,agent);
    COM_window_init_done(name);
    return(0);
  };
  int UpdateAgentCoordinatesFromPane(const std::string &windowName,int paneID,
                                     FEM::SolverAgent &solverAgent)
  {
    Mesh::UnstructuredMesh &uMesh(solverAgent.Mesh());
    double *windowNodeCoords = NULL;
    int dataStride = 0;
    int dataCap = 0;
    COM_get_array((windowName+".nc").c_str(),paneID,&windowNodeCoords,&dataStride,&dataCap);
    if(!windowNodeCoords)
      return(1);
    int numberOfNodes = (dataStride*dataCap)/3;
    uMesh.nc.init_copy(numberOfNodes,windowNodeCoords,dataStride);
    return(0);
  } 
  int PopulateSolutionDataFromPane(const std::string &windowName,int paneID,
                                   FEM::SolutionData &solutionData,bool copyMode = false)
  {
    FEM::SolutionData::DataContainer &data(solutionData.Data());
    FEM::SolutionMetaData &meta(solutionData.Meta());
    FEM::SolutionMetaData::iterator mdi = meta.begin();
    while(mdi != meta.end()){
      Mesh::IndexType dataIndex = mdi - meta.begin();
      FEM::FieldMetaData &metaData(*mdi++);
      std::string dataItemName(windowName+"."+metaData.name);
      int getPaneID = paneID;
      if(metaData.loc == 's')
        getPaneID = 0;
      Mesh::IndexType bufferSize = data[dataIndex].size();
      int dataStride = 0;
      void *dataArray = NULL;
      COM_get_array(dataItemName.c_str(),getPaneID,&dataArray,&dataStride);
      if(!dataArray)
        continue;
      if(copyMode){
        std::memcpy(const_cast<char *>(reinterpret_cast<const char *>(data[dataIndex].data())),
                    dataArray,bufferSize);
      } else {
        solutionData.SetFieldBuffer(metaData.name,dataArray);
      }
    }
    return(0);
  };
  int CreateSolutionFromPane(const std::string &windowName,int paneID,
                             FEM::SolverAgent &solverAgent,bool copyMode = false)
  {
    FEM::SolutionData &solutionData(solverAgent.Solution());
    if(PopulateSolutionMetaDataFromPane(windowName,paneID,solutionData))
      return(1);
    solutionData.Create(solverAgent.Mesh());
    if(PopulateSolutionDataFromPane(windowName,paneID,solutionData,copyMode))
      return(2);
  };
  int PaneToAgent(const std::string &windowName,int paneID,
                  FEM::SolverAgent &solverAgent,bool copyMode = false)
  {
    int returnCode = PaneToUnstructuredMesh(windowName,paneID,solverAgent.Mesh());
    if(returnCode)
      return(returnCode);
    returnCode = CreateSolutionFromPane(windowName,paneID,solverAgent,copyMode);
    if(returnCode)
      return(returnCode+2);
  };
  int PopulateSolverAgentFromWindow(const std::string &windowName,
                                    FEM::SolverAgent &solverAgent,
                                    bool copyMode = false)
  {

    // First, check that the window exists, if not, return an error = 1
    int windowHandle = COM_get_window_handle(windowName);
    if(!(windowHandle > 0))
      return(1); 
    // Next, check to make sure there is only one pane. Multi-pane windows are not
    // yet supported. Returns 2 on this error.
    std::vector<int> paneIDs;
    COM_get_panes(windowName.c_str(),paneIDs);
    if(paneIDs.size() > 1)
      return(2);
    int returnCode = PaneToAgent(windowName,paneIDs[0],solverAgent,copyMode);
    if(returnCode)
      return(returnCode+2);
    
    return(0);
  };

  int RegisterSolverInto(const std::string &name,FEM::SolverAgent &agent)
  {

    
    // First, pass through the metadata and create new dataitems.  For
    // solver-global dataitems, go ahead and set the size and array.
    // For other, non solver-global dataitems, delay processing until
    // "pane" setup.
    //    COM_new_window(name);
    CreateDataItemsFromSolution(name,agent.Solution());
    // Now that the solver global data is handled, we work on 
    // the mesh-specific or Window Pane-specific data.
    //
    int pane_id = 101; // default - arbitrary, shouldnt be zero
    AgentToPane(name,pane_id,agent);
    COM_window_init_done(name);
    return(0);
  };

  std::string
  TimeString( double t)
  {
    std::ostringstream Ostr;
    Ostr << std::scientific << std::setprecision(5) << t*1e10;
    std::string buf(Ostr.str());
    std::string timestring(buf.substr(9)+"."+buf.substr(0,1)+buf.substr(2,5));
    return(timestring);
  }
  
  double
  TimeString(const std::string &ts)
  {
    std::string xp_s(ts.substr(0,2));
    std::string num_s(ts.substr(3));
    std::istringstream X_inst(ts);
    int xp = 0;
    X_inst >> xp;
    xp = xp - 15;
    double x = pow(10,(double)xp);
    std::istringstream Num_inst(num_s);
    double num = 0;
    Num_inst >> num;
    return(num*=x); 
  }

  std::string CWD(void)
  {
    char buf[1024];
    return(std::string(getcwd(buf,1024)));
  }

  bool
  WriteWindow(const std::string &wname,double t = 0)
  //,const std::string &winpath,
  //	      const std::string &cntl_name,const std::string &cntl_path,
  //	      double t,unsigned int id,MPI_Comm comm,std::ostream *ouf)
  {

    std::string timestring(TimeString(t));
    //    std::string homedir(CWD());
    //    int rank = 0;
    //    int nproc = 1;
    //    if(comm != MPI_COMM_NULL){
    //      MPI_Comm_rank(comm,&rank);
    //      MPI_Comm_size(comm,&nproc);
    //    }

    //    int *srcpane_ids;
    //    int npanes;
    int id = 0;
    //    std::vector<int> pane_id;
    //    COM_get_panes( wname.c_str(), &npanes, &srcpane_ids);
    //    pane_id.resize(npanes);
    //    for(int i = 0;i < npanes;i++)
    //      pane_id[i] = srcpane_ids[i];
    //    COM_free_buffer( &srcpane_ids);
    
    
    COM_LOAD_MODULE_STATIC_DYNAMIC(SimOUT,"Rocout");
    int OUT_set_option = COM_get_function_handle( "Rocout.set_option");
    std::string rankstr("0");
    COM_call_function( OUT_set_option, "rankwidth", rankstr.c_str());
    int whand = COM_get_function_handle("Rocout.write_dataitem");    
    int all = COM_get_dataitem_handle((wname+".all"));
    std::ostringstream Ostr;
    Ostr << wname << "_" << timestring << "_" << std::setw(5)
	 << std::setfill('0') << id;
    //    CD(winpath,ouf);
    COM_call_function(whand,Ostr.str().c_str(),&all,
		      wname.c_str(),timestring.c_str());
    COM_UNLOAD_MODULE_STATIC_DYNAMIC(SimOUT,"Rocout");
    
    //    if(comm != MPI_COMM_NULL)
    //      MPI_Barrier(comm);

    //    CD(homedir,ouf);

    //    if(comm != MPI_COMM_NULL)
    //      MPI_Barrier(comm);

    return(true);
  };


};
#endif
