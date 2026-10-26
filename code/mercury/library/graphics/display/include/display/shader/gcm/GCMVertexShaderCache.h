#ifndef DISPLAY_SHADER_GCM_GCM_VERTEX_SHADER_CACHE_H
#define DISPLAY_SHADER_GCM_GCM_VERTEX_SHADER_CACHE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"
#include "Display/Shader/GCM/GCMVertexShader.h"

namespace Display
{

namespace GCM
{

struct C_VertexShaderCommandBlock
{
	U32						m_StartSlot;
	U32						m_EndSlot;
	const C_VertexShader *	m_pVertexShader;
};

typedef Ut::Vector<C_VertexShaderCommandBlock>	C_VertexShaderCommandBlockArray;

class C_VertexShaderCache
{
public:

	C_VertexShaderCache();
	~C_VertexShaderCache();

	U32		Allocate(const C_VertexShader * pVertexShader);
	void	Free(const C_VertexShader * pVertexShader);
	void	Clear();

private:

	S32		FindBlockOfSize(const U32 size);

private:

	C_VertexShaderCommandBlockArray	m_Blocks;

};

}; // namespace GCM
}; // namespace Display

#include "Display/Shader/GCM/GCMVertexShaderCache.inl"

#endif // DISPLAY_SHADER_GCM_GCM_VERTEX_SHADER_CACHE_H
