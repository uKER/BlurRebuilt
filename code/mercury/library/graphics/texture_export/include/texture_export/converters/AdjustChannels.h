#ifndef __TEXTURE_EXPORT__RAMP_MIPMAP_ALPHA_H__
#define __TEXTURE_EXPORT__RAMP_MIPMAP_ALPHA_H__

namespace Texutils
{

struct AdjustmentLUT
{
	u8	table[256];
};

ACTRESULT AdjustChannels( ActTools::Surface* aSurface,  const AdjustmentLUT& lut, u32 channelMask );

//darkens 'shadows', brightens 'highlights'
ACTRESULT EnhanceMipContrast( ActTools::Texture* aTexture, u32 channelMask );

}

#endif
