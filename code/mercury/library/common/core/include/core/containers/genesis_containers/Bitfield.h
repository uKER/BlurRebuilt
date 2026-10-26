#ifndef GENESIS_BITFIELD_H
#define GENESIS_BITFIELD_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Util
{

// [WALTER: TODO GRAB FORTHCOMING ImmBitfield FROM PHYSICS HELPER,]
// [ Should be applicable to most situations, creating these on the stack with shared memory new[] 
//	is completely evil .. ]
template <int bAssert = 1>
class C_BitfieldTemplate
{    
public:
	// does not intitialise bit values
    C_BitfieldTemplate( const U32 noBits ) 
        :   m_NoBits( noBits ),
            m_Data( ItemCount() )
    {
    }

    C_BitfieldTemplate()
        :   m_NoBits( 0 )
    {
    }

    ~C_BitfieldTemplate()
    {
    }

	void operator = (const C_BitfieldTemplate& other)
	{
		SetNoBits(other.NoBits());
		memcpy( m_Data.Data(), other.m_Data.Data(), DataSize() );			
	}
    
	// does not intitialise bit values
    void SetNoBits( const U32 noBits )
    {
        if(m_NoBits != noBits)
        {
            m_NoBits = noBits;
            m_Data.Capacity( ItemCount() );
        }        
    }
    
    const U32 NoBits() const
    {
        return m_NoBits;
    }
        
	FORCE_INLINE void Set( const U32 bitNo )
	{
		if(bAssert)
		{
			assert( bitNo < m_NoBits );
		}
		m_Data[ ItemIndex( bitNo ) ] |= ( 1 << ItemBit( bitNo ) );
	}
    
	void Clear( const U32 bitNo )
	{
		if(bAssert)
		{
			assert( bitNo < m_NoBits );
		}
		m_Data[ ItemIndex( bitNo ) ] &= ~( 1 << ItemBit( bitNo ) );
	}

	const U32 Test( const U32 bitNo ) const
	{
		if(bAssert)
		{
			assert( bitNo < m_NoBits );
		}
		return ( m_Data[ ItemIndex( bitNo ) ] >> ItemBit( bitNo ) ) & 1;
	}
    
	NOALIAS __forceinline const U32 TestAndSet( const U32 bitNo ) __restrict
	{
		if(bAssert)
		{
			assert( bitNo < m_NoBits );
		}
	    const U32 itemIndex( ItemIndex( bitNo ) );
	    const U32 itemBit( ItemBit( bitNo ) );
	    
	    #if !defined(PLATFORM_PS3)
	    
	    const U32 result( ( m_Data[itemIndex] >> itemBit ) & 1 );
		m_Data[ itemIndex ] |= ( 1 << itemBit );
		
		#else
		
		// PowerPC optimisation: This halves the number of costly micro-coded instructions!
		const S32 itemBitMask( 1 << itemBit );
	    const S32 testBit( m_Data[itemIndex] & itemBitMask );
	    const U32 result( (U32)( testBit | -testBit ) >> 31 );
		m_Data[ itemIndex ] |= itemBitMask;
		
		#endif
		
		return result;
	}
    
	void SetAll()
	{
		memset( m_Data.Data(), 255, DataSize() );
	}
    
	void ClearAll()
	{
        memset( m_Data.Data(), 0, DataSize() );
	}

    const U32 ItemCount() const
    {
        return ( m_NoBits + (BitfieldItemBitCount-1) ) / BitfieldItemBitCount;
    }

    const bool TestItem( const U32 itemIndex )
    {
        if( m_Data[itemIndex] )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    static const U32 ItemBitCount()    
    {
        return BitfieldItemBitCount;
    }
    
    static const U32 ItemStartBit( const U32 itemIndex )    
    {
        return itemIndex / BitfieldItemBitCount;    
    }

	template <typename Archive>
	void Serialise( Archive& ar )
	{
		ar << m_NoBits;
		for (U32 i=0; i<m_Data.Elements(); i++)
		{	
			ar << m_Data[i];
		}
	}

	template <typename Archive>
	void Deserialise( Archive& ar ) 
	{
		ar >> m_NoBits;
		m_Data.Expand((m_NoBits+(BitfieldItemBitCount-1))/BitfieldItemBitCount);
		
		for (U32 i=0; i<m_Data.Elements(); i++)
		{	
			ar >> m_Data[i];
		}

	}

	const C_BitfieldTemplate& operator &= (const C_BitfieldTemplate& other)
	{
		if(bAssert)
		{
			assert(m_Data.Elements() == other.m_Data.Elements());
		}
		//Perform bitwise AND with all bits
		const U32 nItems = m_Data.Elements();
		for (U32 i=0; i<nItems; i++)
		{	
			m_Data[i] &= other.m_Data[i];
		}
		return *this;
	}

	const C_BitfieldTemplate& operator |= (const C_BitfieldTemplate& other)
	{
		if(bAssert)
		{
			assert(m_Data.Elements() == other.m_Data.Elements());
		}
		//Perform bitwise OR with all bits
		const U32 nItems = m_Data.Elements();
		for (U32 i=0; i<nItems; i++)
		{	
			m_Data[i] |= other.m_Data[i];
		}
		return *this;
	}

private:
    typedef U32 C_Item;
        
    static const U32 BitfieldItemBitCount = sizeof( C_Item ) * 8;
    static const U32 BitfieldItemBitMask = BitfieldItemBitCount - 1;
    
    const U32 ItemIndex( const U32 bitIndex ) const
    {
        return bitIndex / BitfieldItemBitCount;
    }
    
    const U32 ItemBit( const U32 bitIndex ) const
    {
        return bitIndex & BitfieldItemBitMask;
    }
        
    const U32 DataSize() const
    {
        return ItemCount() * sizeof( C_Item );
    }
    
	U32                             m_NoBits;
    C_DynamicArray<C_Item>          m_Data;
};

typedef C_BitfieldTemplate<1> C_Bitfield;





}; // namespace Util


#endif
