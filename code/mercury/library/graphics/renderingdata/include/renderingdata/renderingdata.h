#ifndef RD_RENDERING_DATA_H
#define RD_RENDERING_DATA_H
#ifdef _MSC_VER
#pragma once
#endif

#include "core/maths.h"
#include "btl/utility/string.h"
#include "btl/utility/autopointer.h"
#include "btl/utility/scopedpointer.h"
#include "btl/utility/pair.h"
#include "btl/containers/vector.h"
#include "btl/containers/bitvector.h"
#include "btl/allocators/stackallocator.h"
#include "misc_shared/archive/archive.h"
#include "system/serialise/i_file.h"

// include everything
#if CFG_CONVERTER
#include "build.h"
#else
#include "target.h"
#include "streaming.h"
#endif

#endif
