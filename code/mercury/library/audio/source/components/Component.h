#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#if defined AUDIO_STRING_HASH_COMPONENTS
#include "StringHash.h"
//#include "..\Object.h"
#endif

#define NEW_EVENT_CODE

//#define AMBISONICS_1ST_ORDER
#define AMBISONICS_2ND_ORDER
//#define AMBISONICS_3RD_ORDER

//void Audio::ComponentError(Audio::Component* pCom, Debug::DebugLevel errorType, const char* funcname, const char* format, ...);

#if defined AUDIO_STRING_HASH_COMPONENTS
extern Debug::Bool gbAudioLogComponentHashValues;
#endif

namespace Audio
{

	typedef U32 ixInit;
	typedef U32 ixState;
	typedef U32 ixCore;

	const ixCore ixMixerNone	= 0xffffffff;
	const ixCore ixDecoderNone	= 0xffffffff;
	const ixCore ixWaveNone		= 0xffffffff;

	#if !defined CFG_GOLD && !defined PLATFORM_SPU
	#define COMPONENT_FATAL(...) Audio::Component::Log(DebugLevel::Fatal, "", Type(), __VA_ARGS__)
	#define COMPONENT_ERROR(...) Audio::Component::Log(DebugLevel::Error, "", Type(), __VA_ARGS__)
	#define COMPONENT_WARN(...) Audio::Component::Log(DebugLevel::Warn, "", Type(), __VA_ARGS__)
	#else
	#define COMPONENT_FATAL(...)	do {} while (0)
	#define COMPONENT_ERROR(...)	do {} while (0)
	#define COMPONENT_WARN(...)		do {} while (0)
	#endif

////////////////////////////////////////////////////////////////////////////////
//	forward class defs
////////////////////////////////////////////////////////////////////////////////
    class Object;

	namespace Core
	{
		class IGenerator;
		class Decoder;

		namespace DSP { class Effect; }
	}

#ifndef XML_TO_DAT
	typedef S32 StrOffset;
	typedef U16 ArraySize;
	typedef F32 AudioFloat;
	typedef S32 AudioInt;
	typedef S16 Com;
	typedef U16 Out;
	typedef U16 Msg;
#else
	class AudioS32Convertor
	{
	public:
		AudioS32Convertor();
		AudioS32Convertor(S32 s32);
//		S32* operator =(S32 s32);
//		bool operator ==(AudioS32Convertor audioS32Convertor);
//		AudioS32Convertor Audio::AudioS32Convertor::operator =(const Audio::AudioS32Convertor audioS32Convertor);
//		AudioS32Convertor* operator =(S32 s32);
//		AudioS32Convertor &operator =(S32 s32);
		operator S32();

	private:
		S32 m_s32;
	};
	class AudioF32Convertor
	{
	public:
		AudioF32Convertor();
		AudioF32Convertor(F32 f32);
		F32* operator =(F32 f32);
		bool operator ==(AudioF32Convertor audioF32Convertor);
		bool operator <=(F32 f32);
		operator F32 ();

	private:
		F32 m_f32;
	};
	class AudioS16Convertor
	{
	public:
		AudioS16Convertor();
		AudioS16Convertor(S16 s16);
		S16* operator =(S16 s16);
//		bool operator ==(AudioS16Convertor audioS16Convertor);
		operator S16();

	private:
		S16 m_s16;
	};
	class AudioU16Convertor
	{
	public:
		AudioU16Convertor();
		AudioU16Convertor(U16 u16);
		U16* operator =(U16 u16);
		bool operator ==(AudioU16Convertor audioU16Convertor);
		U16 operator *(U16 u16Operand);
		bool operator <(U16 u16Operand);
		void operator ++(int);
		operator AudioS32Convertor();
		U16 GetStoredValue() { return m_u16; }

	private:
		U16 m_u16;
	};
	typedef AudioS32Convertor StrOffset;
	typedef AudioU16Convertor ArraySize;
	typedef AudioF32Convertor AudioFloat;
	typedef AudioS32Convertor AudioInt;
	typedef AudioS16Convertor Com;
	typedef AudioU16Convertor Out;
	typedef AudioU16Convertor Msg;
#endif

