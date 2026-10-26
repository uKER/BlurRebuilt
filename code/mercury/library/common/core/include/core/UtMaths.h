#ifndef UTMATHS_H
#define UTMATHS_H

#ifdef _MSC_VER
#pragma once
#endif

/*****************************************************************************/
// Name: UtMaths.h - Includes files necessary to use UtMaths lib 
//
// Author: Ian Wilson
//
// Description: This file is simply a meta include to bring together the
// necessary files to use the UtMaths lib. UtMaths is a templated maths library
// allowing vectors and matrices of arbitrary type and dimension. e.g. Use this 
// if you want to add 5D vectors together, or use other types than F32. Of 
// course this can be used as a 4*F32 vector solution, but there is a more 
// traditional library (HVec) which is hardware optimised for multiple 
// platforms.
/*****************************************************************************/

#include "core/core.h"
#include "maths/MathsUtil.h"
#include "utility/utility.h"
#include "maths/utmaths/operator.h"
#include "maths/utmaths/recurse.h"
#include "maths/utmaths/mvector.h"
#include "maths/utmaths/mmatrix.h"
#include "maths/utmaths/maths.h"

#endif //UTMATHS_H
