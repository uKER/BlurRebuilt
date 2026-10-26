#include "stdafx.h"
#include "Display/Render/TextureRender.h"
#include "Display/Render/BasicRender.h"
#include "Display/Render/FontRender.h"

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Display/Shaders/Render/PS3/psTextureRenderRGB.shader"
#include "Display/Shaders/Render/PS3/psTextureRenderA.shader"
#endif
#endif

EXTERN_SHADER(psTextureRenderRGB);
EXTERN_SHADER(psTextureRenderA);

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

Debug::Int 	g_debugDisplayTextureRenderTransparency	("Graphics.Display.Render.TextureRender.Transparency",	220,	Debug::Range(0, 255));
Debug::Int 	g_debugDisplayTextureRenderOffset		("Graphics.Display.Render.TextureRender.Offset",		0);

Debug::Int 	g_debugDisplayTextureRenderPageSize		("Graphics.Display.Render.TextureRender.PageSize",		100,	Debug::Range(0, 200));
Debug::Int 	g_debugDisplayTextureRenderCurrPage		("Graphics.Display.Render.TextureRender.CurrPage",		0,		Debug::Range(0, 20));
Debug::Int 	g_debugDisplayTextureRenderTexIndex		("Graphics.Display.Render.TextureRender.TexIndex",		0,		Debug::Range(0, 100));

Debug::Int 	g_debugDisplayTextureRenderIndex		("Graphics.Display.Render.TextureRender.Index",			0);

Debug::Int 	g_debugDisplayTextureRenderCount		("Graphics.Display.Render.TextureRender.Count",			10, 	Debug::Range(1, 500));
Debug::Bool	g_debugDisplayTextureRenderInfo			("Graphics.Display.Render.TextureRender.Info",			true);
Debug::Bool	g_debugDisplayTextureRenderBefore		("Graphics.Display.Render.TextureRender.Before",		true);
Debug::Bool	g_debugDisplayTextureRenderCenter		("Graphics.Display.Render.TextureRender.Center",		true);
Debug::Bool	g_debugDisplayTextureRenderAfter		("Graphics.Display.Render.TextureRender.After",			true);
Debug::Bool	g_debugDisplayTextureRenderDetails		("Graphics.Display.Render.TextureRender.Details",		true);

enum E_TextureRenderMode
{
	E_TextureRenderMode_Single,
	E_TextureRenderMode_Multiple,
};

DEBUG_ENUM(E_TextureRenderMode)
	DE_OPTION(E_TextureRenderMode_Single,		"Single")
	DE_OPTION(E_TextureRenderMode_Multiple,		"Multiple")
END_DEBUG_ENUM

Debug::Enum<E_TextureRenderMode> g_debugDisplayTextureRenderMode("Graphics.Display.Render.TextureRender.Mode", E_TextureRenderMode_Multiple);