	const Com sUnused = (Com)-1;

////////////////////////////////////////////////////////////////////////////////
//	Component
////////////////////////////////////////////////////////////////////////////////
    class Component
    {

		friend class Object;

    public:
        enum Types
        {
			eUndefined = -1,

            //  (user interface)
            eParameterIn,			//0
            eParameterOut,
            eActionIn,
            eActionOut,
            eVectorIn,
            eVectorOut,				//5

            //  (object/object interface)
            eControlInput,
            eControlOutput,
            eEventInput,
            eEventOutput,
            eCoordinateInput,		//10
            eCoordinateOutput,
            eMonoAudioInput,
            eMonoAudioOutput,
            eStereoAudioInput,
            eStereoAudioOutput,		//15
            eSurroundAudioInput,
            eSurroundAudioOutput,
			eSurroundLFEAudioInput,
			eSurroundLFEAudioOutput,

            //  (variables)
            eVariable,				//20

            //  (control functions)
            ePiecewiseLinearFunction,
            ePiecewiseCubicFunction,
            eSource,
            eRolloff,
            eCone,					//25
            eLowPass,
            eHighPass,
            eTimer,
            eMetronome,
            eRotarySwitch,			//30
            eCrossFade,
            eRandom,
			eSelector,
			eStateMachine,
			eQuantize,				//35
            eSum,
            eProduct,
            eAbsolute,
            eNegate,
            eExponent,				//40
            eMax,
            eMin,
			eInvert,
            eGradient,
            eSpring,				//45
            eCosine,
            eSine,
            eRelay,
            eToggle,
            ePeakHold,				//50
            eSampleAndHold,
            eEnvelope,
            eLowFrequencyOscillator,
			eGateSwitch,
            eArray,					//55
			eFraction,

            //  (graphs)
            ePiecewiseLinearGraph,
            ePiecewiseCubicGraph,

            //  (audio generators)
            eMonoOneShotSound,
            eStereoOneShotSound,	//60
            eMonoOneShotSoundTable,
            eStereoOneShotSoundTable,
            eMonoLoopingSound,
            eStereoLoopingSound,
            eMonoStreamingSound,	//65
            eStereoStreamingSound,
            eOscillator,

            //  (audio processors)
            eMonoToMonoMixer,
            eMonoToStereoMixer,
            eMonoToSurroundMixer,	//70
			eMonoToSurroundLFEMixer,
            eStereoToMonoMixer,
            eStereoToStereoMixer,
			eStereoToQuadMixer,
            eStereoToSurroundMixer,	//75
			eQuadToSurroundMixer,
            eSurroundToMonoMixer,
            eSurroundToStereoMixer,
            eSurroundToSurroundMixer,
			eSurroundToSurroundLFEMixer,//80
			eSurroundLFEToSurroundLFEMixer,
            eMonoToMonoSpectralMixer,
            eMonoToStereoSpectralMixer,
            eMonoToSurroundSpectralMixer,
            eStereoToMonoSpectralMixer,	//85
            eStereoToStereoSpectralMixer,
            eStereoToSurroundSpectralMixer,
            eSurroundToMonoSpectralMixer,
            eSurroundToStereoSpectralMixer,
            eSurroundToSurroundSpectralMixer,	//90
            eMonoSpectralToMonoMixer,
            eMonoSpectralToStereoMixer,
            eMonoSpectralToSurroundMixer,
            eStereoSpectralToMonoMixer,
            eStereoSpectralToStereoMixer,//95
            eStereoSpectralToSurroundMixer,
            eSurroundSpectralToMonoMixer,
            eSurroundSpectralToStereoMixer,
            eSurroundSpectralToSurroundMixer,
            eMonoSpectralToMonoSpectralMixer,//100
            eMonoSpectralToStereoSpectralMixer,
            eMonoSpectralToSurroundSpectralMixer,
            eStereoSpectralToMonoSpectralMixer,
            eStereoSpectralToStereoSpectralMixer,
            eStereoSpectralToSurroundSpectralMixer,//105
            eSurroundSpectralToMonoSpectralMixer,
            eSurroundSpectralToStereoSpectralMixer,
            eSurroundSpectralToSurroundSpectralMixer,
 
