#include "CorePch.h"

#if 0 && defined( USER_JDENTON )
#include <Core/Maths/Hvec/New/IncludeCompleteGA.h>
#include <Core/Maths/Hvec/New/IncludeCompleteGA.inl>
#endif

namespace Maths
{
/// @addtogroup GA
/// @{

#if 0 && defined( USER_JDENTON )

const GA::Origin Const< GA::Origin >::m_One( Const<VF32>::One() );
const GA::Origin Const< GA::Origin >::m_MinusOne( Const<VF32>::MinusOne() );

const GA::Infinity Const< GA::Infinity >::m_One( Const<VF32>::One() );
const GA::Infinity Const< GA::Infinity >::m_MinusOne( Const<VF32>::MinusOne() );

const GA::PseudoScalar Const< GA::PseudoScalar >::m_One( Const<VF32>::One() );
const GA::PseudoScalar Const< GA::PseudoScalar >::m_Inverse( Const<VF32>::MinusOne() );

const GA::PseudoScalarE3 Const< GA::PseudoScalarE3 >::m_One( Const<VF32>::One() );
const GA::PseudoScalarE3 Const< GA::PseudoScalarE3 >::m_Inverse( Const<VF32>::MinusOne() );

#endif

/// @}

namespace GA /// 5d Geometric Algebra
{
/// @addtogroup GA
/// @{


/// @}
}; // namespace GA
}; // namespace Maths


