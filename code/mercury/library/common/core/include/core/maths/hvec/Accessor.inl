#ifndef ACCESSOR_INL
#define ACCESSOR_INL

#ifdef _MSC_VER
#pragma once
#endif

#include <core/utility/coredebug.h>

namespace Maths
{
//
// ---------------------------------------------------------------------------------------
// |-- VectorAccessor : Provides indexed read/write access to hardware vector elements --|
// ---------------------------------------------------------------------------------------
//

/// Vector & index Constructor
template < class VectorType >
FORCE_INLINE
VectorAccessor<VectorType>::VectorAccessor( VectorType& vector, const unsigned index ) :
    m_Vector( vector ),
    m_Index( index )
{
    assert(index < VectorType::m_Elements);
}

/// Cast to ElementType
template < class VectorType >
FORCE_INLINE
//VectorAccessor<VectorType>::operator const typename VectorType::m_DataType () const
VectorAccessor<VectorType>::operator const typename VectorType::T_Element () const
{
    switch ( m_Index )
    {
        case 0  : return m_Vector.X();
        case 1  : return m_Vector.Y();
        case 2  : return m_Vector.Z();
        case 3  : return m_Vector.W();
        default : return 0.0f;
    }
}

/// ElementType Assignment
template < class VectorType >
FORCE_INLINE
void VectorAccessor<VectorType>::operator = ( const ElementType& value ) const
{
    switch ( m_Index )
    {
        case 0 : m_Vector.X( value ); break;
        case 1 : m_Vector.Y( value ); break;
        case 2 : m_Vector.Z( value ); break;
        case 3 : m_Vector.W( value ); break;
    }
}



//
// --------------------------------------------------------------------------------------
// |-- ConstVecAccessor : Provides indexed read access to hardware vector elements --|
// --------------------------------------------------------------------------------------
//

/// Vector & index Constructor
template < class VectorType >
FORCE_INLINE
ConstVecAccessor<VectorType>::ConstVecAccessor( const VectorType& vector, const unsigned index ) :
    m_Vector( vector ),
    m_Index( index )
{
    assert( index < VectorType::m_Elements);
}

/// Cast to ElementType
template < class VectorType >
FORCE_INLINE
//ConstVecAccessor<VectorType>::operator const typename VectorType::m_DataType () const
ConstVecAccessor<VectorType>::operator const typename VectorType::T_Element () const
{
    switch ( m_Index )
    {
        case 0  : return m_Vector.X();
        case 1  : return m_Vector.Y();
        case 2  : return m_Vector.Z();
        case 3  : return m_Vector.W();
        default : return 0.0f;
    }
}


}; // namespace Maths

#endif

