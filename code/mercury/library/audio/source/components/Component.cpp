#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif

#ifndef PLATFORM_SPU
Debug::Bool gbAudioLogComponentHashValues("Audio.System.LogComponentHashValues", false);
#ifndef CFG_GOLD
Debug::Bool g_bAudioForceAmbisonics("Audio.System.ForceAmbisonics", false);
#endif
#endif


//#pragma optimize("", off)

////////////////////////////////////////////////////////////////////////////////
//	new
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU

void* Audio::Component::operator new(size_t sz)
{
	return System::Alloc(sz);
}

////////////////////////////////////////////////////////////////////////////////
//	new
////////////////////////////////////////////////////////////////////////////////
void* Audio::Component::operator new(size_t sz, void* pHere)
{
	return pHere;
}

////////////////////////////////////////////////////////////////////////////////
//	delete
////////////////////////////////////////////////////////////////////////////////
void Audio::Component::operator delete(void* ptr)
{
	System::Free(ptr);
}

#ifdef COMPONENTS_USE_ALLOCATOR
////////////////////////////////////////////////////////////////////////////////
//	new
////////////////////////////////////////////////////////////////////////////////
void* Audio::Component::operator new( size_t sz, IndexedAllocator* pAlloc )
{
	return pAlloc->AllocatePtr( sz );
}

////////////////////////////////////////////////////////////////////////////////
//	delete
////////////////////////////////////////////////////////////////////////////////
void Audio::Component::operator delete( void* ptr, IndexedAllocator* pAlloc )
{
	pAlloc->Free();
}
#endif // COMPONENTS_USE_ALLOCATOR
#endif	//	#ifndef PLATFORM_SPU
////////////////////////////////////////////////////////////////////////////////
//	(destructor)
////////////////////////////////////////////////////////////////////////////////
Audio::Component::~Component()
{
}

