#ifndef __SOLVERAGENT_H__
#define __SOLVERAGENT_H__

#include "FEM.H"
#include "FieldData.H"
#include "NetUtils.H"
#include "FDUtils.H"

namespace SolverUtils { namespace FEM {

  class SolverAgent
  {
  private:
    Mesh::UnstructuredMesh mesh;
    FEM::SolutionData      soln;
  public:
    const FEM::SolutionData &Solution() const { return(soln); };
    FEM::SolutionData &Solution(){ return(soln); };
    const Mesh::UnstructuredMesh &Mesh() const { return(mesh); };
    Mesh::UnstructuredMesh &Mesh() { return(mesh); };
    void ReadMeshFromStream(std::istream &Istr)
    {
      Istr >> Mesh().nc;
      Istr >> Mesh().con;
    };
    void WriteMeshToStream(std::ostream &Ostr)
    {
      Ostr << Mesh().nc << std::endl << Mesh().con;
    };
    void ReadCoordsFromStream(std::istream &Istr)
    {
      Istr >> Mesh().nc;
    };
    void WriteCoordsToStream(std::ostream &Ostr)
    {
      Ostr << Mesh().nc;
    };
    void ReadSolnMetaFromStream(std::istream &Istr)
    { 
      Solution().Meta().ReadFromStream(Istr);
    };
    void WriteSolnMetaToStream(std::ostream &Ostr)
    { 
      Solution().Meta().WriteToStream(Ostr);
    };
    void CreateSoln()
    {
      Solution().Create(Mesh());
    };
    void ReadSolnFromStream(std::istream &Istr,const std::string &name)
    {
      Solution().ReadFieldFromStream(Istr,name);
    };
    void WriteSolnToStream(std::ostream &Ostr,const std::string &name)
    {
      Solution().WriteFieldToStream(Ostr,name);
    };
    std::ostream &ReportMeshToStream(std::ostream &Ostr)
    {
      Mesh::IndexType number_of_nodes = mesh.nc.Size();
      Ostr << "Number of nodes: " << number_of_nodes << std::endl;
      std::vector<double> bounds(6,0.0);
      Mesh::GetCoordinateBounds(mesh.nc,bounds);
      Ostr << "Mesh Bounds: (" << bounds[0] << "," << bounds[1] 
	   << ") (" << bounds[2] << "," << bounds[3] 
	   << ") (" << bounds[4] << "," << bounds[5] << ")" << std::endl;
      Mesh::Connectivity::iterator ci = mesh.con.begin();
      bool bump_node_indices_plus1 = false;
      if(bump_node_indices_plus1){
	while(ci != mesh.con.end()){
	  std::vector<Mesh::IndexType>::iterator ii = ci->begin();
	  while(ii != ci->end())
	    (*ii++)++;
	  ci++;
	}
	ci = mesh.con.begin();
      }
      Mesh::IndexType ntet  = 0;
      Mesh::IndexType nhex  = 0;
      Mesh::IndexType npyr  = 0;
      Mesh::IndexType npris = 0;
      Mesh::IndexType ntri  = 0;
      Mesh::IndexType nquad = 0;
      while(ci != mesh.con.end()){
	Mesh::IndexType esize = ci->size();
	Mesh::GenericElement el(4);
	bool known_element_type = false;
	switch(esize){
	case 3:
	  ntri++;
	  break;
	case 4:
	  if(el.ShapeOK(*ci,mesh.nc))
	    ntet++;
	  else
	    nquad++;
	  break;
	case 5:
	  npyr++;
	  break;
	case 6:
	  npris++;
	  break;
	case 8:
	  nhex++;
	  break;
	case 10:
	  ntet++;
	  break;
	case 20:
	  nhex++;
	  break;
	default:
	  assert(known_element_type);
	}
	ci++;
      }
      Mesh::IndexType number_of_elements = mesh.con.Nelem();
      Ostr << "Number of elements: " << number_of_elements
	   << std::endl;
      if(ntet > 0)
	Ostr << "Number of tetrahedra: " << ntet << std::endl;
      if(npyr > 0)
	Ostr << "Number of pyramids:   " << npyr << std::endl;
      if(npris > 0)
	Ostr << "Number of prisms:     " << npris << std::endl;
      if(nhex > 0)
	Ostr << "Number of hexahedra:  " << nhex << std::endl;
      if(ntri > 0)
	Ostr << "Number of triangles:  " << ntri << std::endl;
      if(nquad > 0) 
	Ostr << "Number of quads:      " << nquad << std::endl;
      return(Ostr);
    };
    ~SolverAgent(){};
  };