namespace Display
{

C_TextureRender::C_TextureRender() : I_Renderer("TextureRender"), m_Created(false)
{
	assert(g_pTextureRender == NULL);
	g_pTextureRender = this;
}

C_TextureRender::~C_TextureRender()
{
	DestroyRenderer();

	assert(g_pTextureRender == this);
	g_pTextureRender = NULL; 
}

Util::C_String C_TextureRender::GetDetails(const S32 centerTexture, const S32 totalTextures, Display2::C_Texture* pTexture, Display2::C_ResourceInfo* pResourceInfo) const
{
	const Display2::E_ResourceType eResourceType = (Display2::E_ResourceType)pTexture->GetType();
	Util::C_String type = GetTypeName(eResourceType);

	const Display2::E_TextureFormat eTextureFormat = pTexture->GetFormat();
	Util::C_String format = Display2::GetTextureFormatAsString(eTextureFormat);

	U32 address = 0;

	Util::C_String details;

	details.Printf("%i of %i / %.2fmb / %ix%ix%i / %i / 0x%.8x / ",
		centerTexture,
		totalTextures,
		(F32)pTexture->GetSize() / (1024.0f*1024.0f),
		pTexture->GetWidth(),
		pTexture->GetHeight(),
		pTexture->GetDepth(),
		pTexture->GetLevels(),
		address);

	details += type;
	details += " / ";
	details += format;
	details += " / ";
	details += "'";
	details += pResourceInfo ? pResourceInfo->m_Name.Chars() : "Unknown";
	details += "'";

	return details;
}

Util::C_String C_TextureRender::GetTypeName(const Display2::E_ResourceType type) const
{
	switch(type)
	{
	case Display2::E_ResourceType_Texture2D:		return "2D";
	case Display2::E_ResourceType_Texture3D:		return "3D";
	case Display2::E_ResourceType_TextureCube:		return "Cube";
	default:										return "Unknown";
	}
}

Util::C_String C_TextureRender::GetFormatName(const Display2::E_TextureFormat format) const
{
	switch(format)
	{
	case Display2::E_TextureFormat_A8R8G8B8:		return "A8R8G8B8";
	case Display2::E_TextureFormat_DXT1:			return "DXT1";
	case Display2::E_TextureFormat_DXT3:			return "DXT3";
	case Display2::E_TextureFormat_DXT5:			return "DXT5";
	default:										return "Unknown";
	}
}

bool C_TextureRender::OnCreate()
{
	assert(!m_Created);

	RESOURCE_MEMORY_ID("BasicRender");

#if defined(DEVICE_PCD3D11) || defined(DEVICE_PS3GCM) // [Blur1PCDX11] embedded built-ins
	if(!CreatePixelShaderFromFunc("Shaders/psTextureRenderRGB", 0, &m_PixelShader[E_TextureRenderPixelShader_RGB], REF_SHADER(psTextureRenderRGB)))
		return false;
	if(!CreatePixelShaderFromFunc("Shaders/psTextureRenderA", 0, &m_PixelShader[E_TextureRenderPixelShader_A], REF_SHADER(psTextureRenderA)))
		return false;
#else
	if(!CreatePixelShaderFromFile("Shaders/psTextureRenderRGB", 0, &m_PixelShader[E_TextureRenderPixelShader_RGB]))
		return false;
	if(!CreatePixelShaderFromFile("Shaders/psTextureRenderA", 0, &m_PixelShader[E_TextureRenderPixelShader_A]))
		return false;
#endif
	
	m_Created = true;

	return true;
}

void C_TextureRender::OnDestroy()
{
	Device()->Destroy(&m_PixelShader[E_TextureRenderPixelShader_RGB]);
	Device()->Destroy(&m_PixelShader[E_TextureRenderPixelShader_A]);

	m_Created = false;
}

void C_TextureRender::Draw()
{
#if TRACK_DISPLAY2_RESOURCES

	Display2::Device()->LockMutex();

	S32 centerTexture = (g_debugDisplayTextureRenderPageSize * g_debugDisplayTextureRenderCurrPage) + g_debugDisplayTextureRenderTexIndex;

	S32 totalTextures = 0;

	Display2::C_ResourceListIt it1 = Display2::Device()->GetTextureEnd();

	{
		Display2::C_ResourceListIt it = Display2::Device()->GetTextureBegin();
		
		while(it != Display2::Device()->GetTextureEnd())
		{
			if(totalTextures == centerTexture)
			{
				it1 = it;
			}

			totalTextures++;

			it++;
		}
	}

	if(it1 != Display2::Device()->GetTextureEnd())
	{
		F32 x1 = 30.0f;
		F32 y1 = 30.0f;
		F32 x2 = 1024.0f-30.0f;
		F32 y2 = 768.0f-30.0f;

		F32 x = x1;
		F32 y = y1;

		Display2::C_Texture* pTexture = (Display2::C_Texture*)it1->GetKey();
		Display2::C_ResourceInfo* pResourceInfo = it1->GetValue();

		if(g_debugDisplayTextureRenderDetails)
		{
			Util::C_String details = GetDetails(centerTexture, totalTextures, pTexture, pResourceInfo);
			
			F32 s = 16.0f;
			F32 spacing = FontRender()->GetSpacing(details.Chars(), s);

			BasicRender()->DrawColoured2DRect(x, y - (s/2.0f), spacing, s, Maths::RGBA(0, 0, 0, 255));
			FontRender()->Draw(details.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
			y += 20.f;
		}

		if(g_debugDisplayTextureRenderMode == E_TextureRenderMode_Multiple)
		{
			F32 s = (y2-y1) / (F32)(sqrt((double)g_debugDisplayTextureRenderCount));

#if 0
			if(g_debugDisplayTextureRenderBefore)
			{
				F32 x = ((x1+x2) / 2.0f) - s;
				F32 y = ((y1+y2) / 2.0f);
				
				Display2::C_TextureMap::Iterator it = it1;
				if(it != Display2::Device()->m_Textures.Begin())
				{
					it--;

					while(it != Display2::Device()->m_Textures.Begin())
					{
						Display2::C_Texture* pTexture = it->GetKey();

						if(pTexture->IsCreated())
							BasicRender()->DrawTextured2DRect(x-(s/2.0f), y-(s/2.0f), s, s, WHITE, pTexture, &m_PixelShader[E_TextureRenderPixelShader_RGB]);
						else
							BasicRender()->DrawColoured2DRect(x-(s/2.0f), y-(s/2.0f), s, s, GREY);
						x -= s;

						if(x <= (x1+(s/2.0f)))
						{
							x = x2-(s/2.0f);
							y -= s;
						}

						it--;
					}
				}
			}
#endif

			if(g_debugDisplayTextureRenderAfter)
			{
				F32 x = ((x1+x2) / 2.0f) + s;
				F32 y = ((y1+y2) / 2.0f);

				Display2::C_ResourceListIt it = it1;
				if(it != Display2::Device()->GetTextureEnd())
				{
					it++;

					while(it != Display2::Device()->GetTextureEnd())
					{
						Display2::C_Texture* pTexture = (Display2::C_Texture*)it->GetKey();

						if(pTexture->IsCreated())
							BasicRender()->DrawTextured2DRect(x-(s/2.0f), y-(s/2.0f), s, s, WHITE, pTexture, &m_PixelShader[E_TextureRenderPixelShader_RGB]);
						else
							BasicRender()->DrawColoured2DRect(x-(s/2.0f), y-(s/2.0f), s, s, GREY);
						x += s;

						if(x >= (x2-(s/2.0f)))
						{
							x = x1+(s/2.0f);
							y += s;
						}

						it++;
					}
				}
			}
			
			if(g_debugDisplayTextureRenderCenter)
			{
				F32 x = (x1+x2) / 2.0f;
				F32 y = (y1+y2) / 2.0f;
				F32 b = 2.0f;

				BasicRender()->DrawColoured2DRect(x-(s/2.0f)-b, y-(s/2.0f)-b, s+(b*2.0f), s+(b*2.0f), RED);

				if(pTexture->IsCreated())
					BasicRender()->DrawTextured2DRect(x-(s/2.0f), y-(s/2.0f), s, s, WHITE, pTexture, &m_PixelShader[E_TextureRenderPixelShader_RGB]);
				else
					BasicRender()->DrawColoured2DRect(x-(s/2.0f), y-(s/2.0f), s, s, GREY);
			}
		}
		else
		{
			F32 s = (x2-x1) / 2.0f;

			if(pTexture->IsCreated())
				BasicRender()->DrawTextured2DRect(x, y, s, s, WHITE, pTexture, &m_PixelShader[E_TextureRenderPixelShader_RGB]);
			else
				BasicRender()->DrawColoured2DRect(x, y, s, s, GREY);
			x += s;

			if(pTexture->IsCreated())
				BasicRender()->DrawTextured2DRect(x, y, s, s, WHITE, pTexture, &m_PixelShader[E_TextureRenderPixelShader_A]);
			else
				BasicRender()->DrawColoured2DRect(x, y, s, s, GREY);
		}
	}

	Display2::Device()->UnlockMutex();

#if 0
	if(g_debugDisplayTextureRenderMode == E_TextureRenderMode_Multiple)
	{
		F32 x = 0.f;
		F32 y = 0.f;
		F32 w = 1024.f;
		F32 h = 768.f;

		S32 centerTexture = g_debugDisplayTextureRenderIndex+g_debugDisplayTextureRenderOffset;
		
		C_Texture2D* iCenterTexture = NULL;
		Ut::Vector<C_Texture2D*> iBeforeTextures;
		Ut::Vector<C_Texture2D*> iAfterTextures;

		S32 totalTextures = 0;

		C_Device::SetType<C_Texture2D*>::Iterator it = Device()->m_Texture2Ds.Begin();
		while(it != Device()->m_Texture2Ds.End())
		{
			if(totalTextures < centerTexture)
			{
				iBeforeTextures.PushBack(*it);
			}
			else
			if(totalTextures > centerTexture)
			{
				iAfterTextures.PushBack(*it);
			}
			else
			{
				iCenterTexture = *it;
			}

			totalTextures++;

			it++;
		}

		if(iCenterTexture)
		{
			F32 x1 = x;
			F32 y1 = y;
			F32 x2 = x+w;
			F32 y2 = y+h;
			F32 x3 = (x1+x2)/2.0f;
			F32 y3 = (y1+y2)/2.0f;

			F32 s = h/(F32)(sqrt((double)g_debugDisplayTextureRenderCount));

			if(g_debugDisplayTextureRenderBefore)
			{
				F32 x4 = x3-s;
				F32 y4 = y3;
				for(S32 i=(iBeforeTextures.Size()-1); i>=0; i--)
				{
					BasicRender()->DrawTextured2DRect(x4-(s/2.0f), y4-(s/2.0f), s, s, WHITE, iBeforeTextures[i], &m_PixelShader[E_TextureRenderPixelShader_RGB]);
					x4 -= s;

					if(x4 <= (x1+(s/2.0f)))
					{
						x4 = x2-(s/2.0f);
						y4 -= s;
					}
				}
			}

			if(g_debugDisplayTextureRenderAfter)
			{
				F32 x4 = x3+s;
				F32 y4 = y3;
				for(S32 i=0; i<iAfterTextures.Size(); i++)
				{
					BasicRender()->DrawTextured2DRect(x4-(s/2.0f), y4-(s/2.0f), s, s, WHITE, iAfterTextures[i], &m_PixelShader[E_TextureRenderPixelShader_RGB]);
					x4 += s;

					if(x4 >= (x2-(s/2.0f)))
					{
						x4 = x1+(s/2.0f);
						y4 += s;
					}
				}
			}

			if(g_debugDisplayTextureRenderCenter)
			{
				F32 b = 2.0f;

				BasicRender()->DrawColoured2DRect(
					x3-(s/2.0f)-b,
					y3-(s/2.0f)-b,
					s+(b*2.0f),
					s+(b*2.0f),
					RED);

				BasicRender()->DrawTextured2DRect(
					x3-(s/2.0f),
					y3-(s/2.0f),
					s,
					s,
					WHITE,
					iCenterTexture,
					&m_PixelShader[E_TextureRenderPixelShader_RGB]);
			}			

			if(g_debugDisplayTextureRenderInfo)
			{
				char format[32];

				switch(iCenterTexture->GetFormat())
				{
				case E_Format_A8R8G8B8:
					strcpy_s(format, sizeof(format), "A8R8G8B8");
					break;
				case E_Format_DXT1:
					strcpy_s(format, sizeof(format), "DXT1");
					break;
				case E_Format_DXT3:
					strcpy_s(format, sizeof(format), "DXT3");
					break;
				case E_Format_DXT5:
					strcpy_s(format, sizeof(format), "DXT5");
					break;
				default:
					strcpy_s(format, sizeof(format), "Unknown");
					break;
				}

				U32 address = 0;

				char temp[256];
				
				sprintf_s(temp, sizeof(temp), "%i of %i / %.2fmb / %ix%i / %i  / %i / %s / %s",
					centerTexture,
					totalTextures,
					(F32)iCenterTexture->GetSize() / (1024.f*1024.f),
					iCenterTexture->GetWidth(),
					iCenterTexture->GetHeight(),
					iCenterTexture->GetLevels(),
					address,
					format,
					"TODO: Fix me!");

				F32 x1 = 50.0f;
				F32 y1 = 50.0f;
				F32 s1 = 20.0f;
				F32 spacing = FontRender()->GetSpacing(temp, s1);

				BasicRender()->DrawColoured2DRect(
					x1,
					y1-(s1/2.0f),
					spacing,
					s1,
					Maths::RGBA(0, 0, 0, 255));

				FontRender()->Draw(
					temp,
					x1,
					y1,
					s1,
					s1,
					Maths::RGBA(255, 255, 255, 255));
			}
		}

		iBeforeTextures.Clear();
		iAfterTextures.Clear();
	}
	else
	{
		F32 x = 20.f;
		F32 y = 20.f;
		F32 w = 1024.f - (x * 2);
		F32 h = 728.f - (y * 2);
		BasicRender()->DrawColoured2DRect(x, y, w, h, Maths::RGBA(0, 0, 0, g_debugDisplayTextureRenderTransparency));

		x += 30.f;
		y += 40.f;
		FontRender()->Draw(":: Texture Viewer ::", 50.f, 60.f, 20.f, 20.f, GREEN);

		C_Device::SetType<C_Texture2D*>::Iterator it;
		
		S32 count = 0;
		it = Device()->m_Texture2Ds.Begin();
		while(it != Device()->m_Texture2Ds.End())
		{
			count++;
			it++;
		}

		if(count == 0)
			g_debugDisplayTextureRenderIndex = 0;
		else
		if(g_debugDisplayTextureRenderIndex < 0)
			g_debugDisplayTextureRenderIndex = 0;
		else
		if(g_debugDisplayTextureRenderIndex >= count)
			g_debugDisplayTextureRenderIndex = count - 1;

		U32 totalTextures = 0;
		it = Device()->m_Texture2Ds.Begin();
		while(it != Device()->m_Texture2Ds.End())
		{
			C_Texture2D* pTexture = *it;

			if(totalTextures == U32(g_debugDisplayTextureRenderIndex+g_debugDisplayTextureRenderOffset))
			{
				char format[32];
				switch(pTexture->GetFormat())
				{
				case E_Format_A8R8G8B8:
					strcpy_s(format, sizeof(format), "A8R8G8B8");
					break;
				case E_Format_DXT1:
					strcpy_s(format, sizeof(format), "DXT1");
					break;
				case E_Format_DXT3:
					strcpy_s(format, sizeof(format), "DXT3");
					break;
				case E_Format_DXT5:
					strcpy_s(format, sizeof(format), "DXT5");
					break;
				default:
					strcpy_s(format, sizeof(format), "Unknown");
					break;
				}

				char temp[256];
				const F32 size = (F32)pTexture->GetSize() / (1024.f * 1024.f);
				sprintf_s(temp, sizeof(temp), "%i of %i / %.2fmb / %ix%i / %i / %s / %s",
					totalTextures + 1,
					count,
					size,
					pTexture->GetWidth(),
					pTexture->GetHeight(),
					pTexture->GetLevels(),
					format,
					"TODO: Fix me!");
				
				x += 10.f;
				y += 50.f;
				FontRender()->Draw(temp, x, y, 15.f, 15.f, Maths::RGBA(255, 255, 255, 255));

				y += 20.f;
				F32 s = ((w - (x * 2.f)) / 2.f);
				BasicRender()->DrawTextured2DRect(x, y, s, s, WHITE, pTexture, &m_PixelShader[E_TextureRenderPixelShader_RGB]);

				x += s;
				BasicRender()->DrawTextured2DRect(x, y, s, s, WHITE, pTexture, &m_PixelShader[E_TextureRenderPixelShader_A]);

				break;
			}
			totalTextures++;
			it++;
		}
	}
#endif

#endif
}

C_TextureRender* g_pTextureRender = NULL;

void TextureRender(C_TextureRender* pTextureRender)
{
	g_pTextureRender = pTextureRender;
}

}; // namespace Display
