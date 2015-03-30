//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#ifndef __SURF_BASIC_H_
#define __SURF_BASIC_H_

#define SURF_BEGIN_NAMESPACE  namespace SURF {
#define SURF_END_NAMESPACE    }
#define USE_SURF_NAMESPACE    using namespace SURF;

#include "mapbasic.h"
#include <cstdlib>
#include "Element_accessors.hpp"

SURF_BEGIN_NAMESPACE

using COM::Element_node_enumerator;
using COM::Element_node_enumerator_str_2;
using COM::Element_node_enumerator_uns;
using COM::Facet_node_enumerator;
using COM::Element_vectors_k_const;
using COM::Element_vectors_k;
using COM::Element_node_vectors_k_const;
using COM::Element_node_vectors_k;

using MAP::Origin;
using MAP::Null_vector;
using MAP::Vector_3;
using MAP::Point_3;
using MAP::Vector_2;
using MAP::Point_2;

typedef double Real;

// Modes of element_to_nodes.
enum { E2N_USER=0, E2N_ONE=1, E2N_AREA=2, E2N_ANGLE=3, E2N_SPHERE=4};

SURF_END_NAMESPACE

#endif