            //  (time-domain effects)
            eMonoGain,
            eStereoGain,//110
            eSurroundGain,
            eMonoDelay,
            eStereoDelay,
            eSurroundDelay,
            eMonoPeakLevel,
            eStereoPeakLevel,
            eSurroundPeakLevel,
            eMonoRMSLevel,
            eStereoRMSLevel,
            eSurroundRMSLevel,
            eMonoSaturator,
            eStereoSaturator,
            eSurroundSaturator,
			eMonoDistortion,
			eStereoDistortion,
			eSurroundDistortion,
            eMonoPhaser,
            eStereoPhaser,
            eSurroundPhaser,
            eMonoConvolution,
            eStereoConvolution,
            eSurroundConvolution,
            eMonoFeedbackDelay,
            eStereoFeedbackDelay,
            eSurroundFeedbackDelay,
            eMonoVariableDelay,
            eStereoVariableDelay,
            eSurroundVariableDelay,
            eMonoAllPass,
            eStereoAllPass,
            eSurroundAllPass,
            eMonoLowPass,
            eStereoLowPass,
            eSurroundLowPass,
            eMonoHighPass,
            eStereoHighPass,
            eSurroundHighPass,
            eMonoResonantLowPass,
            eStereoResonantLowPass,
            eSurroundResonantLowPass,
            eMonoResonantHighPass,
            eStereoResonantHighPass,
            eSurroundResonantHighPass,
			eMono24dBResonantLowPass,
			eStereo24dBResonantLowPass,
			eSurround24dBResonantLowPass,
            eMonoLowShelf,
            eStereoLowShelf,
            eSurroundLowShelf,
            eMonoHighShelf,
            eStereoHighShelf,
            eSurroundHighShelf,
            eMonoPeakEQ,
            eStereoPeakEQ,
            eSurroundPeakEQ,
            eMonoCompressor,
            eStereoCompressor,
            eSurroundCompressor,
			eMonoLipSync,
			eStereoPitchShift,

            //  (frequency-domain effects)
            eMono2BandCompressor,
            eStereo2BandCompressor,
            eSurround2BandCompressor,
            eMono3BandCompressor,
            eStereo3BandCompressor,
            eSurround3BandCompressor,
            eMono4BandCompressor,
            eStereo4BandCompressor,
            eSurround4BandCompressor,
            eMonoGraphicEQ,
            eStereoGraphicEQ,
            eSurroundGraphicEQ,
            eMonoGraphicPhaseShift,
            eStereoGraphicPhaseShift,
            eSurroundGraphicPhaseShift,
            eMonoSpectrumAnalyser,
            eStereoSpectrumAnalyser,
            eSurroundSpectrumAnalyser,

            //  (scripts)
            eScript,

			// vector-based components
			eVector,
			eVectorNearestPointOnLine,
			eDopplerShift,
			
			// debug stuff
			eDebugPrint,

			eSequence,
			eMonoSpectralReverb,

			eMonoToAmbisonicsMixer,
			eAmbisonicsToSurroundMixer,

			eImpactFilter,
			eMessageDelay,
			eMonoSpectralEnvelope,
			eStereoSpectralEnvelope,
            eMonoFlanger,
            eStereoFlanger,
            eSurroundFlanger,
            eMonoBitCrusher,
            eStereoBitCrusher,
            eSurroundBitCrusher,
            eMonoRingModulator,
            eStereoRingModulator,
            eSurroundRingModulator,
            eMonoOscilloscope,
            eStereoOscilloscope,
            eSurroundOscilloscope,
            eMonoSend,
            eStereoSend,
            eSurroundSend,

            eMonoReturn,
            eStereoReturn,
            eSurroundReturn,

            NumComponents
        };

