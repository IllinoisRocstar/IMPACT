//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//
/// \file
/// \ingroup support
/// \brief FEM implementation
///
#include "FEM.H"
#include "FieldData.H"

namespace SolverUtils {
namespace FEM {
Mesh::IndexType BuildSymbolicStiffness(IRAD::Primitive::IndexVecList &SymbStiff,
                                       Mesh::Connectivity &ElementDofs) {
  Mesh::IndexType nelem = ElementDofs.Nelem();
  Mesh::IndexType nnz = 0;
  for (Mesh::IndexType el = 0; el < nelem; el++) {
    std::vector<Mesh::IndexType>::iterator dIt = ElementDofs[el].begin();
    while (dIt != ElementDofs[el].end()) {
      std::vector<Mesh::IndexType>::iterator dIt2 = ElementDofs[el].begin();
      while (dIt2 != ElementDofs[el].end())
        SymbStiff[*dIt - 1].push_back(*dIt2++);
      dIt++;
    }
  }
  IRAD::Primitive::IndexVecList::iterator ssIt = SymbStiff.begin();
  while (ssIt != SymbStiff.end()) {
    ssIt->sort();
    ssIt->unique();
    nnz += ssIt->size();
    ssIt++;
  }
  return (nnz);
}

//  Mesh::IndexType BuildDofCon(Mesh::Connectivity &DofCon,Mesh::Connectivity
//  &NodalDofs,
//		     Mesh::Connectivity &ElementDofs,Mesh::Connectivity &ec,
//		     Mesh::Connectivity &dc,
//		     IRAD::Primitive::IndexVecList &adjnodes)
//  {
//    IRAD::Primitive::IndexVecList::iterator anIt;
//    Mesh::Connectivity::iterator dIt;
//    Mesh::Connectivity::iterator dendIt;
//    IRAD::Primitive::IndexVec::iterator dnIt;
//    IRAD::Primitive::IndexVec::iterator dnendIt;

//    dIt    = NodalDofs.begin();
//    dendIt = NodalDofs.end();
//    anIt    = adjnodes.begin();

//     while(dIt != dendIt){
//       IRAD::Primitive::IndexList iterator anodeIt = anIt->begin();
//       dnIt = dIt->begin();
//       dnendIt = dIt->end();
//       // for each adjacent node
//       while(anodeIt != anIt->end()){
// 	Mesh::IndexType nodeind = *anodeIt-1;
// 	while(dnIt != dnendIt){
// 	  rdofIt = NodalDofs[nodeind].begin();
// 	  DofCon[*dnIt - 1].push_back(
// 	  dnIt++;
// 	}
// 	anodeIt++;
//       }

//       dIt++;
//     }
//    return(0);
//  }

// Assumes each node and element have the same number of dofs as every
// other node and element, that is, the full compliment of dofs on each
// mesh entity.  Need to be able to have different numbers of dofs on
// each node/element/mesh entity.  Need to match each dof with it's field
// and component and mesh entity.
Mesh::IndexType AssignDofsByElement(
    Mesh::Connectivity &ec,                // input
    Mesh::Connectivity &dc,                // input
    Mesh::Connectivity &efields,           // input (sorted)
    Mesh::Connectivity &nfields,           // output
    Mesh::Connectivity &NodalDofs,         // output
    Mesh::Connectivity &ElementDofs,       // output
    const std::vector<FieldData> &fields)  // input
{
  Mesh::IndexType dofnum = 1;
  Mesh::IndexType nel = ec.Nelem();
  Mesh::IndexType nnodes = NodalDofs.Nelem();
  //    Mesh::IndexType field_id = 1;
  unsigned int num_fields = fields.size();
  // Assume that efields is sorted
  // 1. we build nfields as a sorted list of fields for each node
  // 2. Then we loop over elements, and for each node on the element we
  // number the dofs by field and if the element has the asssoicated
  // field, then add them to the sorted list of dofs for that element.
  // 3. we number the dofs internal to the element (i.e. non-nodal based) and
  // add them to the sorted list of dofs for each element
  //
  // Note: we can improve our initial bandwidth by looping over elements by
  // neighbor, or breadth-first in step 2.

  // scale down an efields array which contains only the fields which
  // touch nodes
  Mesh::Connectivity enfields;
  enfields.resize(nel);
  enfields.Sync();
  for (Mesh::IndexType jj = 0; jj < nel; jj++) {
    std::vector<Mesh::IndexType>::iterator efi = efields[jj].begin();
    while (efi != efields[jj].end()) {
      if (fields[*efi - 1].Order() > 0) enfields[jj].push_back(*efi);
      efi++;
    }
  }
  // Step 1
  //  - This is done with the handy dandy list inversions
  // Takes N[E] and E[F] and produces N[F]
  // Like neighborhood, but for disparate quantities
  dc.GetAdjacent(nfields, enfields, num_fields);

  // Step 2
  std::vector<bool> processed_node(nnodes, false);
  for (Mesh::IndexType el_index = 0; el_index < nel; el_index++) {
    ElementDofs[el_index].resize(0);
    std::vector<Mesh::IndexType>::iterator eni = ec[el_index].begin();
    while (eni != ec[el_index].end()) {
      Mesh::IndexType node_id = *eni++;
      Mesh::IndexType node_index = node_id - 1;
      if (!nfields[node_index].empty()) {
        if (!processed_node[node_index]) {
          NodalDofs[node_index].resize(0);
          // if this node has not been processed, then dof it up
          // and add it's dofs to the element
          // if it has been processed, then add the existing
          processed_node[node_index] = true;
          std::vector<Mesh::IndexType>::iterator nfi =
              nfields[node_index].begin();
          while (nfi != nfields[node_index].end()) {
            unsigned int ncomp = fields[*nfi - 1].Components();
            for (Mesh::IndexType j = 0; j < ncomp; j++) {
              NodalDofs[node_index].push_back(dofnum);
              ElementDofs[el_index].push_back(dofnum++);
            }
            nfi++;
          }
        } else {
          std::vector<Mesh::IndexType>::iterator ndi =
              NodalDofs[node_index].begin();
          while (ndi != NodalDofs[node_index].end())
            ElementDofs[el_index].push_back(*ndi++);
        }
      }
    }
    std::vector<Mesh::IndexType>::iterator efi = efields[el_index].begin();
    while (efi != efields[el_index].end()) {
      Mesh::IndexType field_id = *efi++;
      Mesh::IndexType field_index = field_id - 1;
      unsigned int ncomp = fields[field_index].Components();
      if (fields[field_index].Order() == 0)
        for (Mesh::IndexType i = 0; i < ncomp; i++)
          ElementDofs[el_index].push_back(dofnum++);
    }
  }
  return (dofnum - 1);
}

// Assumes each node and element have the same number of dofs as every
// other node and element, that is, the full compliment of dofs on each
// mesh entity.  Need to be able to have different numbers of dofs on
// each node/element/mesh entity.  Need to match each dof with it's field
// and component and mesh entity.
// Numbers 1 - nlocaldofs,nlocaldofs+1 - ntotaldofs
Mesh::IndexType AssignLocalDofsByElement(
    Mesh::Connectivity &ec,           // input
    Mesh::Connectivity &dc,           // input
    Mesh::Connectivity &nbr,          // input element nbr list
    Mesh::Connectivity &efields,      // input (sorted)
    Mesh::Connectivity &nfields,      // output
    Mesh::Connectivity &NodalDofs,    // output
    Mesh::Connectivity &ElementDofs,  // output
    const std::vector<FieldData> &fields, Mesh::PartInfo &info,
    std::list<Mesh::IndexType> &border_element_list,
    std::vector<Mesh::Border> &borders, Mesh::IndexType &nlocaldofs, int rank) {
  rank = 1;  // just disables debugging output for now
  Mesh::IndexType dofnum = 1;
  Mesh::IndexType nel = ec.Nelem();
  Mesh::IndexType nnodes = NodalDofs.Nelem();
  //    Mesh::IndexType field_id = 1;
  unsigned int num_fields = fields.size();
  // Assume that efields is sorted
  // 1. we build nfields as a sorted list of fields for each node
  // 2. Then we loop over elements, and for each node on the element we
  // number the dofs by field and if the element has the asssoicated
  // field, then add them to the sorted list of dofs for that element.
  // 3. we number the dofs internal to the element (i.e. non-nodal based) and
  // add them to the sorted list of dofs for each element
  //
  // Note: we can improve our initial bandwidth by looping over elements by
  // neighbor, or breadth-first in step 2.

  // scale down an efields array which contains only the fields which
  // touch nodes
  Mesh::Connectivity enfields;
  enfields.resize(nel);
  enfields.Sync();
  for (Mesh::IndexType jj = 0; jj < nel; jj++) {
    std::vector<Mesh::IndexType>::iterator efi = efields[jj].begin();
    while (efi != efields[jj].end()) {
      if (fields[*efi - 1].Order() > 0) enfields[jj].push_back(*efi);
      efi++;
    }
  }
  // Step 1
  //  - This is done with the handy dandy list inversions
  // Takes N[E] and E[F] and produces N[F]
  // Like neighborhood, but for disparate quantities
  dc.GetAdjacent(nfields, enfields, num_fields);

  // Step 2
  std::vector<bool> processed_node(nnodes, false);
  std::vector<bool> processed_element(nel, false);
  std::vector<bool> processed_borderel(nel, false);
  border_element_list.clear();
  std::list<Mesh::IndexType> queue;
  Mesh::IndexType nprocessed = 0;
  Mesh::IndexType outerloop = 0;
  queue.push_back(1);
  //    while(nprocessed < nel && outerloop < nel){
  while (!queue.empty() && nprocessed < nel && outerloop < nel) {
    Mesh::IndexType elid = outerloop + 1;
    Mesh::IndexType el_index = outerloop;
    if (queue.empty()) {
      while (processed_element[outerloop]) outerloop++;
    } else {
      std::list<Mesh::IndexType>::iterator eIt = queue.begin();
      elid = *eIt;
      el_index = elid - 1;
      queue.pop_front();
    }
    if (!processed_element[el_index]) {
      processed_element[el_index] = true;
      NumberElementDofsLocal(ec, ElementDofs, NodalDofs, efields, nfields,
                             fields, dofnum, el_index, info, processed_node,
                             processed_borderel, border_element_list, false,
                             rank);
      nprocessed++;
    }
    // Loop thru this element's neighbors
    std::vector<Mesh::IndexType>::iterator eni = nbr[el_index].begin();
    while (eni != nbr[el_index].end()) {
      Mesh::IndexType nbrid = *eni++;
      Mesh::IndexType nbrindex = nbrid - 1;
      if (!processed_element[nbrindex]) {
        queue.push_back(nbrid);
        processed_element[nbrindex] = true;
        nprocessed++;
        NumberElementDofsLocal(ec, ElementDofs, NodalDofs, efields, nfields,
                               fields, dofnum, nbrindex, info, processed_node,
                               processed_borderel, border_element_list, false,
                               rank);
      }
    }
  }
  nlocaldofs = dofnum - 1;
  if (!rank)
    std::cout << "nlocaldofs = " << nlocaldofs << std::endl
              << border_element_list.size() << " border elements";
  // Now put local numbers on the nodes owned by other procs
  std::list<Mesh::IndexType>::iterator bei = border_element_list.begin();
  while (bei != border_element_list.end()) {
    Mesh::IndexType elid = *bei++;
    Mesh::IndexType el_index = elid - 1;
    if (!rank) std::cout << "border element " << elid << std::endl;
    NumberElementDofsLocal(ec, ElementDofs, NodalDofs, efields, nfields, fields,
                           dofnum, el_index, info, processed_node,
                           processed_borderel, border_element_list, true, rank);
  }
  //    assert(queue.empty());
  return (dofnum - 1);
}

bool IsBorderElement(std::vector<Mesh::IndexType> &ec, Mesh::PartInfo &info) {
  std::vector<Mesh::IndexType>::iterator eni = ec.begin();
  while (eni != ec.end()) {
    Mesh::IndexType node_id = *eni++;
    if (node_id > info.nlocal) return (true);
  }
  return (false);
}

void NumberElementDofsLocalII(
    Mesh::Connectivity &ec, Mesh::Connectivity &ElementDofs,
    Mesh::Connectivity &NodalDofs, Mesh::Connectivity &efields,
    Mesh::Connectivity &nfields, const std::vector<FieldData> &fields,
    Mesh::IndexType &dofnum, Mesh::IndexType el_index, Mesh::PartInfo &info,
    std::vector<bool> &processed_node, std::vector<bool> &is_border_element,
    bool remote, int rank) {
  if (!rank) std::cout << "Processing element " << el_index + 1 << std::endl;
  // determine if this is a border element
  bool border_element = is_border_element[el_index];
  if (border_element && !rank)
    std::cout << "This is a border element." << std::endl;
  std::vector<Mesh::IndexType> redofs;
  if (remote) {
    //      redofs = ElementDofs[el_index];
    //      redofs = eldofs[el_index];
  }
  //    eldofs[el_index].resize(0);
  ElementDofs[el_index].resize(0);
  std::vector<Mesh::IndexType>::iterator eni = ec[el_index].begin();
  while (eni != ec[el_index].end()) {
    Mesh::IndexType node_id = *eni++;
    Mesh::IndexType node_index = node_id - 1;
    if (!nfields[node_index].empty()) {
      if (!processed_node[node_index]) {
        NodalDofs[node_index].resize(0);
        // if this node has not been processed, then dof it up
        // and add it's dofs to the element
        // if it has been processed, then add the existing
        std::vector<Mesh::IndexType>::iterator nfi =
            nfields[node_index].begin();
        while (nfi != nfields[node_index].end()) {
          unsigned int ncomp = fields[*nfi - 1].Components();
          for (Mesh::IndexType j = 0; j < ncomp; j++) {
            if (!remote) {
              if (node_id <= info.nlocal) {
                if (!rank)
                  std::cout << "Numbering dofs for node " << node_id
                            << std::endl;
                processed_node[node_index] = true;
                NodalDofs[node_index].push_back(dofnum);
                if (!border_element) {
                  //		    ElementDofs[el_index].push_back(dofnum);
                  //	    eldofs[el_index].push_back(dofnum);
                }
                //		  else if(!rank)
                //		    std::cout << "Skipping numbering dofs of
                //border element." << std::endl;
                dofnum++;
              } else {
                if (!rank)
                  std::cout << "Skipping remote node " << node_id << std::endl;
              }
            }
            // DON'T PUT LOCAL NUMBERS ON REMOTE ENTITIES (yet)
            //	      else if(node_id > info.nlocal){ // remote is enabled and
            //this is a border element 		if(!rank) 		  std::cout << "Numbering dofs
            //for remote node " << node_id << std::endl;
            //		NodalDofs[node_index].push_back(dofnum);
            //		ElementDofs[el_index].push_back(dofnum++);
            //		processed_node[node_index] = true;
            //	      }
          }  // loop over components of nodal field
          nfi++;
        }  // loop over nodal fields
      }    // node has not been processed
           //	else{ // node has been processed
           //	  std::vector<Mesh::IndexType>::iterator ndi =
      //NodalDofs[node_index].begin(); 	  while(ndi !=
      //NodalDofs[node_index].end()){ 	    if((!remote && !border_element) || (remote
      //&& border_element)){ 	      if(!rank) 		std::cout << "Adding dofs from existing
      //node " << node_index+1 << std::endl;
      //	      //	      ElementDofs[el_index].push_back(*ndi);
      //	      //	      eldofs[el_index].push_back(*ndi);
      //	    }
      //	    ndi++;
      //	  }
      //      }
    }
  }
  if (!remote) {
    if (!rank) std::cout << "Numbering dofs for the element." << std::endl;
    std::vector<Mesh::IndexType>::iterator efi = efields[el_index].begin();
    while (efi != efields[el_index].end()) {
      Mesh::IndexType field_id = *efi++;
      Mesh::IndexType field_index = field_id - 1;
      unsigned int ncomp = fields[field_index].Components();
      if (fields[field_index].Order() == 0)
        for (Mesh::IndexType i = 0; i < ncomp; i++)
          ElementDofs[el_index].push_back(dofnum++);
    }
  } else {  // this element already has had it's dofs numbered, recover them
    if (!rank)
      std::cout << "Recovering dof numbers from border element." << std::endl;
    std::vector<Mesh::IndexType>::iterator efi = redofs.begin();
    while (efi != redofs.end()) ElementDofs[el_index].push_back(*efi++);
  }
}

// Assumes each node and element have the same number of dofs as every
// other node and element, that is, the full compliment of dofs on each
// mesh entity.  Need to be able to have different numbers of dofs on
// each node/element/mesh entity.  Need to match each dof with it's field
// and component and mesh entity.
// Numbers 1 - nlocaldofs,nlocaldofs+1 - ntotaldofs
Mesh::IndexType AssignLocalDofsByElementII(
    Mesh::Connectivity &ec,           // input
    Mesh::Connectivity &dc,           // input
    Mesh::Connectivity &nbr,          // input element nbr list
    Mesh::Connectivity &efields,      // input (sorted)
    Mesh::Connectivity &nfields,      // output
    Mesh::Connectivity &NodalDofs,    // output
    Mesh::Connectivity &ElementDofs,  // output
    const std::vector<FieldData> &fields, Mesh::PartInfo &info,
    std::list<Mesh::IndexType> &border_element_list,
    std::vector<Mesh::Border> &borders, Mesh::IndexType &nlocaldofs, int rank) {
  //    rank = 0; // just disables debugging output for now
  Mesh::IndexType dofnum = 1;
  Mesh::IndexType nel = ec.Nelem();
  Mesh::IndexType nnodes = NodalDofs.Nelem();
  //    Mesh::IndexType field_id = 1;
  unsigned int num_fields = fields.size();
  // Assume that efields is sorted
  // 1. we build nfields as a sorted list of fields for each node
  // 2. Then we loop over elements, and for each node on the element we
  // number the dofs by field and if the element has the asssoicated
  // field, then add them to the sorted list of dofs for that element.
  // 3. we number the dofs internal to the element (i.e. non-nodal based) and
  // add them to the sorted list of dofs for each element
  //
  // Note: we can improve our initial bandwidth by looping over elements by
  // neighbor, or breadth-first in step 2.

  // scale down an efields array which contains only the fields which
  // touch nodes
  Mesh::Connectivity enfields;
  enfields.resize(nel);
  enfields.Sync();
  for (Mesh::IndexType jj = 0; jj < nel; jj++) {
    std::vector<Mesh::IndexType>::iterator efi = efields[jj].begin();
    while (efi != efields[jj].end()) {
      if (fields[*efi - 1].Order() > 0) enfields[jj].push_back(*efi);
      efi++;
    }
  }
  // Step 1
  //  - This is done with the handy dandy list inversions
  // Takes N[E] and E[F] and produces N[F]
  // Like neighborhood, but for disparate quantities
  dc.GetAdjacent(nfields, enfields, num_fields);

  // Step 2
  std::vector<bool> processed_node(nnodes, false);
  std::vector<bool> processed_element(nel, false);
  std::vector<bool> is_border_element(nel, false);
  border_element_list.clear();
  std::list<Mesh::IndexType> queue;
  Mesh::IndexType nprocessed = 0;
  Mesh::IndexType outerloop = 0;
  queue.push_back(1);
  //    while(nprocessed < nel && outerloop < nel){
  while (!queue.empty() && nprocessed < nel && outerloop < nel) {
    Mesh::IndexType elid = outerloop + 1;
    Mesh::IndexType el_index = outerloop;
    if (queue.empty()) {
      while (processed_element[outerloop]) {
        outerloop++;
        el_index++;
      }
    } else {
      std::list<Mesh::IndexType>::iterator eIt = queue.begin();
      elid = *eIt;
      el_index = elid - 1;
      queue.pop_front();
    }
    if (!rank) std::cout << "Considering element " << el_index + 1 << std::endl;
    if (!processed_element[el_index]) {
      is_border_element[el_index] = IsBorderElement(ec[el_index], info);
    }
    if (is_border_element[el_index]) {
      if (!rank)
        std::cout << "Found border element " << el_index + 1 << std::endl;
      processed_element[el_index] = true;
      border_element_list.push_back(el_index + 1);
    } else if (!processed_element[el_index]) {
      if (!rank)
        std::cout << "Not a border and not processed, processing "
                  << el_index + 1 << std::endl;
      processed_element[el_index] = true;
      NumberElementDofsLocalII(ec, ElementDofs, NodalDofs, efields, nfields,
                               fields, dofnum, el_index, info, processed_node,
                               is_border_element, false, rank);
      nprocessed++;
    }
    // Loop thru this element's neighbors
    std::vector<Mesh::IndexType>::iterator eni = nbr[el_index].begin();
    while (eni != nbr[el_index].end()) {
      Mesh::IndexType nbrid = *eni++;
      Mesh::IndexType nbrindex = nbrid - 1;
      if (!processed_element[nbrindex]) {
        is_border_element[nbrindex] = IsBorderElement(ec[el_index], info);
      }
      if (is_border_element[nbrindex]) {
        processed_element[nbrindex] = true;
        border_element_list.push_back(el_index + 1);
      } else {
        queue.push_back(nbrid);
        nprocessed++;
        processed_element[nbrindex] = true;
        NumberElementDofsLocalII(ec, ElementDofs, NodalDofs, efields, nfields,
                                 fields, dofnum, nbrindex, info, processed_node,
                                 is_border_element, false, rank);
      }
    }
  }
  nlocaldofs = dofnum - 1;
  if (!rank)
    std::cout << "nlocaldofs = " << nlocaldofs << std::endl
              << border_element_list.size() << " border elements";
  // WE DONT WANT TO DO THIS PART FOR THE NEW WAY - In the new way
  // entities that I do not own DO NOT GET LOCAL DOFS (yet).  If we
  // have dofs that we want there which are in addition to the owner's
  // dofs, then we can assign them and keep'em local.
  // Now put local numbers on the nodes owned by other procs
  //
  // Instead, put local numbers on as of yet unnumbered local entities
  // that live on border elements.
  //    border_element_list.sort();
  //    border_element_list.unique();
  std::list<Mesh::IndexType>::iterator bei = border_element_list.begin();
  while (bei != border_element_list.end()) {
    Mesh::IndexType elid = *bei++;
    Mesh::IndexType el_index = elid - 1;
    if (!rank) std::cout << "numbering border element " << elid << std::endl;
    //      NumberElementDofsLocal(ec,ElementDofs,NodalDofs,efields,nfields,fields,dofnum,el_index,
    //			     info,processed_node,processed_borderel,border_element_list,true,rank);
    NumberElementDofsLocalII(ec, ElementDofs, NodalDofs, efields, nfields,
                             fields, dofnum, el_index, info, processed_node,
                             is_border_element, false, rank);
  }
  //    assert(queue.empty());
  return (dofnum - 1);
}

void NumberElementDofsLocal(
    Mesh::Connectivity &ec, Mesh::Connectivity &ElementDofs,
    Mesh::Connectivity &NodalDofs, Mesh::Connectivity &efields,
    Mesh::Connectivity &nfields, const std::vector<FieldData> &fields,
    Mesh::IndexType &dofnum, Mesh::IndexType el_index, Mesh::PartInfo &info,
    std::vector<bool> &processed_node, std::vector<bool> &processed_element,
    std::list<Mesh::IndexType> &borderelements, bool remote, int rank) {
  if (!rank) std::cout << "Processing element " << el_index + 1 << std::endl;
  // determine if this is a border element
  bool border_element = false;
  if (!remote) {
    std::vector<Mesh::IndexType>::iterator eni = ec[el_index].begin();
    bool done = false;
    while (eni != ec[el_index].end() && !done) {
      Mesh::IndexType node_id = *eni++;
      if (node_id > info.nlocal) {
        if (!processed_element[el_index]) {
          processed_element[el_index] = true;
          borderelements.push_back(el_index + 1);
        }
        border_element = true;
        done = true;
      }
    }
  } else
    border_element = true;
  if (border_element && !rank)
    std::cout << "This is a border element." << std::endl;
  std::vector<Mesh::IndexType> redofs;
  if (remote) {
    redofs = ElementDofs[el_index];
  }
  ElementDofs[el_index].resize(0);
  std::vector<Mesh::IndexType>::iterator eni = ec[el_index].begin();
  while (eni != ec[el_index].end()) {
    Mesh::IndexType node_id = *eni++;
    Mesh::IndexType node_index = node_id - 1;
    if (!nfields[node_index].empty()) {
      if (!processed_node[node_index]) {
        NodalDofs[node_index].resize(0);
        // if this node has not been processed, then dof it up
        // and add it's dofs to the element
        // if it has been processed, then add the existing
        std::vector<Mesh::IndexType>::iterator nfi =
            nfields[node_index].begin();
        while (nfi != nfields[node_index].end()) {
          unsigned int ncomp = fields[*nfi - 1].Components();
          for (Mesh::IndexType j = 0; j < ncomp; j++) {
            if (!remote) {
              if (node_id <= info.nlocal) {
                if (!rank)
                  std::cout << "Numbering dofs for node " << node_id
                            << std::endl;
                processed_node[node_index] = true;
                NodalDofs[node_index].push_back(dofnum);
                if (!border_element)
                  ElementDofs[el_index].push_back(dofnum);
                else if (!rank)
                  std::cout << "Skipping numbering dofs of border element."
                            << std::endl;
                dofnum++;
              } else {
                if (!rank)
                  std::cout << "Skipping remote node " << node_id << std::endl;
              }
            } else if (node_id > info.nlocal) {  // remote is enabled and this
                                                 // is a border element
              if (!rank)
                std::cout << "Numbering dofs for remote node " << node_id
                          << std::endl;
              NodalDofs[node_index].push_back(dofnum);
              ElementDofs[el_index].push_back(dofnum++);
              processed_node[node_index] = true;
            }
          }  // loop over components of nodal field
          nfi++;
        }     // loop over nodal fields
      }       // node has not been processed
      else {  // node has been processed
        std::vector<Mesh::IndexType>::iterator ndi =
            NodalDofs[node_index].begin();
        while (ndi != NodalDofs[node_index].end()) {
          if ((!remote && !border_element) || (remote && border_element)) {
            if (!rank)
              std::cout << "Adding dofs from existing node " << node_index + 1
                        << std::endl;
            ElementDofs[el_index].push_back(*ndi);
          }
          ndi++;
        }
      }
    }
  }
  if (!remote) {
    if (!rank) std::cout << "Numbering dofs for the element." << std::endl;
    std::vector<Mesh::IndexType>::iterator efi = efields[el_index].begin();
    while (efi != efields[el_index].end()) {
      Mesh::IndexType field_id = *efi++;
      Mesh::IndexType field_index = field_id - 1;
      unsigned int ncomp = fields[field_index].Components();
      if (fields[field_index].Order() == 0)
        for (Mesh::IndexType i = 0; i < ncomp; i++)
          ElementDofs[el_index].push_back(dofnum++);
    }
  } else {  // this element already has had it's dofs numbered, recover them
    if (!rank)
      std::cout << "Recovering dof numbers from border element." << std::endl;
    std::vector<Mesh::IndexType>::iterator efi = redofs.begin();
    while (efi != redofs.end()) ElementDofs[el_index].push_back(*efi++);
  }
}
// Assumes each node and element have the same number of dofs as every
// other node and element, that is, the full compliment of dofs on each
// mesh entity.  Need to be able to have different numbers of dofs on
// each node/element/mesh entity.  Need to match each dof with it's field
// and component and mesh entity.
Mesh::IndexType AssignDofsByElementII(
    Mesh::Connectivity &ec,                // input
    Mesh::Connectivity &dc,                // input
    Mesh::Connectivity &nbr,               // input element nbr list
    Mesh::Connectivity &efields,           // input (sorted)
    Mesh::Connectivity &nfields,           // output
    Mesh::Connectivity &NodalDofs,         // output
    Mesh::Connectivity &ElementDofs,       // output
    const std::vector<FieldData> &fields)  // input
{
  Mesh::IndexType dofnum = 1;
  Mesh::IndexType nel = ec.Nelem();
  Mesh::IndexType nnodes = NodalDofs.Nelem();
  //    Mesh::IndexType field_id = 1;
  unsigned int num_fields = fields.size();
  // Assume that efields is sorted
  // 1. we build nfields as a sorted list of fields for each node
  // 2. Then we loop over elements, and for each node on the element we
  // number the dofs by field and if the element has the asssoicated
  // field, then add them to the sorted list of dofs for that element.
  // 3. we number the dofs internal to the element (i.e. non-nodal based) and
  // add them to the sorted list of dofs for each element
  //
  // Note: we can improve our initial bandwidth by looping over elements by
  // neighbor, or breadth-first in step 2.

  // scale down an efields array which contains only the fields which
  // touch nodes
  Mesh::Connectivity enfields;
  enfields.resize(nel);
  enfields.Sync();
  for (Mesh::IndexType jj = 0; jj < nel; jj++) {
    std::vector<Mesh::IndexType>::iterator efi = efields[jj].begin();
    while (efi != efields[jj].end()) {
      if (fields[*efi - 1].Order() > 0) enfields[jj].push_back(*efi);
      efi++;
    }
  }
  // Step 1
  //  - This is done with the handy dandy list inversions
  // Takes N[E] and E[F] and produces N[F]
  // Like neighborhood, but for disparate quantities
  dc.GetAdjacent(nfields, enfields, num_fields);

  // Step 2
  std::vector<bool> processed_node(nnodes, false);
  std::vector<bool> processed_element(nel, false);
  std::list<Mesh::IndexType> queue;
  Mesh::IndexType nprocessed = 0;
  Mesh::IndexType outerloop = 0;
  queue.push_back(1);
  //    while(nprocessed < nel && outerloop < nel){
  while (!queue.empty() && nprocessed < nel && outerloop < nel) {
    Mesh::IndexType elid = outerloop + 1;
    Mesh::IndexType el_index = outerloop;
    if (queue.empty()) {
      while (processed_element[outerloop]) outerloop++;
    } else {
      std::list<Mesh::IndexType>::iterator eIt = queue.begin();
      elid = *eIt;
      el_index = elid - 1;
      queue.pop_front();
    }
    if (!processed_element[el_index]) {
      processed_element[el_index] = true;
      NumberElementDofs(ec, ElementDofs, NodalDofs, efields, nfields, fields,
                        dofnum, el_index, processed_node);
      nprocessed++;
    }
    // Loop thru this element's neighbors
    std::vector<Mesh::IndexType>::iterator eni = nbr[el_index].begin();
    while (eni != nbr[el_index].end()) {
      Mesh::IndexType nbrid = *eni++;
      Mesh::IndexType nbrindex = nbrid - 1;
      if (!processed_element[nbrindex]) {
        queue.push_back(nbrid);
        processed_element[nbrindex] = true;
        nprocessed++;
        NumberElementDofs(ec, ElementDofs, NodalDofs, efields, nfields, fields,
                          dofnum, nbrindex, processed_node);
      }
    }
  }
  //    assert(queue.empty());
  return (dofnum - 1);
}

void NumberElementDofs(Mesh::Connectivity &ec, Mesh::Connectivity &ElementDofs,
                       Mesh::Connectivity &NodalDofs,
                       Mesh::Connectivity &efields, Mesh::Connectivity &nfields,
                       const std::vector<FieldData> &fields,
                       Mesh::IndexType &dofnum, Mesh::IndexType el_index,
                       std::vector<bool> &processed_node) {
  ElementDofs[el_index].resize(0);
  std::vector<Mesh::IndexType>::iterator eni = ec[el_index].begin();
  while (eni != ec[el_index].end()) {
    Mesh::IndexType node_id = *eni++;
    Mesh::IndexType node_index = node_id - 1;
    if (!nfields[node_index].empty()) {
      if (!processed_node[node_index]) {
        NodalDofs[node_index].resize(0);
        // if this node has not been processed, then dof it up
        // and add it's dofs to the element
        // if it has been processed, then add the existing
        processed_node[node_index] = true;
        std::vector<Mesh::IndexType>::iterator nfi =
            nfields[node_index].begin();
        while (nfi != nfields[node_index].end()) {
          unsigned int ncomp = fields[*nfi - 1].Components();
          for (Mesh::IndexType j = 0; j < ncomp; j++) {
            NodalDofs[node_index].push_back(dofnum);
            ElementDofs[el_index].push_back(dofnum++);
          }
          nfi++;
        }
      } else {
        std::vector<Mesh::IndexType>::iterator ndi =
            NodalDofs[node_index].begin();
        while (ndi != NodalDofs[node_index].end())
          ElementDofs[el_index].push_back(*ndi++);
      }
    }
  }
  std::vector<Mesh::IndexType>::iterator efi = efields[el_index].begin();
  while (efi != efields[el_index].end()) {
    Mesh::IndexType field_id = *efi++;
    Mesh::IndexType field_index = field_id - 1;
    unsigned int ncomp = fields[field_index].Components();
    if (fields[field_index].Order() == 0)
      for (Mesh::IndexType i = 0; i < ncomp; i++)
        ElementDofs[el_index].push_back(dofnum++);
  }
}
// Assumes each node and element have the same number of dofs as every
// other node and element, that is, the full compliment of dofs on each
// mesh entity.  Need to be able to have different numbers of dofs on
// each node/element/mesh entity.  Need to match each dof with it's field
// and component and mesh entity.
Mesh::IndexType AssignUniformDofs(
    Mesh::Connectivity &ec, Mesh::Connectivity &dc, Mesh::Connectivity &efields,
    Mesh::Connectivity &nfields, Mesh::Connectivity &NodalDofs,
    Mesh::Connectivity &ElementDofs, const std::vector<FieldData> &fields) {
  Mesh::IndexType dofnum = 1;
  //    Mesh::IndexType nel    = ec.size();
  Mesh::IndexType nnodes = NodalDofs.size();
  Mesh::IndexType field_id = 1;

  std::vector<FieldData>::const_iterator fi = fields.begin();
  while (fi != fields.end()) {
    int ncomp = fi->Components();
    Mesh::Connectivity::iterator di;
    Mesh::Connectivity::iterator end;
    if (fi->Order() == 1) {
      for (Mesh::IndexType jj = 0; jj < nnodes; jj++) {
        nfields[jj].push_back(field_id);
        for (int i = 0; i < ncomp; i++) {
          NodalDofs[jj].push_back(dofnum + i);
          std::vector<Mesh::IndexType>::iterator dcIt = dc[jj].begin();
          while (dcIt != dc[jj].end())
            ElementDofs[*dcIt++ - 1].push_back(dofnum + i);
        }
        dofnum += ncomp;
      }
    } else if (fi->Order() == 0) {
      di = ElementDofs.begin();
      end = ElementDofs.end();
      while (di != end) {
        for (int i = 0; i < ncomp; i++) di->push_back(dofnum++);
        di++;
      }
    }
    fi++;
    field_id++;
  }
  return (dofnum - 1);
}

// Assumes each node and element have the same number of dofs as every
// other node and element, that is, the full compliment of dofs on each
// mesh entity.  Need to be able to have different numbers of dofs on
// each node/element/mesh entity.  Need to match each dof with it's field
// and component and mesh entity.
Mesh::IndexType AssignUniformFields(
    Mesh::Connectivity &ec, Mesh::Connectivity &dc, Mesh::Connectivity &efields,
    Mesh::Connectivity &nfields, Mesh::Connectivity &NodalDofs,
    Mesh::Connectivity &ElementDofs, const std::vector<FieldData> &fields) {
  Mesh::IndexType dofnum = 1;
  //    Mesh::IndexType nel    = ec.size();
  Mesh::IndexType nnodes = NodalDofs.size();
  Mesh::IndexType field_id = 1;

  std::vector<FieldData>::const_iterator fi = fields.begin();
  while (fi != fields.end()) {
    int ncomp = fi->Components();
    Mesh::Connectivity::iterator di;
    Mesh::Connectivity::iterator end;
    if (fi->Order() == 1) {
      for (Mesh::IndexType jj = 0; jj < nnodes; jj++) {
        nfields[jj].push_back(field_id);
        for (int i = 0; i < ncomp; i++) {
          NodalDofs[jj].push_back(dofnum + i);
          std::vector<Mesh::IndexType>::iterator dcIt = dc[jj].begin();
          while (dcIt != dc[jj].end())
            ElementDofs[*dcIt++ - 1].push_back(dofnum + i);
        }
        dofnum += ncomp;
      }
    } else if (fi->Order() == 0) {
      di = ElementDofs.begin();
      end = ElementDofs.end();
      while (di != end) {
        for (int i = 0; i < ncomp; i++) di->push_back(dofnum++);
        di++;
      }
    }
    fi++;
    field_id++;
  }
  return (dofnum - 1);
}

void AssembleSpecialDofList(Mesh::Connectivity &econ, Mesh::IndexType elindex,
                            Mesh::Connectivity &NodalDofs,
                            Mesh::Connectivity &ElementDofs) {}

int AssembleFullDofList(Mesh::Connectivity &econ, Mesh::IndexType elindex,
                        Mesh::Connectivity &NodalDofs,
                        Mesh::Connectivity &ElementDofs,
                        std::vector<Mesh::IndexType> &dofs) {
  std::vector<Mesh::IndexType>::iterator eni = econ[elindex].begin();
  Mesh::IndexType vindex = 0;
  while (eni != econ[elindex].end()) {
    std::vector<Mesh::IndexType>::iterator ndi = NodalDofs[*eni - 1].begin();
    while (ndi != NodalDofs[*eni - 1].end()) dofs[vindex++] = *ndi++;
    eni++;
  }
  eni = ElementDofs[elindex].begin();
  while (eni != ElementDofs[elindex].end()) dofs[vindex++] = *eni++;
  return (vindex);
}

// new one
int FastAssembleLocalElements(
    std::list<Mesh::IndexType> element_queue, Mesh::Connectivity &econ,
    FEM::DummyStiffness<double, Mesh::IndexType, Mesh::Connectivity,
                        std::vector<Mesh::IndexType> > &k,
    Mesh::Connectivity &NodalDofs, Mesh::Connectivity &ElementDofs,
    std::vector<Mesh::IndexType> &NDofE, Mesh::PartInfo &info) {
  // the row size is the same every time, else we wouldn't have multiple
  // rows to set.
  // elindex;
  // Loop thru the element's DOF's.
  //  1. Loop thru the Element's nodes
  Mesh::IndexType Rrow = 0;
  unsigned int Rnrow = 0;
  std::vector<double> dofdat(10000, 1.0);
  unsigned int datind = 0;
  std::vector<Mesh::IndexType>::iterator eni;
  std::vector<Mesh::IndexType>::iterator enIt;
  std::vector<Mesh::IndexType>::iterator Rbegin;
  std::vector<Mesh::IndexType>::iterator Rend;
  std::vector<Mesh::IndexType>::iterator Sbegin;
  std::vector<Mesh::IndexType>::iterator Send;
  std::vector<Mesh::IndexType>::iterator CurPos;
  //  std::vector<Mesh::IndexType> doflist(24,0);
  Mesh::IndexType rowsize;
  Mesh::IndexType datindbase;
  unsigned int colcount = 0;
  unsigned int rowcount = 0;
  Mesh::IndexType elindex = 0;
  Mesh::IndexType col = 0;
  unsigned int ncol = 0;
  Mesh::IndexType last_col = 0;
  unsigned int ndofe = 0;
  Mesh::IndexType index = 0;
  unsigned int edofs = 0;
  int nsearches = 0;
  unsigned int searcherr = 0;
  //     std::ofstream MyErrFile;
  //     std::ostringstream MyErrStr;
  //     MyErrStr << "AssErr_" << info.part;
  //     MyErrFile.open(MyErrStr.str().c_str());
  //     MyErrFile << "info.doffset = " << info.doffset << std::endl;
  std::list<Mesh::IndexType>::iterator eIt = element_queue.begin();
  while (eIt != element_queue.end()) {
    //    std::list<Mesh::IndexType> doflist;
    //    AssembleFullDofList(econ,*eIt-1,NodalDofs,ElementDofs,doflist);
    edofs = ElementDofs.Esize(*eIt);  // All the dofs of the element?
    elindex = *eIt++ - 1;
    //    ndofe = (INTEGRATE ELEMENT); // integrates element and populates
    //    dofdat (add physics here)
    ndofe = NDofE[elindex];  // Number of element centered dofs
    std::vector<Mesh::IndexType>::iterator eni = econ[elindex].begin();
    while (eni !=
           econ[elindex].end()) {  // Loop through the nodes of the element
      //     The row is the node's first dof
      Rrow = *NodalDofs[*eni - 1].begin();      // local dof number - row number
      Rbegin = (*(k._dofs))[Rrow - 1].begin();  // beginning of sparse row
      Rend = (*(k._dofs))[Rrow - 1].end();      // end of sparse row
      Sbegin = Rbegin;
      Send = Rend;
      CurPos = Sbegin;
      //     The number of rows is the number of dofs on the node
      Rnrow = (NodalDofs.Esize(*eni++));
      rowsize = k._sizes[Rrow] - k._sizes[Rrow - 1];
      // Place the element's dof bundles in Rnrows starting at Rrow
      enIt = econ[elindex].begin();   // iterator thru element node id's
      col = NodalDofs[*enIt - 1][0];  // the first dof id of the first node
                                      // (should be sorted i suppose)
      datindbase = rowcount *
                   ndofe;  // rowcount wrt element times number dofs in element
      colcount = 0;
      while (enIt != econ[elindex].end()) {  // nodal dofs of element
        last_col = col;
        col = NodalDofs[*enIt - 1][0] + info.doffset;
        ncol = NodalDofs.Esize(*enIt++);
        if (last_col < col) {
          Sbegin = CurPos;
          Send = Rend;
        } else if (last_col > col) {
          Sbegin = Rbegin;
          Send = CurPos;
        } else {
          Sbegin = Rbegin;
          Send = Rend;
        }
        nsearches++;
        CurPos = std::lower_bound(Sbegin, Send, col);
        // 	  if(CurPos == Send){
        // 	    searcherr++;
        // 	    MyErrFile <<
        // "nn(Rrow,Rnrow,Sbegin,Send):(col,ncol,colcount):(datindbase,nsearches)
        // = (" << Rrow << ","
        // 		      << Rnrow << "," << *Sbegin << "," << *Send << "):("
        // << col
        // 		      << "," << ncol << "," << colcount << "):(" <<
        // datindbase
        // 		      << "," << nsearches << ")" << std::endl;
        // 	    Util::DumpContents(MyErrFile,(*(k._dofs))[Rrow-1]," ");
        // 	    MyErrFile << std::endl;
        // 	  }
        index = k._sizes[Rrow - 1] + (CurPos - Rbegin);
        datind = datindbase + colcount;
        colcount += ncol;
        for (unsigned int l = 0; l < Rnrow; l++) {
          for (unsigned int ii = 0; ii < ncol; ii++)
            k[ii] += dofdat[datind + ii];  // symbolic assembly?
          // k[index+ii] += dofdat[datind+ii]; // real assembly?
          index += rowsize;
          datind += ndofe;
        }
      }
      // This only needs to be done if there are actually elemental dofs
      if (edofs > 0) {
        last_col = col;
        col = ElementDofs[elindex][0] + info.doffset;
        ncol = ElementDofs.Esize(elindex + 1);
        if (last_col < col) {
          Sbegin = CurPos;
          Send = Rend;
        } else if (last_col > col) {
          Sbegin = Rbegin;
          Send = CurPos;
        } else {
          Sbegin = Rbegin;
          Send = Rend;
        }
        nsearches++;
        CurPos = std::lower_bound(Sbegin, Send, col);
        // 	  if(CurPos == Send){
        // 	    searcherr++;
        // 	    MyErrFile <<
        // "ne(Rrow,Rnrow):(col,ncol,colcount):(datindbase,nsearches) = (" <<
        // Rrow << ","
        // 		      << Rnrow << "):(" << col
        // 		      << "," << ncol << "," << colcount << "):(" <<
        // datindbase
        // 		      << "," << nsearches << ")" << std::endl;
        // 	    Util::DumpContents(MyErrFile,(*(k._dofs))[Rrow-1]);
        // 	  }
        //	  assert(CurPos != Send);
        index = k._sizes[Rrow - 1] + (CurPos - Rbegin);
        datind = datindbase + colcount;
        for (unsigned int l = 0; l < Rnrow; l++) {
          for (unsigned int ii = 0; ii < ncol; ii++)
            k[ii] += dofdat[datind + ii];  // Symbolic assembly?
          //	      k[index+ii] += dofdat[datind+ii]; // Real Assembly??
          index += rowsize;
          datind += ndofe;
        }
      }
    }
    // Now still need to finish looping thru element's dofs.  The elemental dofs
    // if there are any, go here.
    if (edofs > 0) {
      //     The row is the node's first dof
      Rrow = *ElementDofs[elindex].begin();
      Rbegin = (*(k._dofs))[Rrow - 1].begin();
      Rend = (*(k._dofs))[Rrow - 1].end();
      Sbegin = Rbegin;
      Send = Rend;
      CurPos = Sbegin;
      //     The number of rows is the number of dofs on the node
      Rnrow = edofs;
      rowsize = k._sizes[Rrow] - k._sizes[Rrow - 1];
      // Place the element's dof bundles in Rnrows starting at Rrow
      enIt = econ[elindex].begin();
      col = NodalDofs[*enIt - 1][0] + info.doffset;
      datindbase = rowcount *
                   ndofe;  // rowcount wrt element times number dofs in element
      colcount = 0;
      while (enIt != econ[elindex].end()) {  // nodal dofs of element
        last_col = col;
        col = NodalDofs[*enIt - 1][0] + info.doffset;
        ncol = NodalDofs.Esize(*enIt++);
        if (last_col < col) {
          Sbegin = CurPos;
          Send = Rend;
        } else if (last_col > col) {
          Sbegin = Rbegin;
          Send = CurPos;
        } else {
          Sbegin = Rbegin;
          Send = Rend;
        }
        nsearches++;
        CurPos = std::lower_bound(Sbegin, Send, col);
        // 	  if(CurPos == Send){
        // 	    searcherr++;
        // 	    MyErrFile <<
        // "en(Rrow,Rnrow):(col,ncol,colcount):(datindbase,nsearches) = (" <<
        // Rrow << ","
        // 		      << Rnrow << "):(" << col
        // 		      << "," << ncol << "," << colcount << "):(" <<
        // datindbase
        // 		      << "," << nsearches << ")" << std::endl;
        // 	    Util::DumpContents(MyErrFile,(*(k._dofs))[Rrow-1]);
        // 	  }
        //	  assert(CurPos != Send);
        index = k._sizes[Rrow - 1] + (CurPos - Rbegin);
        datind = datindbase + colcount;
        colcount += ncol;
        for (unsigned int l = 0; l < Rnrow; l++) {
          for (unsigned int ii = 0; ii < ncol; ii++)
            k[ii] += dofdat[datind + ii];  // symbolic assembly?
          // k[index+ii] += dofdat[datind+ii]; // real assembly?
          index += rowsize;
          datind += ndofe;
        }
      }
      // This only needs to be done if there are actually elemental dofs
      if (edofs > 0) {
        last_col = col;
        col = ElementDofs[elindex][0] + info.doffset;
        ncol = ElementDofs.Esize(elindex + 1);
        if (last_col < col) {
          Sbegin = CurPos;
          Send = Rend;
        } else if (last_col > col) {
          Sbegin = Rbegin;
          Send = CurPos;
        } else {
          Sbegin = Rbegin;
          Send = Rend;
        }
        nsearches++;
        CurPos = std::lower_bound(Sbegin, Send, col);
        // 	  if(CurPos == Send){
        // 	    searcherr++;
        // 	    MyErrFile <<
        // "ee(Rrow,Rnrow):(col,ncol,colcount):(datindbase,nsearches) = (" <<
        // Rrow << ","
        // 		      << Rnrow << "):(" << col
        // 		      << "," << ncol << "," << colcount << "):(" <<
        // datindbase
        // 		      << "," << nsearches << ")" << std::endl;
        // 	    Util::DumpContents(MyErrFile,(*(k._dofs))[Rrow-1]);
        // 	  }
        //	  assert(CurPos != Send);
        index = k._sizes[Rrow - 1] + (CurPos - Rbegin);
        datind = datindbase + colcount;
        for (unsigned int l = 0; l < Rnrow; l++) {
          for (unsigned int ii = 0; ii < ncol; ii++)
            k[ii] += dofdat[datind + ii];  // Symbolic assembly?
          // k[index+ii] += dofdat[datind+ii]; // Real assembly!
          index += rowsize;
          datind += ndofe;
        }
      }
    }
  }
  //    MyErrFile << "NSearchErrors/Nsearches: " << searcherr << "/" <<
  //    nsearches << std::endl; MyErrFile.close();
  return (nsearches);
}

Mesh::IndexType AssembleLocalElement(
    std::vector<Mesh::IndexType> &con, std::vector<Mesh::IndexType> &edofs,
    Mesh::IndexType &endof, Mesh::IndexType &nedofs,
    Mesh::Connectivity &NodalDofs,
    FEM::DummyStiffness<double, Mesh::IndexType, Mesh::Connectivity,
                        std::vector<Mesh::IndexType> > &k,
    std::vector<Mesh::IndexType> &local_dof_to_global,
    Mesh::IndexType &nglobal_dofs, Mesh::IndexType &doffset,
    Mesh::PartInfo &info) {
  double dummy = 1.0;
  Mesh::IndexType nsearches = 0;
  double *dofdata = &dummy;
  // commented for symbolic assembly
  //      std::vector<double> dofdata(endof,1.0);
  // for every dof on this element, write it's row
  for (Mesh::IndexType dofn = 0; dofn < endof; dofn++) {
    //    Mesh::IndexType row = ElementDofs[nnn][dofn];
    Mesh::IndexType row = edofs[dofn];
    if (row > 0) {
      Mesh::IndexType datind = 0;
      std::vector<Mesh::IndexType>::iterator eni = con.begin();
      // write each node's contribution, searching once per node
      // since the dofs on each node are sequential
      while (eni != con.end()) {
        Mesh::IndexType node_id = *eni++;
        // write ndof at a time
        Mesh::IndexType ndof = NodalDofs.Esize(node_id);
        if (ndof > 0) {
          Mesh::IndexType node_index = node_id - 1;
          // get the beginning dof id on this node
          // for the search column
          //	  Mesh::IndexType bdof = NodalDofs[node_index][0];
          //	  if(node_id > info.nlocal)
          //	    bdof = local_dof_to_global[bdof-nglobal_dofs-1];
          //	  else
          //	    bdof += doffset;
          nsearches++;
          Mesh::IndexType bdof = NodalDofs[node_index][0] + doffset;
          FEM::AssembleIJ(row, bdof, &dofdata[datind], k, ndof);
          datind += ndof;
        }
      }
      //      if(edof_sizes[nnn] > 0){
      if (nedofs > 0) {
        nsearches++;
        Mesh::IndexType col =
            edofs[datind] + doffset;  // all of these are local
        FEM::AssembleIJ(row, col, &dofdata[datind], k, nedofs);
      }
    }
  }
  return (nsearches);
}

Mesh::IndexType AssembleLocalElementII(
    std::vector<Mesh::IndexType> &con, std::vector<Mesh::IndexType> &edofs,
    Mesh::IndexType &endof, Mesh::IndexType &nedofs,
    Mesh::Connectivity &NodalDofs,
    FEM::DummyStiffness<double, Mesh::IndexType, Mesh::Connectivity,
                        std::vector<Mesh::IndexType> > &k,
    std::vector<double> &dofdata, Mesh::PartInfo &info) {
  //  double dummy = 1.0;
  Mesh::IndexType nsearches = 0;
  Mesh::IndexType datind = 0;
  //  double *dofdata = &dummy;
  // Dskip is hardcoded for symbolic assembly
  Mesh::IndexType dskip = 0;
  // commented for symbolic assembly
  //      std::vector<double> dofdata(endof,1.0);
  // for every dof on this element, write it's row
  std::vector<Mesh::IndexType>::iterator eni = con.begin();
  while (eni != con.end()) {
    datind = 0;
    Mesh::IndexType node_id = *eni++;
    Mesh::IndexType nrow = NodalDofs.Esize(node_id);
    Mesh::IndexType brow = 0;
    if (nrow > 0) {
      brow = NodalDofs[node_id - 1][0];
      std::vector<Mesh::IndexType>::iterator ieni = con.begin();
      while (ieni != con.end()) {
        Mesh::IndexType inode_id = *ieni++;
        Mesh::IndexType ncol = NodalDofs.Esize(inode_id);
        if (ncol > 0) {
          Mesh::IndexType bcol = NodalDofs[inode_id - 1][0] + info.doffset;
          nsearches++;
          // dskip needs to to be the total number of dofs for the element.
          //      Mesh::IndexType dskip = endof;
          FEM::AssembleIJMN(brow, bcol, dofdata, k, datind, dskip, nrow, ncol);
          datind += ncol;
        }
      }
      if (nedofs > 0) {
        nsearches++;
        //      Mesh::IndexType bcol = edofs[datind]+info.doffset; // all of
        //      these are local
        Mesh::IndexType bcol =
            edofs[0] + info.doffset;  // all of these are local
        FEM::AssembleIJMN(brow, bcol, dofdata, k, datind, dskip, nrow, nedofs);
      }
    }
  }
  if (nedofs > 0) {
    Mesh::IndexType nrow = nedofs;
    //    Mesh::IndexType nrow = edofs.Nelem();
    //    Mesh::IndexType brow = edofs[datind]+info.doffset;
    //    Mesh::IndexType brow = edofs[0]+info.doffset;
    // No, don't use global numbering for K rows, which are
    // numbered locally.
    Mesh::IndexType brow = edofs[0];
    std::vector<Mesh::IndexType>::iterator ieni = con.begin();
    Mesh::IndexType datind = 0;
    while (ieni != con.end()) {
      Mesh::IndexType inode_id = *ieni++;
      Mesh::IndexType ncol = NodalDofs.Esize(inode_id);
      if (ncol > 0) {
        Mesh::IndexType bcol = NodalDofs[inode_id - 1][0] + info.doffset;
        nsearches++;
        //      Mesh::IndexType dskip = endof;
        FEM::AssembleIJMN(brow, bcol, dofdata, k, datind, dskip, nrow, ncol);
        datind += ncol;
      }
    }
    nsearches++;
    //    Mesh::IndexType bcol = edofs[datind]+info.doffset; // all of these are
    //    local
    Mesh::IndexType bcol = edofs[0] + info.doffset;  // all of these are local
    FEM::AssembleIJMN(brow, bcol, dofdata, k, datind, dskip, nrow, nedofs);
  }
  return (nsearches);
}

Mesh::IndexType AssembleBorderElementII(
    std::vector<Mesh::IndexType> &con, std::vector<Mesh::IndexType> &edofs,
    Mesh::IndexType &endof,   // number of dofs to assemble
    Mesh::IndexType &nedofs,  // ndofs on element only (not nodes)
    Mesh::Connectivity &NodalDofs, Mesh::Connectivity &RemoteNodalDofs,
    FEM::DummyStiffness<double, Mesh::IndexType, Mesh::Connectivity,
                        std::vector<Mesh::IndexType> > &k,
    std::vector<Mesh::Border> &borders,
    std::vector<std::pair<unsigned int, unsigned int> > &bnode_map,
    std::vector<double> &dofdata, Mesh::PartInfo &info, std::ostream *Out,
    bool verbose) {
  //  double dummy = 1.0;
  Mesh::IndexType nsearches = 0;
  Mesh::IndexType datind = 0;
  //  double *dofdata = &dummy;
  // Dskip is hardcoded for symbolic assembly
  Mesh::IndexType dskip = 0;
  // commented for symbolic assembly
  //      std::vector<double> dofdata(endof,1.0);
  // for every dof on this element, write it's row
  std::vector<Mesh::IndexType>::iterator eni = con.begin();
  while (eni != con.end()) {
    datind = 0;
    Mesh::IndexType color = 0;
    Mesh::IndexType nrow, brow;
    Mesh::IndexType node_id = *eni++;
    if (node_id > info.nlocal) {
      Mesh::IndexType rnid = node_id - info.nlocal;
      nrow = RemoteNodalDofs.Esize(rnid);
      brow = bnode_map[rnid - 1].second;
      color = bnode_map[rnid - 1].first;
      // RemoteNodalDofs[rnid-1][0];
    } else {
      nrow = NodalDofs.Esize(node_id);
      brow = NodalDofs[node_id - 1][0];
    }
    if (nrow > 0) {
      std::vector<Mesh::IndexType>::iterator ieni = con.begin();
      while (ieni != con.end()) {
        Mesh::IndexType bcol, ncol;
        Mesh::IndexType inode_id = *ieni++;
        if (inode_id > info.nlocal) {
          Mesh::IndexType rnid = inode_id - info.nlocal;
          bcol = RemoteNodalDofs[rnid - 1][0];
          ncol = RemoteNodalDofs.Esize(rnid);
        } else {
          bcol = NodalDofs[inode_id - 1][0] + info.doffset;
          ncol = NodalDofs.Esize(inode_id);
        }
        nsearches++;
        // dskip needs to to be the total number of dofs for the element.
        //      Mesh::IndexType dskip = endof;
        if (node_id <= info.nlocal) {
          FEM::AssembleIJMN(brow, bcol, dofdata, k, datind, dskip, nrow, ncol);
        } else {
          AssembleToCommBuffer(brow, bcol, dofdata, borders[color - 1].data,
                               datind, dskip, nrow, ncol);
        }
        //      datind+=ncol;
      }
      if (nedofs > 0) {
        nsearches++;
        //      Mesh::IndexType bcol = edofs[datind]+info.doffset; // all of
        //      these are local
        Mesh::IndexType bcol =
            edofs[0] + info.doffset;  // all of these are local
        if (node_id <= info.nlocal) {
          FEM::AssembleIJMN(brow, bcol, dofdata, k, datind, dskip, nrow,
                            nedofs);
        } else {
          AssembleToCommBuffer(brow, bcol, dofdata, borders[color - 1].data,
                               datind, dskip, nrow, nedofs);
        }
      }
    }
  }
  if (nedofs > 0) {
    Mesh::IndexType nrow = nedofs;
    // Nope, don't use the global numbering for row nums!
    //    Mesh::IndexType brow = edofs[0]+info.doffset;
    Mesh::IndexType brow = edofs[0];
    std::vector<Mesh::IndexType>::iterator ieni = con.begin();
    Mesh::IndexType datind = 0;
    while (ieni != con.end()) {
      Mesh::IndexType bcol = 0;
      Mesh::IndexType ncol = 0;
      Mesh::IndexType inode_id = *ieni++;
      if (inode_id > info.nlocal) {
        ncol = RemoteNodalDofs.Esize(inode_id - info.nlocal);
        if (ncol > 0) bcol = RemoteNodalDofs[inode_id - info.nlocal - 1][0];
      } else {
        ncol = NodalDofs.Esize(inode_id);
        if (ncol > 0) bcol = NodalDofs[inode_id - 1][0] + info.doffset;
      }
      nsearches++;
      //      Mesh::IndexType dskip = endof;
      if (ncol > 0) {
        FEM::AssembleIJMN(brow, bcol, dofdata, k, datind, dskip, nrow, ncol);
      }
      // should we skip update of datind for symbolic?
      //      datind+=ncol;
    }
    nsearches++;
    //    Mesh::IndexType bcol = edofs[datind]+info.doffset; // all of these are
    //    local
    Mesh::IndexType bcol = edofs[0] + info.doffset;  // all of these are local
    FEM::AssembleIJMN(brow, bcol, dofdata, k, datind, dskip, nrow, nedofs);
  }
  return (nsearches);
}

void AssembleToCommBuffer(Mesh::IndexType &i, Mesh::IndexType &j,
                          std::vector<double> &dofdat, Mesh::BorderData &data,
                          Mesh::IndexType datind, unsigned int dskip,
                          unsigned int N, unsigned int M) {
  std::vector<Mesh::IndexType>::iterator iIt = data.SendAi.begin();
  std::vector<Mesh::IndexType>::iterator iEnd = data.SendAi.begin();
  iIt += data.SendAp[i - 1];
  iEnd += data.SendAp[i];
  //  std::vector<Mesh::IndexType>::iterator iFind =
  //  std::lower_bound(iIt,iEnd,j); assert(iFind != iEnd); assert(*iFind == j);
  Mesh::IndexType index =
      std::lower_bound(iIt, iEnd, j) - iIt + data.SendAp[i - 1];
  //  assert(index >= 0);
  for (unsigned int l = 0; l < N; l++) {
    double *dest = &(data.SendBuffer[index]);
    memcpy(&dofdat[datind], dest, M * sizeof(double));
    index += data.SendAp[i];
    datind += dskip;
  }
}

int RecvBufAssembly(
    Mesh::Border &border, Mesh::Connectivity &NodalDofs,
    FEM::DummyStiffness<double, Mesh::IndexType, Mesh::Connectivity,
                        std::vector<Mesh::IndexType> > &k,
    std::vector<double> &dofdat) {
  Mesh::IndexType nsearch = 0;
  std::vector<Mesh::IndexType>::iterator rni = border.nrecv.begin();
  Mesh::IndexType bnodeid = 0;
  while (rni != border.nrecv.end()) {
    bnodeid++;
    Mesh::IndexType node_id = *rni++;
    Mesh::IndexType nrow = NodalDofs[node_id - 1].size();
    Mesh::IndexType brow = 0;
    if (nrow > 0) {
      brow = NodalDofs[node_id - 1][0];
      Mesh::IndexType ncol_total =
          border.data.RecvAp[bnodeid] - border.data.RecvAp[bnodeid - 1];
      Mesh::IndexType datind = 0;
      if (ncol_total > 0) {
        Mesh::IndexType curind = border.data.RecvAp[bnodeid - 1];
        std::vector<Mesh::IndexType>::iterator AiIt =
            border.data.RecvAi.begin();
        AiIt += curind;
        while (datind < ncol_total && AiIt != border.data.RecvAi.end()) {
          Mesh::IndexType theindex = 0;
          Mesh::IndexType begindex = *AiIt++;
          Mesh::IndexType bcol = begindex;
          Mesh::IndexType ncol_this_write = 1;
          while (*AiIt == (begindex + 1) && datind < ncol_total) {
            begindex = *AiIt++;
            ncol_this_write++;
            datind++;
          }
          // Commented out for assembly tests
          //		Mesh::IndexType dskip = ncol_total;
          Mesh::IndexType dskip = 0;
          nsearch++;
          //	  FEM::AssembleIJMN(brow,bcol,dofdat,k,curind+theindex,dskip,nrow,ncol_this_write);
          FEM::AssembleIJMN(brow, bcol, dofdat, k, 0, dskip, nrow,
                            ncol_this_write);
          theindex += ncol_this_write;
        }
      }
    }
  }
  return (nsearch);
}

Mesh::IndexType AssembleBorderElement(
    std::vector<Mesh::IndexType> &con, std::vector<Mesh::IndexType> &edofs,
    Mesh::IndexType &endof, Mesh::IndexType &nedofs,
    Mesh::Connectivity &NodalDofs,
    FEM::DummyStiffness<double, Mesh::IndexType, Mesh::Connectivity,
                        std::vector<Mesh::IndexType> > &k,
    std::vector<Mesh::IndexType> &local_dof_to_global,
    Mesh::IndexType &nglobal_dofs, Mesh::IndexType &doffset,
    Mesh::PartInfo &info) {
  double dummy = 1.0;
  Mesh::IndexType nsearches = 0;
  double *dofdata = &dummy;
  // commented for symbolic assembly
  //      std::vector<double> dofdata(endof,1.0);
  // for every dof on this element, write it's row
  for (Mesh::IndexType dofn = 0; dofn < endof; dofn++) {
    //    Mesh::IndexType row = ElementDofs[nnn][dofn];
    Mesh::IndexType row = edofs[dofn];
    if (row > 0) {
      Mesh::IndexType datind = 0;
      std::vector<Mesh::IndexType>::iterator eni = con.begin();
      // write each node's contribution, searching once per node
      // since the dofs on each node are sequential
      while (eni != con.end()) {
        Mesh::IndexType node_id = *eni++;
        // write ndof at a time
        Mesh::IndexType ndof = NodalDofs.Esize(node_id);
        if (ndof > 0) {
          Mesh::IndexType node_index = node_id - 1;
          // get the beginning dof id on this node
          // for the search column
          Mesh::IndexType bdof = NodalDofs[node_index][0];
          if (node_id > info.nlocal)
            bdof = local_dof_to_global[bdof - nglobal_dofs - 1];
          else
            bdof += doffset;
          nsearches++;
          FEM::AssembleIJ(row, bdof, &dofdata[datind], k, ndof);
          datind += ndof;
        }
      }
      //      if(edof_sizes[nnn] > 0){
      if (nedofs > 0) {
        nsearches++;
        Mesh::IndexType col =
            edofs[datind] + doffset;  // all of these are local
        FEM::AssembleIJ(row, col, &dofdata[datind], k, nedofs);
      }
    }
  }
  return (nsearches);
}

std::ostream &operator<<(std::ostream &Ostr, const FEM::DataBuffer &buf) {
  if (!buf._data_ptr) return (Ostr);
  std::ostringstream OStr;
  OStr << buf._number_of_items << " " << buf._item_size << std::endl;
  bool known_data_type = false;
  if (buf._item_size == 1) {
    buf.OutputToStream<char>(OStr);
  } else if (buf._item_size == 2) {
    buf.OutputToStream<short>(OStr);
  } else if (buf._item_size == 4) {
    buf.OutputToStream<int>(OStr);
  } else if (buf._item_size == 8) {
    buf.OutputToStream<double>(OStr);
  } else
    assert(known_data_type);
  Ostr << OStr.str();
  return (Ostr);
}

std::istream &operator>>(std::istream &Istr, FEM::DataBuffer &buf) {
  int in_nitems = 0;
  int in_size = 0;
  std::string line;
  while (line.empty()) std::getline(Istr, line);
  std::istringstream IStr(line);
  IStr >> in_nitems;
  IStr >> in_size;
  int read_size = in_nitems * in_size;
  int expected_size = buf._number_of_items * buf._item_size;
  if (expected_size <= 0) {
    expected_size = read_size;
    buf._number_of_items = in_nitems;
    buf._item_size = in_size;
  }
  bool known_data_type = false;
  if (buf._data_ptr != NULL) {
    if (expected_size != read_size) {
      std::cout << "FEM::ERROR line      = " << line << std::endl
                << "FEM::ERROR in_nitems = " << in_nitems << std::endl
                << "FEM::ERROR in_size   = " << in_size << std::endl
                << "FEM::ERROR read_size = " << read_size << std::endl
                << "FEM::ERROR expected  = " << expected_size << std::endl;
      std::getline(Istr, line);
      std::cout << "FEM::ERROR next_line = " << line << std::endl;
    }
    assert(expected_size == read_size);
  } else
    buf.Allocate(in_nitems, in_size);
  if (buf._item_size == 1)
    buf.ReadFromStream<char>(Istr);
  else if (buf._item_size == 2)
    buf.ReadFromStream<short>(Istr);
  else if (buf._item_size == 4)
    buf.ReadFromStream<int>(Istr);
  else if (buf._item_size == 8)
    buf.ReadFromStream<double>(Istr);
  else
    assert(known_data_type);
  return (Istr);
}
std::istream &operator>>(std::istream &IStr, FieldMetaData &md) {
  //    std::string line;
  //    std::getline(Istr,line);
  //    std::istringstream InStr(line);
  //    InStr >> name >> unit >> loc >> ncomp >> dsize;
  IStr >> md.name >> md.unit >> md.loc >> md.ncomp >> md.dsize;
  return (IStr);
}
std::ostream &operator<<(std::ostream &OStr, const FieldMetaData &md) {
  OStr << md.name << " " << md.unit << " " << md.loc << " " << md.ncomp << " "
       << md.dsize << " ";
  return (OStr);
}

}  // namespace FEM
}  // namespace SolverUtils
