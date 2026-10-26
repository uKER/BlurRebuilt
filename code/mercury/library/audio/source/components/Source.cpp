#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::Source::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Source::InitContext(Instance* pInstance)
{
    using namespace Audio::Maths;
    
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
    
    const Scalar one( _Scalar( 1.0f ) );

    //  get source orientation
    pState->vObjectRelativePosition = MakeVector3( pInitData->fSourcePositionXVal, pInitData->fSourcePositionYVal, pInitData->fSourcePositionZVal );
    pState->vObjectRelativeDirection = MakeVector3( pInitData->fSourceDirectionXVal, pInitData->fSourceDirectionYVal, pInitData->fSourceDirectionZVal );
    
    //  get object orientation
    const Maths::Vector object_pos( MakeVector3( pInitData->fObjectPositionXVal, pInitData->fObjectPositionYVal, pInitData->fObjectPositionZVal ) );
    const Maths::Vector object_az( MakeVector3( pInitData->fObjectDirectionXVal, pInitData->fObjectDirectionYVal, pInitData->fObjectDirectionZVal ) );
    const Maths::Vector object_ay( MakeVector3( pInitData->fObjectUpXVal, pInitData->fObjectUpYVal, pInitData->fObjectUpZVal ) );
    const Maths::Vector object_ax( object_az ^ object_ay );
    
    //  create object matrix
    pState->mObjectOrientation = Matrix( object_pos, object_ax | one, object_ay | one, object_az | one );
    
    //  get listener orientation
    const Maths::Vector listener_pos( MakeVector3( pInitData->fListenerPositionXVal, pInitData->fListenerPositionYVal, pInitData->fListenerPositionZVal ) );
    const Maths::Vector listener_az( MakeVector3( pInitData->fListenerDirectionXVal, pInitData->fListenerDirectionYVal, pInitData->fListenerDirectionZVal ) );
    const Maths::Vector listener_ay( MakeVector3( pInitData->fListenerUpXVal, pInitData->fListenerUpYVal, pInitData->fListenerUpZVal ) );
    const Maths::Vector listener_ax( listener_ay ^ listener_az );
    
    //  create listener matrix
    pState->mListenerOrientation = Matrix( listener_pos, listener_ax | one, listener_ay | one, listener_az | one );
    
    //  create inverse
    pState->mInvListenerOrientation = !pState->mListenerOrientation;
    
    pState->vWorldPosition = pState->mObjectOrientation.Transform(pState->vObjectRelativePosition);
    pState->vWorldDirection = pState->mObjectOrientation.Rotate(pState->vObjectRelativeDirection);
    
    pState->vListenerRelativePosition = pState->mInvListenerOrientation.Transform(pState->vWorldPosition);
    pState->vListenerRelativeDirection = pState->mInvListenerOrientation.Rotate(pState->vWorldDirection);
    
    //  distance is listener to source magnitude
	pState->fDistance = _float( !pState->vListenerRelativePosition - _Scalar( pInitData->optSize.fVal ) );

    //  azimuth, elevation
	pState->fAzimuth   = ArcTan2( _float( X( pState->vListenerRelativePosition ) ), _float( Z( pState->vListenerRelativePosition ) ) );
    pState->fElevation = ArcTan2( _float( Y( pState->vListenerRelativePosition ) ), _float( Z( pState->vListenerRelativePosition ) ) );
    
    //  source incidence
	pState->fSourceIncidence = ArcTan2( _float( X( pState->vListenerRelativeDirection ) ), _float( Z( pState->vListenerRelativeDirection ) ) ) * fReciprocalPi;
    
    //  listener incidence
    pState->fListenerIncidence = ArcTan2( _float( X( pState->vListenerRelativePosition ) ), _float( Z( pState->vListenerRelativePosition ) ) ) * fReciprocalPi;
    
    //  velocity
    pState->fVelocity = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Source::Update( Instance* pInstance )
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.Source");

    using namespace Audio::Maths;
    
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetNewStateData();
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

    bool bRegenPosition = false;
    bool bRegenDirection = false;
    bool bRegenObjectMatrix = false;
    bool bRegenListenerMatrix = false;

    //  get size
	F32 fSize = Maths::Max( GetOptionValue( pInstance, pInitData->optSize ), 0.f );

    //  get local position
    if( pInitData->sSourcePositionVec != -1 )
    {
        //  get vector
        Maths::Vector vLocalPosition;
        m_pObject->GetComponentCoords( pInitData->sSourcePositionVec, pInstance, &vLocalPosition );

        //  set local position, if changed
        if ( IsEqual( pState->vObjectRelativePosition, vLocalPosition ) == false )
        {
            bRegenPosition = true;
            pState->vObjectRelativePosition = vLocalPosition;
        }
    }
    
    const Scalar one( _Scalar( 1.0f ) );
    
    //  get local direction
    if( pInitData->sSourceDirectionVec != -1 )
    {
        //  get vector
        Maths::Vector vLocalDirection;
        m_pObject->GetComponentCoords( pInitData->sSourceDirectionVec, pInstance, &vLocalDirection );

        //  normalize
        if( _float( !vLocalDirection ) == 0.f )
            vLocalDirection = Vector3_UnitZ();
        else
            vLocalDirection |= one;

        //  set local direction, if changed
        if( IsEqual( pState->vObjectRelativePosition, vLocalDirection ) == false )
        {
            bRegenDirection = true;
            pState->vObjectRelativeDirection = vLocalDirection;
        }
    }

    //  get object position
    if( pInitData->sObjectPositionVec != -1 )
    {
        //  get vector
        Maths::Vector vObjectPosition;
        m_pObject->GetComponentCoords( pInitData->sObjectPositionVec, pInstance, &vObjectPosition );

        //  set object position, if changed
        if( IsEqual( Pos( pState->mObjectOrientation ), vObjectPosition ) == false )
        {
            bRegenPosition = true;
            bRegenDirection = true;
            bRegenObjectMatrix = true;
            SetPos( pState->mObjectOrientation, vObjectPosition );
        }
    }

    //  get object direction
    if( pInitData->sObjectDirectionVec != -1 )
    {
        //  get vector
        Maths::Vector vObjectDirection;
        m_pObject->GetComponentCoords( pInitData->sObjectDirectionVec, pInstance, &vObjectDirection );

        //  normalize
        if ( _float( !vObjectDirection ) == 0.f)
            vObjectDirection = Vector3_UnitZ();
        else
            vObjectDirection |= one;

        //  set object direction, if changed
        if( IsEqual( AZ( pState->mObjectOrientation ), vObjectDirection ) == false )
        {
            bRegenPosition = true;
            bRegenDirection = true;
            bRegenObjectMatrix = true;
            SetAZ( pState->mObjectOrientation, vObjectDirection );
        }
    }

    //  get object up vector
    if( pInitData->sObjectUpVec != -1 )
    {
        //  get vector
        Maths::Vector vObjectUp;
        m_pObject->GetComponentCoords( pInitData->sObjectUpVec, pInstance, &vObjectUp );

        //  normalize
        if ( _float( !vObjectUp ) == 0.f)
            vObjectUp = Vector3_UnitY();
        else
            vObjectUp |= one;

        //  set object up vector, if changed
        if ( IsEqual( AY( pState->mObjectOrientation ), vObjectUp ) == false )
        {
            bRegenPosition = true;
            bRegenDirection = true;
            bRegenObjectMatrix = true;
            SetAY( pState->mObjectOrientation, vObjectUp );
        }
    }

    //  get listener position
    if( pInitData->sListenerPositionVec != -1 )
    {
        //  get vector
        Maths::Vector vListenerPosition;
        m_pObject->GetComponentCoords( pInitData->sListenerPositionVec, pInstance, &vListenerPosition );

        //  set listener position, if changed
        if( IsEqual( Pos( pState->mListenerOrientation ), vListenerPosition ) == false )
        {
            bRegenPosition = true;
            bRegenDirection = true;
            bRegenListenerMatrix = true;
            SetPos( pState->mListenerOrientation, vListenerPosition );
        }
    }

    //  get listener direction
    if(pInitData->sListenerDirectionVec != -1 )
    {
        //  get vector
        Maths::Vector vListenerDirection;
        m_pObject->GetComponentCoords( pInitData->sListenerDirectionVec, pInstance, &vListenerDirection );

        //  normalize
        if( _float( !vListenerDirection ) == 0.f )
            vListenerDirection = Vector3_UnitZ();
        else
            vListenerDirection |= one;

        //  set listener direction, if changed
        if( IsEqual( AZ( pState->mListenerOrientation ), vListenerDirection ) == false )
        {
            bRegenPosition = true;
            bRegenDirection = true;
            bRegenListenerMatrix = true;
            SetAZ( pState->mListenerOrientation, vListenerDirection );
        }
    }

    //  get listener up vector
    if( pInitData->sListenerUpVec != -1 )
    {
        //  get vector
        Maths::Vector vListenerUp;
        m_pObject->GetComponentCoords( pInitData->sListenerUpVec, pInstance, &vListenerUp );

        //  normalize
        if( _float( !vListenerUp ) == 0.f )
            vListenerUp = Vector3_UnitY();
        else
            vListenerUp |= one;

        //  set object up vector, if changed
        if( IsEqual( AY( pState->mListenerOrientation ), vListenerUp ) == false )
        {
            bRegenPosition = true;
            bRegenDirection = true;
            bRegenListenerMatrix = true;
            SetAY( pState->mListenerOrientation, vListenerUp );
        }
    }

    //  recalculate object orientation matrix, if changed
    if( bRegenObjectMatrix )
	{
        SetAX( pState->mObjectOrientation, ( AY( pState->mObjectOrientation ) ^ AZ( pState->mObjectOrientation ) ) | one );
        SetAY( pState->mObjectOrientation, ( AZ( pState->mObjectOrientation ) ^ AX( pState->mObjectOrientation ) ) | one );
	}

    //  recalculate global position, if changed
    if( bRegenPosition || bRegenObjectMatrix )
        pState->vWorldPosition = pState->mObjectOrientation.Transform( pState->vObjectRelativePosition );

    //  recalculate global direction, if changed
    if( bRegenDirection || bRegenObjectMatrix )
        pState->vWorldDirection = pState->mObjectOrientation.Rotate( pState->vObjectRelativeDirection );

    //  recalculate listener orientation matrix, if changed
    if( bRegenListenerMatrix )
    {
        SetAX( pState->mListenerOrientation, ( AY( pState->mListenerOrientation ) ^ AZ( pState->mListenerOrientation ) ) | one );
        SetAY( pState->mListenerOrientation, ( AZ( pState->mListenerOrientation ) ^ AX( pState->mListenerOrientation ) ) | one );

        //  make inverse
        pState->mInvListenerOrientation = !pState->mListenerOrientation;
    }

    //  recalculate listener relative position, if changed
    if( bRegenPosition || bRegenListenerMatrix )
        pState->vListenerRelativePosition = pState->mInvListenerOrientation.Transform( pState->vWorldPosition );

    //  recalculate world direction, if changed
    if( bRegenDirection || bRegenListenerMatrix )
        pState->vListenerRelativeDirection = pState->mInvListenerOrientation.Rotate( pState->vWorldDirection );

    //  distance is listener-to-source magnitude
    pState->fDistance = _float( !pState->vListenerRelativePosition );

	//	if not a point source
    if( fSize != 0.f )
    {
        F32 fRadius = 0.5f * fSize;

		//	get visible width using radius and source distance
		if( fRadius < pState->fDistance )
			pState->fWidth = ArcSin( fRadius / pState->fDistance ) / Maths::fHalfPi;
		else
			pState->fWidth = 1.f;

		//	use nearest point on source as distance output
		pState->fDistance = Maths::Max( pState->fDistance - fRadius, 0.f );
    }
	else
		pState->fWidth = 0.f;

    //  azimuth
    pState->fAzimuth = ArcTan2( _float( X( pState->vListenerRelativePosition ) ), _float( Z( pState->vListenerRelativePosition ) ) ) * fReciprocalPi;

    //  elevation
	F32 fX2 = _float( Z( pState->vListenerRelativePosition ) ) * _float( Z( pState->vListenerRelativePosition ) );
	F32 fZ2 = _float( X( pState->vListenerRelativePosition ) ) * _float( X( pState->vListenerRelativePosition ) );
    pState->fElevation = ArcTan2( _float( Y( pState->vListenerRelativePosition ) ), Maths::Sqrt( fX2 + fZ2 ) ) * fReciprocalPi;

	const Maths::Vector vSrcList( pState->vWorldPosition - Pos( pState->mListenerOrientation ) );

    //  source incidence angle
	if( _float( Magnitude2( vSrcList ) ) == 0.f )
		pState->fSourceIncidence = 0.f;
	else
		pState->fSourceIncidence = 1.f - Angle( vSrcList | one, pState->vWorldDirection ) * fReciprocalPi;
    
    //  listener incidence angle
	if( _float( Magnitude2( pState->vListenerRelativePosition ) ) == 0.f )
		pState->fListenerIncidence = 0.f;
	else
		pState->fListenerIncidence = Angle( ( pState->vListenerRelativePosition | one ), Vector3_UnitZ() ) * fReciprocalPi;

#ifndef CFG_GOLD
	if( !Maths::IsFinite( pState->fDistance ) )
	{
		pState->fDistance = fMax * 0.5f;
		AUDIO_SYS_WARN("Source '%s' in '%s': distance clamped to FLOAT_MAX / 2", Name(), GetObject()->Name());
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Source::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case AzimuthCtrl:
        return pState->fAzimuth;
        break;

    case ElevationCtrl:
        return pState->fElevation;
        break;

    case DistanceCtrl:
        return pState->fDistance;
        break;

    case WidthCtrl:
		return pState->fWidth;
        break;

    case VelocityCtrl:
        return pState->fVelocity;
        break;

    case SourceIncidenceCtrl:
        return pState->fSourceIncidence;
        break;

    case ListenerIncidenceCtrl:
        return pState->fListenerIncidence;
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInfo
////////////////////////////////////////////////////////////////////////////////
#if defined(ENABLE_DEBUG_EXTENSIONS)
void Audio::Source::GetInfo(Instance* pInstance, Info* pInfo)
{
	U8* pStateData = pInstance->GetStateData();
    Source::State* pState = (Source::State*)&pStateData[m_uStateOffset];

	System::Get()->GetLock()->Acquire();

	//pInfo->fXPos = pState->vWorldPosition.x;
	//pInfo->fYPos = pState->vWorldPosition.y;
	//pInfo->fZPos = pState->vWorldPosition.z;
	//pInfo->fXDir = pState->vWorldDirection.x;
	//pInfo->fYDir = pState->vWorldDirection.y;
	//pInfo->fZDir = pState->vWorldDirection.z;
	//pInfo->fXPos = X( pState->vWorldPosition );
	//pInfo->fYPos = Y( pState->vWorldPosition );
	//pInfo->fZPos = Z( pState->vWorldPosition );
	//pInfo->fXDir = X( pState->vWorldDirection );
	//pInfo->fYDir = Y( pState->vWorldDirection );
	//pInfo->fZDir = Z( pState->vWorldDirection );
	pInfo->fXPos = _float( X( pState->vWorldPosition ) );
	pInfo->fYPos = _float( Y( pState->vWorldPosition ) );
	pInfo->fZPos = _float( Z( pState->vWorldPosition ) );
	pInfo->fXDir = _float( X( pState->vWorldDirection ) );
	pInfo->fYDir = _float( Y( pState->vWorldDirection ) );
	pInfo->fZDir = _float( Z( pState->vWorldDirection ) );

	System::Get()->GetLock()->Release();
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
