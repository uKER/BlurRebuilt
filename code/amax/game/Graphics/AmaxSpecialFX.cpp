#include "stdafx.h"

#include "PostSpecialFX.h"
#include "SplashCameraSpecialFX.h"


XT_LIBRARY(AmaxSpecialFX)
XT_DEPENDS_ON(SpecialFX)
XT_REGISTER_TYPE(Game::C_PostSpecialFX)
XT_REGISTER_TYPE(Game::C_SplashCameraSpecialFX)
XT_END_LIBRARY