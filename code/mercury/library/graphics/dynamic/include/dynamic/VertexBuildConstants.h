#ifndef RENDER_DYNAMICEFFECT_VERTEXBUILDCONSTANTS_H
#define RENDER_DYNAMICEFFECT_VERTEXBUILDCONSTANTS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{

class C_TextureCoords;

// these are required for the vertex building
struct VertexBuildConstants
{
	MathsStorage::Float4A16		vForward;
	MathsStorage::Float4A16		vRight;
	MathsStorage::Float4A16		vUp;
	MathsStorage::Float4A16		vPosition;
	MathsStorage::Float44A16	mProj;
	F32							fFrameTime;
	mutable C_TextureCoords*	pTexCoords;
	S32*						pVertexBuildingJobs;
	bool						sort;
};

}

#endif // RENDER_DYNAMICEFFECT_VERTEXBUILDCONSTANTS_H
