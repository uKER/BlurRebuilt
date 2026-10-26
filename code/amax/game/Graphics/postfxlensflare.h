#ifndef AMAX_GRAPHICS_POSTFXLENSFLARE_H
#define AMAX_GRAPHICS_POSTFXLENSFLARE_H

#include <Xt/XtGame.h>

namespace Game
{

struct PostFxLensFlare
{
	XtVec2	m_Scale;
	XtRgb	m_Colour;
};

struct PostFxLensFlares
{
	PostFxLensFlare				m_Flare0;
	PostFxLensFlare				m_Flare1;
	PostFxLensFlare				m_Flare2;
	PostFxLensFlare				m_Flare3;
	XtVec2						m_Scale;
	XtRgb						m_Colour;
};

}; // namespace Game

#endif // AMAX_GRAPHICS_POSTFXLENSFLARE_H