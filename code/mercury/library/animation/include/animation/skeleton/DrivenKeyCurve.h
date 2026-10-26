#ifndef ANIMATION_DRIVENKEYCURVE_H
#define ANIMATION_DRIVENKEYCURVE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{
	class C_DrivenKeyCurve
	{
	public:
	
		void SetRanges( F32 minX, F32 maxX, F32 minY, F32 maxY );
		
		void SetKey( U32 keyIndex, F32 value );
		
		U32 GetNumKeys() const;
		
		F32 Evaluate( F32 x ) const;
		
		template <typename T>
		void Serialise( T& stream );

		template <typename T>
		void Deserialise( T& stream );

	
	private:
		// 40 bytes
		static const U32 kNumKeys = 24;
		F32 m_minX;
		F32 m_maxX;
		F32 m_minY;
		F32 m_maxY;
		U8 m_keys[ kNumKeys ];
	};
	
	inline void C_DrivenKeyCurve::SetRanges( F32 minX, F32 maxX, F32 minY, F32 maxY )
	{
		m_minX = minX;
		m_maxX = maxX;
		m_minY = minY;
		m_maxY = maxY;
	}
	
	inline void C_DrivenKeyCurve::SetKey( U32 keyIndex, F32 value )
	{
		F32 clampedValue = Maths::Clamped( value, m_minY, m_maxY );
		U32 packedValue = static_cast< U32 >( 255.f * ( clampedValue - m_minY ) / ( m_maxY - m_minY ) );
		BIZ_ASSERT( packedValue < 256 );
		m_keys[ keyIndex ] = static_cast< U8 >( packedValue );
	}
	
	inline U32 C_DrivenKeyCurve::GetNumKeys() const
	{
		return kNumKeys;
	}
	
	inline F32 C_DrivenKeyCurve::Evaluate( F32 x ) const
	{
		F32 clampedX = Maths::Clamped( x, m_minX, m_maxX );
		
		F32 fKeyIndex = ( clampedX - m_minX ) * ( GetNumKeys() - 1.f ) / ( m_maxX - m_minX );
				
		const S32 currKeyIndex = static_cast< S32 >( fKeyIndex );
		const S32 nextKeyIndex = Maths::Min( currKeyIndex + 1, static_cast< S32 >( GetNumKeys() ) - 1 );
		
		BIZ_ASSERT( currKeyIndex >= 0 && currKeyIndex < (S32) GetNumKeys() );
		
		const F32 scale = ( m_maxY - m_minY ) / 255.f;
		
		F32 prevValue = m_minY + ( m_keys[ currKeyIndex ] * scale );
		F32 nextValue = m_minY + ( m_keys[ nextKeyIndex ] * scale );
				
		return prevValue + ( nextValue - prevValue ) * ( fKeyIndex - floorf( fKeyIndex ) );
	}


	template <typename T>
	inline void C_DrivenKeyCurve::Serialise( T& stream )
	{
		stream << m_minX << m_maxX << m_minY << m_maxY;
		for ( U32 i = 0; i < GetNumKeys(); ++i )
		{
			stream << m_keys[ i ];
		}
	}

	template <typename T>
	inline void C_DrivenKeyCurve::Deserialise( T& stream )
	{
		stream >> m_minX >> m_maxX >> m_minY >> m_maxY;
		for ( U32 i = 0; i < GetNumKeys(); ++i )
		{
			stream >> m_keys[ i ];
		}
	}
	
}

#endif
