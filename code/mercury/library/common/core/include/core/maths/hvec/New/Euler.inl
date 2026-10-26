#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_EULER_INL
#define SOFT_MATHS_HVEC_NEW_EULER_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_EULER_INL
#define MATHS_HVEC_NEW_EULER_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Euler
/// @{

/// Make EulerXYZ from Vector3
NOALIAS __forceinline const EulerXYZ MakeEulerXYZ( const Vector3 PCREF v )
{
    EulerXYZ result;
    result.m_V = v;
    return result;
}

/// Make EulerXYZ from three VF32's
NOALIAS __forceinline const EulerXYZ MakeEulerXYZ( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    EulerXYZ result;
    result.m_V = MakeVector3( x, y, z );
    return result;
}

/// EulerXYZ get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerXYZ PCREF e )
{
    return e.m_V;
}

/// EulerXYZ Get X
NOALIAS __forceinline const VF32 X( const EulerXYZ PCREF e )
{
    return X( e.m_V );
}

/// EulerXYZ Get Y
NOALIAS __forceinline const VF32 Y( const EulerXYZ PCREF e )
{
    return Y( e.m_V );
}

/// EulerXYZ Get Z
NOALIAS __forceinline const VF32 Z( const EulerXYZ PCREF e )
{
    return Z( e.m_V );
}


/// Make EulerXZY from Vector3
NOALIAS __forceinline const EulerXZY MakeEulerXZY( const Vector3 PCREF v )
{
    EulerXZY result;
    result.m_V = v;
    return result;
}

/// Make EulerXZY from three VF32's
NOALIAS __forceinline const EulerXZY MakeEulerXZY( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    EulerXZY result;
    result.m_V = MakeVector3( x, y, z );
    return result;
}

/// EulerXZY get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerXZY PCREF e )
{
    return e.m_V;
}

/// EulerXZY Get X
NOALIAS __forceinline const VF32 X( const EulerXZY PCREF e )
{
    return X( e.m_V );
}

/// EulerXZY Get Y
NOALIAS __forceinline const VF32 Y( const EulerXZY PCREF e )
{
    return Y( e.m_V );
}

/// EulerXZY Get Z
NOALIAS __forceinline const VF32 Z( const EulerXZY PCREF e )
{
    return Z( e.m_V );
}


/// Make EulerYXZ from Vector3
NOALIAS __forceinline const EulerYXZ MakeEulerYXZ( const Vector3 PCREF v )
{
    EulerYXZ result;
    result.m_V = v;
    return result;
}

/// Make EulerYXZ from three VF32's
NOALIAS __forceinline const EulerYXZ MakeEulerYXZ( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    EulerYXZ result;
    result.m_V = MakeVector3( x, y, z );
    return result;
}

/// EulerYXZ get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerYXZ PCREF e )
{
    return e.m_V;
}

/// EulerYXZ Get X
NOALIAS __forceinline const VF32 X( const EulerYXZ PCREF e )
{
    return X( e.m_V );
}

/// EulerYXZ Get Y
NOALIAS __forceinline const VF32 Y( const EulerYXZ PCREF e )
{
    return Y( e.m_V );
}

/// EulerYXZ Get Z
NOALIAS __forceinline const VF32 Z( const EulerYXZ PCREF e )
{
    return Z( e.m_V );
}


/// Make EulerYZX from Vector3
NOALIAS __forceinline const EulerYZX MakeEulerYZX( const Vector3 PCREF v )
{
    EulerYZX result;
    result.m_V = v;
    return result;
}

/// Make EulerYZX from three VF32's
NOALIAS __forceinline const EulerYZX MakeEulerYZX( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    EulerYZX result;
    result.m_V = MakeVector3( x, y, z );
    return result;
}

/// EulerYZX get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerYZX PCREF e )
{
    return e.m_V;
}

/// EulerYZX Get X
NOALIAS __forceinline const VF32 X( const EulerYZX PCREF e )
{
    return X( e.m_V );
}

/// EulerYZX Get Y
NOALIAS __forceinline const VF32 Y( const EulerYZX PCREF e )
{
    return Y( e.m_V );
}

/// EulerYZX Get Z
NOALIAS __forceinline const VF32 Z( const EulerYZX PCREF e )
{
    return Z( e.m_V );
}


/// Make EulerZXY from Vector3
NOALIAS __forceinline const EulerZXY MakeEulerZXY( const Vector3 PCREF v )
{
    EulerZXY result;
    result.m_V = v;
    return result;
}

/// Make EulerZXY from three VF32's
NOALIAS __forceinline const EulerZXY MakeEulerZXY( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    EulerZXY result;
    result.m_V = MakeVector3( x, y, z );
    return result;
}

/// EulerZXY get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerZXY PCREF e )
{
    return e.m_V;
}

/// EulerZXY Get X
NOALIAS __forceinline const VF32 X( const EulerZXY PCREF e )
{
    return X( e.m_V );
}

/// EulerZXY Get Y
NOALIAS __forceinline const VF32 Y( const EulerZXY PCREF e )
{
    return Y( e.m_V );
}

/// EulerZXY Get Z
NOALIAS __forceinline const VF32 Z( const EulerZXY PCREF e )
{
    return Z( e.m_V );
}


/// Make EulerZYX from Vector3
NOALIAS __forceinline const EulerZYX MakeEulerZYX( const Vector3 PCREF v )
{
    EulerZYX result;
    result.m_V = v;
    return result;
}

/// Make EulerZYX from three VF32's
NOALIAS __forceinline const EulerZYX MakeEulerZYX( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    EulerZYX result;
    result.m_V = MakeVector3( x, y, z );
    return result;
}

/// EulerZYX get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerZYX PCREF e )
{
    return e.m_V;
}

/// EulerZYX Get X
NOALIAS __forceinline const VF32 X( const EulerZYX PCREF e )
{
    return X( e.m_V );
}

/// EulerZYX Get Y
NOALIAS __forceinline const VF32 Y( const EulerZYX PCREF e )
{
    return Y( e.m_V );
}

/// EulerZYX Get Z
NOALIAS __forceinline const VF32 Z( const EulerZYX PCREF e )
{
    return Z( e.m_V );
}






/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