		static const U32 flagParameterIn                        = 0x00000001;
        static const U32 flagParameterOut                       = 0x00000002;
		static const U32 flagActionIn                           = 0x00000004;
		static const U32 flagActionOut                          = 0x00000008;
		static const U32 flagVectorIn                           = 0x00000010;
		static const U32 flagVectorOut                          = 0x00000020;
		static const U32 flagControl                            = 0x00000040;
		static const U32 flagEvent                              = 0x00000080;
		static const U32 flagVector                             = 0x00000100;
		static const U32 flagVariable							= 0x00000200;
		static const U32 flagGraph                              = 0x00000400;
		static const U32 flagMono                               = 0x00000800;
		static const U32 flagStereo                             = 0x00001000;
		static const U32 flagQuad								= 0x00002000;
		static const U32 flagSurround                           = 0x00004000;
		static const U32 flagSurroundLFE                        = 0x00008000;
		static const U32 flagAmbisonics                         = 0x00010000;
		static const U32 flagAudioOutput                        = 0x00020000;
		static const U32 flagTimeEffect                         = 0x00040000;
		static const U32 flagFreqEffect                         = 0x00080000;
		static const U32 flagSound                              = 0x00100000;
		static const U32 flagLooping                            = 0x00200000;
		static const U32 flagInputConnector                     = 0x00400000;
		static const U32 flagOutputConnector                    = 0x00800000;
		static const U32 flagControlConnector                   = 0x01000000;
		static const U32 flagEventConnector                     = 0x02000000;
		static const U32 flagAudioConnector                     = 0x04000000;
		static const U32 flagVectorConnector                    = 0x08000000;
		static const U32 flagUpdater                            = 0x10000000;
		static const U32 flagTransmitter                        = 0x20000000;
		static const U32 flagReciever                           = 0x40000000;
		static const U32 flagScript								= 0x80000000;
 
        struct Control
        {
		    Com sCom;
		    Out uOut;
        };

        struct Option
        {
		    Control ctrl;
		    AudioFloat fVal;
        };

        struct Event
        {
		    Com sCom;
		    Msg uMsg;
        };

    public:
#ifndef PLATFORM_SPU
		static void* operator new(size_t sz);
		static void* operator new(size_t sz, void* pHere);
#ifdef COMPONENTS_USE_ALLOCATOR
		static void* operator new( size_t sz, IndexedAllocator* pAlloc );
		static void operator delete( void* ptr, IndexedAllocator* pAlloc );
#endif
		static void operator delete(void* ptr);
#endif	//	#ifndef PLATFORM_SPU

    public:
        Component(Object* pObject)
        {
#if defined AUDIO_STRING_HASH_COMPONENTS
			StaticAssert( sizeof( StringHash ) == sizeof( U32 ), SIZEOF_STRINGHASH_IS_NOT_S32 );
#endif
			m_pchName = NULL;
            m_pObject = pObject;
            m_uInitOffset = (ixInit)-1;
            m_uStateOffset = (ixState)-1;
        }
        audio_virtual ~Component();

#if !defined (PLATFORM_SPU)
		template <typename T>
		T* PreInit(void* pData);
//		{
//			m_uInitOffset = (U8*)pData - (U8*)m_pObject->GetInitData();
//			T* pInitData = (T*)m_pObject->GetInitData(m_uInitOffset);
//
//			m_pchName = m_pObject->GetString(pInitData->strName);
//
//#if defined AUDIO_STRING_HASH_COMPONENTS 
//			m_nameHash = StringHash(m_pchName);
//
//			if (gbAudioLogComponentHashValues)
//				AUDIO_SYS_DEBUG("Hashing %s->%s as 0x%x\n", m_pObject->Name(), m_pchName, m_nameHash.GetValue());
//#endif
//
//			return (T*)(pInitData);
//			//return (T*)(NULL);
//		}
        audio_virtual void* Init(void* pData) = 0;
        audio_virtual U32 ContextSize();
        audio_virtual void InitContext(Instance* pInstance) = 0;
#endif
private:
#if !defined (PLATFORM_SPU)
		audio_virtual void SetValueDirect(Instance* pInstance, F32 fValue);
        audio_virtual void DoMessage(Instance* pInstance, U32 uMessage);
#endif
public:
#if !defined (PLATFORM_SPU)
        audio_virtual U32 GetNumMessages() { return StaticGetNumMessages(); }
		audio_virtual void ProcessConnection(Instance* pInstance);
#endif
		static U32 StaticGetNumMessages() { return 0; }

#ifndef CFG_GOLD
		audio_virtual void DumpProcessFlags( Instance* pInstance );
#endif

private:
#if !defined (PLATFORM_SPU)
        audio_virtual void Update(Instance* pInstance);

