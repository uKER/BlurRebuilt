#ifndef UIPARTICLE_H
#define UIPARTICLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Graphics/UIColourClasses.h"
#include "ui/Graphics/Simple2d.h"

namespace UI
{
typedef U32	TPoolId;

class C_UIParticle
{
public:
	C_UIParticle() : m_PreLife(0), m_Life(0), m_Angle(0), m_AngleInc(0) { }

	// Reset the less commonly used items to default valuse
	void				Reset( )
	{
		m_PreLife			= 0.0f;
		m_Angle				= 0.0f;
		m_AngleInc			= 0.0f;
		m_TotalLife			= 0.0f;
		m_Scale				= 1.0f;
		m_ScaleInc			= 0.0f;
		m_AlwaysReset		= true;
		m_NumTextureStages	= 1;

		m_fClipX1 = m_fClipX2 = m_fClipY1 = m_fClipY2 = 0.0f;

		// Stage 0 always active
		m_Texture[0].m_TextureID = C_TextureHandleIndex(0,0);

		// The rest are inactive by default
		for(U32 i = 1; i < m_MaxTextureStages; i++)
			m_Texture[i].m_TextureID = 0;

		m_RenderState.Init();
	}
	const bool				Update(const F32 time);
	void					Render( );

public:
	static const U8			m_MaxTextureStages = C_Sprite::m_NumTextureStages;

	Maths::HVec3			m_Pos;
	Maths::HVec3			m_Vel;
	Maths::HVec3			m_Acc;
	Maths::HVec3			m_Size;
	F32						m_PreLife;
	F32						m_Life;
	F32						m_TotalLife;
	F32						m_Angle;
	F32						m_AngleInc;
	F32						m_Scale;
	F32						m_ScaleInc;
	bool					m_AlwaysReset;
	C_TextureParam			m_Texture[ m_MaxTextureStages ];

	U8						m_NumTextureStages;
	LayerIndex				m_Layer;
	PanelIndex				m_Panel;
	C_UISpriteColours		m_ColourFrom;
	C_UISpriteColours		m_ColourTo;
	C_RenderState			m_RenderState;
							
	F32						m_fClipX1;
	F32						m_fClipX2;
	F32						m_fClipY1;
	F32						m_fClipY2;
};




class UIParticleMan	
{
public:
	typedef Btl::Vector<C_UIParticle*> TParticleList;

	class C_Pool
	{
	public:
		C_Pool( const TPoolId id, const U32 uReserve = 0 )
			: m_Id( id )
		{
			m_Particles.Reserve( uReserve );
		};

		TPoolId			m_Id;
		TParticleList	m_Particles;
	};
	typedef Btl::Vector<C_Pool>	TPoolList;

public:
	void				Init( const U32 numParticles );
	void				Reset( const bool forceResetAll = false );
	void				Exit();

	void				Update(const F32 fFrameTime );
	void				Render();

	const TPoolId		AddPool();
	void				RemovePool( const TPoolId uId );

	C_UIParticle*		Get( const TPoolId uId );

	const U32			NumParticles() const
						{
							return m_NumParticles;
						}

	const U32			AliveParticles() const
						{
							return m_AliveParticles;
						}

private:
	C_UIParticle*		m_Particles;
	U32					m_NumParticles;
	U32					m_AliveParticles;

	TParticleList		m_DeadParticles;	// The dead pool
	TPoolList			m_PoolList;			// particles owned by emitters.
};

extern UIParticleMan	gUIParticleMan;

};

#endif // UIPARTICLE
