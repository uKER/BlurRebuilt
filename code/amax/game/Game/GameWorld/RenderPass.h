#ifndef GAME_WORLD_RENDER_PASS_H
#define GAME_WORLD_RENDER_PASS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

enum E_RenderPass
{
	E_RenderPass_Solid,
	E_RenderPass_Decal,
	E_RenderPass_Punch,
	E_RenderPass_Refract,
	E_RenderPass_Alpha,
	// custom render passes
	E_RenderPass_DynamicObjectSolid,
	E_RenderPass_VehicleSolid,
	E_RenderPass_AnimatedObjectSolid,
	E_RenderPass_AnimatedObjectPunch,
	E_RenderPass_CharacterSolid,
	E_RenderPass_CharacterPunch,
	E_RenderPass_CharacterAlpha,
	E_RenderPass_Sky,
	E_RenderPass_TwoDimensional,
	E_RenderPass_EffectDecal,
	E_RenderPass_Ghost,
	E_RenderPass_VehicleAlpha,
	E_RenderPass_AlphaVehicleFlares,
	E_RenderPass_Shadows,
	E_RenderPass_RefractAlpha,
	E_RenderPass_Racification
};

} // namespace Game

#endif // GAME_WORLD_RENDER_PASS_H
