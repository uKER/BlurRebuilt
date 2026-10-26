#include "stdafx.h"
#include <Graphics/PostFxLensFlare.h>

XT_TYPE(Game::PostFxLensFlare, "PostFxLensFlare")
	XT_FIELD(m_Scale, "scale")
	XT_FIELD(m_Colour, "colour")
XT_END_TYPE

XT_TYPE(Game::PostFxLensFlares, "PostFxLensFlares")
	XT_FIELD(m_Flare0, "flare0")
	XT_FIELD(m_Flare1, "flare1")
	XT_FIELD(m_Flare2, "flare2")
	XT_FIELD(m_Flare3, "flare3")
	XT_FIELD(m_Scale, "scale")
	XT_FIELD(m_Colour, "colour")
XT_END_TYPE

namespace Game
{
}; // namespace Game
