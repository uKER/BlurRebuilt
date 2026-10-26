#ifndef MOVIE_W32_D3D11_BINK_TEXTURES_H
#define MOVIE_W32_D3D11_BINK_TEXTURES_H

// [Blur1PCDX11] The phase-one Bink SDK is absent. Its existing stub never
// opens a movie, so no decoder-owned GPU textures are created in this build.
#include "Bink.h"
struct BINKTEXTURESET
{
    BINKFRAMEBUFFERS bink_buffers;
};
#endif
