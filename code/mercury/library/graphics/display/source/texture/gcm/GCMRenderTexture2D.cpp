#include "stdafx.h"
#include "Display/Texture/GCM/GCMRenderTexture2D.h"

namespace Display
{
namespace GCM
{

C_RenderTexture2D::C_RenderTexture2D()
{
}

C_RenderTexture2D::~C_RenderTexture2D()
{
}

void C_RenderTexture2D::Resolve() const
{
	if(m_Flags & E_Usage_ReadWrite)
	{
		// alternate the surface and texture indicies
		++m_SurfaceIndex;
		++m_TextureIndex;
		
		m_SurfaceIndex &= ~(1<<1);
		m_TextureIndex &= ~(1<<1);
	}
}

}; // namespace GCM
}; // namespace Display
