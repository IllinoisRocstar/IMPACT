/* *******************************************************************
 * Rocstar Simulation Suite                                          *
 * Version: Sandia Evaluation Distribution                           *
 * Licensed To: Sandia National Laboratories                         *
 * License Type: Evaluation                                          *
 * License Expiration: March 13, 2013                                *
 *********************************************************************/
/* *******************************************************************
 * Rocstar Simulation Suite                                          *
 * Copyright@2012, IllinoisRocstar LLC. All rights reserved.         *
 *                                                                   *
 * The Rocstar Simulation Suite is the property of IllinoisRocstar   *
 * LLC. No use or distribution of this version of the Rocstar        *
 * Simulation Suite beyond the license provided through separate     *
 * contract is permitted.                                            *
 *                                                                   *
 * IllinoisRocstar LLC                                               *
 * Champaign, IL                                                     *
 * www.illinoisrocstar.com                                           *
 * sales@illinoisrocstar.com                                         *
 *********************************************************************/
/* *******************************************************************
 *  Initial open source Rocstar software developed by                *
 *     Center for Simulation of Advanced Rockets                     *
 *     University of Illinois at Urbana-Champaign                    *
 *     Urbana, IL                                                    *
 *     www.csar.uiuc.edu                                             *
 *                                                                   *
 * Copyright@2008, University of Illinois.  All rights reserved.     *
 *                                                                   *
 * @ Redistributions of source code must retain the above copyright  *
 *   notice, this list of conditions and the following disclaimers.  *
 *                                                                   *
 * @ Redistributions in binary form must reproduce the above         *
 *   copyright notice, this list of conditions and the following     *
 *   disclaimers in the documentation and/or other materials         *
 *   provided with the distribution.                                 *
 *                                                                   *
 * @ Neither the names of the Center for Simulation of Advanced      *
 *   Rockets, the University of Illinois, nor the names of its       *
 *   contributors may be used to endorse or promote products derived *
 *   from this Software without specific prior written permission.   *
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
// $Id: commpi.C,v 1.14 2009/01/22 23:56:54 gzagaris Exp $

/** @file commpi.C
 *  Contains a dummy implementation of MPI subroutines for one thread.
 *  If DUMMY_MPI is not define, it does produce anything.
 *  Note: This implementation is not thread-safe and it need not to be
 *  because it is only intended for a placeholder of MPI for one thread.
 *  It works even if MPI_Init was not called.
 *  @see commpi.h
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include "commpi.h"

/// A map from message tags to data addresses
typedef std::map<int, void *> Msg_Queue;

/// Get the size of a given MPI data type.
static int get_sizeof(MPI_Datatype i) {
  if (i == MPI_DOUBLE)
    return sizeof(double);
  else if (i == MPI_FLOAT)
    return sizeof(float);
  else if (i == MPI_INT)
    return sizeof(int);
  else if (i == MPI_BYTE)
    return sizeof(char);
  else if (i == MPI_CHAR)
    return sizeof(char);
  else
    throw(-1);
  // static int get_sizeof( int i)  {
  //  switch (i) {
  //  case MPI_DOUBLE: return sizeof(double);
  //  case MPI_FLOAT:  return sizeof(float);
  //  case MPI_INT:    return sizeof(int);
  //  case MPI_BYTE:   return sizeof(char);
  //  case MPI_CHAR:   return sizeof(char);
  //  default: throw(-1);
  //  }
}

int COM_send(Msg_Queue &sendQ, Msg_Queue &recvQ, void *buf, int count,
             MPI_Datatype datatype, int tag) {
  try {
    Msg_Queue::iterator it;
    if ((it = recvQ.find(tag)) != recvQ.end()) {
      int extent = count * get_sizeof(datatype);
      std::memcpy(it->second, (char *)buf, extent);
      recvQ.erase(it);
    } else
      sendQ[tag] = buf;
  } catch (int) {
    std::cerr << "Unsupported data type " << datatype << " used for MPI_Isend"
              << std::endl;
    std::abort();
  }
  return 0;
}

int COM_recv(Msg_Queue &sendQ, Msg_Queue &recvQ, void *buf, int count,
             MPI_Datatype datatype, int tag) {
  try {
    Msg_Queue::iterator it;
    if ((it = sendQ.find(tag)) != sendQ.end()) {
      int extent = count * get_sizeof(datatype);
      std::memcpy(buf, it->second, extent);
      sendQ.erase(it);
    } else
      recvQ[tag] = buf;
  } catch (int) {
    std::cerr << "Unsupported data type " << datatype << " used for MPI_Irecv"
              << std::endl;
    std::abort();
  }
  return 0;
}

static Msg_Queue *sendQ = NULL;
static Msg_Queue *recvQ = NULL;

/// Begins a nonblocking send
int COMMPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
                 MPI_Comm comm, MPI_Request *request) {
#ifndef DUMMY_MPI
  if (COMMPI_Initialized())
    //  if ( COMMPI_Initialized() && dest!=COMMPI_Comm_rank(comm))
    return MPI_Isend(buf, count, datatype, dest, tag, comm, request);
#endif
  try {
    if (sendQ == NULL) sendQ = new Msg_Queue();
    if (recvQ == NULL) recvQ = new Msg_Queue();
  } catch (...) {
    std::cerr << "Out of memory" << std::endl;
    std::abort();
  }
  return COM_send(*sendQ, *recvQ, buf, count, datatype, tag);
}

/// Begins a nonblocking receive
int COMMPI_Irecv(void *buf, int count, MPI_Datatype datatype, int src, int tag,
                 MPI_Comm comm, MPI_Request *request) {
#ifndef DUMMY_MPI
  if (COMMPI_Initialized())
    //  if ( COMMPI_Initialized() && src!=COMMPI_Comm_rank( comm))
    return MPI_Irecv(buf, count, datatype, src, tag, comm, request);
#endif
  try {
    if (sendQ == NULL) sendQ = new Msg_Queue();
    if (recvQ == NULL) recvQ = new Msg_Queue();
  } catch (...) {
    std::cerr << "Out of memory" << std::endl;
    std::abort();
  }
  return COM_recv(*sendQ, *recvQ, buf, count, datatype, tag);
}

#ifdef DUMMY_MPI

/// Begins a nonblocking send
int MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *request) {
  return COMMPI_Isend(buf, count, datatype, dest, tag, comm, request);
}

/// Begins a nonblocking receive
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int src, int tag,
              MPI_Comm comm, MPI_Request *request) {
  return COMMPI_Irecv(buf, count, datatype, src, tag, comm, request);
}

int MPI_Init(int *, char ***) { return 0; }

int MPI_Finalize() {
  return 0;
  if (sendQ != NULL) {
    delete sendQ;
    sendQ = NULL;
  }
  if (recvQ != NULL) {
    delete recvQ;
    recvQ = NULL;
  }
  return 0;
}

int MPI_Allreduce(void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
  try {
    std::memcpy(recvbuf, sendbuf, count * get_sizeof(datatype));
  } catch (int) {
    std::cerr << "Unsupported data type " << datatype
              << " used for MPI_Allreduce" << std::endl;
    std::abort();
  }
  return 0;
}

int MPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                  void *recvbuf, int recvcount, MPI_Datatype recvtype,
                  MPI_Comm comm) {
  try {
    std::memcpy(recvbuf, sendbuf, sendcount * get_sizeof(sendtype));
  } catch (int) {
    std::cerr << "Unsupported data type " << sendtype
              << " used for MPI_Allgather" << std::endl;
    std::abort();
  }
  return 0;
}

int MPI_Allgatherv(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                   void *recvbuf, int *recvcounts, int *displs,
                   MPI_Datatype recvtype, MPI_Comm comm) {
  try {
    std::memcpy((char *)recvbuf + *displs * get_sizeof(sendtype), sendbuf,
                sendcount * get_sizeof(sendtype));
  } catch (int) {
    std::cerr << "Unsupported data type " << sendtype
              << " used for MPI_Allgatherv" << std::endl;
    std::abort();
  }
  return 0;
}

int MPI_Alltoall(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcnt, MPI_Datatype recvtype,
                 MPI_Comm comm) {
  return MPI_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype,
                       comm);
}

int MPI_Alltoallv(void *sendbuf, int *sendcounts, int *senddispls,
                  MPI_Datatype sendtype, void *recvbuf, int *recvcounts,
                  int *recvdispls, MPI_Datatype recvtype, MPI_Comm comm) {
  return MPI_Allgather((char *)sendbuf + *senddispls, *sendcounts, sendtype,
                       (char *)recvbuf + *recvdispls, *recvcounts, recvtype,
                       comm);
}

#endif /* DUMMY_MPI */
