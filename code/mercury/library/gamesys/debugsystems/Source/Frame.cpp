#include "DebugSystemsPch.h"
#include <DebugSystems/Frame.h>
#include "Display/Texture/TextureUtils.h"
#include "Render/debug/ImmediateDebugRender.h"


namespace DebugSystems
{
//Common Frame shared
C_Frame* C_Frame::m_pCommonFrame = NULL;

C_Frame::C_Frame( const C_FrameSkin& frameSkin)
{
	RESOURCE_MEMORY_ID("Frame");

	m_frameSkin = frameSkin;

	m_frameSkin.m_ShadowOffset = Maths::Min(frameSkin.m_ShadowOffset, m_frameSkin.m_BorderSize);

	m_FrameTextureHandle = NULL;
	m_BackgroundTextureHandle = NULL;
	
	//Queue texture loads...
	{
		m_resourcesPending = 1;
		m_FrameTextureHandle.Load( m_frameSkin.m_FrameTexture, this );
		//Load background texture
		if (m_frameSkin.m_BackgroundJustification != E_BJ_None)
		{
			m_frameSkin.m_BackgroundJustification = m_frameSkin.m_BackgroundJustification;
			++m_resourcesPending;
			m_BackgroundTextureHandle.Load( m_frameSkin.m_BackgroundTexture, this );
		}
	}
}

C_Frame::~C_Frame()
{
	m_FrameTextureHandle = NULL;
	m_BackgroundTextureHandle = NULL;
}

void C_Frame::Draw( Maths::HVec2 PCREF pos, Maths::HVec2 PCREF size )
{
	//Moved initialisation here to avoid stalling the loading of the game.
	if (m_resourcesPending > 0)
		return;

	//Grab textures...
	TextureResource::C_TextureResource* frameTextureResource = m_FrameTextureHandle;
	Display::C_Texture2D* FrameTexture = (Display::C_Texture2D*)frameTextureResource->GetDisplayTexture();
	Display::C_Texture2D* BackgroundTexture = NULL;
	if (m_frameSkin.m_BackgroundJustification != E_BJ_None)
	{
		TextureResource::C_TextureResource* backgroundTextureResource = m_BackgroundTextureHandle;
		Display::C_Texture2D* BackgroundTexture = (Display::C_Texture2D*)backgroundTextureResource->GetDisplayTexture();
	}

	Render::Debug::C_ImmediateDebugRender* pImmediateDebugRender = Render::Debug::C_ImmediateDebugRender::GetInstance();
	pImmediateDebugRender->SetDepthEnable(false);

	if (pImmediateDebugRender)
	{
		//Vertex List
		Maths::HVec2 Pos[16];
		Maths::HVec2 Tex[16];

		Pos[0] = Maths::HVec2(pos.X() - m_frameSkin.m_BorderSize, pos.Y() - m_frameSkin.m_BorderSize);
		Pos[1] = Maths::HVec2(pos.X(), pos.Y() - m_frameSkin.m_BorderSize);
		Pos[2] = Maths::HVec2(pos.X()- m_frameSkin.m_BorderSize, pos.Y());
		Pos[3] = Maths::HVec2(pos.X(), pos.Y());
		
		Tex[0] = Maths::HVec2(0.f,0.f);
		Tex[1] = Maths::HVec2(0.25f,0.f);
		Tex[2] = Maths::HVec2(0.f,0.25f);
		Tex[3] = Maths::HVec2(0.25f,0.25f);

		Pos[4] = Maths::HVec2(pos.X() + size.X(), pos.Y()- m_frameSkin.m_BorderSize);
		Pos[5] = Maths::HVec2(pos.X() + size.X() + m_frameSkin.m_BorderSize, pos.Y() - m_frameSkin.m_BorderSize);
		Pos[6] = Maths::HVec2(pos.X() + size.X(), pos.Y());
		Pos[7] = Maths::HVec2(pos.X() + size.X() + m_frameSkin.m_BorderSize, pos.Y());
		
		Tex[4] = Maths::HVec2(0.75f,0.f);
		Tex[5] = Maths::HVec2(1.f,0.f);
		Tex[6] = Maths::HVec2(0.75f,0.25f);
		Tex[7] = Maths::HVec2(1.f,0.25f);

		Pos[8] = Maths::HVec2(pos.X() - m_frameSkin.m_BorderSize, pos.Y() + size.Y());
		Pos[9] = Maths::HVec2(pos.X(), pos.Y() + size.Y());
		Pos[10] = Maths::HVec2(pos.X()- m_frameSkin.m_BorderSize, pos.Y() + size.Y() + m_frameSkin.m_BorderSize);
		Pos[11] = Maths::HVec2(pos.X(), pos.Y() + size.Y() + m_frameSkin.m_BorderSize);
		
		Tex[8] = Maths::HVec2(0.f,0.75f);
		Tex[9] = Maths::HVec2(0.25f,0.75f);
		Tex[10] = Maths::HVec2(0.f,1.f);
		Tex[11] = Maths::HVec2(0.25f,1.f);

		Pos[12] = Maths::HVec2(pos.X() + size.X(), pos.Y() + size.Y());
		Pos[13] = Maths::HVec2(pos.X() + size.X() + m_frameSkin.m_BorderSize, pos.Y() + size.Y());
		Pos[14] = Maths::HVec2(pos.X() + size.X(), pos.Y() + size.Y() + m_frameSkin.m_BorderSize);
		Pos[15] = Maths::HVec2(pos.X() + size.X() + m_frameSkin.m_BorderSize, pos.Y() + size.Y() + m_frameSkin.m_BorderSize);
		
		Tex[12] = Maths::HVec2(0.75f,0.75f);
		Tex[13] = Maths::HVec2(1.f,0.75f);
		Tex[14] = Maths::HVec2(0.75f,1.f);
		Tex[15] = Maths::HVec2(1.f,1.f);

		//Fill Vertex Buffer for the frame
		Maths::HVec2 ARenderPos[2 * 9];
		Maths::HVec2 ARenderTex[2 * 9];

		Maths::HVec2 BRenderPos[2 * 9];
		Maths::HVec2 BRenderTex[2 * 9];

		Maths::HVec2 CRenderPos[2 * 9];
		Maths::HVec2 CRenderTex[2 * 9];

		Maths::RGBA RenderColour [2 * 9];

		
		//Draw Shadow
		if (m_frameSkin.m_ShadowOffset > 0.f)
		{
			Maths::RGBA shadowColour = Maths::RGBA(U8(0), U8(0), U8(0), U8(m_frameSkin.m_ShadowTranslucency * 255.f));
			Maths::HVec2 offset(m_frameSkin.m_ShadowOffset, m_frameSkin.m_ShadowOffset);
			
			//Region 0
			ARenderPos[0] = Pos[0] + offset;
			BRenderPos[0] = Pos[1] + offset;
			CRenderPos[0] = Pos[2] + offset;
			ARenderTex[0] = Tex[0];
			BRenderTex[0] = Tex[1];
			CRenderTex[0] = Tex[2];
			RenderColour[0] = shadowColour;

			ARenderPos[1] = Pos[2] + offset;
			BRenderPos[1] = Pos[1] + offset;
			CRenderPos[1] = Pos[3] + offset;
			ARenderTex[1] = Tex[2];
			BRenderTex[1] = Tex[1];
			CRenderTex[1] = Tex[3];
			RenderColour[1] = shadowColour;

			//Region 1
			ARenderPos[2] = Pos[1] + offset;
			BRenderPos[2] = Pos[4] + offset;
			CRenderPos[2] = Pos[3] + offset;
			ARenderTex[2] = Tex[1];
			BRenderTex[2] = Tex[4];
			CRenderTex[2] = Tex[3];
			RenderColour[2] = shadowColour;

			ARenderPos[3] = Pos[3] + offset;
			BRenderPos[3] = Pos[4] + offset;
			CRenderPos[3] = Pos[6] + offset;
			ARenderTex[3] = Tex[3];
			BRenderTex[3] = Tex[4];
			CRenderTex[3] = Tex[6];
			RenderColour[3] = shadowColour;

			//Region 2
			ARenderPos[4] = Pos[4] + offset;
			BRenderPos[4] = Pos[5] + offset;
			CRenderPos[4] = Pos[6] + offset;
			ARenderTex[4] = Tex[4];
			BRenderTex[4] = Tex[5];
			CRenderTex[4] = Tex[6];
			RenderColour[4] = shadowColour;

			ARenderPos[5] = Pos[6] + offset;
			BRenderPos[5] = Pos[5] + offset;
			CRenderPos[5] = Pos[7] + offset;
			ARenderTex[5] = Tex[6];
			BRenderTex[5] = Tex[5];
			CRenderTex[5] = Tex[7];
			RenderColour[5] = shadowColour;

			//Region 3
			ARenderPos[6] = Pos[2] + offset;
			BRenderPos[6] = Pos[3] + offset;
			CRenderPos[6] = Pos[8] + offset;
			ARenderTex[6] = Tex[2];
			BRenderTex[6] = Tex[3];
			CRenderTex[6] = Tex[8];
			RenderColour[6] = shadowColour;

			ARenderPos[7] = Pos[8] + offset;
			BRenderPos[7] = Pos[3] + offset;
			CRenderPos[7] = Pos[9] + offset;
			ARenderTex[7] = Tex[8];
			BRenderTex[7] = Tex[3];
			CRenderTex[7] = Tex[9];
			RenderColour[7] = shadowColour;

			//Region 4
			ARenderPos[8] = Pos[3] + offset;
			BRenderPos[8] = Pos[6] + offset;
			CRenderPos[8] = Pos[9] + offset;
			ARenderTex[8] = Tex[3];
			BRenderTex[8] = Tex[6];
			CRenderTex[8] = Tex[9];
			RenderColour[8] = shadowColour;

			ARenderPos[9] = Pos[9] + offset;
			BRenderPos[9] = Pos[6] + offset;
			CRenderPos[9] = Pos[12] + offset;
			ARenderTex[9] = Tex[9];
			BRenderTex[9] = Tex[6];
			CRenderTex[9] = Tex[12];
			RenderColour[9] = shadowColour;

			//Region 5
			ARenderPos[10] = Pos[6] + offset;
			BRenderPos[10] = Pos[7] + offset;
			CRenderPos[10] = Pos[12] + offset;
			ARenderTex[10] = Tex[6];
			BRenderTex[10] = Tex[7];
			CRenderTex[10] = Tex[12];
			RenderColour[10] = shadowColour;

			ARenderPos[11] = Pos[12] + offset;
			BRenderPos[11] = Pos[7] + offset;
			CRenderPos[11] = Pos[13] + offset;
			ARenderTex[11] = Tex[12];
			BRenderTex[11] = Tex[7];
			CRenderTex[11] = Tex[13];
			RenderColour[11] = shadowColour;

			//Region 6
			ARenderPos[12] = Pos[8] + offset;
			BRenderPos[12] = Pos[9] + offset;
			CRenderPos[12] = Pos[10] + offset;
			ARenderTex[12] = Tex[8];
			BRenderTex[12] = Tex[9];
			CRenderTex[12] = Tex[10];
			RenderColour[12] = shadowColour;

			ARenderPos[13] = Pos[10] + offset;
			BRenderPos[13] = Pos[9] + offset;
			CRenderPos[13] = Pos[11] + offset;
			ARenderTex[13] = Tex[10];
			BRenderTex[13] = Tex[9];
			CRenderTex[13] = Tex[11];
			RenderColour[13] = shadowColour;

			//Region 7
			ARenderPos[14] = Pos[9] + offset;
			BRenderPos[14] = Pos[12] + offset;
			CRenderPos[14] = Pos[11] + offset;
			ARenderTex[14] = Tex[9];
			BRenderTex[14] = Tex[12];
			CRenderTex[14] = Tex[11];
			RenderColour[14] = shadowColour;

			ARenderPos[15] = Pos[11] + offset;
			BRenderPos[15] = Pos[12] + offset;
			CRenderPos[15] = Pos[14] + offset;
			ARenderTex[15] = Tex[11];
			BRenderTex[15] = Tex[12];
			CRenderTex[15] = Tex[14];
			RenderColour[15] = shadowColour;

			//Region 8
			ARenderPos[16] = Pos[12] + offset;
			BRenderPos[16] = Pos[13] + offset;
			CRenderPos[16] = Pos[14] + offset;
			ARenderTex[16] = Tex[12];
			BRenderTex[16] = Tex[13];
			CRenderTex[16] = Tex[14];
			RenderColour[16] = shadowColour;

			ARenderPos[17] = Pos[14] + offset;
			BRenderPos[17] = Pos[13] + offset;
			CRenderPos[17] = Pos[15] + offset;
			ARenderTex[17] = Tex[14];
			BRenderTex[17] = Tex[13];
			CRenderTex[17] = Tex[15];
			RenderColour[17] = shadowColour;

			//Draw Shadow
			pImmediateDebugRender->DrawTextureTriangles2D(
				ARenderPos, 
				BRenderPos, 
				CRenderPos, 
				ARenderTex,
				BRenderTex,
				CRenderTex,
				RenderColour, 2 * 9, *FrameTexture);
		
		}

		//Draw frame

		//Region 0
		ARenderPos[0] = Pos[0];
		BRenderPos[0] = Pos[1];
		CRenderPos[0] = Pos[2];
		ARenderTex[0] = Tex[0];
		BRenderTex[0] = Tex[1];
		CRenderTex[0] = Tex[2];
		RenderColour[0] = m_frameSkin.m_Colour;

		ARenderPos[1] = Pos[2];
		BRenderPos[1] = Pos[1];
		CRenderPos[1] = Pos[3];
		ARenderTex[1] = Tex[2];
		BRenderTex[1] = Tex[1];
		CRenderTex[1] = Tex[3];
		RenderColour[1] = m_frameSkin.m_Colour;

		//Region 1
		ARenderPos[2] = Pos[1];
		BRenderPos[2] = Pos[4];
		CRenderPos[2] = Pos[3];
		ARenderTex[2] = Tex[1];
		BRenderTex[2] = Tex[4];
		CRenderTex[2] = Tex[3];
		RenderColour[2] = m_frameSkin.m_Colour;

		ARenderPos[3] = Pos[3];
		BRenderPos[3] = Pos[4];
		CRenderPos[3] = Pos[6];
		ARenderTex[3] = Tex[3];
		BRenderTex[3] = Tex[4];
		CRenderTex[3] = Tex[6];
		RenderColour[3] = m_frameSkin.m_Colour;

		//Region 2
		ARenderPos[4] = Pos[4];
		BRenderPos[4] = Pos[5];
		CRenderPos[4] = Pos[6];
		ARenderTex[4] = Tex[4];
		BRenderTex[4] = Tex[5];
		CRenderTex[4] = Tex[6];
		RenderColour[4] = m_frameSkin.m_Colour;

		ARenderPos[5] = Pos[6];
		BRenderPos[5] = Pos[5];
		CRenderPos[5] = Pos[7];
		ARenderTex[5] = Tex[6];
		BRenderTex[5] = Tex[5];
		CRenderTex[5] = Tex[7];
		RenderColour[5] = m_frameSkin.m_Colour;

		//Region 3
		ARenderPos[6] = Pos[2];
		BRenderPos[6] = Pos[3];
		CRenderPos[6] = Pos[8];
		ARenderTex[6] = Tex[2];
		BRenderTex[6] = Tex[3];
		CRenderTex[6] = Tex[8];
		RenderColour[6] = m_frameSkin.m_Colour;

		ARenderPos[7] = Pos[8];
		BRenderPos[7] = Pos[3];
		CRenderPos[7] = Pos[9];
		ARenderTex[7] = Tex[8];
		BRenderTex[7] = Tex[3];
		CRenderTex[7] = Tex[9];
		RenderColour[7] = m_frameSkin.m_Colour;

		//Region 4
		ARenderPos[8] = Pos[3];
		BRenderPos[8] = Pos[6];
		CRenderPos[8] = Pos[9];
		ARenderTex[8] = Tex[3];
		BRenderTex[8] = Tex[6];
		CRenderTex[8] = Tex[9];
		RenderColour[8] = m_frameSkin.m_Colour;

		ARenderPos[9] = Pos[9];
		BRenderPos[9] = Pos[6];
		CRenderPos[9] = Pos[12];
		ARenderTex[9] = Tex[9];
		BRenderTex[9] = Tex[6];
		CRenderTex[9] = Tex[12];
		RenderColour[9] = m_frameSkin.m_Colour;

		//Region 5
		ARenderPos[10] = Pos[6];
		BRenderPos[10] = Pos[7];
		CRenderPos[10] = Pos[12];
		ARenderTex[10] = Tex[6];
		BRenderTex[10] = Tex[7];
		CRenderTex[10] = Tex[12];
		RenderColour[10] = m_frameSkin.m_Colour;

		ARenderPos[11] = Pos[12];
		BRenderPos[11] = Pos[7];
		CRenderPos[11] = Pos[13];
		ARenderTex[11] = Tex[12];
		BRenderTex[11] = Tex[7];
		CRenderTex[11] = Tex[13];
		RenderColour[11] = m_frameSkin.m_Colour;

		//Region 6
		ARenderPos[12] = Pos[8];
		BRenderPos[12] = Pos[9];
		CRenderPos[12] = Pos[10];
		ARenderTex[12] = Tex[8];
		BRenderTex[12] = Tex[9];
		CRenderTex[12] = Tex[10];
		RenderColour[12] = m_frameSkin.m_Colour;

		ARenderPos[13] = Pos[10];
		BRenderPos[13] = Pos[9];
		CRenderPos[13] = Pos[11];
		ARenderTex[13] = Tex[10];
		BRenderTex[13] = Tex[9];
		CRenderTex[13] = Tex[11];
		RenderColour[13] = m_frameSkin.m_Colour;

		//Region 7
		ARenderPos[14] = Pos[9];
		BRenderPos[14] = Pos[12];
		CRenderPos[14] = Pos[11];
		ARenderTex[14] = Tex[9];
		BRenderTex[14] = Tex[12];
		CRenderTex[14] = Tex[11];
		RenderColour[14] = m_frameSkin.m_Colour;

		ARenderPos[15] = Pos[11];
		BRenderPos[15] = Pos[12];
		CRenderPos[15] = Pos[14];
		ARenderTex[15] = Tex[11];
		BRenderTex[15] = Tex[12];
		CRenderTex[15] = Tex[14];
		RenderColour[15] = m_frameSkin.m_Colour;

		//Region 8
		ARenderPos[16] = Pos[12];
		BRenderPos[16] = Pos[13];
		CRenderPos[16] = Pos[14];
		ARenderTex[16] = Tex[12];
		BRenderTex[16] = Tex[13];
		CRenderTex[16] = Tex[14];
		RenderColour[16] = m_frameSkin.m_Colour;

		ARenderPos[17] = Pos[14];
		BRenderPos[17] = Pos[13];
		CRenderPos[17] = Pos[15];
		ARenderTex[17] = Tex[14];
		BRenderTex[17] = Tex[13];
		CRenderTex[17] = Tex[15];
		RenderColour[17] = m_frameSkin.m_Colour;
	
		//Draw
		pImmediateDebugRender->DrawTextureTriangles2D(
			ARenderPos,
			BRenderPos,
			CRenderPos, 
			ARenderTex,
			BRenderTex,
			CRenderTex, 
			RenderColour, 2 * 9, *FrameTexture);

		if (m_frameSkin.m_BackgroundJustification != E_BJ_None)
		{
			if ((m_frameSkin.m_BackgroundTextureSize.X() < size.X()) && (m_frameSkin.m_BackgroundTextureSize.Y() < size.Y()))
			{

				Maths::HVec2 ABackPos[2];
				Maths::HVec2 ABackTex[2];

				Maths::HVec2 BBackPos[2];
				Maths::HVec2 BBackTex[2];

				Maths::HVec2 CBackPos[2];
				Maths::HVec2 CBackTex[2];

				Maths::RGBA BackColour [2];

				Maths::HVec2 ori;

				switch (m_frameSkin.m_BackgroundJustification)
				{
					case E_BJ_TopLeft:
						ori = pos;
						break;
					case E_BJ_TopRight:
						ori = Maths::HVec2(pos.X() + size.X() - m_frameSkin.m_BackgroundTextureSize.X(), pos.Y());
						break;
					case E_BJ_BottomLeft:
						ori = Maths::HVec2(pos.X(), pos.Y() + size.Y() - m_frameSkin.m_BackgroundTextureSize.Y());
						break;
					case E_BJ_BottomRight:
						ori = Maths::HVec2(pos.X() + size.X() - m_frameSkin.m_BackgroundTextureSize.X(), pos.Y() + size.Y() - m_frameSkin.m_BackgroundTextureSize.Y());
						break;
					case E_BJ_Center:
						ori = Maths::HVec2(pos.X() + size.X()/2.f - m_frameSkin.m_BackgroundTextureSize.X()/2.f, pos.Y() + size.Y()/2.f - m_frameSkin.m_BackgroundTextureSize.Y()/2.f);
						break;
				}



				ABackPos[0] = ori;
				BBackPos[0] = ori + Maths::HVec2(m_frameSkin.m_BackgroundTextureSize.X(),0.f);
				CBackPos[0] = ori + Maths::HVec2(0.f,m_frameSkin.m_BackgroundTextureSize.Y());
				
				ABackTex[0] = Maths::HVec2(0.f,0.f);
				BBackTex[0] = Maths::HVec2(1.f,0.f);
				CBackTex[0] = Maths::HVec2(0.f,1.f);

				BackColour[0] = m_frameSkin.m_BackgroundColour;

				ABackPos[1] = ori + Maths::HVec2(0.f, m_frameSkin.m_BackgroundTextureSize.Y());
				BBackPos[1] = ori + Maths::HVec2(m_frameSkin.m_BackgroundTextureSize.X(),0.f);
				CBackPos[1] = ori + Maths::HVec2(m_frameSkin.m_BackgroundTextureSize.X(),m_frameSkin.m_BackgroundTextureSize.Y());

				ABackTex[1] = Maths::HVec2(0.f,1.f);
				BBackTex[1] = Maths::HVec2(1.f,0.f);
				CBackTex[1] = Maths::HVec2(1.f,1.f);

				BackColour[1] = m_frameSkin.m_BackgroundColour;

				//Draw
				pImmediateDebugRender->DrawTextureTriangles2D(ABackPos, BBackPos, CBackPos, ABackTex, BBackTex, CBackTex, BackColour, 2, *BackgroundTexture);

			}
			

		}
	}

	pImmediateDebugRender->SetDepthEnable(true);
}

void C_Frame::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
		case Resource::E_Notify_DataReady:
		{				
			--m_resourcesPending;
		}
		break;
		case Resource::E_Notify_DataError:
		case Resource::E_Notify_NoData:
		break;
	}
}

//Default frame values
void C_FrameSkin::Reset()
{
	m_BorderSize = 10.f;
	m_Colour = Maths::RGBA(255,255,255,255);
	m_FrameTexture = "dontship/textures/debugframe.texture";
	m_BackgroundJustification = DebugSystems::E_BJ_BottomRight;
	m_BackgroundTexture = "dontship/textures/debugback.texture";
	m_BackgroundTextureSize = Maths::HVec2(128.f, 160.f);
	m_BackgroundColour = Maths::RGBA(255,255,255,30);
	m_ShadowOffset = 15.f;
	m_ShadowTranslucency = 0.2f;
}
}


