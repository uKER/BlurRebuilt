#ifndef CONTAINERS_H
#define CONTAINERS_H

#ifdef _MSC_VER
#pragma once
#endif

/*****************************************************************************/
// Name: Containers.h - includes all headers necessary for core tech containers
//
// Author: Ian Wilson
//
// Description: This file includes all files necessary for using the core tech
// containers library. This comprises templated containers which are analogous 
// to the std:: namespace, such as std::vector<>, std::list<>, std::map<>, 
// std::set<> and their multi equivalents. There are also string and string 
// table classes and compressed and un-compressed bitfields. 
//
/*****************************************************************************/

#include "containers/vector.h"
#include "containers/list.h"
#include "containers/sortedbinarytree.h"
#include "containers/bitvector.h"
#include "containers/rlebitvector.h"
#include "containers/string.h"
#include "containers/fixedstring.h"
#include "containers/openhashtable.h"
#include "containers/containerserialisation.h"




#endif
