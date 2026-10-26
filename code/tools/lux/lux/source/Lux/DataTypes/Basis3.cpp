#include "StdAfx.h"

#include "Lux\Sampler.h"
#include "Lux\LightSample.h"
#include "Lux\DataTypes.h"

namespace LUX
{

namespace DataTypes
{

#define SQRT2 1.4142135623730950488016887242097
#define SQRT3 1.7320508075688772935274463415059
#define SQRT6 2.4494897427831780981972840747059
#define SQRT2OVER3 0.81649658092772603273242802490196

#define LMAPAXIS1 SQRT2OVER3, 0, 1.0/SQRT3
#define LMAPAXIS2 -1.0/SQRT6, 1.0/SQRT2, 1.0/SQRT3
#define LMAPAXIS3 -1.0/SQRT6,-1.0/SQRT2, 1.0/SQRT3

const float3 Basis3::m_vAxis[3] = { float3(LMAPAXIS1), float3(LMAPAXIS2), float3(LMAPAXIS3) };


}

}