		audio_virtual void Generate(Instance* pInstance);
#endif

public:
#if !defined (PLATFORM_SPU)
		audio_virtual void Release() { }
        audio_virtual void ReleaseContext(Instance* pInstance) {}
#endif

#if !defined (PLATFORM_SPU)
        audio_virtual F32 Value(Instance* pInstance, U32 uOutput = 0);

        audio_virtual void Coords(Instance* pInstance, Maths::Vector* pVector);
#endif

#ifndef SPU_MEMORYSAVER
        audio_virtual const char* Type() const;
#endif
		inline U32 TypeId() { return *((U32*)this - 1); }

public:
#ifndef PLATFORM_SPU
        audio_virtual const U32 Flags()
        {
            COMPONENT_ERROR("Component::Flags() called");
            return 0;
        }
#endif

private:
        audio_virtual void Execute(Instance* pInstance);
public:
        //  Get core dsp effect	//	class AudioEffectComponent
		audio_virtual U32 DSPEffectUnit(U8* pStateData, U32 uIndex);
		audio_virtual U32 DSPEffectType(U8* pStateData, U32 uIndex);
		audio_virtual U32 DSPEffectCount( );

#if !defined (PLATFORM_SPU)
		audio_virtual U32 CoreMemory(U32 uOffset) { return uOffset; }

        inline U32 SetStateOffset(U32 uOffset)
        {
            m_uStateOffset = uOffset;
            return uOffset + ContextSize();
        }
#endif

		inline const char* Name() const { return m_pchName; }

#if defined AUDIO_STRING_HASH_COMPONENTS
		const StringHash NameHash() const { return m_nameHash; }
#endif
        inline Object* GetObject() const { return m_pObject; }

		F32 GetOptionValue( Instance* pInstance, Option& opt );
		F32 GetControlValue( Instance* pInstance, Control& ctrl );
		void DoEvent( Instance* pInstance, Event& ev );

#ifdef PLATFORM_SPU
		void SetObject(Audio::Object* pObj) { m_pObject = pObj; }
#endif

#if defined(ENABLE_DEBUG_EXTENSIONS)
		audio_virtual IDBSource* DBSource() { return NULL; }
		audio_virtual IDBMixer* DBMixer() { return NULL; }
		audio_virtual IDBOut* DBOut() { return NULL; }
#endif

#if !defined CFG_GOLD && !defined PLATFORM_SPU
		void Log(DebugLevel::Level errorType, const char* pchFunc, const char* pchComType, const char* format, ...) const;
#endif

		audio_virtual bool IsAudio() { return false; }

		F32 GetLevel( Instance* pInstance, Option& optLin, Option& optLog );
		F32 GetFrequency( Instance* pInstance, Option& optLin, Option& optLog );

    protected:
#ifdef PLATFORM_SPU
		U32 u32VfTablePad;
#endif
        const char* m_pchName;

#if defined AUDIO_STRING_HASH_COMPONENTS
		StringHash m_nameHash;
#else
		U32 u32HashPad;
#endif

        Object* m_pObject;
        ixInit m_uInitOffset;
        ixState m_uStateOffset;
    };

////////////////////////////////////////////////////////////////////////////////
//	AudioComponent
////////////////////////////////////////////////////////////////////////////////
	class AudioComponent : public Component
	{
	public:
		AudioComponent( Object* pObject ) : Component( pObject ) {}

