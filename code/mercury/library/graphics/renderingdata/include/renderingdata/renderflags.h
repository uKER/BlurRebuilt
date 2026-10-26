#ifndef RD_PLATFORM_RENDERFLAGS_H
#define RD_PLATFORM_RENDERFLAGS_H
#ifdef _MSC_VER
#pragma once
#endif

namespace RenderingData {
namespace Target
{

// render flags allow us to alter rendering behaviour
struct RenderFlags
{
	RenderFlags() { data = -1; }

    union
    {
        struct
        {
			U32 unused			: 31;
            U32 setPass			: 1;
        };
        U32 data;
    };
};

}}

#endif