////////////////////////////////////////////////////////////////////////////////
//	ContextSize
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
U32 Audio::Component::ContextSize()
{
	return 0;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	SetValue
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::Component::SetValueDirect(Instance* pInstance, F32 fValue)
{
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::Component::DoMessage(Instance* pInstance, U32 uMessage)
{
}

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Component::Update(Instance* pInstance)
{
    //COMPONENT_ERROR("Component::Update() called");
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	ProcessConnection
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::Component::ProcessConnection(Instance* pInstance)
{
    COMPONENT_ERROR("Component::ProcessConnection() called");
}
#endif	//	#ifndef PLATFORM_SPU

#ifndef CFG_GOLD
////////////////////////////////////////////////////////////////////////////////
//	DumpProcessFlags
////////////////////////////////////////////////////////////////////////////////
void Audio::Component::DumpProcessFlags( Instance* pInstance )
{
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Generate
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::Component::Generate(Instance* pInstance)
{
    COMPONENT_ERROR("Component::Generate() called");
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Component::Value(Instance* pInstance, U32 uOutput)
{
    //COMPONENT_ERROR("Component::Value() called");
    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	Coords
////////////////////////////////////////////////////////////////////////////////
void Audio::Component::Coords(Instance* pInstance, Maths::Vector* pVector)
{
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Type
////////////////////////////////////////////////////////////////////////////////
#ifndef SPU_MEMORYSAVER
const char* Audio::Component::Type() const
{
	// this must not be a COMPONENT_ERROR otherwise stack overflow will occur
	AUDIO_SYS_ERROR("'%s', '%s': Component::Type() called", GetObject()->Name(), Name());
    return "Component";
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Execute
////////////////////////////////////////////////////////////////////////////////
void Audio::Component::Execute(Instance* pInstance)
{
    COMPONENT_ERROR("Component::Execute() called");
}

////////////////////////////////////////////////////////////////////////////////
//	DSPEffectUnit
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Component::DSPEffectUnit(U8* pStateData, U32 uIndex)
{
    COMPONENT_ERROR("Component::DSPEffectUnit() called");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	DSPEffectType
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Component::DSPEffectType(U8* pStateData, U32 uIndex)
{
    //COMPONENT_ERROR("Component::DSPEffectType() called");
    return (U32)-1;
}

////////////////////////////////////////////////////////////////////////////////
//	DSPEffectCount
//		returns 1 unless overridden
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Component::DSPEffectCount( )
{
    //COMPONENT_ERROR("Component::DSPEffectCount() called");
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
//	GetValue
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Component::GetOptionValue( Instance* pInstance, Option& opt )
{
#ifndef PLATFORM_SPU
	if( opt.ctrl.sCom != -1 )
		return pInstance->GetObject()->GetComponent( opt.ctrl.sCom )->Value( pInstance, opt.ctrl.uOut );
	else
		return opt.fVal;
#else
	if( opt.ctrl.sCom != -1 )
		return pInstance->GetObject()->GetComponentValue(opt.ctrl.sCom, pInstance, opt.ctrl.uOut);
	else
		return opt.fVal;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	GetControlValue
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Component::GetControlValue( Instance* pInstance, Control& ctrl )
{
	Assert ( ctrl.sCom != eUndefined );

#ifndef PLATFORM_SPU
	if( ctrl.sCom != -1 )
		return pInstance->GetObject()->GetComponent( ctrl.sCom )->Value( pInstance, ctrl.uOut );
	else
		return 0.f;
#else
	if( ctrl.sCom != -1 )
		return pInstance->GetObject()->GetComponentValue( ctrl.sCom, pInstance, ctrl.uOut );
	else
		return 0.f;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutOffset
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::AudioComponent::AudioOutOffset(Instance* pInstance) const
{
    COMPONENT_ERROR("Component::AudioOutOffset() called");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::AudioComponent::AudioOutType(Instance* pInstance) const
{
    //COMPONENT_ERROR("Component::AudioOutType() called");
    return (U32)-1;
}

////////////////////////////////////////////////////////////////////////////////
//	GetLevel
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Component::GetLevel( Instance* pInstance, Option& optLin, Option& optLog )
{
	F32 fLinVal = GetOptionValue( pInstance, optLin );
	F32 fLogVal = GetOptionValue( pInstance, optLog );

	if( fLogVal != 0.f )
		fLinVal *= Maths::dBToFloat( fLogVal );

	return fLinVal;
}

////////////////////////////////////////////////////////////////////////////////
//	GetLevel
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Component::GetFrequency( Instance* pInstance, Option& optLin, Option& optLog )
{
#if defined (PLATFORM_SPU)
	F32 fLinVal;
	if( optLin.ctrl.sCom != -1 )
		fLinVal = pInstance->GetObject()->GetComponentValue( optLin.ctrl.sCom, pInstance, optLin.ctrl.uOut );
	else
		fLinVal = optLin.fVal;

	F32 fLogVal;
	if( optLog.ctrl.sCom != -1 )
		fLogVal = pInstance->GetObject()->GetComponentValue( optLog.ctrl.sCom, pInstance, optLog.ctrl.uOut );
	else
		fLogVal = optLog.fVal;
#else
	F32 fLinVal;
	if( optLin.ctrl.sCom != -1 )
		fLinVal = pInstance->GetObject()->GetComponent( optLin.ctrl.sCom )->Value( pInstance, optLin.ctrl.uOut );
	else
		fLinVal = optLin.fVal;

	F32 fLogVal;
	if( optLog.ctrl.sCom != -1 )
		fLogVal = pInstance->GetObject()->GetComponent( optLog.ctrl.sCom )->Value( pInstance, optLog.ctrl.uOut );
	else
		fLogVal = optLog.fVal;
#endif

	if( fLogVal != 0.f )
		fLinVal *= Maths::OctToHz( fLogVal );

	return fLinVal;
}

////////////////////////////////////////////////////////////////////////////////
//	DoEvent
////////////////////////////////////////////////////////////////////////////////
void Audio::Component::DoEvent( Instance* pInstance, Event& ev )
{
	if( ev.sCom != -1 )
	{
#if defined (AUDIO_ENABLE_PROFILING)
		Profiler::Instance().PostEventMessage(pInstance, ev.sCom, ev.uMsg);
#endif

#ifndef PLATFORM_SPU
		pInstance->GetObject()->GetComponent( ev.sCom )->DoMessage( pInstance, ev.uMsg );
#else
		pInstance->GetObject()->DoComponentMessage(ev.sCom, pInstance, ev.uMsg);
#endif
	}
}

//////////////////////////////////////////////////////////////////////////////////
////	PreInit
//////////////////////////////////////////////////////////////////////////////////
//#if !defined PLATFORM_SPU
//template <typename T>
//T* Audio::Component::PreInit(void* pData)
//{
//	m_uInitOffset = (U8*)pData - (U8*)m_pObject->GetInitData();
//	T* pInitData = (T*)m_pObject->GetInitData(m_uInitOffset);
//
//	m_pchName = m_pObject->GetString(pInitData->strName);
//
//#ifdef AUDIO_STRING_HASH_COMPONENTS 
//	m_nameHash = StringHash(m_pchName);
//
//	if (gbAudioLogComponentHashValues)
//		AUDIO_SYS_DEBUG("Hashing %s->%s as 0x%x\n", m_pObject->Name(), m_pchName, m_nameHash.GetValue());
//#endif	// #ifdef AUDIO_STRING_HASH_COMPONENTS
//
//	return (T*)(pInitData);
//}
//#endif // #if !defined PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Error
////////////////////////////////////////////////////////////////////////////////
#if !defined CFG_GOLD && !defined PLATFORM_SPU
void Audio::Component::Log(DebugLevel::Level errorType, const char* funcname, const char* pchComType, const char* format, ...) const
{
	char buffer[ 1024 ];
	va_list arglist;
	va_start( arglist, format );
	vsprintf_s( buffer, 1024, format, arglist );
	va_end( arglist );
	if (errorType == DebugLevel::Error)	AUDIO_SYS_ERROR("'%s' component: '%s' in object '%s', %s. In %s", pchComType, Name(), GetObject()->Name(), buffer, funcname); else
	if (errorType == DebugLevel::Warn)	AUDIO_SYS_WARN ("'%s' component: '%s' in object '%s', %s. In %s", pchComType, Name(), GetObject()->Name(), buffer, funcname); else
	if (errorType == DebugLevel::Fatal)	AUDIO_SYS_FATAL("'%s' component: '%s' in object '%s', %s. In %s", pchComType, Name(), GetObject()->Name(), buffer, funcname);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	GetNumDSPSlots
////////////////////////////////////////////////////////////////////////////////
U16 Audio::AudioComponent::GetNumDSPSlots()
{
	U32* aEffects = (U32*)m_pObject->GetInitData(m_uEffectOffset);

	U16 uTotal = 0;
    for (U32 uEffect = 0; uEffect < m_uNumEffects; uEffect++)
		uTotal += m_pObject->GetComponent(aEffects[uEffect])->DSPEffectCount();

	return uTotal;
}

////////////////////////////////////////////////////////////////////////////////
//	ConfigureAudio
////////////////////////////////////////////////////////////////////////////////
#if !defined (PLATFORM_SPU)
void Audio::MixerComponent::ConfigureAudio(Instance* pInstance)
{
	//Com* aEffects = (Com*)m_pObject->GetInitData(m_uEffectOffset);
	U32* aEffects = (U32*)m_pObject->GetInitData(m_uEffectOffset);
	U8* pStateData = pInstance->GetStateData();
	U8* pCoreData = pInstance->GetCoreData();
    Core::Mixer* pMixer = (Core::Mixer*)&pCoreData[m_uMixer];

    //  do effects..
	//U16 uSlot = 0;
 //   for (U32 uEffect = 0; uEffect < m_uNumEffects; uEffect++)
 //   {
	//	for (U32 uCount = 0; uCount < m_pObject->GetComponent(aEffects[uEffect])->DSPEffectCount(); uCount++)
	//	{
	//		pMixer->SetEffect(uSlot + uCount, m_pObject->GetComponent(aEffects[uEffect])->DSPEffectType(pStateData, uCount), 
	//									m_pObject->GetComponent(aEffects[uEffect])->DSPEffectUnit(pStateData, uCount));
	//	}

	//	uSlot += m_pObject->GetComponent(aEffects[uEffect])->DSPEffectCount();
 //   }
	U32 uSlot = 0;
    for (U32 uEffect = 0; uEffect < m_uNumEffects; uEffect++)
    {
        if (aEffects[uEffect] != sUnused)
		{
			U32 uNumUnits = m_pObject->GetComponent(aEffects[uEffect])->DSPEffectCount( );

			for (U32 uUnit = 0; uUnit < uNumUnits; uUnit++)
			{
				if (pMixer)
				{
					pMixer->SetEffect(uSlot + uUnit, m_pObject->GetComponent(aEffects[uEffect])->DSPEffectType(pStateData, uUnit), 
											m_pObject->GetComponent(aEffects[uEffect])->DSPEffectUnit(pStateData, uUnit));
				}

				uSlot += uNumUnits;
			}
		}
    }

    //  do inputs
    for (U32 uInput = 0; uInput < m_uNumInputs; uInput++)
    {
        //if (aInputs[uInput].sInputCom != -1)
        //{
        //    if ((m_pObject->GetComponentFlags(aInputs[uInput].sInputCom) & flagMono) &&
        //        (m_pObject->GetComponentFlags(aInputs[uInput].sInputCom) & flagAudioOutput))    //  TODO: others
        //        pMixer->SetInput(uInput, m_pObject->GetComponent(aInputs[uInput].sInputCom)->AudioOutType(pInstance), m_pObject->GetComponent(aInputs[uInput].sInputCom)->AudioOutOffset(pInstance));
        //    else
        //        COMPONENT_ERROR("input is not mono audio");
        //}
		Com comInput = GetInput( m_pObject->GetInitData( m_uInitOffset ), uInput );
        if (comInput != -1)
		{
			//Component* pCom = m_pObject->GetComponent( aInputs[ uInput ].sInputCom );
			Component* pCom = m_pObject->GetComponent( comInput );

			Assert ( pCom->IsAudio( ) );

			AudioComponent* pAudioCom = (AudioComponent*)pCom;

            pMixer->SetInput( uInput, pAudioCom->AudioOutType( pInstance ), pAudioCom->AudioOutOffset( pInstance ) );
		}
    }
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	GetSurroundVolumes
////////////////////////////////////////////////////////////////////////////////
void Audio::MixerComponent::GetSurroundVolumes( PanningMode eMode, SurroundMode eSurround, F32 fAzimuth, F32 fElevation, F32 fDistance, F32 fWidth, F32 fLevel, Speakers& sVolumes )
{
#if !defined CFG_GOLD && !defined PLATFORM_SPU
	if( g_bAudioForceAmbisonics )
		eSurround = eAmbisonics;
#endif

	//	zero volumes
	memset( &sVolumes, 0, sizeof(Speakers) );

	if( eMode == eMono )
	{
		//	mono has no panning
		sVolumes.fCentre = fLevel;
	}
	else
	{
		//	deal with negative distances by rotating azimuth
		if( fDistance < 0.f )
		{
			fDistance *= -1.f;
			fAzimuth += 1.f;
		}

		//  deal with elevation angle wrap around
		fElevation = Maths::MakeMinusOneToOne( fElevation );

		//	rotate azimuth for elevations in the rear hemisphere
		if( fElevation < -0.5f )
		{
			fElevation = -1.f - fElevation;
			fAzimuth += 1.f;
		}
		else if( fElevation >= 0.5f )
		{
			fElevation = 1.f - fElevation;
			fAzimuth += 1.f;
		}

		//  deal with azimuth angle wrap around
		fAzimuth = Maths::MakeMinusOneToOne( fAzimuth );

		//  scale distance by system speaker distance for correct behaviour within speaker distance
		fDistance /= System::Get()->GetSpeakerDistance();

		//	project 3d distance onto horizontal plane
		fDistance *= Maths::Cos( fElevation * Maths::fPi );

		//  clamp distance for inner/outer cross fade
		fDistance = Maths::Clamp( fDistance, 0.f, 1.f );

		//  clamp width for inner/outer cross fade
		fWidth = Maths::Max( fWidth, 0.f );

		//	create array for speaker volumes
		F32 afLevels[ MAX_NUM_SURROUND_SPEAKERS + 1 ];
		memset( afLevels, 0, ( MAX_NUM_SURROUND_SPEAKERS + 1 ) * sizeof(F32) );

		F32* afAngles = System::Get()->GetSpeakerAngles();
		U32 uNumChannels = System::Get()->GetNumSpeakers();

		//	generate speaker levels (const power)
		if( eMode == eFivePointZero && eSurround == eAmbisonics )
			CreateAmbisonicPannerLevels( fAzimuth, afAngles, afLevels );
		else
		{
			if( fWidth == 0.f )
				CreatePairwisePannerLevels( fAzimuth, afAngles, afLevels, uNumChannels );
			else
				CreateWidePairwisePannerLevels( fAzimuth, fWidth, afAngles, afLevels, uNumChannels );
		}

		if( fDistance < 1.f )
		{
			//	inside speaker perimeter so generate volumes for opposite side and cross fade based on distance
			F32 afBackLevels[ MAX_NUM_SURROUND_SPEAKERS + 1 ];
			memset( afBackLevels, 0, ( MAX_NUM_SURROUND_SPEAKERS + 1 ) * sizeof(F32) );

			if( eMode == eFivePointZero && eSurround == eAmbisonics )
				CreateAmbisonicPannerLevels( Maths::MakeMinusOneToOne( fAzimuth + 1.f ), afAngles, afBackLevels );
			else
			{
				if( fWidth == 0.f )
					CreatePairwisePannerLevels( Maths::MakeMinusOneToOne( fAzimuth + 1.f ), afAngles, afBackLevels, uNumChannels );
				else
					CreateWidePairwisePannerLevels( Maths::MakeMinusOneToOne( fAzimuth + 1.f ), fWidth, afAngles, afBackLevels, uNumChannels );
			}

			//	get front + back ratios
			F32 fHalfDistance = 0.5f * fDistance;
			F32 fFront = 0.5f + fHalfDistance;
			F32 fBack = 0.5f - fHalfDistance;

			//	mix front + back levels, retain signs
			for ( U32 uChannel = 0; uChannel < uNumChannels; uChannel++ )
			{
				F32 fFrontSign = Maths::Select( afLevels[ uChannel ], 1.f, -1.f );
				F32 fBackSign = Maths::Select( afBackLevels[ uChannel ], 1.f, -1.f );
				F32 fSqrLevel = fFrontSign * fFront * afLevels[ uChannel ] * afLevels[ uChannel ] + fBackSign * fBack * afBackLevels[ uChannel ] * afBackLevels[ uChannel ];
				F32 fOutSign = Maths::Select( fSqrLevel, 1.f, -1.f );
				afLevels[ uChannel ] = fOutSign * Maths::Sqrt( fOutSign * fSqrLevel );
			}
		}

		//	set output volumes
		if( uNumChannels >= 7 )
		{
			sVolumes.fBackLeft = fLevel * afLevels[ uNumChannels - 3 ];
			sVolumes.fBackRight = fLevel * afLevels[ 3 ];
		}
		if( uNumChannels >= 5 )
		{
			sVolumes.fSurroundLeft = fLevel * afLevels[ uNumChannels - 2 ];
			sVolumes.fSurroundRight = fLevel * afLevels[ 2 ];
		}
		if( uNumChannels >= 3 )
		{
			sVolumes.fFrontLeft = fLevel * afLevels[ uNumChannels - 1 ];
			sVolumes.fFrontRight = fLevel * afLevels[ 1 ];
		}

		sVolumes.fCentre = fLevel * afLevels[ 0 ];

		//	if mode has no centre, fold it into fronts
		if( eMode == eStereo || eMode == eQuad )
			MakePhantomCentre( sVolumes );
	}
}

////////////////////////////////////////////////////////////////////////////////
//	CreateAmbisonicPannerLevels
//	-	uses 4th order ambisonic algorithm (Craven)
//	-	afOuterLevels must have 5 entries
////////////////////////////////////////////////////////////////////////////////
void Audio::MixerComponent::CreateAmbisonicPannerLevels( F32 fAzimuth, F32* afAngles, F32* afOuterLevels )
{
// disable on spu for now, to save memory
#ifndef PLATFORM_SPU
	//	make azimuth 0->2 - [nickb] hack, flipped L/R
	fAzimuth = Maths::Select( -fAzimuth, -fAzimuth, -fAzimuth + 2.f );

	//	4th order ambisonic panner for ITU 5.1 (Craven)
	for( U32 uChannel = 0; uChannel < 5; uChannel++ )
	{
		//	pre-scale azimuth to actual (non-ITU) speaker positions for each segment
		if( fAzimuth >= afAngles[ uChannel ] && fAzimuth <= afAngles[ uChannel + 1 ] )
		{
			//	azimuth is in this speaker segment, calc new azimuth for ITU 5.1
			F32 fRatio = ( fAzimuth - afAngles[ uChannel ] ) / ( afAngles[ uChannel + 1 ] - afAngles[ uChannel ] );

			//	adjust azimuth
			switch (uChannel)
			{
			case 0: fAzimuth = fRatio * ( 30.f / 180.f ); break;
			case 1: fAzimuth = ( 30.f / 180.f ) + fRatio * ( 90.f / 180.f ); break;
			case 2: fAzimuth = ( 120.f / 180.f ) + fRatio * ( 120.f / 180.f ); break;
			case 3: fAzimuth = ( 240.f / 180.f ) + fRatio * ( 90.f / 180.f ); break;
			case 4: fAzimuth = ( 330.f / 180.f ) + fRatio * ( 30.f / 180.f ); break;
			}
		}
	}

	//	temp vars
	F32 fAzimuthRads = Maths::fPi * fAzimuth;

	F32 fSin = Maths::Sin( fAzimuthRads );
	F32 fSin2 = Maths::Sin( 2.f * fAzimuthRads );
	F32 fSin3 = Maths::Sin( 3.f * fAzimuthRads );
	F32 fSin4 = Maths::Sin( 4.f * fAzimuthRads );

	F32 fCos = Maths::Cos( fAzimuthRads );
	F32 fCos2 = Maths::Cos( 2.f * fAzimuthRads );
	F32 fCos3 = Maths::Cos( 3.f * fAzimuthRads );
	F32 fCos4 = Maths::Cos( 4.f * fAzimuthRads );

	//	generate output levels
	afOuterLevels[ 0 ] = 0.10492f + 0.33223f * fCos + 0.26500f * fCos2 + 0.16902f * fCos3 + 0.05978f * fCos4;	//	C

	afOuterLevels[ 1 ] = 0.16656f + 0.24162f * fCos - 0.27215f * fSin - 0.05322f * fCos2 - 0.22189f * fSin2
		- 0.08418f * fCos3 - 0.05939f * fSin3 - 0.06994f * fCos4 - 0.08435f * fSin4;							//	FR

	afOuterLevels[ 2 ] = 0.35579f - 0.35965f * fCos - 0.42548f * fSin - 0.06361f * fCos2 + 0.11778f * fSin2
		+ 0.00012f * fCos3 + 0.04692f * fSin3 + 0.02722f * fCos4 + 0.06146f * fSin4;							//	RR

	afOuterLevels[ 3 ] = 0.35579f - 0.35965f * fCos + 0.42548f * fSin - 0.06361f * fCos2 - 0.11778f * fSin2
		+ 0.00012f * fCos3 - 0.04692f * fSin3 + 0.02722f * fCos4 - 0.06146f * fSin4;							//	RL

	afOuterLevels[ 4 ] = 0.16656f + 0.24162f * fCos + 0.27215f * fSin - 0.05322f * fCos2 + 0.22189f * fSin2
		- 0.08418f * fCos3 + 0.05939f * fSin3 - 0.06994f * fCos4 + 0.08435f * fSin4;							//	FL
#endif	//	#ifndef PLATFORM_SPU
}

////////////////////////////////////////////////////////////////////////////////
//	CreatePairwisePannerLevels
//	-	afOuterLevels must include space for wraparound centre channel (+1)
//	-	uses cosine curve panning algorithm
////////////////////////////////////////////////////////////////////////////////
void Audio::MixerComponent::CreatePairwisePannerLevels( F32 fAzimuth, F32* afAngles, F32* afOuterLevels, U32 uNumChannels )
{
	//	make azimuth 0->2
	fAzimuth = Maths::Select( fAzimuth, fAzimuth, fAzimuth + 2.f );

	//	regular cosine pair-wise panning
	for( U32 uChannel = 0; uChannel < uNumChannels; uChannel++ )
	{
		if( fAzimuth >= afAngles[ uChannel ] && fAzimuth <= afAngles[ uChannel + 1 ] )
		{
			//	in this segment
			F32 fSegWidth = ( afAngles[ uChannel + 1 ] - afAngles[ uChannel ] );
			F32 fRatio = ( fAzimuth - afAngles[ uChannel ] ) / fSegWidth;

			afOuterLevels[ uChannel ] = Maths::Cos( Maths::fHalfPi * fRatio );
			afOuterLevels[ uChannel + 1 ] = Maths::Sin( Maths::fHalfPi * fRatio );

			break;
		}
	}

	//	merge first and last centre levels (const power maintained as at one value is zero)
	afOuterLevels[ 0 ] += afOuterLevels[ uNumChannels ];
	afOuterLevels[ uNumChannels ] = 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	CreateWidePairwisePannerLevels
//	-	similar to Speaker-Placement Correction Amplitude Panning algorithm
////////////////////////////////////////////////////////////////////////////////
void Audio::MixerComponent::CreateWidePairwisePannerLevels( F32 fAzimuth, F32 fWidth, F32* afAngles, F32* afOuterLevels, U32 uNumChannels )
{
	//	make azimuth 0->2
	fAzimuth = Maths::Select( fAzimuth, fAzimuth, fAzimuth + 2.f );

	F32 fHalfWidth = 0.5f * fWidth;

	//	SPCAP style algorithm for wide sources
	F32 fStartAngle = fAzimuth - fHalfWidth;
	fStartAngle = Maths::Select( fStartAngle, fStartAngle, fStartAngle + 2.f );
	F32 fEndAngle = fAzimuth + fHalfWidth;
	fEndAngle = Maths::Select( fEndAngle - 2.f , fEndAngle - 2.f, fEndAngle );

	for( U32 uChannel = 0; uChannel < uNumChannels; uChannel++ )
	{
		F32 fSegWidth = ( afAngles[ uChannel + 1 ] - afAngles[ uChannel ] );

		//	for each speaker segment, does it start in this segment?
		if( fStartAngle >= afAngles[ uChannel ] && fStartAngle < afAngles[ uChannel + 1 ] )
		{
			//	range starts in this segment, does it end?
			if( fEndAngle >= afAngles[ uChannel ] && fEndAngle < afAngles[ uChannel + 1 ] )
			{
				//	ends in this segment
				F32 fStart = Maths::fHalfPi * ( fStartAngle - afAngles[ uChannel ] ) / fSegWidth;
				F32 fEnd = Maths::fHalfPi * ( fEndAngle - afAngles[ uChannel ] ) / fSegWidth;

				afOuterLevels[ uChannel ] += fSegWidth * ( Maths::Sin( fEnd ) - Maths::Sin( fStart ) );
				afOuterLevels[ uChannel + 1 ] += fSegWidth * ( -Maths::Cos( fEnd ) + Maths::Cos( fStart ) );
			}
			else
			{
				//	doesn't end in this segment, do remainder
				F32 fStart = Maths::fHalfPi * ( fStartAngle - afAngles[ uChannel ] ) / fSegWidth;

				afOuterLevels[ uChannel ] += fSegWidth * ( 1.f - Maths::Sin( fStart ) );
				afOuterLevels[ uChannel + 1 ] += fSegWidth * Maths::Cos( fStart );
			}
		}
		else
		{
			//	doesn't start in this segment, does it end?
			if( fEndAngle >= afAngles[ uChannel ] && fEndAngle < afAngles[ uChannel + 1 ] )
			{
				//	ends in this segment
				F32 fEnd = Maths::fHalfPi * ( fEndAngle - afAngles[ uChannel ] ) / fSegWidth;

				afOuterLevels[ uChannel ] += fSegWidth * Maths::Sin( fEnd );
				afOuterLevels[ uChannel + 1 ] += fSegWidth * ( 1.f - Maths::Cos( fEnd ) );
			}
			else
			{
				if( fEndAngle > fStartAngle )
				{
					//	trivial case
					if( fStartAngle < afAngles[ uChannel ] && fEndAngle >= afAngles[ uChannel + 1 ] )
					{
						//	segment is betweeen start and end
						afOuterLevels[ uChannel ] += fSegWidth;
						afOuterLevels[ uChannel + 1 ] += fSegWidth;
					}
				}
				else
				{
					//	wraparound case
					if( fStartAngle < afAngles[ uChannel == 0 ? uNumChannels : uChannel ] || fEndAngle >= afAngles[ uChannel + 1 ] )
					{
						//	segment is betweeen start and end
						afOuterLevels[ uChannel ] += fSegWidth;
						afOuterLevels[ uChannel + 1 ] += fSegWidth;
					}
				}
			}
		}
	}

	//	merge first and last centre levels
	afOuterLevels[ 0 ] += afOuterLevels[ uNumChannels ];
	afOuterLevels[ uNumChannels ] = 0.f;

	//	calc sum of squares (not yet const power)
	F32 fTotal = 0.f;
	for( U32 uChannel = 0; uChannel < uNumChannels; uChannel++ )
	{
		afOuterLevels[ uChannel ] *= afOuterLevels[ uChannel ];
		fTotal += afOuterLevels[ uChannel ];
	}

	//	normalize to constant power
	if( fTotal > 0.f )
	{
		for( U32 uChannel = 0; uChannel < uNumChannels; uChannel++ )
		{
			afOuterLevels[ uChannel ] /= fTotal;
			afOuterLevels[ uChannel ] = Maths::Sqrt( afOuterLevels[ uChannel ] );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	MakePhantomCentre
//	-	maintains centre image at 0 deg when L/R angles are not symetrical
////////////////////////////////////////////////////////////////////////////////
void Audio::MixerComponent::MakePhantomCentre( Speakers& sVolumes )
{
	//	half centre level (linear)
	const F32 fHalfCentreLevel = 0.5f * ( sVolumes.fCentre * sVolumes.fCentre );

	sVolumes.fCentre = 0.f;

	//	add centre to fronts (const power)
	sVolumes.fFrontLeft = Maths::Sqrt( sVolumes.fFrontLeft * sVolumes.fFrontLeft + fHalfCentreLevel );
	sVolumes.fFrontRight = Maths::Sqrt( sVolumes.fFrontRight * sVolumes.fFrontRight + fHalfCentreLevel );
}

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::CompressorComponent::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.Compressor");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
	Core::DSP::Compressor* pDSP = (Core::DSP::Compressor*)pInstance->GetCoreData( m_uDSP );

	//  get attack
	F32 fAttack = Maths::Max( GetOptionValue( pInstance, pInitData->optAttack ), 0.f );

    //  has changed
    if( pState->fAttack != fAttack )
    {
        pState->fAttack = fAttack;
		pDSP->SetParam( Core::DSP::Compressor::eAttack, fAttack );
    }

    //  get release
	F32 fRelease = Maths::Max( GetOptionValue( pInstance, pInitData->optRelease ), 0.f );

	//  has changed
    if( pState->fRelease != fRelease )
    {
		pState->fRelease = fRelease;
		pDSP->SetParam( Core::DSP::Compressor::eRelease, fRelease );
	}

    //  get threshold
	F32 fThreshold = Maths::Max( GetLevel( pInstance, pInitData->optThresholdLevel, pInitData->optThresholdGain ), 0.f );

	//  has changed
    if( pState->fThreshold != fThreshold )
    {
		pState->fThreshold = fThreshold;
		pDSP->SetParam( Core::DSP::Compressor::eThreshold, fThreshold );
	}

    //  get ratio
	F32 fRatio = Maths::Clamp( GetOptionValue( pInstance, pInitData->optRatio ), 0.1f, 10000.f );

	//  has changed
    if (pState->fRatio != fRatio)
    {
		pState->fRatio = fRatio;
		pDSP->SetParam( Core::DSP::Compressor::eRatio, fRatio );
	}

    //  get gain make up
	F32 fGain = GetLevel( pInstance, pInitData->optMakeUpLevel, pInitData->optMakeUpGain );

	//  has changed
    if( pState->fGain != fGain )
    {
		pState->fGain = fGain;
		pDSP->SetParam( Core::DSP::Compressor::eGain, fGain );
	}

	if ( pInitData->ctrlSideChainRMS.sCom != eUndefined )
	{
		//  get side chain rms
		F32 fSideChainRMS = GetControlValue( pInstance, pInitData->ctrlSideChainRMS );

		//  has changed
		if( pState->fSideChainRMS != fSideChainRMS )
		{
			pState->fSideChainRMS = fSideChainRMS;
			pDSP->SetParam( Core::DSP::Compressor::eSideChainRMS, fSideChainRMS );
		}
	}

	//	get current attenuation
	pState->fValue = pDSP->GetParam( Core::DSP::Compressor::eAttenuation );
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::CompressorComponent::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	switch(uOutput)
    {
    case ValueCtrl:
		return pState->fValue;
        break;
	default:			
		Assert(0);
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::CompressorComponent::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::Effect* pDSP = (Core::DSP::Effect*)pInstance->GetCoreData( m_uDSP );

	switch( uMessage )
    {
    case EnableMsg:
		pDSP->SetActive( true );
        break;

    case BypassMsg:
		pDSP->SetActive( false );
        break;

    default:
        Assert(0);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::FlangerComponent::Update( Instance* pInstance )
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.Flanger");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
	Core::DSP::Flanger* pDSP = (Core::DSP::Flanger*)pInstance->GetCoreData( m_uDSP );

    //  get delay
	F32 fDelay = GetOptionValue( pInstance, pInitData->optDelay );

    //  set delay
    if( pState->fDelay != fDelay )
    {
        pState->fDelay = fDelay;
		pDSP->SetParam( Core::DSP::Flanger::eDelay, fDelay );
    }

    //  get depth
	F32 fDepth = GetOptionValue( pInstance, pInitData->optDepth );

    //  set depth
    if( pState->fDepth != fDepth )
    {
        pState->fDepth = fDepth;
		pDSP->SetParam( Core::DSP::Flanger::eDepth, fDepth );
    }

    //  get rate
	F32 fRate = GetOptionValue( pInstance, pInitData->optRate );

    //  set rate
    if( pState->fRate != fRate )
    {
        pState->fRate = fRate;
		pDSP->SetParam( Core::DSP::Flanger::eRate, fRate );
    }

    //  get feedback
	F32 fFeedback = GetOptionValue( pInstance, pInitData->optFeedback );

    //  set feedback
    if( pState->fFeedback != fFeedback )
    {
        pState->fFeedback = fFeedback;
		pDSP->SetParam( Core::DSP::Flanger::eFeedback, fFeedback );
    }

    //  get wet
	F32 fWet = GetOptionValue( pInstance, pInitData->optWet );

    //  set wet
    if( pState->fWet != fWet )
    {
        pState->fWet = fWet;
		pDSP->SetParam( Core::DSP::Flanger::eWet, fWet );
    }

    //  get dry
	F32 fDry = GetOptionValue( pInstance, pInitData->optDry );

    //  set dry
    if( pState->fDry != fDry )
    {
        pState->fDry = fDry;
		pDSP->SetParam( Core::DSP::Flanger::eDry, fDry );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::FlangerComponent::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::Flanger* pDSP = (Core::DSP::Flanger*)pInstance->GetCoreData( m_uDSP );

	switch( uMessage )
    {
    case EnableMsg:
		pDSP->SetActive( true );
        break;

    case BypassMsg:
		pDSP->SetActive( false );
        break;

    case ResetMsg:
		pDSP->Reset( );
        break;

    default:
        Assert(0);
        break;
    }
}
