#include "stdafx.h"
#include "Render/Effects/Perlin/Perlin.h"



EXTERN_SHADER(vsPerlin);
EXTERN_SHADER(psPerlin);
EXTERN_SHADER(psPerlin2);
EXTERN_SHADER(psPerlin4);



#ifndef OBJ_SHADERS
#endif



namespace Perlin
{


Debug::Bool		g_Show("Graphics.Render.Perlin.Show", false);
Debug::Colour	g_Persistence( "Graphics.Render.Perlin.Persistence", 0.5f, 0.5f, 0.5f, 0.5f );
Debug::Float	g_AnimationSpeed( "Graphics.Render.Perlin.AnimationSpeed", 0.15f, Debug::Range(-2.0f, 2.0f) );
Debug::Float	g_AnimationSpeedPerLayer( "Graphics.Render.Perlin.PerLayerAnimationSpeed", 2.0f, Debug::Range(1.0f, 8.0f) );
Debug::Colour	g_ChannelScale( "Graphics.Render.Perlin.ChannelScale", 1.0f, 0.5f, 0.25f, 0.125f );



const U32	kTextureSize		=	128;
const U32	kVolumeTextureSize	=	32;
const U32	kVolumeMipMapLevels	=	5;



float Noise(U32 x, U32 y, U32 z, U32 offset)
{
	U32	n	=	x + y * 57 + z * 149 + offset;
		n	=	(n << 13) ^ n;
	
	return 0.5f + 0.5f * ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}



C_Perlin::C_Perlin() : Display::I_Renderer("Perlin")
{
	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_Position, 0);
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0);
	
	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(vertexFormat);

	m_View.m_Type = Display::E_ViewType_2D;

	m_PixelShaderType = E_PixelShader_Default;
	m_BuildMipMaps = false;
}



C_Perlin::~C_Perlin()
{
	DestroyRenderer();
}



