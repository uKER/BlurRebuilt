#pragma once

#include "Dynamic/Strip/Effect.h"
#include "Dynamic/Strip/Build/ScreenAlignedStrip.h"

namespace DynamicEffect
{
namespace Strip
{
class C_Emitter;
class C_Manager;

struct BeamParticle
{
	MathsStorage::Float3	vPosition;
	F32						fWidth;
	F32						fLife;
	F32						fLifeScale;
	F32						fTexCoord;
	F32						fPadding2;
	MathsStorage::Float4	qRotation;

	Maths::HVec3			GetPosition() const { return Maths::Load<Maths::HVec3>(&vPosition); }
};

struct BeamStrip
{
	MathsStorage::Float4	vColour;
	F32						fLife;
	bool					bUpdateStrip;
	bool					bUpdateElements;
};

// inherit from base emitter and create own update and initialisation functions
class C_Beam : public I_StripEffect<BeamParticle, BeamStrip, 100>
{
public:
	C_Beam(C_Renderer* pRenderer, C_Manager* pManager, const U32 iIndex);
	~C_Beam();
	
	C_RenderNode*				Spawn(const C_Emitter* pEmitter);

	void					Update();
	bool					Create();
	void					Destroy();

	void					OnRender(const C_View& view, Display::C_Context* pContext, const C_CompositeRenderNode& batch, const U32 states) const;

	F32						m_fCycle;
	Display::C_RefTexture*	m_pTexture;

	ScreenAlignedStrip<C_Strip<BeamParticle, BeamStrip> >	m_Build;
	
	typedef ScreenAlignedStrip<C_Strip<BeamParticle, BeamStrip> >::Vertex		Vertex;
};


}
}
