#ifndef MATHS_HVEC_CONSTANTS_H
#define MATHS_HVEC_CONSTANTS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

template <> struct Constants< HMat33 >
{
    NOALIAS __forceinline static const HMat33& Zero()     { return m_Zero; }
    NOALIAS __forceinline static const HMat33& Identity() { return m_Identity; }
    
    static const HMat33 m_Zero;
    static const HMat33 m_Identity;
};

template <> struct Constants< HMat43 >
{
    NOALIAS __forceinline static const HMat43& Zero()     { return m_Zero; }
    NOALIAS __forceinline static const HMat43& Identity() { return m_Identity; }
    
    static const HMat43 m_Zero;
    static const HMat43 m_Identity;
};

template <> struct Constants< HMat44 >
{
    NOALIAS __forceinline static const HMat44& Zero()     { return m_Zero; }
    NOALIAS __forceinline static const HMat44& Identity() { return m_Identity; }
    
    static const HMat44 m_Zero;
    static const HMat44 m_Identity;
};


template <> struct Constants< HF32 > 
{
	NOALIAS __forceinline static const HF32 Zero()      { return HF32( ZERO_INIT ); }
	NOALIAS __forceinline static const HF32 One()       { return HF32( c_ScalarOne ); }
	NOALIAS __forceinline static const HF32 Two()       { return HF32( c_ScalarTwo ); }
	NOALIAS __forceinline static const HF32 Three()     { return HF32( c_ScalarThree ); }
	NOALIAS __forceinline static const HF32 Four()      { return HF32( c_ScalarFour ); }
	NOALIAS __forceinline static const HF32 Half()      { return HF32( c_ScalarHalf ); }
	NOALIAS __forceinline static const HF32 Quarter()   { return HF32( c_ScalarQuarter ); }
	NOALIAS __forceinline static const HF32 Sixth()     { return Load<HF32>( &m_Sixth ); }
	NOALIAS __forceinline static const HF32 Min()       { return Load<HF32>( &m_Min ); }
	NOALIAS __forceinline static const HF32 Max()       { return Load<HF32>( &m_Max ); }
	NOALIAS __forceinline static const HF32 Epsilon()   { return Load<HF32>( &m_Epsilon ); }
	NOALIAS __forceinline static const HF32 Epsilon2()	{ return Load<HF32>(&m_Epsilon2 ); }
	NOALIAS __forceinline static const HF32 Epsilon3()	{ return Load<HF32>(&m_Epsilon3 ); }
	NOALIAS __forceinline static const HF32 Pi()        { return Load<HF32>( &m_Pi ); }
	NOALIAS __forceinline static const HF32 Sqrt2()     { return Load<HF32>( &m_Sqrt2 ); }
	NOALIAS __forceinline static const HF32 Sqrt2div2() { return Load<HF32>( &m_Sqrt2div2 ); }
	NOALIAS __forceinline static const HF32 e()         { return Load<HF32>( &m_e ); }
	
	static const F32 m_Sixth;
	static const F32 m_Min;
	static const F32 m_Max;
	static const F32 m_Epsilon;
	static const F32 m_Epsilon2;
	static const F32 m_Epsilon3;
	static const F32 m_Pi;
	static const F32 m_Sqrt2;
	static const F32 m_Sqrt2div2;
	static const F32 m_e;
};


template <> struct Constants< HVec3 > 
{
	NOALIAS __forceinline static const HVec3 Zero()      { return HVec3( ZERO_INIT ); }
	NOALIAS __forceinline static const HVec3 One()       { return HVec3( Constants<HF32>::One() ); }
	NOALIAS __forceinline static const HVec3 Two()       { return HVec3( Constants<HF32>::Two() ); }
	NOALIAS __forceinline static const HVec3 Three()     { return HVec3( Constants<HF32>::Three() ); }
	NOALIAS __forceinline static const HVec3 Four()      { return HVec3( Constants<HF32>::Four() ); }
	NOALIAS __forceinline static const HVec3 Half()      { return HVec3( Constants<HF32>::Half() ); }
	NOALIAS __forceinline static const HVec3 Quarter()   { return HVec3( Constants<HF32>::Quarter() ); }
	NOALIAS __forceinline static const HVec3 Min()       { return HVec3( Constants<HF32>::Min() ); }
	NOALIAS __forceinline static const HVec3 Max()       { return HVec3( Constants<HF32>::Max() ); }
	NOALIAS __forceinline static const HVec3 Epsilon()   { return HVec3( Constants<HF32>::Epsilon() ); }
	NOALIAS __forceinline static const HVec3 Pi()        { return HVec3( Constants<HF32>::Pi() ); }
	NOALIAS __forceinline static const HVec3 Sqrt2()     { return HVec3( Constants<HF32>::Sqrt2() ); }
	NOALIAS __forceinline static const HVec3 Sqrt2div2() { return HVec3( Constants<HF32>::Sqrt2div2() ); }
	NOALIAS __forceinline static const HVec3 Epsilon2()	 { return HVec3(Constants<HF32>::Epsilon2()); }
	NOALIAS __forceinline static const HVec3 Epsilon3()  { return HVec3(Constants<HF32>::Epsilon3()); }
	
    /// Positive X Axis Aligned Unit Vector
    NOALIAS __forceinline static const HVec3 UnitAxisPositiveX();
    
    /// Negative X Axis Aligned Unit Vector
    NOALIAS __forceinline static const HVec3 UnitAxisNegativeX();
    
    /// Positive Y Axis Aligned Unit Vector
    NOALIAS __forceinline static const HVec3 UnitAxisPositiveY();
    
    /// Negative Y Axis Aligned Unit Vector
    NOALIAS __forceinline static const HVec3 UnitAxisNegativeY();
    
    /// Positive Z Axis Aligned Unit Vector
    NOALIAS __forceinline static const HVec3 UnitAxisPositiveZ();
    
    /// Negative Z Axis Aligned Unit Vector
    NOALIAS __forceinline static const HVec3 UnitAxisNegativeZ();
};

template <> struct Constants<Scalar_Bool>
{
	__forceinline static const Scalar_Bool True() {return Scalar_Bool(1);}
	__forceinline static const Scalar_Bool False() {return Scalar_Bool(0);}
};


}; // namespace Maths

#endif