bool C_Perlin::OnCreate()
{
	RESOURCE_MEMORY_ID("Perlin");

	// shaders

	// textures
#ifndef PLATFORM_PS3
	if (!Display::Device()->CreateRenderTexture2D(kTextureSize, kTextureSize, m_BuildMipMaps ? 0 : 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, NULL, &m_RenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_PerlinRT");
	#endif

	if (!Display::Device()->CreateTexture3D(kVolumeTextureSize, kVolumeTextureSize, kVolumeTextureSize, kVolumeMipMapLevels, 0, Display::E_Format_A8R8G8B8, &m_Texture3D))
		return false;

	FillTexture3D(m_Texture3D);
#endif

	// view setup
	m_ViewSetup = Display::C_ViewSetup();
	Display::Device()->SetupView(&m_View, m_ViewSetup, m_RenderTexture.GetSurface().GetWidth(), m_RenderTexture.GetSurface().GetHeight());

	return true;
}



void C_Perlin::OnDestroy()
{
	RESOURCE_MEMORY_ID("Perlin");
#ifndef PLATFORM_PS3
	Display::Device()->DestroyRenderTexture2D(&m_RenderTexture);
#endif
	Display::Device()->DestroyTexture3D(&m_Texture3D);

	Display::Device()->DestroyVertexShader(&m_VertexShader);

	for(S32 i = 0; i < E_PixelShader_Count; ++i)
		Display::Device()->DestroyPixelShader(&m_PixelShader[i]);
}



void C_Perlin::Update(const F32 time)
{
	m_Settings.m_Persistence.x = g_Persistence.Value().r;
	m_Settings.m_Persistence.y = g_Persistence.Value().g;
	m_Settings.m_Persistence.z = g_Persistence.Value().b;
	m_Settings.m_Persistence.w = g_Persistence.Value().a;

	m_Settings.m_Time.x	= time * g_AnimationSpeed.Value();
	m_Settings.m_Time.y	= g_AnimationSpeedPerLayer;

	m_Settings.m_ChannelScale[0].x = g_ChannelScale.Value().r;
	m_Settings.m_ChannelScale[1].x = g_ChannelScale.Value().g;
	m_Settings.m_ChannelScale[2].x = g_ChannelScale.Value().b;
	m_Settings.m_ChannelScale[3].x = g_ChannelScale.Value().a;
}



void C_Perlin::Render()
{
	PERF_EVENT("Perlin");

#ifndef PLATFORM_PS3
	Display::Device()->SetColourSurface(&m_RenderTexture.GetSurface());
	Display::Device()->SetDepthStencilSurface(NULL);
#endif

	Display::Device()->SetView(&m_View);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetVertexShader(&m_VertexShader);
	Display::Device()->SetPixelShader(&m_PixelShader[m_PixelShaderType]);

	Display::Device()->SetPixelShaderConstantF(0, reinterpret_cast<F32 *>(&m_Settings), PERLIN_SETTINGS_REGISTER_COUNT);
	Display::Device()->SetTexture(0, &m_Texture3D);
	Display::Device()->SetTextureAddressUVW(0, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);

	struct C_Vertex
	{
		F32 x, y;
		F32 u, v;
	};

	{
		PERF_EVENT("RenderNoise");

		C_Vertex * pVertex = (C_Vertex *) Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleStrip, 4, sizeof(C_Vertex), m_pVertexDeclaration, true); // This can fail
		if (pVertex)
		{
			pVertex->x = 0.0f;
			pVertex->y = 0.0f;
			pVertex->u = 0.0f;
			pVertex->v = 0.0f;
			pVertex++;

			pVertex->x = 1024.0f;
			pVertex->y = 0.0f;
			pVertex->u = 1.0f;
			pVertex->v = 0.0f;
			pVertex++;

			pVertex->x = 0.0f;
			pVertex->y = 768.0f;
			pVertex->u = 0.0f;
			pVertex->v = 1.0f;
			pVertex++;

			pVertex->x = 1024.0f;
			pVertex->y = 768.0f;
			pVertex->u = 1.0f;
			pVertex->v = 1.0f;
			pVertex++;

			Display::Device()->EndVertices(pVertex);
		}

	}
	
	if(m_BuildMipMaps)
		m_TextureUtils.BuildMipMaps(&m_RenderTexture.GetTexture(), true);

	Display::Device()->SetView(NULL, false);

	Display::Device()->SetColourSurface();
	Display::Device()->SetDepthStencilSurface();
}

void C_Perlin::Show() const
{
	if(g_Show)
	{
		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Quad::DrawFullScreen(&m_RenderTexture.GetTexture());
	}
}

void C_Perlin::FillTexture3D(const Display::C_Texture3D &texture3d)
{


#ifdef PLATFORM_PC
#ifndef GRAPHICS_GL

	IDirect3DVolumeTexture9 *pVolTex	=	reinterpret_cast<IDirect3DVolumeTexture9 *>(texture3d.m_pd3dTexture);
	D3DLOCKED_BOX	box;

	U32		level	=	0;
	U32		size	=	kVolumeTextureSize;

	// while we're bigger than the packed mip-map tail
	while (level < kVolumeMipMapLevels)
	{
		HRESULT hr = pVolTex->LockBox(level, &box, NULL, 0);

		if (SUCCEEDED(hr))
		{
			U32 *	pData		=	reinterpret_cast<U32 *>(box.pBits);
			U32		frequency	=	1 << level;

			for (U32 z = 0; z < size; ++z)
			{
				U32	zOffset	=	z * box.SlicePitch / 4;
				for (U32 y = 0; y < size; ++y)
				{
					U32	yOffset	=	y * box.RowPitch / 4;
					for (U32 x = 0; x < size; ++x)
					{
						U8	red			=	static_cast<U32>(Noise(x * frequency, y * frequency, z * frequency, 65536) * 255);
						U8	green		=	static_cast<U32>(Noise(x * frequency, y * frequency, z * frequency, 232144) * 255);
						U8	blue		=	static_cast<U32>(Noise(x * frequency, y * frequency, z * frequency, 108957) * 255);
						U8	alpha		=	static_cast<U32>(Noise(x * frequency, y * frequency, z * frequency, 598221) * 255);
						U32	colour		=	(alpha << 24) + (red << 16) + (green << 8) + (blue);
						U32	index		=	zOffset + yOffset + x;
						pData[index]	=	colour;
					}
				}
			}

			pVolTex->UnlockBox(level);

		}
		else
		{
			//LOGWARN(g_Log, "Could not lock volume texture box");
		}
		
		level	=	level + 1;
		size	=	size / 2;
	}

#endif
#endif

}



}; // namespace Perlin