  class FDSolverAgent : public SolverAgent
  {
  private:
    IRAD::Sys::fdistream         FDIn;
    IRAD::Sys::fdostream         FDOut;
    std::string            _ackword;
  public:
    FDSolverAgent(int descriptor = -1) : SolverAgent()
    {
      if(descriptor >= 0){
	FDIn.Init(descriptor);
	FDOut.Init(descriptor);
      }
    };
    void Init(int descriptor = -1)
    {
      if(descriptor >= 0){
	FDIn.Init(descriptor);
	FDOut.Init(descriptor);
      }
    };
    IRAD::Sys::fdistream &InStream() { return(FDIn); };
    IRAD::Sys::fdostream &OutStream() { return(FDOut); };
    void SendACK() {
      Send("ack\n");
    };
    std::string ACKWord() const { return(_ackword); };
    int GetACK(){
      Recv(_ackword);
      if(_ackword == "ack")
	return(0);
      else
	return(1);
    };
    void SendWord(const std::string &word){
      SendACK(word);
    };
    void SendACK(const std::string &ack) {
      std::ostringstream Ostr;
      Ostr << ack << "\n";
      Send(Ostr.str());
    };
    std::string ReceiveWord(){
      std::string word;
      Recv(word);
      return(word);
    };
    int ReceiveWord(std::string &word){
      Recv(word);
      return(word.size());
    };
    int GetACK(const std::string &ack){
      Recv(_ackword);
      if(_ackword == ack)
	return(0);
      else
	return(1);
    };
    void ReceiveMesh()
    {
      Recv(Mesh().nc);
      Recv(Mesh().con);
    };
    void SendMesh()
    {
      Send(Mesh().nc);
      Send('\n');
      Send(Mesh().con);
      Send('\n');
    };
    void ReceiveCoords()
    {
      Recv(Mesh().nc);
    };
    void SendCoords()
    {
      Send(Mesh().nc);
      Send('\n');
    };
    void ReceiveSolnMeta()
    { 
      Solution().Meta().ReadFromStream(FDIn);
    };
    void SendSolnMeta()
    { 
      Solution().Meta().WriteToStream(FDOut);
    };
    void ReceiveSoln(const std::string &name)
    {
      Solution().ReadFieldFromStream(FDIn,name);
    };
    void SendSoln(const std::string &name)
    {
      Solution().WriteFieldToStream(FDOut,name);
    };
    void SendSoln(const std::string &name, void *buf,int bsize = 0)
    {
      Solution().WriteFieldToStream(FDOut,name);
    };
    template<class T>
    void Recv(T &object)
    {
      FDIn >> object;
    };
    template<class T>
    void Send(const T &object)
    {
      FDOut << object;
    };
  };

    class TCPSolverClient : public FDSolverAgent, public IRAD::Sys::Net::Client
  {
  public:
    TCPSolverClient() : FDSolverAgent(), IRAD::Sys::Net::Client()
    {};
    int Connect(const std::string &solver_name,int port = 0)
    {
      int retval = IRAD::Sys::Net::Client::Connect(solver_name,port);
      if(retval < 0)
	return(retval);
      Init(this->Descriptor());
      return(this->Descriptor());
    };
  };

  typedef TCPSolverClient SolverClient;
  } };
#endif
