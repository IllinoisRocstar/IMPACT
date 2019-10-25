#ifndef _P_GEOPRIM_H_
#define _P_GEOPRIM_H_
#include "COMM.H"
#include "GeoPrimitives.H"

namespace SolverUtils {
namespace GeoPrim {

class PBox : public GeoPrim::C3Box, public IRAD::Comm::MobileObject {
  virtual int Pack(void **inbuf = NULL);
  virtual int UnPack(const void *outbuf = NULL);
};

}  // namespace GeoPrim
}  // namespace SolverUtils

#endif
