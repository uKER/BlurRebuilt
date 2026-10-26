#include "stdafx.h"

#include "ui/Graphics/UIParticle.h"
#include "ui/Graphics/Simple2d.h"
#include "ui/Graphics/Font/Font.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

UIParticleMan	gUIParticleMan;

// Particle item
const bool C_UIParticle::Update(const F32 time)
{
	if(m_Life<=0.0f)
		return false;

	// Stall any actions unitl PreLife is 0.0
	if(m_PreLife>0.0f)
	{
		m_PreLife -= time;
		if(m_PreLife>0.0f)
			return true; // Still alive but no action taken
	}

	m_Pos = m_Pos + (m_Vel * time);
	m_Vel = m_Vel + (m_Acc * time);

	if(m_AngleInc)
		m_Angle += (m_AngleInc * time);

	if(m_ScaleInc)
	{
		m_Scale += (m_ScaleInc * time);

		if(m_Scale<0)
			m_Scale = 0;
	}
	
	m_Life -= time;

	return true;
}

void C_UIParticle::Render()
{
	if(m_Life<=0.0f)
		return;

//	C_UIClipWindow clipW = gSimple2D.AddClipWindowAuto( m_Layer, m_fClipX1, m_fClipY1, m_fClipX2, m_fClipY2 );

	C_UISpriteColours colour;

	if(m_TotalLife)
	{
		const F32 lerpVal = Maths::Clamped(m_Life/m_TotalLife,0.0f,1.0f);
		colour.Lerp(m_ColourTo,m_ColourFrom,lerpVal);
	}
	else
	{
		colour = m_ColourFrom;
	}
	if(m_Scale==1.0f)
	{
		gSimple2D.AddSprite(m_Layer, (m_Angle==0.0f) ? m_Pos : Maths::HVec3( 0.0f, 0.0f, 0.0f ), m_Size.X(),m_Size.Y(), colour, m_Panel, E_SpriteType_Sprite,m_RenderState);
		gSimple2D.AddJustification(E_Justify_MiddleCentre);
	}
	else
	{
		gSimple2D.AddSprite(m_Layer, m_Pos, m_Size.X() * m_Scale, m_Size.Y() * m_Scale, colour, m_Panel, E_SpriteType_Sprite, m_RenderState);
		gSimple2D.AddJustification(E_Justify_MiddleCentre);
	}

	for(U32 i = 0; i < m_NumTextureStages; i++)
		if(m_Texture[i].Handle().Active())
			gSimple2D.AddTexUVs(i,m_Texture[i]);
			

	if(m_Angle)
	{
		gSimple2D.AddRotation(0,0,m_Angle);
		gSimple2D.AddPivot(-m_Pos);
	}
}


/*---------------------------------------------------------------------------------------
	Particle Manager
---------------------------------------------------------------------------------------*/
void UIParticleMan::Init( const U32 numParticles )
{
	m_NumParticles = numParticles;
	m_AliveParticles = 0;
	m_Particles = new C_UIParticle[ m_NumParticles ];

	// It will contain this many at some point.
	m_DeadParticles.Reserve( m_NumParticles );

	// Add all to dead list
	for ( U32 i = 0; i < m_NumParticles; i++ )
		m_DeadParticles.PushBack( &m_Particles[i] );

	Reset();
}

void UIParticleMan::Exit( )
{
	m_DeadParticles.Clear();
	m_PoolList.Clear();

	delete [] m_Particles;
}

void UIParticleMan::Reset( const bool forceResetAll )
{
	C_UIParticle *p;

	p = m_Particles;
	for(U32 i = 0; i < m_NumParticles; i++, p++)
		if(p->m_AlwaysReset || forceResetAll)
			p->m_Life = 0;
}


const TPoolId UIParticleMan::AddPool()
{
	const TPoolId id = m_PoolList.Size();

	m_PoolList.PushBack( C_Pool( id, m_NumParticles ) );

	return id;
}


void UIParticleMan::RemovePool( const TPoolId uId )
{
	for ( S32 i = 0; i < m_PoolList.Size(); i++ )
	{
		if ( m_PoolList[i].m_Id == uId )
		{
			m_PoolList.Erase( m_PoolList.Begin() + i );
			break;
		}
	}
}


C_UIParticle *UIParticleMan::Get( const TPoolId uId )
{
	if ( m_DeadParticles.Size() )
	{
		C_UIParticle* pParticle = m_DeadParticles.Back();
		m_DeadParticles.PopBack();

		// Add particle to the correct pool.
		for ( S32 i = 0; i < m_PoolList.Size(); i++ )
		{
			if ( m_PoolList[i].m_Id == uId )
			{
				m_PoolList[i].m_Particles.PushBack( pParticle );
				break;
			}
		}
		return pParticle;
	}
	return NULL;
}


void UIParticleMan::Update( const F32 fFrameTime )
{
	m_AliveParticles = 0;

	for( S32 emitter = 0; emitter < m_PoolList.Size(); emitter++ )
	{
		TParticleList& particles =  m_PoolList[emitter].m_Particles;

		S32 index = 0;
		while( index < particles.Size() )
		{
			if ( particles[index]->Update( fFrameTime ) )
			{
				m_AliveParticles++;
				index++;
			}
			else
			{
				// Add the dead particle back into the pool.
				m_DeadParticles.PushBack( particles[index] );

				// Copy the back into it and pop it.
				particles[index] = particles[particles.Size()-1];
				particles.PopBack();
			}
		}
	}
}

void UIParticleMan::Render( )
{
	for( S32 emitter = 0; emitter < m_PoolList.Size(); emitter++ )
	{
		TParticleList& particles =  m_PoolList[emitter].m_Particles;

		for ( S32 i = 0; i < particles.Size(); i++ )
			particles[i]->Render();
	}
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
