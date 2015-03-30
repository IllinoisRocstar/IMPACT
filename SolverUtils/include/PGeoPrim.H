#ifndef _P_GEOPRIM_H_
#define _P_GEOPRIM_H_
#include "GeoPrimitives.H"
#include "COMM.H"

namespace SolverUtils { namespace GeoPrim {

  class PBox : public GeoPrim::C3Box, public IRAD::Comm::MobileObject
  {
    virtual int Pack(void **inbuf = NULL);
    virtual int UnPack(const void *outbuf = NULL);
  };
  
  } };

#endif
