#ifndef RENDER_DYNAMICEFFECT_PARTICLE_PARTICLE_ENUMS_H
#define RENDER_DYNAMICEFFECT_PARTICLE_PARTICLE_ENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{
namespace Particle
{

enum E_VertexShader
{
	E_VertexShader_Unknown = -1,

	E_VertexShader_UV,		// texcoords float2
	E_VertexShader_UVW,		// texcoords float3
	E_VertexShader_Decal,
	E_VertexShader_NormalMap,
	E_VertexShader_NdotL,
	E_VertexShader_Soft,
	E_VertexShader_PointSprite,
	E_VertexShader_TwoDimensional,
	E_VertexShader_TwoDimensionalRefract,
	
	E_VertexShader_Count
};

enum E_VertexShaderVariation
{
	E_VertexShaderVariation_Default = 0,
	E_VertexShaderVariation_PointLights,
		
	E_VertexShaderVariation_Count
};

enum E_PixelShader
{
	E_PixelShader_Unknown = -1,
	E_PixelShader_Default,
	E_PixelShader_A8,
	E_PixelShader_AlphaAndAdditive,
	E_PixelShader_NormalMap,
	E_PixelShader_Soft,
	E_PixelShader_SoftA8,
	E_PixelShader_Decal,
	E_PixelShader_PointSprite,

	E_PixelShader_Count
};

enum E_PixelShaderSet
{
	E_PixelShaderSet_DepthFog,			// shader uses depth fog
	E_PixelShaderSet_DepthHeightFog,	// shader uses depth and height fog

	E_PixelShaderSet_DepthFog_HDR,		// shader does HDR in the shader
	E_PixelShaderSet_DepthFog_HDRU32,	// shader does HDR in the shader with luminance in alpha

	E_PixelShaderSet_Count
};

enum E_Effect
{
	E_Effect_Standard,
	E_Effect_Super
};

enum E_EffectChild
{
	E_EffectChild_Trail,
	E_EffectChild_Death,
	E_EffectChild_Collision,
	E_EffectChild_CollisionOnce
};

enum E_VertexBuild
{
	E_VertexBuild_Unknown = -1,

	// shape 2d
	E_VertexBuild_Shape2D,

	// shape 3d
	E_VertexBuild_Shape3D,
	
	// sprite rotating
	E_VertexBuild_SpriteRotating,

	E_VertexBuild_Count
};

enum E_PhysicsBehaviour
{
	E_PhysicsBehaviour_Default,
	E_PhysicsBehaviour_SplineFollower
};

enum E_Collision
{
	E_Collision_None,
	E_Collision_Planar,
	E_Collision_Full
};

enum E_ParticleShape
{
	E_ParticleShape_Beam = 1,
	E_ParticleShape_BeamCapped,
	E_ParticleShape_BeamCappedNoLifeScale,
	E_ParticleShape_Sprite,
	E_ParticleShape_SpriteLimitSize,
	E_ParticleShape_SpriteDirectional,
	E_ParticleShape_SpriteRotating,
	E_ParticleShape_SpriteRotatingNormalMap,
	E_ParticleShape_SpriteRotatingLimitSize,
	E_ParticleShape_SpriteRotatingNormalMapLimitSize,
	E_ParticleShape_SpriteRotatingSoft,
	E_ParticleShape_SpriteRotatingSoftLimitSize,
	E_ParticleShape_Decal,
	E_ParticleShape_Quad,
	E_ParticleShape_QuadAngleFade,
	E_ParticleShape_QuadAngleFadeLimitSize,
	E_ParticleShape_Point,
	E_ParticleShape_PointSprite,
	E_ParticleShape_PointSpriteLine,
	E_ParticleShape_Cube,
	E_ParticleShape_Pyramid,
	E_ParticleShape_Mesh,
	E_ParticleShape_None,

	E_ParticleShape_Num
};

enum E_ParticleLimitSize
{
	E_ParticleLimitSize_NoLimit,
	E_ParticleLimitSize_Clamp,
	E_ParticleLimitSize_AlphaFade
};

enum E_ParticleSort
{
	E_ParticleSort_PerBlock = 0,
	E_ParticleSort_PerParticle
};

enum E_EmitterSpawn
{
	E_EmitterSpawn_Normal,
	E_EmitterSpawn_Burst,
	E_EmitterSpawn_Persistent
};

enum E_EmitterSpace
{
	E_EmitterSpace_World,
	E_EmitterSpace_Local
};

enum E_EmitterOrientation
{
	E_EmitterOrientation_Local,
	E_EmitterOrientation_World
};

enum E_EmitterDistribution
{
	E_EmitterDistribution_Point,
	E_EmitterDistribution_Line,
	E_EmitterDistribution_Sphere,
	E_EmitterDistribution_Capsule,
	E_EmitterDistribution_Box,
	E_EmitterDistribution_Mesh,
	E_EmitterDistribution_Spline
};

enum E_EmitterColourMode
{
	E_EmitterColourMode_SingleColour,
	E_EmitterColourMode_LerpColour,
	E_EmitterColourMode_LerpChannels
};

enum E_EmitterUV
{
	E_EmitterUV_DefaultUV,
	E_EmitterUV_FlipU,
	E_EmitterUV_FlipV,
	E_EmitterUV_FlipUV,
	E_EmitterUV_Random
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_PARTICLE_ENUMS_H
