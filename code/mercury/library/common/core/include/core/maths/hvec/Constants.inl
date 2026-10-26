#ifndef MATHS_HVEC_CONSTANTS_INL
#define MATHS_HVEC_CONSTANTS_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

/// Positive X Axis Aligned Unit Vector Constructor
NOALIAS __forceinline const HVec3 Constants<HVec3>::UnitAxisPositiveX()
{
    #if defined( _VMX128_ )
    
    const __vector4 unit_x( c_UnitVectorX );
    
    HVec3 result( UNDEFINED );
    result.m_V = unit_x;
    return result;
    
    #else
    return HVec3( c_UnitVectorX );
    #endif
}

/// Negative X Axis Aligned Unit Vector Constructor
NOALIAS __forceinline const HVec3 Constants<HVec3>::UnitAxisNegativeX()
{
    #if defined( _VMX128_ )
    
    const __vector4 unit_x( c_UnitVectorX );
    const __vector4 signMask( c_ScalarSignMask );
    
    HVec3 result( UNDEFINED );
    result.m_V = __vxor( unit_x, signMask );
    return result;
    
    #else
    return -HVec3( c_UnitVectorX );
    #endif
}

/// Positive Y Axis Aligned Unit Vector Constructor
NOALIAS __forceinline const HVec3 Constants<HVec3>::UnitAxisPositiveY()
{
    #if defined( _VMX128_ )
    
    const __vector4 unit_y( c_UnitVectorY );
    
    HVec3 result( UNDEFINED );
    result.m_V = unit_y;
    return result;
    
    #else
    return HVec3( c_UnitVectorY );
    #endif
}

/// Negative Y Axis Aligned Unit Vector Constructor
NOALIAS __forceinline const HVec3 Constants<HVec3>::UnitAxisNegativeY()
{
    #if defined( _VMX128_ )
    
    const __vector4 unit_y( c_UnitVectorY );
    const __vector4 signMask( c_ScalarSignMask );
    
    HVec3 result( UNDEFINED );
    result.m_V = __vxor( unit_y, signMask );
    return result;
    
    #else
    return -HVec3( c_UnitVectorY );
    #endif
}

/// Positive Z Axis Aligned Unit Vector Constructor
NOALIAS __forceinline const HVec3 Constants<HVec3>::UnitAxisPositiveZ()
{
    #if defined( _VMX128_ )
    
    const __vector4 unit_z( c_UnitVectorZ );
    
    HVec3 result( UNDEFINED );
    result.m_V = unit_z;
    return result;
    
    #else
    return HVec3( c_UnitVectorZ );
    #endif
}

/// Negative Z Axis Aligned Unit Vector Constructor
NOALIAS __forceinline const HVec3 Constants<HVec3>::UnitAxisNegativeZ()
{
    #if defined( _VMX128_ )
    
    const __vector4 unit_z( c_UnitVectorZ );
    const __vector4 signMask( c_ScalarSignMask );
    
    HVec3 result( UNDEFINED );
    result.m_V = __vxor( unit_z, signMask );
    return result;
    
    #else
    return -HVec3( c_UnitVectorZ );
    #endif
}

}; // namespace Maths

#endif



