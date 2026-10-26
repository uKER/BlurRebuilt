#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_CONST_H
#define SOFT_MATHS_HVEC_NEW_GA_CONST_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_CONST_H
#define MATHS_HVEC_NEW_GA_CONST_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup GA
/// @{

/// GA Origin constants
template <> struct Const< GA::Origin > 
{
	NOALIAS __forceinline static const GA::Origin One();      ///< Constant 1.0f
	NOALIAS __forceinline static const GA::Origin MinusOne(); ///< Constant -1.0f
    
    static const GA::Origin m_One;      ///< Constant 1.0f
    static const GA::Origin m_MinusOne; ///< Constant -1.0f
};

/// GA Infinity constants
template <> struct Const< GA::Infinity > 
{
	NOALIAS __forceinline static const GA::Infinity One();      ///< Constant 1.0f
	NOALIAS __forceinline static const GA::Infinity MinusOne(); ///< Constant -1.0f
    
    static const GA::Infinity m_One;      ///< Constant 1.0f
    static const GA::Infinity m_MinusOne; ///< Constant -1.0f
};

/// GA PseudoScalar constants
template <> struct Const< GA::PseudoScalar > 
{
	NOALIAS __forceinline static const GA::PseudoScalar One();     ///< Constant 1.0f
	NOALIAS __forceinline static const GA::PseudoScalar Inverse(); ///< Constant -1.0f
    
    static const GA::PseudoScalar m_One;     ///< Constant 1.0f
    static const GA::PseudoScalar m_Inverse; ///< Constant -1.0f
};

/// GA PseudoScalar constants
template <> struct Const< GA::PseudoScalarE3 > 
{
	NOALIAS __forceinline static const GA::PseudoScalarE3 One();     ///< Constant 1.0f
	NOALIAS __forceinline static const GA::PseudoScalarE3 Inverse(); ///< Constant -1.0f
    
    static const GA::PseudoScalarE3 m_One;     ///< Constant 1.0f
    static const GA::PseudoScalarE3 m_Inverse; ///< Constant -1.0f
};

/// @}

namespace GA /// 5d Geometric Algebra
{
/// @addtogroup GA
/// @{


/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


