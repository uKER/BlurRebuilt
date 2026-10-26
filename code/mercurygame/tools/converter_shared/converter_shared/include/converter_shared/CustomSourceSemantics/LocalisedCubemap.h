#ifndef __LOCALISED_CUBEMAP_H__
#define __LOCALISED_CUBEMAP_H__

namespace RenderDataConversion
{
class I_CubemapInform;
}

namespace LocalisedCubemap
{
void Register();

void SetCubemapInform(const RenderDataConversion::I_CubemapInform *p);
}

#endif