		//	TODO: move these to a new audio base component
//		F32 GetLevel( Instance* pInstance, Option& optLin, Option& optLog );
//		F32 GetFrequency( Instance* pInstance, Option& optLin, Option& optLog );

#if defined (AUDIO_ENABLE_PROFILING)
		audio_virtual U16 GetNumInputs() { return 0; }
		audio_virtual Com GetInputCom(U32 uIndex) { return -1; }
		audio_virtual bool IsActive(Instance* pInstance) { return false; }
		audio_virtual void SetMute(Instance* pInstance, bool bMuted) {}
		audio_virtual Instance* GetOtherInstance(Instance* pThisInstance) { return NULL; }
		audio_virtual Com GetOtherComponent(Instance* pThisInstance) { return -1; }
#endif

        //  Get core generator
		audio_virtual const U32 AudioOutType(Instance* pInstance) const;
		audio_virtual const U32 AudioOutOffset(Instance* pInstance) const;

		audio_virtual bool IsAudio() { return true; }

#if !defined (PLATFORM_SPU)
		audio_virtual void ConfigureAudio(Instance* pInstance) {};
#endif

	protected:
		audio_virtual Com GetInput( U8* pInitData, U32 uInput ) { return -1; }
		audio_virtual U32 GetNumInputs( U8* pInitData ) { return 0; }
		U16 GetNumDSPSlots();

    protected:
        ixInit m_uEffectOffset;

		U16 m_uNumEffects;
	};

////////////////////////////////////////////////////////////////////////////////
//	SoundComponent
////////////////////////////////////////////////////////////////////////////////
	class SoundComponent : public AudioComponent
	{
	public:
		static const S32 iMinVolume = -60;

        struct Flags
        {
			U32 nAlwaysOn : 1;
			U32 nPad : 31;
        };

	public:
		SoundComponent( Object* pObject ) : AudioComponent( pObject )
		{
#ifdef AUDIO_NEW_DECODERS
			m_pDecoder = NULL;
#endif // AUDIO_NEW_DECODERS
		}

#ifdef AUDIO_NEW_DECODERS
	protected:
		Audio::Core::DecoderBase* m_pDecoder;
#endif // AUDIO_NEW_DECODERS
	};

////////////////////////////////////////////////////////////////////////////////
//	MixerComponent
////////////////////////////////////////////////////////////////////////////////
	class MixerComponent : public AudioComponent
	{
	public:
		struct Speakers	//	TODO: move to System
		{
			F32 fCentre;
			F32 fFrontLeft;
			F32 fFrontRight;
			F32 fSurroundLeft;
			F32 fSurroundRight;
			F32 fBackLeft;
			F32 fBackRight;
		};

		enum PanningMode
		{
			eMono,
			eStereo,
			eStereoPlusCentre,
			eQuad,
			eFivePointZero,
			eSevenPointZero,

			eNumPanningModes,
		};

		//	Flag bitfield enums
		enum SurroundMode
		{
			eUndefinedSurroundMode = -1,
			ePairwise,
			eAmbisonics
		};

		enum Domain
		{
			eUndefinedDomain = -1,
			eTime,
			eFrequency
		};

		enum AlwaysOn
		{
			eUndefinedAlwaysOn = -1,
			eFalse,
			eTrue
		};

        struct Flags
        {
			static const U32 nSurroundModeBits = 2;
			static const U32 nDomainBits = 1;
			static const U32 nAlwaysOnBits = 1;
			static const U32 nPadBits = 28;

			union
			{
				struct
				{
					U32 nSurroundMode : nSurroundModeBits;
					U32 nDomain : nDomainBits;
					U32 nAlwaysOn : nAlwaysOnBits;
					U32 nPad : nPadBits;
				};

				U32 uFlags;
			};
        };

        enum Controls
        {
            ActiveCtrl
        };

	public:
		MixerComponent( Object* pObject ) : AudioComponent( pObject )
		{
			m_uMixer = (U32)-1;
			m_uEffectOffset = (U32)-1;
			m_uInputOffset = (U32)-1;
			m_uNumInputs = 0;
			m_uNumEffects = 0;
		}

#if !defined (PLATFORM_SPU)
        void ConfigureAudio( Instance* pInstance );
#endif

