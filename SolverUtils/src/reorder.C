#include <vector>
#include "mpi.h"

void Reorder(int *Nrows, int *XADJ, int *ADJ, int *Order, MPI_Comm comm) {
  int rank = 0;
  MPI_Comm_rank(comm, &rank);
  int nproc = 1;
  MPI_Comm_size(comm, &nproc);

  // **NOTES:
  // 1. Graph/Matrix must not include edges to self - this
  //    amounts to meaning the graph matrix should have 0's
  //    along the diagonal.
  // 2. If this is a finite element neighborhood graph, then
  //    Metis performs far better when all edges are to
  //    facial neighbors instead of nodal neighbors.

  // Nrows[0:nproc-1]: The number of nodes of the sparse graph on each
  //                   processor
  //
  int nrows = Nrows[rank];
  // VTXDIST[0:NPROC]: needs to specify how many nodes of the sparse graph
  //                   are on each processor.   The sparse matrix representation
  //                   of the graph has a row for each node in the graph.
  //       The format is like XADJ: 0 nrows_on_0 nrows_on_0+nrows_on_1 ....
  //
  std::vector<idxtype> vtxdist;
  vtxdist.resize(nproc + 1);
  int *nri = Nrows;
  std::vector<idxtype>::iterator vtdi = vtxdist.begin();
  *vtdi = 0;
  while (vtdi != vtxdist.end()) {
    idxtype nn = static_cast<idxtype>(*vtdi++ + *nri++);
    *vtdi = nn;
  }

  // X: Index that runs (1 <= X <= Nrows)
  //
  // XADJ[0:Nrows]: Is the parallel version of the "Ap" vector.
  //
  //       The number of edges in Row X of the
  //       sparse graph matrix is: XADJ[X] - XADJ[X-1]
  //       The total number of edges is: XADJ[nrows]
  //       We make our own copy of this because of the possibility
  //       of incompatible data types between Metis idxtype and
  //       the programmer's int graph.
  std::vector<idxtype> xadj;
  xadj.resize(nrows + 1);
  std::vector<idxtype>::iterator xai = xadj.begin();
  int *XAi = XADJ;
  while (xai != xadj.end()) *xai++ = *XAi++;
  //
  // ADJ[0:XADJ[Nrows]-1]: Indicates the index of the node to
  //                       which each edge of the graph connects.
  //                       Or the column index for each entry in
  //                       the sparse matrix.
  //       The non-zero entries in Row X of the
  //       sparse graph matrix are in columns:
  //       ADJ[XADJ[X-1]:XADJ[X]-1]
  std::vector<idxtype> adj;
  adj.resize(nrows + 1);
  std::vector<idxtype>::iterator ai = xadj.begin();
  int *Ai = ADJ;
  while (ai != adj.end()) *ai++ = *Ai++;

  //
  // ORDER[0:Nrows-1]: On output, ORDER[n] indicates the new number for Row n
  //                   where 0<=n<Nrows.
  //       One can either renumber their graph by hand or note that
  //       on output ORDER is actually an ADJ array to an XADJ that
  //       has only one edge per row.  Together, they make up a permutation
  //       matrix, P,  in CSR format. If you have sparse matrix multiply,
  //       then multiplying your input graph , G, produces the new sparse
  //       graph, N:
  //       N = P_transpose * G * P
  //       This acheives the reordering as fast as your parallel SMM can
  //       do it.
  std::vector<idxtype> order(nrows, 0);

  // Sizes baffles me - on output it holds the sizes of the "separators"
  // used in the reordering algo.  I've never needed this information.
  std::vector<idxtype> sizes(nproc * 2, 0);
  // Some options for Metis
  int numflag = 0;
  int options[5];
  options[0] = 1;
  options[1] = 3;
  options[2] = 15;
#ifdef PMETIS
  ParMETIS_V3_NodeND(&vtxdist[0], &xadj[0], &adj[0], &numflag, options,
                     &order[0], &sizes[0], &comm);
#endif

  // Finally, copy the new ordering from idxtype to ints.
  int *Oi = Order;
  std::vector<idxtype>::iterator oi = order.begin();
  while (oi != order.end()) *Oi++ = *oi++;
  order.resize(0);
}
