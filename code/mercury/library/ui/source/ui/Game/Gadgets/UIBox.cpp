#include "stdafx.h"

#include "ui/Game/UILua.h"
#include "ui/Game/Gadgets/UIBox.h"
#include "ui/Graphics/Simple2d.h"

// XT Definition
XT_TYPE( UI::UIStaticBox, "UIStaticBox" )
	XT_CALLBACK( UI::UiXt::XtReferenceRecordCB< UI::UIStaticBox > )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_MaxCorner_x, "cornerx" )
	XT_FIELD( m_MaxCorner_y, "cornery" )
	XT_FIELD( m_Corner_u, "corneru" )
	XT_FIELD( m_Corner_v, "cornerv" )
XT_END_TYPE

namespace UI
{

void UIStaticBox::Init( )
{
	m_MaxCorner_x = 32.0f;
	m_MaxCorner_y = 32.0f;
	m_Corner_u = 0.5f;
	m_Corner_v = 0.5f;

	C_GadgetBase::Init();
}

void UIStaticBox::Render()
{
	if(!m_Activated)
		return;

	if(m_TextureParams.count == 0)
		return;

	F32 cornersizex;
	F32 cornersizey;

	cornersizex=m_Size.x/2.f;
	if(cornersizex > m_MaxCorner_x) cornersizex = m_MaxCorner_x;

	cornersizey=m_Size.y/2.f;
	if(cornersizey > m_MaxCorner_y) cornersizey = m_MaxCorner_y;

	const bool add_rotation(m_Rotation.x || m_Rotation.y || m_Rotation.z);
	const bool add_scale(m_Scale.x!=1.0f || m_Scale.y!=1.0f);

	const F32 sizex		= m_Size.x;
	const F32 sizey		= m_Size.y;

	if( m_Size.x == 0.0f || m_Size.y == 0.0f )
		return;

	// Texture 0
	C_TextureParam& t0 = GetTextureParam(0);
	const S32 texture0	= t0.m_TextureID;
	const F32 u0		= t0.m_Position.u;
	const F32 v0		= t0.m_Position.v;
	const F32 sizeu0	= t0.m_Size.u;
	const F32 sizev0	= t0.m_Size.v;

	// Texture 1
	C_TextureParam& t1 = GetTextureParam(1);
	const F32 u1		= t1.m_Position.u;
	const F32 v1		= t1.m_Position.v;
	const F32 sizeu1	= t1.m_Size.u;
	const F32 sizev1	= t1.m_Size.v;

	const F32 pu1		= t1.m_Position.u;
	const F32 pv1		= t1.m_Position.v;
	const F32 su1		= t1.m_Size.u;
	const F32 sv1		= t1.m_Size.v;
	const F32 cu1		= cornersizex / m_Size.x * su1;
	const F32 cv1		= cornersizey / m_Size.y * sv1;

	const bool texture_1_avail	= (m_TextureParams.count >= 2 && m_TextureParams[1].Handle().Active() );
	const C_TextureHandleIndex texture1	= (texture_1_avail) ? m_TextureParams[1].m_TextureID : 0;

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	F32 x = m_Position.x;
	F32 y = m_Position.y;
	F32 z = m_Position.z;

	Maths::HVec3 justify = gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size;

	x += justify.X();
	y += justify.Y();
	z += justify.Z();

	//corners
	//top left
	gSimple2D.AddSprite(m_Layer, Maths::HVec3( x, y, z ), cornersizex, cornersizey, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddTexUVs(0, texture0, u0, v0, m_Corner_u, m_Corner_v);
	if(texture_1_avail)
		gSimple2D.AddTexUVs(1, texture1, pu1, pv1, cu1, cv1);

	//top right
	gSimple2D.AddSprite(m_Layer, Maths::HVec3( x+sizex-cornersizex,y, z ), cornersizex,cornersizey, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddTexUVs(0, texture0, u0+sizeu0-m_Corner_u, v0, m_Corner_u, m_Corner_v);

	if(texture_1_avail)
		gSimple2D.AddTexUVs(1, texture1, (pu1 + su1) - cu1, pv1, cu1, cv1);

	//bottom left
	gSimple2D.AddSprite(m_Layer, Maths::HVec3( x, y+(sizey-cornersizey)*Up(), z ), cornersizex, cornersizey, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddTexUVs(0, texture0, u0, v0+sizev0-m_Corner_v, m_Corner_u, m_Corner_v);
	if(texture_1_avail)
		gSimple2D.AddTexUVs(1, texture1, pu1, (pv1 + sv1) - cv1, cu1, cv1);

	//bottom right
	gSimple2D.AddSprite(m_Layer,Maths::HVec3( x+sizex-cornersizex, y+(sizey-cornersizey)*Up(), z ), cornersizex, cornersizey, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddTexUVs(0, texture0, u0+sizeu0-m_Corner_u, v0+sizev0-m_Corner_v, m_Corner_u, m_Corner_v);
	if(texture_1_avail)
		gSimple2D.AddTexUVs(1, texture1, (pu1 + su1) - cu1, (pv1 + sv1) - cv1, cu1, cv1);

	F32 middlesizex;
	F32 middlesizey;

	middlesizex=sizex-(cornersizex*2.f);
	if(middlesizex < 0) middlesizex=0;

	middlesizey=sizey-(cornersizey*2.f);
	if(middlesizey < 0) middlesizey=0;

	const F32 midU = sizeu0 - (m_Corner_u*2.0f);
	const F32 midV = sizev0 - (m_Corner_v*2.0f);

	//middle
	gSimple2D.AddSprite(m_Layer, Maths::HVec3( x+cornersizex,y+(cornersizey)*Up(), z ), middlesizex,middlesizey, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddTexUVs(0, texture0, u0+m_Corner_u, v0+m_Corner_v, midU, midV);
	if(texture_1_avail)
		gSimple2D.AddTexUVs(1, texture1, pu1 + cu1, pv1 + cv1, su1 - (cu1*2.0f), sv1 - (cv1*2.0f));

	//sides
	//top
	gSimple2D.AddSprite(m_Layer, Maths::HVec3( x+cornersizex, y, z ), middlesizex, cornersizey, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddTexUVs(0, texture0, u0 + m_Corner_u, v0, midU, m_Corner_v);
	if(texture_1_avail)
		gSimple2D.AddTexUVs(1, texture1, pu1 + cu1, pv1, su1 - (cu1*2.0f), cv1);

	//left
	gSimple2D.AddSprite(m_Layer, Maths::HVec3( x,y +(cornersizey)*Up(), z ), cornersizex, middlesizey, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddTexUVs(0, texture0, u0, v0+m_Corner_v, m_Corner_u, midV);
	if(texture_1_avail)
		gSimple2D.AddTexUVs(1, texture1, pu1, pv1 + cv1, cu1, sv1 - (cv1*2.0f));

	//bottom
	gSimple2D.AddSprite(m_Layer, Maths::HVec3( x+cornersizex, y+(sizey-cornersizey)*Up(), z ), middlesizex, cornersizey, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddTexUVs(0, texture0, u0+m_Corner_u, v0+sizev0-m_Corner_v, midU, m_Corner_v);
	if(texture_1_avail)
		gSimple2D.AddTexUVs(1, texture1, pu1 + cu1, pv1 + sv1 - cv1, su1 - (cu1*2.0f), cv1);

	//right
	gSimple2D.AddSprite(m_Layer, Maths::HVec3( x+sizex-cornersizex, y+(cornersizey)*Up(), z ), cornersizex, middlesizey, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddTexUVs(0, texture0, u0+sizeu0-m_Corner_u, v0+m_Corner_v, m_Corner_u, midV);
	if(texture_1_avail)
		gSimple2D.AddTexUVs(1, texture1, pu1 + su1 - cu1, pv1 + cv1, cu1, sv1 - (cv1*2.0f));
}

}; // namespace UI

