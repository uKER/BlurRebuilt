#ifndef __GRAPHICS_FX_RENDERFLAGS_H__
#define __GRAPHICS_FX_RENDERFLAGS_H__

namespace FX
{

struct RenderFlags
{
	RenderFlags() { data = -1; }

    union
    {
        struct
        {
			U32 unused			: 28;
			U32 setPixelShader	: 1;
			U32 setBlendState	: 1;
			U32 setAlphaToMask	: 1;
            U32 setCullMode		: 1;
        };
        U32 data;
    };
};

} // namespace FX

#endif
