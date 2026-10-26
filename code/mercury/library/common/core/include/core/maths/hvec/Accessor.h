#ifndef ACCESSOR_H
#define ACCESSOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{
    //
    // ---------------------------------------------------------------------------------------
    // |-- VectorAccessor : Provides indexed read/write access to hardware vector elements --|
    // ---------------------------------------------------------------------------------------
    //
    
    template < class VectorType >
    class VectorAccessor
    {
    public:
        
        //typedef typename VectorType::m_DataType ElementType;
        typedef typename VectorType::T_Element ElementType;
        
        /// Vector & index Constructor
        VectorAccessor( VectorType& vector, const unsigned index );
        
        /// Cast to ElementType
        operator const ElementType () const;
        
        /// ElementType Assignment
        void operator = ( const ElementType& value ) const;
        
        void operator = ( const VectorAccessor<VectorType>& copy ) { m_Vector = copy.m_Vector; m_Index = copy.m_Index; }
        
    private:
        
        VectorType&     m_Vector;
        U32             m_Index;
    };
    
    
    
    //
    // --------------------------------------------------------------------------------------
    // |-- ConstVecAccessor : Provides indexed read access to hardware vector elements --|
    // --------------------------------------------------------------------------------------
    //
    
    template < class VectorType >
    class ConstVecAccessor
    {
    public:
        
        //typedef typename VectorType::m_DataType ElementType;
        typedef typename VectorType::T_Element ElementType;
        
        /// Vector & index Constructor
        ConstVecAccessor( const VectorType& vector, const unsigned index );
        
        /// Cast to ElementType
        operator const ElementType () const;
        
        void operator = ( const ConstVecAccessor<VectorType>& copy ) { m_Vector = copy.m_Vector; m_Index = copy.m_Index; }
        
    private:
        
        const VectorType&   m_Vector;
        U32                 m_Index;
    };
    
    
}; // namespace Maths



#endif
