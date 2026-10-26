#include "stdafx.h"
#include "DropletSection.h"

#include "System/Debug/I_DebugRender.h"
#include "Display/Render/BasicRender.h"

#include "Display/Render/BasicRender.h"

namespace Droplet
{

Debug::Logger &g_Log = Debug::GetLogger("DropletSection");


Debug::Bool g_DropletClear( "Droplet.Clear", false );
Debug::Bool g_DropletGenerateRandom( "Droplet.GenerateRandom", false );
Debug::Float g_DropletRandomScale( "Droplet.RandomScale", 1.0f, Debug::Range(0.0f, 5.0f) );
Debug::Float g_DropletRandomPower( "Droplet.RandomPower", 10.0f, Debug::Range(0.0f, 20.0f) );
Debug::Bool g_DropletSplat( "Droplet.Splat", false );
Debug::Float g_DropletSplatScale( "Droplet.SplatScale", 2.0f, Debug::Range(0.0f, 20.0f) );

C_Simulation::C_Simulation()
{
}

void C_Simulation::Init( const S32 width, const S32 height )
{
	m_CurrBuffer = 0;
	m_Width = width;
	m_Height = height;

	m_Data[0].Resize( width * height );
	m_Data[1].Resize( width * height );

	GenerateRandom();
}

void C_Simulation::Clear()
{
	S32 size = m_Width * m_Height;
	for(S32 i=0; i<size; i++)
	{
		m_Data[m_CurrBuffer][i] = 0.0f;
	}
}

void C_Simulation::Splat()
{
	S32 x = rand() % m_Width;
	S32 y = rand() % m_Height;

	for(S32 i=0; i<10; i++)
	{
		F32 dx = (rand() % 256) / 256.0f - 0.5f;
		F32 dy = (rand() % 256) / 256.0f - 0.5f;
		dx = dx * 3.0f;
		dy = dy * 3.0f;
		m_Data[m_CurrBuffer][GetX(x + (S32)dx) + GetY(y+ (S32)dy) * m_Width] = ((rand() % 256) / 256.0f) * g_DropletSplatScale;
	}
}

void C_Simulation::GenerateRandom()
{
	S32 size = m_Width * m_Height;
	for(S32 i=0; i<size; i++)
	{
		F32 value = (F32)(rand() % 256) / 256.0f;
		m_Data[m_CurrBuffer][i] = pow(value, g_DropletRandomPower) * g_DropletRandomScale;
	}
}

F32 C_Simulation::GetValue( const S32 x, const S32 y ) const
{
	return m_Data[m_CurrBuffer][GetX(x) + GetY(y) * m_Width];
}

Debug::Bool g_DropletBlur( "Droplet.Blur", false );

Debug::Float g_DropletAccelX( "Droplet.AccelX", 0.0f, Debug::Range(-5.0f, 5.0f) );
Debug::Float g_DropletAccelY( "Droplet.AccelY", -0.1f, Debug::Range(-5.0f, 5.0f) );
Debug::Float g_DropletTension( "Droplet.Tension", 1.0f, Debug::Range(0.0f, 10.0f) );
Debug::Float g_DropletTensionPower( "Droplet.TensionPower", 1.0f, Debug::Range(0.0f, 10.0f) );
Debug::Float g_DropletTensionReduction( "Droplet.DropletTensionReduction", 1.0f, Debug::Range(0.0f, 10.0f) );

Debug::Float g_DropletWind( "Droplet.Wind", 0.0f, Debug::Range(0.0f, 10.0f) );

Debug::Float g_DropletCurve( "Droplet.Curve", 1.0f, Debug::Range(0.0f, 10.0f) );

void C_Simulation::Update()
{
	if(g_DropletBlur)
	{
		S32 nextBuffer = (m_CurrBuffer + 1) % 2;

		Ut::Vector<F32>& dst = m_Data[nextBuffer];
		for(S32 y=0; y<m_Height; y++)
		{
			for(S32 x=0; x<m_Width; x++)
			{
				F32 avg = GetValue(x - 1, y) 
						+ GetValue(x + 1, y) 
						+ GetValue(x, y - 1) 
						+ GetValue(x, y + 1) 
						+ GetValue(x,y);
				avg = avg / 5.0f;

				dst[x + y * m_Width] = avg;
			}
		}

		m_CurrBuffer = nextBuffer;
	}


	{
		S32 nextBuffer = (m_CurrBuffer + 1) % 2;

		Ut::Vector<F32>& src = m_Data[m_CurrBuffer];
		Ut::Vector<F32>& dst = m_Data[nextBuffer];

		//Ut::Vector<F32> gradBuf;
		//gradBuf.Resize( m_Width * m_Height );

		//for(S32 y=0; y<m_Height; y++)
		//{
		//	for(S32 x=0; x<m_Width; x++)
		//	{
		//		F32 dx = GetValue(x - 1, y) - GetValue(x + 1, y);
		//		F32 dy = GetValue(x, y - 1) - GetValue(x, y + 1);
		//		F32 grad = sqrt(dx * dx + dy * dy);
		//		gradBuf[ x + y * m_Width ] = grad;
		//	}
		//}
		for(S32 y=0; y<m_Height; y++)
		{
			for(S32 x=0; x<m_Width; x++)
			{
				dst[x + y * m_Width] = 0;
			}
		}

		for(S32 y=0; y<m_Height; y++)
		{
			for(S32 x=0; x<m_Width; x++)
			{
				//F32 avgGrad = gradBuf[ x + y * m_Width ];
				F32 value = GetValue(x,y);

				F32 dx = GetValue(x - 1, y) - GetValue(x + 1, y);
				F32 dy = GetValue(x, y - 1) - GetValue(x, y + 1);
				
				F32 len = dx * dx + dy * dy;
				if( len > 0.0f )
					len = sqrt(len);

				if( len > 0.0f )
				{
					dx = dx / len;
					dy = dy / len;
				}

				if( len > 0)
					len = pow(len, g_DropletTensionPower);
				//len = len * len;
				
				/*
				len = len / g_DropletTensionReduction;
				len = (1.0f * 1.0f - len * len);
				if( len < 0.0f )
					len = 0.0f;
				if( len > 0.0f)
					len = sqrt(len);
				len = len * g_DropletTensionReduction;
*/
				len = len;
				
				dx = dx * len;
				dy = dy * len;
				
				F32 mass = value;

				dx = dx + g_DropletAccelX;
				dy = dy + g_DropletAccelY;

				F32 posX = (((F32)x / (F32)m_Width) - 0.5f) * 2.0f;
				F32 posY = (((F32)y / (F32)m_Height) - 0.5f) * 2.0f;
				dx = dx - g_DropletWind * posX;
				dy = dy - g_DropletWind * posY;	

				if( value > 0 )
				{
					dx = dx * value;
					dy = dy * value;
				}

				dx = dx * g_DropletTension;
				dy = dy * g_DropletTension;

				F32 newX = (F32)x - dx;
				F32 newY = (F32)y - dy;

				S32 dstX = (S32)(newX);
				S32 dstY = (S32)(newY);
				F32 fX = newX - dstX;
				F32 fY = newY - dstY;

				
				dst[GetX(dstX)		+ GetY(dstY) * m_Width]		+= value * (1.0f - fX) * (1.0f - fY);
				dst[GetX(dstX+1)	+ GetY(dstY) * m_Width]		+= value * (fX) * (1.0f - fY);
				dst[GetX(dstX)		+ GetY(dstY+1) * m_Width]	+= value * (1.0f - fX) * (fY);
				dst[GetX(dstX+1)	+ GetY(dstY+1) * m_Width]	+= value * (fX) * (fY);
			}
		}

		m_CurrBuffer = nextBuffer;
	}
}



C_DropletSection::C_DropletSection(const Util::C_String& name)
: C_TestSection(name)
{
	m_Simulation.Init( 128, 128 );	

	Display::Device()->CreateTexture2D(m_Simulation.GetWidth(), m_Simulation.GetHeight(), 1, Display::E_Usage_Managed | Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &m_Texture);
}

C_DropletSection::~C_DropletSection()
{
}

void C_DropletSection::OnUpdate()
{
	m_Simulation.Update();

	if(g_DropletGenerateRandom)
	{
		m_Simulation.GenerateRandom();
		g_DropletGenerateRandom = false;
	}

	if(g_DropletClear)
	{
		m_Simulation.Clear();
		g_DropletClear = false;
	}

	if(g_DropletSplat)
	{
		m_Simulation.Splat();
		g_DropletSplat = false;
	}
}


void C_DropletSection::OnPreRender()
{
	PERF_EVENT( "OnPreRender" );
}

void C_DropletSection::OnPostRender()
{
}

void C_DropletSection::OnRender2DView()
{
	PERF_EVENT( "OnRender2DView" );

	// draw here

	Display::C_LockedRect lockedRect = m_Texture.Lock(0);

	const F32* pSrc = m_Simulation.GetData();
	U32* pDst = (U32*)lockedRect.pBits;

	const S32 width = m_Simulation.GetWidth();
	const S32 height = m_Simulation.GetHeight();

	for(S32 y=0; y<height; y++)
	{
		for(S32 x=0; x<width; x++)
		{
			F32 v = *pSrc;

			if( v > 0 )
				v = pow( v, g_DropletCurve );

			S32 shade = (S32)(v * 255);
			if(shade < 0)
				shade = 0;
			if(shade > 255)
				shade = 255;
			*pDst = shade + (shade << 8) + (shade << 16) + (255 << 24);

			pSrc++;
			pDst++;
		}
	}


	m_Texture.Unlock(0);

	Display::BasicRender()->DrawTextured2DRect( 0.0f, 0.0f, 1024.0f, 768.0f, Maths::RGBA(255, 255, 255, 255), &m_Texture);
}

}