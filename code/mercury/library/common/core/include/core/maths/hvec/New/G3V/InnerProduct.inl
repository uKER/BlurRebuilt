#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_INNER_PRODUCT_INL
#define SOFT_MATHS_HVEC_NEW_G3V_INNER_PRODUCT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_INNER_PRODUCT_INL
#define MATHS_HVEC_NEW_G3V_INNER_PRODUCT_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_InnerProduct
/// @{

/// Hestenes Inner Product Operator: Scalar | Scalar
//NOALIAS __forceinline const VF32 operator | ( const VF32 VECREF a, const VF32 VECREF b ); // Zero!

/// Hestenes Inner Product Operator: Scalar | Vector
//NOALIAS __forceinline const Vector operator | ( const VF32 VECREF a, const Vector PCREF b ); // Zero!

/// Hestenes Inner Product Operator: Scalar | BiVector
//NOALIAS __forceinline const BiVector operator | ( const VF32 VECREF a, const BiVector PCREF b ); // Zero!

/// Hestenes Inner Product Operator: Scalar | TriVector
//NOALIAS __forceinline const TriVector operator | ( const VF32 VECREF a, const TriVector PCREF b ); // Zero!

/// Hestenes Inner Product Operator: Vector | Scalar
//NOALIAS __forceinline const Vector operator | ( const Vector PCREF a, const VF32 VECREF b ); // Zero!

/// Hestenes Inner Product Operator: BiVector | Scalar
//NOALIAS __forceinline const BiVector operator | ( const BiVector PCREF a, const VF32 VECREF b ); // Zero!

/// Hestenes Inner Product Operator: TriVector | Scalar
//NOALIAS __forceinline const TriVector operator | ( const TriVector PCREF a, const VF32 VECREF b ); // Zero!


/// Hestenes Inner Product Operator: Vector | Vector
NOALIAS __forceinline const VF32 operator | ( const Vector PCREF a, const Vector PCREF b )
{
    return a % b;
}

/// Hestenes Inner Product Operator: Vector | BiVector
NOALIAS __forceinline const Vector operator | ( const Vector PCREF a, const BiVector PCREF b )
{
    return a << b;
}

/// Hestenes Inner Product Operator: Vector | TriVector
NOALIAS __forceinline const BiVector operator | ( const Vector PCREF a, const TriVector PCREF b )
{
    return a << b;
}

/// Hestenes Inner Product Operator: BiVector | Vector
NOALIAS __forceinline const Vector operator | ( const BiVector PCREF a, const Vector PCREF b )
{
    return a >> b;
}

/// Hestenes Inner Product Operator: TriVector | Vector
NOALIAS __forceinline const BiVector operator | ( const TriVector PCREF a, const Vector PCREF b )
{
    return a >> b;
}


/// Hestenes Inner Product Operator: BiVector | BiVector
NOALIAS __forceinline const VF32 operator | ( const BiVector PCREF a, const BiVector PCREF b )
{
    return a % b;
}

/// Hestenes Inner Product Operator: BiVector | TriVector
NOALIAS __forceinline const Vector operator | ( const BiVector PCREF a, const TriVector PCREF b )
{
    return a << b;
}

/// Hestenes Inner Product Operator: TriVector | BiVector
NOALIAS __forceinline const Vector operator | ( const TriVector PCREF a, const BiVector PCREF b )
{
    return a >> b;
}


/// Hestenes Inner Product Operator: TriVector | TriVector
NOALIAS __forceinline const VF32 operator | ( const TriVector PCREF a, const TriVector PCREF b )
{
    return a % b;
}






/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


