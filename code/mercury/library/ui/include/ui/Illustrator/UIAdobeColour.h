#ifndef UI_UIADOBECOLOUR_H
#define UI_UIADOBECOLOUR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace UI
{

/// @addtogroup Illustrator
/// @{

//-------------------------------------------------------------------
// Class : C_AdobeColour
//-------------------------------------------------------------------
/// Colour class
class C_AdobeColour
{
public:
	C_AdobeColour( )
	{

	}

	C_AdobeColour( const F32 r, const F32 g, const F32 b ) : 
		m_Red( r ),
		m_Green( g ),
		m_Blue( b )
	{

	}

public:
	template < class T_Stream >
	void Serialise( T_Stream &stream )
	{
		stream  << m_Red
				<< m_Green
				<< m_Blue;
	}

	template < class T_Stream >
	void Deserialise( T_Stream &stream )
	{
		stream  >> m_Red
				>> m_Green
				>> m_Blue;
	}

public:
	F32		m_Red;
	F32		m_Green;
	F32		m_Blue;
};
//-------------------------------------------------------------------
// Class : C_AdobeGradient
//-------------------------------------------------------------------
/// Gradient class
class C_AdobeGradient
{
public:
	class C_Stop
	{
	public:
		F32				m_Mid;
		F32				m_Ramp;
		C_AdobeColour	m_Colour;

	public:
		template < class T_Stream >
		void Serialise( T_Stream &stream )
		{
			stream  << m_Mid
					<< m_Ramp
					<< m_Colour;
		}

		template < class T_Stream >
		void Deserialise( T_Stream &stream )
		{
			stream  >> m_Mid
					>> m_Ramp
					>> m_Colour;
		}
	};
	
public:
	C_AdobeGradient( );

	C_AdobeGradient( const C_AdobeGradient& copy );

	C_AdobeGradient( const S16 isLinear, const U16 stopCount, const C_Stop* stops ) :
		m_IsLinear( isLinear ),
		m_StopCount( stopCount ),
		m_pStops( new C_Stop[ stopCount ] )
	{
		assert( m_pStops );
		assert( stops );

		memcpy(m_pStops, stops, sizeof(C_Stop) * m_StopCount);
	}

	~C_AdobeGradient( );

	C_AdobeGradient& operator =(const C_AdobeGradient& other)
	{
		if(m_pStops)
		{
			delete [] m_pStops;
		}

		m_StopCount = other.m_StopCount;
		m_IsLinear = other.m_IsLinear;

		if(m_StopCount > 0 && other.m_pStops)
		{
			m_pStops = new C_Stop[ m_StopCount ];
			if(m_pStops)
				memcpy(m_pStops, other.m_pStops, sizeof(C_Stop) * m_StopCount);
		}

		// m_Name = other.m_Name;

		return *this;
	}

	template < class T_Stream >
	void Serialise( T_Stream &stream )
	{
		stream  << m_IsLinear
				<< m_StopCount;

		for(S16 count = 0; count < m_StopCount; count++)
		{
			stream << m_pStops[ count ];
		}
	}

	template < class T_Stream >
	void Deserialise( T_Stream &stream )
	{
		stream  >> m_IsLinear
				>> m_StopCount;
	
		// Read in the gradient stop data
		m_pStops = new C_Stop[ m_StopCount ];
		if(!m_pStops)
		{
//			LOGERROR( g_Log, DBSTR " : Failed to allocate stops");
			return;
		}

		for(S16 count = 0; count < m_StopCount; count++)
		{
			stream >> m_pStops[ count ];
		}
	}

	// Access
	const bool		IsLinear( )	const				{ return m_IsLinear != 0; }
	const S16		StopCount( ) const				{ return m_StopCount; }
	const C_Stop&	Stop( const U32 index) const	{ assert(m_pStops); assert( index < m_StopCount); return m_pStops[ index ]; }

private:
	S16				m_IsLinear;
	U16				m_StopCount;
	C_Stop*			m_pStops;
};


//-------------------------------------------------------------------
// Class : C_AdobeGradientStyle
//-------------------------------------------------------------------
/// Gradient style
class C_AdobeGradientStyle
{
public:
	template < class T_Stream >
	void Serialise( T_Stream &stream )
	{
		stream  << m_DictionaryIndex
				<< m_Angle
				<< m_Px
				<< m_Py
				<< m_Length;
	}

	template < class T_Stream >
	void Deserialise( T_Stream &stream )
	{
		stream  >> m_DictionaryIndex
				>> m_Angle
				>> m_Px
				>> m_Py
				>> m_Length;
	}

public:
	// Dictionary lookup
	S32				m_DictionaryIndex;
	// Rotation
	F32				m_Angle;
	// Position
	F32				m_Px;
	F32				m_Py;
	// Scale
	F32				m_Length;
};


//-------------------------------------------------------------------
// Class : C_AdobeFillStyle
//-------------------------------------------------------------------
/// Fill style
class C_AdobeFillStyle
{
public:
	C_AdobeFillStyle( );

	C_AdobeFillStyle( const C_AdobeColour& colour );

	C_AdobeFillStyle( const C_AdobeGradientStyle& gradient );

	template < class T_Stream >
	void Serialise( T_Stream &stream )
	{
		stream	<< (S32) m_Kind;

		switch(m_Kind)
		{
		case E_FillStyle_Kind_Solid :
			stream << m_SolidColour;
			break;

		case E_FillStyle_Kind_Gradient :
			stream << m_GradientStyle;
			break;

		default:
			break;
		}
	}

	template< class T_Stream >
	void Deserialise( T_Stream &stream ) 
	{
		S32 kind;
		stream	>> kind;

		m_Kind = (E_FillStyle_Kind)kind;

		switch(m_Kind)
		{
		case E_FillStyle_Kind_Solid:
			stream >> m_SolidColour;
			break;

		case E_FillStyle_Kind_Gradient:
			stream >> m_GradientStyle;
			break;

		default:
			break;
		}
	}

public:
	enum E_FillStyle_Kind
	{
		E_FillStyle_Kind_None = 0,
		E_FillStyle_Kind_Solid,
		E_FillStyle_Kind_Gradient,

		E_FillStyle_Kind_Count
	};

	const E_FillStyle_Kind		Kind( ) const { return m_Kind; }

	const void					Kind( const E_FillStyle_Kind kind )	{ m_Kind = kind; }

	const C_AdobeColour&		Colour( ) const						{ assert(m_Kind == E_FillStyle_Kind_Solid); return m_SolidColour; }

	const C_AdobeGradientStyle	GradientStyle( ) const				{ assert(m_Kind == E_FillStyle_Kind_Gradient); return m_GradientStyle; }

private:
	E_FillStyle_Kind		m_Kind;
	C_AdobeGradientStyle	m_GradientStyle;
	C_AdobeColour			m_SolidColour;
};


//-------------------------------------------------------------------
// Class : C_AdobeStrokeStyle
//-------------------------------------------------------------------
/// Stroke style
class C_AdobeStrokeStyle
{
public:
	C_AdobeStrokeStyle( );
	C_AdobeStrokeStyle( const C_AdobeColour& colour, const F32 width );

	template < class T_Stream >
	void Serialise( T_Stream &stream )
	{
		stream	<< m_SolidColour
				<< m_Width;
	}

	template< class T_Stream >
	void Deserialise( T_Stream &stream )
	{
		stream	>> m_SolidColour
				>> m_Width;
	}

	const C_AdobeColour&	Colour( ) const { return m_SolidColour; }

	const F32				Width( ) const	{ return m_Width; }

	const void				Width( const F32 width ){ m_Width = width; }

private:
	C_AdobeColour			m_SolidColour;
	F32						m_Width;
};

//-------------------------------------------------------------------
// Class : C_AdobeBlendStyle
//-------------------------------------------------------------------
/// Blend style
class C_AdobeBlendStyle
{
public:
	// Supported adobe blend modes
	enum E_BlendMode
	{
		E_BlendMode_None = 0,
		E_BlendMode_Normal, 
		E_BlendMode_Multiply,		
	};

	/// Default constructor (no blend)
	C_AdobeBlendStyle( ) : 
		m_Transparency( 255 ),
		m_BlendMode( E_BlendMode_None )
	{

	}

	/// Extended constructor
	C_AdobeBlendStyle( const E_BlendMode blend, const F32 transparency ) :
		m_Transparency( (U8)Maths::Clamped( transparency*255.0f, 0.0f, 255.0f) ),
		m_BlendMode( (U8)blend )
	{
		
	}

	/// Blend mode active?
	const bool	Active( ) const
	{ 
		return( m_BlendMode != E_BlendMode_None );
	}
	/// Get alpha value ( 0 - 255 )
	const U8	Transparency( ) const
	{
		return m_Transparency;
	}
	/// Get the blend render mode
	const E_BlendMode	BlendMode( ) const
	{
		return (E_BlendMode)m_BlendMode;
	}

	template < class T_Stream >
	void Serialise( T_Stream &stream )
	{
		stream	<< m_Transparency
				<< m_BlendMode;
	}

	template< class T_Stream >
	void Deserialise( T_Stream &stream )
	{
		stream	>> m_Transparency
				>> m_BlendMode;
	}

private:
	U8						m_Transparency;
	U8						m_BlendMode;
};

/// @}

} // namespace UI

#endif // UI_UIADOBECOLOUR_H
