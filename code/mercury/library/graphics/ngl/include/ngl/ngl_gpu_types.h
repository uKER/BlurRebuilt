#ifndef NGL_GPU_TYPES_H
#define NGL_GPU_TYPES_H

#if NGL_GPU_D3D
#include "ngl/dx/ngl_dx_gpu_types.h"
#endif
#if NGL_GPU_GCM
#include "ngl/ps3/ngl_ps3_gpu_types.h"
#endif
#if NGL_GPU_REV
#error REV
#include "rev/ngl_rev_gpu_types.h"
#endif

#endif
