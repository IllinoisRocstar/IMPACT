///
/// \file 
/// \ingroup support
/// \brief Finite Element Stuff
///
#ifndef __FIELD_DATA_H__
#define __FIELD_DATA_H__

namespace SolverUtils { namespace FEM {
#ifndef byte
  typedef unsigned char byte;
#endif
  //  extern enum DataType;
  //  extern size_t DataSize[];
  
  class DataBuffer {
    friend std::ostream &operator<<(std::ostream &Ostr,const FEM::DataBuffer &buf);
    friend std::istream &operator>>(std::istream &Istr,FEM::DataBuffer &buf);
  public:
    enum DataType {DTCHAR,DTBYTE,DTINT,DTFLOAT,DTDOUBLE};
    size_t DataSize[5];
  private:
    char         *_data_ptr;
    int           _item_size;
    bool          _my_data;
    int           _number_of_items;
    //    int           _mysize;
  public:
    DataBuffer() : _data_ptr(NULL), _item_size(1), _my_data(false), _number_of_items(0) //, _mysize(0)
    {
      DataSize[0] =  DataSize[1] = 1;
      DataSize[2] =  DataSize[3] = 4;
      DataSize[4] = 8;
    };
    DataBuffer(void *buf,int number,int type) : _data_ptr(reinterpret_cast<char *>(buf)), _item_size(type), 
							      _my_data(false), _number_of_items(number)
    {//_mysize = _number_of_items * FEM::DataSize[_item_size];
      DataSize[0] =  DataSize[1] = 1;
      DataSize[2] =  DataSize[3] = 4;
      DataSize[4] = 8;
    };
    DataBuffer(int number,int type) : _data_ptr(NULL), _item_size(type),_my_data(false),
				      _number_of_items(number) // , _mysize(0)
    {
      DataSize[0] =  DataSize[1] = 1;
      DataSize[2] =  DataSize[3] = 4;
      DataSize[4] = 8;
    };
    void Destroy()
    {
      if(_my_data && _data_ptr){
	if((_item_size > 1) || (_number_of_items > 1))
	  delete [] _data_ptr;
	else
	  delete _data_ptr;
      }
      _item_size = 1;
      _data_ptr = NULL;
      _number_of_items = 0;
      _my_data = false;
      //      _mysize = 0;
    };
    template<class T>
    T *Data() { return(reinterpret_cast<T *>(_data_ptr));};
    template<class T>
    const T *Data() const { return(reinterpret_cast<const T *>(_data_ptr)); };
    int ItemSize() const { return(_item_size); };
    int NItems() const { return(_number_of_items); };
    int &ItemSize() { return(_item_size); };
    int &NItems() { return(_number_of_items); };
    void Set(void *buf = NULL,int nitems = 0,int itemsize = 1)
    {
      Destroy();
      if(nitems <= 0)
	nitems = _number_of_items;
      if(itemsize < 0)
	itemsize = _item_size;
      _my_data = false;
      _data_ptr = reinterpret_cast<char *>(buf);
      _number_of_items = nitems;
      _item_size = itemsize;
      //      _mysize = _number_of_items * FEM::DataSize[_item_size];
    };
    int Allocate(int nitems = -1,int isize = -1)
    {
      Destroy();
      if(nitems >= 0)
	_number_of_items = nitems;
      if(isize > 0)
	_item_size = isize;
      int bufsize = _number_of_items * _item_size;
      if(bufsize > 0)
	_data_ptr = new char [bufsize];
      _my_data = true;
      return(bufsize);
    };
    int size() const
    {
      return(_data_ptr ? (_number_of_items * _item_size) : 0); 
    };
    int resize(int newsize)
    {
      int save_item_size = _item_size;
      int save_nitems = _number_of_items;
      Destroy();
      if(newsize > 0){
	if((newsize%save_item_size) == 0){
	  _item_size = save_item_size;
	  _number_of_items = (newsize/_item_size);
	}
	_data_ptr = new char [newsize];
	_my_data = true;
      }
      return(_number_of_items * _item_size);
    };
    const char *data() const { return(_data_ptr); };

    template<class T>
    std::ostream &OutputToStream(std::ostream &Ostr) const
    {
      char delimiter;
      std::ostringstream OutStr;
      if(sizeof(T) != 1)
	delimiter = ' ';
      T *dataptr = (T *)_data_ptr;
      for(int i = 0;i < _number_of_items;i++){
	OutStr << *dataptr++;
	if(i != (_number_of_items - 1))
	  OutStr << delimiter;
      }
      OutStr << std::endl;
      Ostr << OutStr.str();
      return(Ostr);
    };

    template<class T>
    std::istream &ReadFromStream(std::istream &Istr)
    {
      T *dataptr = (T *)_data_ptr;
      std::string line;
      std::getline(Istr,line);
      std::istringstream InStr(line);
      for(int i = 0;i < _number_of_items;i++)
	InStr >> *dataptr++;
      return(Istr);
    };
  };



  class FieldMetaData {
    friend std::istream &operator>>(std::istream &IStr,FieldMetaData &md);
    friend std::ostream &operator<<(std::ostream &OStr,const FieldMetaData &md);
  public:
    std::string name;
    std::string unit;
    char loc;
    unsigned int ncomp;
    unsigned int dsize;
  };

