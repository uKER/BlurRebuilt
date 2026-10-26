#ifndef RENDER_DYNAMICEFFECT_TYPEINDEX_H
#define RENDER_DYNAMICEFFECT_TYPEINDEX_H

namespace DynamicEffect
{

enum E_EffectTypeIndex
{
	// particles
	E_EffectTypeIndex_Shape2DCollision = 0,
	E_EffectTypeIndex_Shape2DSimple,
	E_EffectTypeIndex_Shape2DSimpleSuper,
	E_EffectTypeIndex_Shape2DCollisionSuper,
	E_EffectTypeIndex_Shape3DCollision,
	E_EffectTypeIndex_Shape3DCollisionSuper,
	E_EffectTypeIndex_Shape3DSimple,
	E_EffectTypeIndex_Shape3DSimpleSuper,
	E_EffectTypeIndex_Shape3DDecal,
	E_EffectTypeIndex_SpriteRotating,
	E_EffectTypeIndex_SpriteRotatingCollision,
	E_EffectTypeIndex_Shape2DSpline,
	E_EffectTypeIndex_SpriteRotatingSpline,
	E_EffectTypeIndex_SpriteRotatingSuper,
	E_EffectTypeIndex_SpriteRotatingCollisionSuper,

	// cloth
	E_EffectTypeIndex_Cloth,

	// strip
	E_EffectTypeIndex_Strip_Beam,

	// light
	E_EffectTypeIndex_Light2dBeam,
	E_EffectTypeIndex_Light2dCross,
	E_EffectTypeIndex_Light2dQuad,
	E_EffectTypeIndex_Light3dBeam,
	E_EffectTypeIndex_Light3dBeamCapped,
	E_EffectTypeIndex_Light3dDecal,
	E_EffectTypeIndex_Light3dQuad,

	// tracer
	E_EffectTypeIndex_TracerBeamCapped,

	// fog
	E_EffectTypeIndex_FogCuboid,
	E_EffectTypeIndex_FogCylinder,
	E_EffectTypeIndex_FogEllipsoid,

	// tracks
	E_EffectTypeIndex_Tracks,

	E_EffectTypeIndex_Count
};

enum E_ManagerTypeIndex
{
	E_ManagerTypeIndex_Particle = 0,
	E_ManagerTypeIndex_Light,
	E_ManagerTypeIndex_Tracer,
	E_ManagerTypeIndex_Fog,

	E_ManagerTypeIndex_Count
};

};
#endif
