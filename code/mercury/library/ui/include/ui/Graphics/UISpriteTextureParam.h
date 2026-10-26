#ifndef UISPRITETEXTUREPARAM_H
#define UISPRITETEXTUREPARAM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>
#include "ui/Game/UILua.h"

namespace UI
{

/// @addtogroup UI_Types
/// @{

/// Used to be a S16 where if positive it would do a lookup from UITexture manager else lookup an -(n+1) 'alternative' texture, it still works in this fasion except it's been C++'ed to death.
class C_TextureHandleIndex
{
public:
	C_TextureHandleIndex( ) :
		m_Active( 0 )
	{

	}

	C_TextureHandleIndex( const S16 index, const bool alternative ) :
		m_Index( index ),
		m_Alternative( alternative ? 1 : 0 ),
		m_Active( 1 )
	{

	}

	C_TextureHandleIndex( const S32 index )
	{
		this->operator=( index ); // Uwwwe
	}

	/// Assignment operator
	C_TextureHandleIndex& operator =( const S32 index )
	{
		if( index < 0 )
		{
			m_Index			= (U16)-(index + 1);
			m_Alternative	= 1;
		}
		else
		{
			m_Index			= (U16)index;
			m_Alternative	= 0;
		}

		m_Active = 1;

		return *this;
	}

	void		Active( const bool active )				{ m_Active = ( active ) ? 1 : 0; }
	void		Alternative( const bool alternative )	{ m_Alternative = ( alternative ) ? 1 : 0; }
	void		Index( const U16 index )				{ m_Index = index; }
	void		Clear()									{ m_Index = 0; m_Alternative = 0; m_Active = 0; }

	const bool	Active( ) const							{ return( m_Active != 0 ); }
	const bool	Alternative( ) const					{ return( m_Alternative != 0 ); }
	const U16	Index( ) const							{ return m_Index; }

	operator S32( ) const
	{
		if( m_Active == 0 )
			return 0;

		if( m_Alternative == 1 )
		{
			S32 ret = (m_Index + 1);

			return -ret;
		}

		return (S32)m_Index;
	}

private:
	U16		m_Index			: 14;
	U16		m_Alternative	: 1;
	U16		m_Active		: 1;
};

/// @}

/// A quad's non-rotated / scaled texture mapping description
class C_TextureParam
{
public:
	class C_Vector2UV
	{
	public:
		C_Vector2UV( const F32 u = 0.0f, const F32 v = 0.0f ) : u( u ), v( v )
		{

		}

	public:
		F32		u;
		F32		v;
	};

public:
	C_TextureParam( ) : 
		m_TextureID( 0 ),
		m_Position( 0.0f, 0.0f ),
		m_Size( 1.0f, 1.0f ),
		m_Name( XtSz8(NULL) )
	{

	}

	C_TextureParam( const S32 textureID, const F32 u = 0.0f, const F32 v = 0.0f, const F32 sizeu = 1.0f, const F32 sizev = 1.0f )
	{
		Set( textureID, u, v, sizeu, sizev );
	}

	void						Bind( );
	const C_TextureHandleIndex	Handle( ) const	{ return m_TextureID; }

	void						Set( const S32 textureID, const F32 u = 0.0f, const F32 v = 0.0f, const F32 sizeu = 1.0f, const F32 sizev = 1.0f )
								{
									m_TextureID = textureID;
									m_Position.u = u;
									m_Position.v = v;
									m_Size.u = sizeu;
									m_Size.v = sizev;
								}

	static void					SorceryCallback( XtEventType type, XtRecord* pRecord, C_TextureParam* pBase, XtCallbackContext* pContext );

public:
	XtSz8					m_Name;
	C_Vector2UV				m_Position;
	C_Vector2UV				m_Size;
	S32						m_TextureID;

private:
	enum E_CallbackContext
	{
		E_CallbackContext_Bind = 0,
	};
};

}; // namespace UI
#endif // UISPRITETEXTUREPARAM_H