  class SolutionMetaData : public std::vector<FEM::FieldMetaData>
  {
  public:
    void ReadFromStream(std::istream &InStr)
    {
      int num_vars;
      std::string line;
      while(line.empty())
	std::getline(InStr,line);
      std::istringstream Istr(line);
      Istr >> num_vars;
      this->resize(num_vars);
      int num = 0;
      while(num < num_vars){
	std::getline(InStr,line);
	std::istringstream IStr(line);
	IStr >> (*this)[num++];
      }
    };
    void WriteToStream(std::ostream &OutStr)
    {
      int num_vars = this->size();
      int num = 0;
      OutStr << num_vars << std::endl;
      while(num < num_vars)
	OutStr << (*this)[num++] << std::endl;
    };
    void AddField(const std::string &name,char loc,unsigned int ncomp,unsigned int dsize,const std::string &unit)
    {
      FEM::FieldMetaData md;
      md.name = name;
      md.unit = unit;
      md.loc = loc;
      md.ncomp = ncomp;
      md.dsize = dsize;
      this->push_back(md);
    };
  };

  class SolutionData
  {
  public:
    typedef FEM::DataBuffer             DataBufferType;
    typedef std::vector<DataBufferType> DataContainer;
  private:
    DataContainer data;
    FEM::SolutionMetaData     meta;
  public:
    FEM::SolutionMetaData &Meta() { return (meta); };
    const FEM::SolutionMetaData &Meta() const { return (meta); };
    DataContainer &Data() { return (data); };
    const DataContainer &Data() const { return (data); };
    int Create(const Mesh::UnstructuredMesh &mesh)
    {
      int all_data_size = 0;
      //      if(meta.empty())
      //	return(-1);
      assert(!meta.empty());
      data.resize(meta.size());
      FEM::SolutionMetaData::iterator mdi = meta.begin();
      Mesh::IndexType number_of_cells = mesh.con.Nelem();
      Mesh::IndexType number_of_nodes = mesh.nc.Size();
      Mesh::IndexType data_index = 0;
      while(mdi != meta.end()){
	// location can be:
	// (s) - solver global
	// (m) - mesh global
	// (c) - mesh cells
	// (n) - mesh nodes
	Mesh::IndexType nitems = ((mdi->loc == 's' || mdi->loc == 'm') ? 1 :
				   mdi->loc == 'c' ? number_of_cells :
				   mdi->loc == 'n' ? number_of_nodes :
				  0);
	if(nitems == 0){
	  std::cerr << "SolutionData::Create: Error, unknown location: " << mdi->loc << std::endl;
	}
	assert(nitems > 0);
	// Total size will be the number of components (i.e. scalar = 1, vector = 2,3, etc) times
	// the number of items (i.e. number of nodes, elements, etc) times the size of each item
	Mesh::IndexType total_size = nitems * mdi->dsize * mdi->ncomp;
	all_data_size += total_size;
	data[data_index++].Allocate(nitems*mdi->ncomp,mdi->dsize);
	mdi++;
      }
      return(all_data_size);
    };

    int GetDataIndex(const std::string &name) const
    {
      FEM::SolutionMetaData::const_iterator mdi = meta.begin();
      int index = 0;
      while(mdi != meta.end()){
	if(mdi++->name == name)
	  return(index);
	index++;
      }
      return(-1);
    };

    FEM::FieldMetaData &GetFieldMetaData(const std::string &name)
    {
      FEM::SolutionMetaData::iterator mdi = meta.begin();
      while(mdi != meta.end()){
	if(mdi->name == name)
	  return(*mdi);
	mdi++;
      }
      bool invalid_field_name = false;
      assert(invalid_field_name);
      return(*mdi); // never get here
    };

    DataBufferType &GetFieldData(const std::string &name) 
    {
      
      int known_field = GetDataIndex(name);
      assert(known_field >= 0);
      return(data[known_field]);
    };
    const DataBufferType &GetFieldData(const std::string &name) const
    {
      int known_field = GetDataIndex(name);
      assert(known_field >= 0);
      return(data[known_field]);
    };
    void SetFieldBuffer(const std::string &name,void *buf)
    {
      int known_field = GetDataIndex(name);
      assert(known_field >= 0);
      data[known_field].Set(buf);
    };

    template<class DataType>
    void SetFieldBuffer(const std::string &name,std::vector<DataType> &invec)
    {
      int dsize = sizeof(DataType);
      int known_field = GetDataIndex(name);
      assert(known_field >= 0);
      data[known_field].Set(&invec[0],invec.size(),dsize);
    };

    const FEM::FieldMetaData &GetFieldMetaData(const std::string &name) const
    {
      
      FEM::SolutionMetaData::const_iterator mdi = meta.begin();
      while(mdi != meta.end()){
	if(mdi->name == name)
	  return(*mdi);
	mdi++;
      }
      bool invalid_field_name = false;
      assert(invalid_field_name);
      return(*mdi); // never get here
    };

    std::ostream &WriteFieldToStream(std::ostream &OutStream,const std::string &name) const
    {
      int known_field = GetDataIndex(name);
      assert(known_field >= 0);
      OutStream << data[known_field];
      // to endl or not to endl?
      //      OutStream << std::endl;
      return(OutStream);
    };

    std::istream &ReadFieldFromStream(std::istream &InStream,const std::string &name)
    {
      int known_field = GetDataIndex(name);
      assert(known_field >= 0);
      InStream >> data[known_field];
      return(InStream);
    };
  };
  } };

#endif