        static S32 GetControl( const char* pchControl )
        {
            if (strcmp(pchControl, "Active") == 0)
                return 0;
            else
                return -1;
        }

		static SurroundMode GetSurroundMode( const char* pchName )
		{
			if( strcmp( pchName, "Pairwise" ) == 0 )
				return ePairwise;
			else if( strcmp( pchName, "Ambisonics" ) == 0 )
				return eAmbisonics;
			else
				return eUndefinedSurroundMode;
		}

		static Domain GetDomain( const char* pchName )
		{
			if( strcmp( pchName, "Time" ) == 0 )
				return eTime;
			else if( strcmp( pchName, "Frequency" ) == 0 )
				return eFrequency;
			else
				return eUndefinedDomain;
		}

		static AlwaysOn GetAlwaysOn( const char* pchName )
		{
			if( strcmp( pchName, "False" ) == 0 )
				return eFalse;
			else if( strcmp( pchName, "True" ) == 0 )
				return eTrue;
			else
				return eUndefinedAlwaysOn;
		}

	protected:
		void GetSurroundVolumes( PanningMode eMode, SurroundMode eSurround, F32 fAzimuth, F32 fElevation, F32 fDistance, F32 fWidth, F32 fLevel, Speakers& sVolumes );

		void CreateAmbisonicPannerLevels( F32 fAzimuth, F32* afAngles, F32* afOuterLevels );
		void CreatePairwisePannerLevels( F32 fAzimuth, F32* afAngles, F32* afOuterLevels, U32 uNumChannels );
		void CreateWidePairwisePannerLevels( F32 fAzimuth, F32 fWidth, F32* afAngles, F32* afOuterLevels, U32 uNumChannels );
		void MakePhantomCentre( Speakers& sVolumes );

    protected:
        ixInit m_uInputOffset;
		ixCore m_uMixer;

		U16 m_uNumInputs;
	};

////////////////////////////////////////////////////////////////////////////////
//	CompressorComponent
////////////////////////////////////////////////////////////////////////////////
	class CompressorComponent : public Component
	{

	public:
        CompressorComponent(Object* pObject) : Component(pObject)
        {
        }

		struct Data
        {
			StrOffset strName;
			Control ctrlSideChainRMS;
            Option optAttack;
            Option optRelease;
            Option optThresholdGain;
            Option optThresholdLevel;
            Option optRatio;
            Option optMakeUpGain;
            Option optMakeUpLevel;
        };

        struct State
        {
			F32 fSideChainRMS;
            F32 fAttack;
            F32 fRelease;
            F32 fThreshold;
            F32 fRatio;
            F32 fGain;
			F32 fValue;
        };

		enum Controls
        {
            ValueCtrl,

            NumControls
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,

            NumMessages
        };

		void Update(Instance* pInstance);
		F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage( Instance* pInstance, U32 uMessage );

    protected:
		ixCore m_uDSP;
	};

///////////////////////////////////////////////////////////////////////////////
//	FlangerComponent
////////////////////////////////////////////////////////////////////////////////
    class FlangerComponent : public Component
    {
    public:
        FlangerComponent(Object* pObject) : Component(pObject)
        {
        }

        struct Data
        {
			StrOffset strName;
			AudioFloat fMaxLength;
            Option optDelay;
            Option optDepth;
            Option optRate;
            Option optFeedback;
            Option optWet;
            Option optDry;
        };

        struct State
        {
            F32 fDelay;
            F32 fDepth;
            F32 fRate;
            F32 fFeedback;
			F32 fWet;
			F32 fDry;
        };

        enum Messages
        {
            EnableMsg,
            BypassMsg,
            ResetMsg,

            NumMessages
        };

        void Update(Instance* pInstance);
        void DoMessage( Instance* pInstance, U32 uMessage );

	protected:
		ixCore m_uDSP;
	};
}

#endif // _COMPONENT_H_


