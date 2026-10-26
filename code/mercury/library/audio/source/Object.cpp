#include "AudioPch.h"

#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "SystemHeaders.h"
#endif

#include <System/memory/MemoryTracker.h>
#include "CpuCounters.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
//#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#ifndef PLATFORM_SPU
////////////////////////////////////////////////////////////////////////////////
//	new
////////////////////////////////////////////////////////////////////////////////
void* Audio::Object::operator new(size_t sz)
{
//	MEMORY_TRACKER_PUSH("oldtech.audio.Object");
	return System::Alloc(sz,16);
//	MEMORY_TRACKER_POP("oldtech.audio.Object");
}

////////////////////////////////////////////////////////////////////////////////
//	delete
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::operator delete(void* ptr)
{
	System::Free(ptr);
}

////////////////////////////////////////////////////////////////////////////////
//	Object
////////////////////////////////////////////////////////////////////////////////
static U32 u32NumObjects = 0;
Audio::Object::Object( U32 uFrameSkip )
{
    m_pManager = NULL;
    m_pWaveManager = NULL;
	m_u32ActiveInstanceCount = 0;
	m_u32TotalInstanceCount = 0;
	m_u32InstanceFootprint = 0;
	m_u32SpuInstanceFootprint = 0;
	m_u32TotalInstanceFootprint = 0;
	m_u32SuspendedInstanceCount = 0;

	m_u32ComponentFootprintSize = 0;

	m_u32DynamicBufferSizeInSamples = 0;
#ifdef PLATFORM_PS3
	m_u32LargestRequiredDmaBuffer = 2048;
#endif

	#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU) && !defined CFG_GOLD
	m_uNameHash = 0xffffffff;
	#endif

	m_u32NumStreams = 0;
	m_u32NumAdpcmWaves = 0;

	m_pMasterInstance = NULL;
	m_uMasterInstanceSize = 0;

	m_pComponents = NULL;

	m_iDefinition = 0;

	m_pInitData = NULL;

	m_u32CoreSize = 0;

	m_uFrameSkip = uFrameSkip;
	m_fFrameSkip = (F32)uFrameSkip;

	bObjectIsDoomed = false;

	u32NumObjects++;
}

////////////////////////////////////////////////////////////////////////////////
//	~Object
////////////////////////////////////////////////////////////////////////////////
Audio::Object::~Object()
{
	if (m_pMasterInstance)
	{
		Instance* pInstance = (Instance*)m_pMasterInstance;
		pInstance->~Instance();
		MEMORY_TRACKER_PUSH("Audio.Objects.Instances");
		System::Free(m_pMasterInstance);
		MEMORY_TRACKER_POP("Audio.Objects.Instances");
	}

	u32NumObjects--;
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::Object::Init(Definition* pDefinition, IWaveManager* pWaveManager)
{
	MEMORY_TRACKER_PUSH("Audio.Objects.Objects");

    m_pWaveManager = pWaveManager;

    Definition* pDef = (Definition*)pDefinition;
	m_iDefinition = pDef;

	pDef->SetObject(this);

    m_pchStringTable = pDef->StringTable();

	m_pInitData = (U8*)m_iDefinition->ComponentData();

	//AUDIO_SYS_DEBUG("Initializing Object '%s'", m_pchStringTable);

	AUDIO_SYS_DEBUG("Component init data size %d bytes (%.2fk)", m_iDefinition->GetDataSize(), (F32)m_iDefinition->GetDataSize() / 1024.f);

    U32 uNumComponents = pDef->NumComponents();

    m_apComponents.Reallocate(uNumComponents);

    U32* puType = pDef->ComponentData();

#ifndef AUDIO_USE_COMPONENT_ARRAY
	U32 uTotalSize = CalculateComponentFootprint(pDef) + m_apComponents.GetIndexArraySize();
#else
	U32 uTotalSize = CalculateComponentFootprint(pDef);
#endif

	AUDIO_SYS_DEBUG("Component classes data size %d bytes (%.2fk)", uTotalSize, (F32)uTotalSize / 1024.f);

	m_pComponents = new U8[uTotalSize];
#ifndef AUDIO_USE_COMPONENT_ARRAY
	m_apComponents.SetDataPointer(m_pComponents);
	m_apComponents.UpdateIndexWithNumComponents();

	m_pComponents = m_pComponents + m_apComponents.GetIndexArraySize();
#endif

	U8* pNextCom = m_pComponents;

	Component* pCom = NULL;
    U32 uComponent = 0;
    while (uComponent++ < uNumComponents)
    {
        void* pData = (void*)(puType + 1);

		System::Get()->PushCurrentMemoryAllocationId(2);

		// store component type at start of component's data slab
		*((U32*)pNextCom) = *puType;
		pNextCom += sizeof(U32);

		switch(*puType)
        {
        case Component::eArray: pCom = new(pNextCom) Array(this); pNextCom += Array::Size(); break;
        case Component::eCone: pCom = new(pNextCom) Cone(this); pNextCom += Cone::Size(); break;
        case Component::eActionIn: pCom = new(pNextCom) ActionIn(this); pNextCom += ActionIn::Size(); break;
        case Component::eActionOut: pCom = new(pNextCom) ActionOut(this); pNextCom += ActionOut::Size(); break;
        case Component::eDebugPrint: pCom = new(pNextCom) DebugPrint(this); pNextCom += DebugPrint::Size(); break;
        case Component::eDopplerShift: pCom = new(pNextCom) DopplerShift(this); pNextCom += DopplerShift::Size(); break;
        case Component::eEnvelope: pCom = new(pNextCom) Envelope(this); pNextCom += Envelope::Size(); break;
        case Component::eSpring: pCom = new(pNextCom) Spring(this); pNextCom += Spring::Size(); break;
        case Component::eEventInput: pCom = new(pNextCom) EventInput(this); pNextCom += EventInput::Size(); break;
        case Component::eEventOutput: pCom = new(pNextCom) EventOutput(this); pNextCom += EventOutput::Size(); break;
        case Component::eCoordinateInput: pCom = new(pNextCom) CoordinateInput(this); pNextCom += CoordinateInput::Size(); break;
        case Component::eCoordinateOutput: pCom = new(pNextCom) CoordinateOutput(this); pNextCom += CoordinateOutput::Size(); break;
        case Component::eExponent: pCom = new(pNextCom) Exponent(this); pNextCom += Exponent::Size(); break;
        case Component::eFraction: pCom = new(pNextCom)Fraction(this); pNextCom += Fraction::Size(); break;
      	case Component::eGradient: pCom = new(pNextCom) Gradient(this); pNextCom += Gradient::Size(); break;
        case Component::eGateSwitch: pCom = new(pNextCom) GateSwitch(this); pNextCom += GateSwitch::Size(); break;
        case Component::eInvert: pCom = new(pNextCom) Invert(this); pNextCom += Invert::Size(); break;
        case Component::eLowPass: pCom = new(pNextCom) LowPass(this); pNextCom += LowPass::Size(); break;
		case Component::eMax: pCom = new(pNextCom) Max(this); pNextCom += Max::Size(); break;
		case Component::eMin: pCom = new(pNextCom) Min(this); pNextCom += Min::Size(); break;
        case Component::eMonoAudioInput: pCom = new(pNextCom) MonoAudioInput(this); pNextCom += MonoAudioInput::Size(); break;
        case Component::eMonoAudioOutput: pCom = new(pNextCom) MonoAudioOutput(this); pNextCom += MonoAudioOutput::Size(); break;
        case Component::eMonoConvolution: pCom = new(pNextCom) MonoConvolution(this); pNextCom += MonoConvolution::Size(); break;
        case Component::eMonoDelay: pCom = new(pNextCom) MonoDelay(this); pNextCom += MonoDelay::Size(); break;
//        case Component::eMonoFeedbackDelay: pCom = new(pNextCom) MonoFeedbackDelay(this); pNextCom += MonoFeedbackDelay::Size(); break;
        case Component::eMonoFeedbackDelay:
			AUDIO_SYS_FATAL("Mono Feedback Delay no longer exists");
			break;
//        case Component::eMonoGain: pCom = new(pNextCom) MonoGain(this); pNextCom += MonoGain::Size(); break;
        case Component::eMonoGain:
			AUDIO_SYS_FATAL("Mono Gain no longer exists");
			break;
        case Component::eMonoSend: pCom = new(pNextCom) MonoSend(this); pNextCom += MonoSend::Size(); break;
        case Component::eMonoReturn: pCom = new(pNextCom) MonoReturn(this); pNextCom += MonoReturn::Size(); break;
        case Component::eStereoSend: pCom = new(pNextCom) StereoSend(this); pNextCom += StereoSend::Size(); break;
        case Component::eStereoReturn: pCom = new(pNextCom) StereoReturn(this); pNextCom += StereoReturn::Size(); break;
        case Component::eMonoLoopingSound: pCom = new(pNextCom) MonoLoopingSound(this); pNextCom += MonoLoopingSound::Size(); break;
        case Component::eMonoLowPass: pCom = new(pNextCom) MonoLowPass(this); pNextCom += MonoLowPass::Size(); break;
        case Component::eMonoHighPass: pCom = new(pNextCom) MonoHighPass(this); pNextCom += MonoHighPass::Size(); break;
        case Component::eSurroundHighPass: pCom = new(pNextCom) SurroundHighPass(this); pNextCom += SurroundHighPass::Size(); break;
//        case Component::eMonoLowShelf: pCom = new(pNextCom) MonoLowShelf(this); pNextCom += MonoLowShelf::Size(); break;
        case Component::eMonoLowShelf:
			AUDIO_SYS_FATAL("Mono Low Shelf no longer exists");
			break;
//        case Component::eMonoLowShelf: pCom = new(pNextCom) MonoLowShelf(this); pNextCom += MonoLowShelf::Size(); break;
        case Component::eMonoHighShelf:
			AUDIO_SYS_FATAL("Mono High Shelf no longer exists");
			break;
        case Component::eMonoPeakEQ: pCom = new(pNextCom) MonoPeakEQ(this); pNextCom += MonoPeakEQ::Size(); break;
		case Component::eMonoOneShotSound: pCom = new(pNextCom) MonoOneShotSound(this); pNextCom += MonoOneShotSound::Size(); break; 
		case Component::eMonoOneShotSoundTable: pCom = new(pNextCom) MonoOneShotSoundTable(this); pNextCom += MonoOneShotSoundTable::Size(); pNextCom += MonoOneShotSoundTable::WaveHandleArraySize(pData); break;
        case Component::eMonoResonantLowPass: pCom = new(pNextCom) MonoResonantLowPass(this); pNextCom += MonoResonantLowPass::Size(); break;
        case Component::eMono24dBResonantLowPass: pCom = new(pNextCom) Mono24dBResonantLowPass(this); pNextCom += Mono24dBResonantLowPass::Size(); break;
        case Component::eSurround24dBResonantLowPass: pCom = new(pNextCom) Surround24dBResonantLowPass(this); pNextCom += Surround24dBResonantLowPass::Size(); break;
        case Component::eMonoSaturator: pCom = new(pNextCom) MonoSaturator(this); pNextCom += MonoSaturator::Size(); break;
        case Component::eMonoBitCrusher: pCom = new(pNextCom) MonoBitCrusher(this); pNextCom += MonoBitCrusher::Size(); break;
        case Component::eSurroundBitCrusher: pCom = new(pNextCom) SurroundBitCrusher(this); pNextCom += SurroundBitCrusher::Size(); break;
        case Component::eMonoRingModulator: pCom = new(pNextCom) MonoRingModulator(this); pNextCom += MonoRingModulator::Size(); break;
        case Component::eSurroundRingModulator: pCom = new(pNextCom) SurroundRingModulator(this); pNextCom += SurroundRingModulator::Size(); break;
        case Component::eMonoCompressor: pCom = new(pNextCom) MonoCompressor(this); pNextCom += MonoCompressor::Size(); break;
//        case Component::eMonoDistortion: pCom = new(pNextCom) MonoDistortion(this); pNextCom += MonoDistortion::Size(); break;
        case Component::eMonoDistortion:
			AUDIO_SYS_FATAL("Trying to set up a MonoDistortion component, it has been removed");
			break;
//		case Component::eMonoLipSync: pCom = new(pNextCom) MonoLipSync(this); pNextCom += MonoLipSync::Size(); break;
		case Component::eMonoLipSync:
			AUDIO_SYS_FATAL("Trying to set up a Lip Sync component, it has been removed");
			break;
		case Component::eMonoSpectralReverb: pCom = new(pNextCom) MonoSpectralReverb(this); pNextCom += MonoSpectralReverb::Size(); break;
//		case Component::eMonoSpectralEnvelope: pCom = new(pNextCom) MonoSpectralEnvelope(this); pNextCom += MonoSpectralEnvelope::Size(); break;
		case Component::eMonoSpectralEnvelope:
			AUDIO_SYS_FATAL("Trying to set up a Mono Spectral Envelope component, it has been removed");
			break;
//		case Component::eStereoSpectralEnvelope: pCom = new(pNextCom) StereoSpectralEnvelope(this); pNextCom += StereoSpectralEnvelope::Size(); break;
		case Component::eStereoSpectralEnvelope:
			AUDIO_SYS_FATAL("Trying to set up a Stereo Spectral Envelope component, it has been removed");
			break;
//		case Component::eMonoPeakLevel: pCom = new(pNextCom) MonoPeakLevel(this); pNextCom += MonoPeakLevel::Size(); break;
		case Component::eMonoPeakLevel:
			AUDIO_SYS_FATAL("Trying to set up a Mono Peak Level component, it has been removed");
			break;
//		case Component::eMonoPhaser: pCom = new(pNextCom) MonoPhaser(this); pNextCom += MonoPhaser::Size(); break;
		case Component::eMonoPhaser:
			AUDIO_SYS_FATAL("Trying to set up a Mono Phaser component, it has been removed");
			break;
		case Component::eMonoFlanger: pCom = new(pNextCom) MonoFlanger(this); pNextCom += MonoFlanger::Size(); break;
		case Component::eSurroundFlanger: pCom = new(pNextCom) SurroundFlanger(this); pNextCom += SurroundFlanger::Size(); break;
		case Component::eMonoRMSLevel: pCom = new(pNextCom) MonoRMSLevel(this); pNextCom += MonoRMSLevel::Size(); break;
//		case Component::eMonoOscilloscope: pCom = new(pNextCom) MonoOscilloscope(this); pNextCom += MonoOscilloscope::Size(); break;
		case Component::eMonoOscilloscope:
			AUDIO_SYS_FATAL("Trying to set up a Mono Oscilloscope component, it has been removed");
			break;
        case Component::eMonoToMonoMixer: pCom = new(pNextCom) MonoToMonoMixer(this); pNextCom += MonoToMonoMixer::Size(); break;
        //case Component::eMonoToMonoSpectralMixer: pCom = new(pNextCom) MonoToMonoSpectralMixer(this); pNextCom += MonoToMonoSpectralMixer::Size(); break;
        case Component::eMonoToStereoMixer: pCom = new(pNextCom) MonoToStereoMixer(this); pNextCom += MonoToStereoMixer::Size(); break;
        case Component::eMonoToSurroundMixer: pCom = new(pNextCom) MonoToSurroundMixer(this); pNextCom += MonoToSurroundMixer::Size(); break;
        case Component::eMonoToSurroundLFEMixer: pCom = new(pNextCom) MonoToSurroundLFEMixer(this); pNextCom += MonoToSurroundLFEMixer::Size(); break;
        case Component::eMonoToAmbisonicsMixer: pCom = new(pNextCom) MonoToAmbisonicsMixer(this); pNextCom += MonoToAmbisonicsMixer::Size(); break;
        case Component::eAmbisonicsToSurroundMixer: pCom = new(pNextCom) AmbisonicsToSurroundMixer(this); pNextCom += AmbisonicsToSurroundMixer::Size(); break;
        case Component::eParameterIn: pCom = new(pNextCom) ParameterIn(this); pNextCom += ParameterIn::Size(); break;
        case Component::eParameterOut: pCom = new(pNextCom) ParameterOut(this); pNextCom += ParameterOut::Size(); break;
        case Component::eControlInput: pCom = new(pNextCom) ControlInput(this); pNextCom += ControlInput::Size(); break;
        case Component::eControlOutput: pCom = new(pNextCom) ControlOutput(this); pNextCom += ControlOutput::Size(); break;
        case Component::ePiecewiseLinearFunction: pCom = new(pNextCom) PiecewiseLinearFunction(this); pNextCom += PiecewiseLinearFunction::Size(); break;
        case Component::eStateMachine: pCom = new(pNextCom) StateMachine(this); pNextCom += StateMachine::Size(); break;
        case Component::eSequence: pCom = new(pNextCom) Sequence(this); pNextCom += Sequence::Size(); break;
        case Component::eProduct: pCom = new(pNextCom) Product(this); pNextCom += Product::Size(); break;
        case Component::eQuantize: pCom = new(pNextCom) Quantize(this); pNextCom += Quantize::Size(); break;
        case Component::eRandom: pCom = new(pNextCom) Random(this); pNextCom += Random::Size(); break;
        case Component::eRolloff: pCom = new(pNextCom) Rolloff(this); pNextCom += Rolloff::Size(); break;
        case Component::eSampleAndHold: pCom = new(pNextCom) SampleAndHold(this); pNextCom += SampleAndHold::Size(); break;
        case Component::eScript: pCom = new(pNextCom) Script(this); pNextCom += Script::Size(); break;
		case Component::eSelector: pCom = new(pNextCom) Selector(this); pNextCom += Selector::Size(); break;
        case Component::eSine: pCom = new(pNextCom) Sine(this); pNextCom += Sine::Size(); break;
        case Component::eSource: pCom = new(pNextCom) Source(this); pNextCom += Source::Size(); break;
        case Component::eStereoAudioInput: pCom = new(pNextCom) StereoAudioInput(this); pNextCom += StereoAudioInput::Size(); break;
        case Component::eStereoAudioOutput: pCom = new(pNextCom) StereoAudioOutput(this); pNextCom += StereoAudioOutput::Size(); break;
        case Component::eStereoHighPass: pCom = new(pNextCom) StereoHighPass(this); pNextCom += StereoHighPass::Size(); break;
        case Component::eStereoPeakEQ: pCom = new(pNextCom) StereoPeakEQ(this); pNextCom += StereoPeakEQ::Size(); break;
		case Component::eStereoRMSLevel: pCom = new(pNextCom) StereoRMSLevel(this); pNextCom += StereoRMSLevel::Size(); break;
		case Component::eStereoLoopingSound: pCom = new(pNextCom) StereoLoopingSound(this); pNextCom += StereoLoopingSound::Size(); break;
        case Component::eStereoLowPass: pCom = new(pNextCom) StereoLowPass(this); pNextCom += StereoLowPass::Size(); break;
//        case Component::eStereoHighShelf: pCom = new(pNextCom) StereoHighShelf(this); pNextCom += StereoHighShelf::Size(); break;
        case Component::eStereoHighShelf:
			AUDIO_SYS_FATAL("Trying to set up a Stereo High Shelf component, it has been removed");
			break;
		case Component::eStereoOneShotSound: pCom = new(pNextCom) StereoOneShotSound(this); pNextCom += StereoOneShotSound::Size(); break;
		case Component::eStereoOneShotSoundTable: pCom = new(pNextCom) StereoOneShotSoundTable(this); pNextCom += StereoOneShotSoundTable::Size(); pNextCom += StereoOneShotSoundTable::WaveHandleArraySize(pData); break;
        case Component::eStereoResonantLowPass: pCom = new(pNextCom) StereoResonantLowPass(this); pNextCom += StereoResonantLowPass::Size(); break;
		case Component::eStereoSaturator: pCom = new(pNextCom) StereoSaturator(this); pNextCom += StereoSaturator::Size(); break;
		case Component::eStereoCompressor: pCom = new(pNextCom) StereoCompressor(this); pNextCom += StereoCompressor::Size(); break;
//		case Component::eStereoDistortion: pCom = new(pNextCom) StereoDistortion(this); pNextCom += StereoDistortion::Size(); break;
		case Component::eStereoDistortion:
			AUDIO_SYS_FATAL("Trying to set up a Stereo Distortion component, it has been removed");
			break;
//		case Component::eStereoFeedbackDelay: pCom = new(pNextCom) StereoFeedbackDelay(this); pNextCom += StereoFeedbackDelay::Size(); break;
		case Component::eStereoFeedbackDelay:
			AUDIO_SYS_FATAL("Trying to set up a Stereo Feedback Delay component, it has been removed");
			break;

		case Component::eStereoPitchShift: pCom = new(pNextCom) StereoPitchShift(this); pNextCom += StereoPitchShift::Size(); break;       

		case Component::eStereoToMonoMixer: pCom = new(pNextCom) StereoToMonoMixer(this); pNextCom += StereoToMonoMixer::Size(); break;
        case Component::eStereoToStereoMixer: pCom = new(pNextCom) StereoToStereoMixer(this); pNextCom += StereoToStereoMixer::Size(); break;
        case Component::eStereoToQuadMixer: pCom = new(pNextCom) StereoToQuadMixer(this); pNextCom += StereoToQuadMixer::Size(); break;
        case Component::eQuadToSurroundMixer: pCom = new(pNextCom) QuadToSurroundMixer(this); pNextCom += QuadToSurroundMixer::Size(); break;
        case Component::eStereoToSurroundMixer: pCom = new(pNextCom) StereoToSurroundMixer(this); pNextCom += StereoToSurroundMixer::Size(); break;
        case Component::eSum: pCom = new(pNextCom) Sum(this); pNextCom += Sum::Size(); break;
        case Component::eSurroundAudioInput: pCom = new(pNextCom) SurroundAudioInput(this); pNextCom += SurroundAudioInput::Size(); break;
        case Component::eSurroundAudioOutput: pCom = new(pNextCom) SurroundAudioOutput(this); pNextCom += SurroundAudioOutput::Size(); break;
        case Component::eSurroundLFEAudioInput: pCom = new(pNextCom) SurroundLFEAudioInput(this); pNextCom += SurroundLFEAudioInput::Size(); break;
        case Component::eSurroundLFEAudioOutput: pCom = new(pNextCom) SurroundLFEAudioOutput(this); pNextCom += SurroundLFEAudioOutput::Size(); break;
//        case Component::eSurroundHighShelf: pCom = new(pNextCom) SurroundHighShelf(this); pNextCom += SurroundHighShelf::Size(); break;
        case Component::eSurroundHighShelf:
			AUDIO_SYS_FATAL("Trying to set up a Surround High Shelf component, it has been removed");
			break;
//		case Component::eSurroundLowShelf: pCom = new(pNextCom) SurroundLowShelf(this); pNextCom += SurroundLowShelf::Size(); break;
		case Component::eSurroundLowShelf:
			AUDIO_SYS_FATAL("Trying to set up a Surround Low Shelf component, it has been removed");
			break;
 		case Component::eSurroundLowPass: pCom = new(pNextCom) SurroundLowPass(this); pNextCom += SurroundLowPass::Size(); break;
        case Component::eSurroundResonantLowPass: pCom = new(pNextCom) SurroundResonantLowPass(this); pNextCom += SurroundResonantLowPass::Size(); break;
		case Component::eSurroundSaturator: pCom = new(pNextCom) SurroundSaturator(this); pNextCom += SurroundSaturator::Size(); break;
		case Component::eSurroundCompressor: pCom = new(pNextCom) SurroundCompressor(this); pNextCom += SurroundCompressor::Size(); break;
        case Component::eSurroundToSurroundMixer: pCom = new(pNextCom) SurroundToSurroundMixer(this); pNextCom += SurroundToSurroundMixer::Size(); break;
        case Component::eSurroundToSurroundLFEMixer: pCom = new(pNextCom) SurroundToSurroundLFEMixer(this); pNextCom += SurroundToSurroundLFEMixer::Size(); break;
        case Component::eSurroundLFEToSurroundLFEMixer: pCom = new(pNextCom) SurroundLFEToSurroundLFEMixer(this); pNextCom += SurroundLFEToSurroundLFEMixer::Size(); break;
		case Component::eSurroundPeakEQ: pCom = new(pNextCom) SurroundPeakEQ(this); pNextCom += SurroundPeakEQ::Size(); break;
		case Component::eSurroundRMSLevel: pCom = new(pNextCom) SurroundRMSLevel(this); pNextCom += SurroundRMSLevel::Size(); break;   		
		case Component::eTimer: pCom = new(pNextCom) Timer(this); pNextCom += Timer::Size(); break;
   		case Component::eMetronome: pCom = new(pNextCom) Metronome(this); pNextCom += Metronome::Size(); break;
        case Component::eVariable: pCom = new(pNextCom) Variable(this); pNextCom += Variable::Size(); break;
        case Component::eVector: pCom = new(pNextCom) Vector(this); pNextCom += Vector::Size(); break;
        case Component::eVectorIn: pCom = new(pNextCom) VectorIn(this); pNextCom += VectorIn::Size(); break;
		case Component::eVectorNearestPointOnLine: pCom = new(pNextCom) VectorNearestPointOnLine(this); pNextCom += VectorNearestPointOnLine::Size(); break;
		case Component::eImpactFilter: pCom = new(pNextCom) ImpactFilter(this); pNextCom += ImpactFilter::Size(); break;
		case Component::eMessageDelay: pCom = new(pNextCom) MessageDelay(this); pNextCom += MessageDelay::Size(); break;
		default:
			AUDIO_SYS_FATAL("Error: Unknown component type %d", *puType);
			break;
        };

		System::Get()->PopCurrentMemoryAllocationId();

		if (pCom)
		{
			//	add to all components
			m_apComponents.PushBack(pCom);

			//	if not a script, add to updating components
			if (pCom->Flags() & Component::flagUpdater)
				m_apUpdaters.PushBack(pCom);

			//	add to sound components
			if (pCom->Flags() & Component::flagSound)
				m_apSounds.PushBack(pCom);

			//	add to generating components
			if (pCom->Flags() & Component::flagTransmitter)
				m_apTransmitters.PushBack(pCom);

			//	add to receiving components
			if (pCom->Flags() & Component::flagReciever)
				m_apReceivers.PushBack(pCom);

			//	add to script components
			if (pCom->Flags() & Component::flagScript)
				m_apScripts.PushBack(pCom);

			//	add to audio output components
			if ((pCom->Flags() & Component::flagOutputConnector && pCom->Flags() & Component::flagMono)					// if a mono audio output..
				|| (pCom->Flags() & Component::flagOutputConnector && pCom->Flags() & Component::flagStereo)			// or a stereo audio output..
				|| (pCom->Flags() & Component::flagOutputConnector && pCom->Flags() & Component::flagSurround)			// or a surround audio output..
				|| (pCom->Flags() & Component::flagOutputConnector && pCom->Flags() & Component::flagSurroundLFE)		// or a surround lfe audio output..
				)
				m_apAudioConnections.PushBack(pCom);

			//	add to output components
			if ((pCom->Flags() & Component::flagOutputConnector && pCom->Flags() & Component::flagEventConnector)		// if an event output..
				|| (pCom->Flags() & Component::flagInputConnector && pCom->Flags() & Component::flagControlConnector)	// or a control input..
				|| (pCom->Flags() & Component::flagInputConnector && pCom->Flags() & Component::flagVectorConnector)	// or a coordinate input..
#ifndef CFG_GOLD
				|| ((*puType) == Component::eDebugPrint)																// or it's a debug print
#endif
				)
				m_apConnections.PushBack(pCom);

			//	add to action out components
			if (pCom->Flags() & Component::flagActionOut)
				m_apActionOuts.PushBack(pCom);

			//	init
			puType = (U32*)pCom->Init(pData);

			AUDIO_SYS_DEBUG("Created %s '%s'", pCom->Type(), pCom->Name());

#ifdef AUDIO_STRING_HASH_COMPONENTS
			if (!pCom->NameHash().IsValid())
				AUDIO_SYS_ERROR("Component '%s' name was not hashed", pCom->Name());
#endif
		}
    };

    U32 uOffset = 0;
    m_u32CoreSize = 0;
    //  set instance memory offsets : CREATE_CORE_CLASSES
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        uOffset = (U32)m_apComponents[uCount]->SetStateOffset(uOffset);
        //uOffset += 0x0f;
        //uOffset &= 0xfffffff0;
        uOffset += 0x0f;
        uOffset &= ~0x0f;

		//	TODO:only flagAudioOutputs
		m_u32CoreSize = m_apComponents[uCount]->CoreMemory(m_u32CoreSize);
    }

	//AUDIO_SYS_DEBUG("Object core data size 0x%x", m_u32CoreSize);

	//	only do this once and create copies from it
	CreateMasterInstance();

	MEMORY_TRACKER_POP("Audio.Objects.Objects");

#ifdef PLATFORM_PS3
	U32 u32DataSize = (GetDefinition()->GetDataSize()
		+ m_u32ComponentFootprintSize
		+ m_uMasterInstanceSize
		+ (m_u32DynamicBufferSizeInSamples * sizeof(F32))) / 1024;

	if (u32DataSize > 80)
	{
		AUDIO_SYS_ERROR("Size of object %s is %.2d\n", Name(), u32DataSize);
		return OutOfMemory;
	}

#endif


    return Success;
}
#endif	//	#ifndef PLATFORM_SPU


////////////////////////////////////////////////////////////////////////////////
//	CalculateComponentFootprint
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
U32 Audio::Object::CalculateComponentFootprint(Definition* iDef)
{
	U32 uSizeInSamples = 0;

    U32* puType = iDef->ComponentData();

    U32 uComponent = 0;

    while (uComponent++ < iDef->NumComponents())
    {
		//printf("%d\n", uSizeInSamples);
		//printf("%d\n", (U32)puType - (U32)m_iDefinition->ComponentData());

        void* pData = (void*)(puType + 1);

        switch(*puType)
        {
        case Component::eArray:
			uSizeInSamples += Array::Size();
			puType += Array::DataSize(pData) / 4;
            break;

        case Component::eCone:
            uSizeInSamples += Cone::Size();
			puType += Cone::DataSize(pData) / 4;
            break;

        case Component::eActionIn:
            uSizeInSamples += ActionIn::Size();
			puType += ActionIn::DataSize(pData) / 4;
            break;

        case Component::eActionOut:
            uSizeInSamples += ActionOut::Size();
			puType += ActionOut::DataSize(pData) / 4;
            break;

        case Component::eDebugPrint:
            uSizeInSamples += DebugPrint::Size();
			puType += DebugPrint::DataSize(pData) / 4;
            break;

		case Component::eDopplerShift:
            uSizeInSamples += DopplerShift::Size();
			puType += DopplerShift::DataSize(pData) / 4;
            break;

        case Component::eEnvelope:
            uSizeInSamples += Envelope::Size();
			puType += Envelope::DataSize(pData) / 4;
            break;

        case Component::eSpring:
            uSizeInSamples += Spring::Size();
			puType += Spring::DataSize(pData) / 4;
            break;

        case Component::eEventInput:
            uSizeInSamples += EventInput::Size();
			puType += EventInput::DataSize(pData) / 4;
            break;

        case Component::eEventOutput:
            uSizeInSamples += EventOutput::Size();
			puType += EventOutput::DataSize(pData) / 4;
            break;

        case Component::eCoordinateInput:
            uSizeInSamples += CoordinateInput::Size();
			puType += CoordinateInput::DataSize(pData) / 4;
            break;

        case Component::eCoordinateOutput:
            uSizeInSamples += CoordinateOutput::Size();
			puType += CoordinateOutput::DataSize(pData) / 4;
            break;

        case Component::eExponent:
            uSizeInSamples += Exponent::Size();
			puType += Exponent::DataSize(pData) / 4;
            break;

#ifndef AUDIO_SPU_MINIMAL
		case Component::eFraction:
            uSizeInSamples += Fraction::Size();
			puType += Fraction::DataSize(pData) / 4;
            break;
#endif

        case Component::eGradient:
            uSizeInSamples += Gradient::Size(); 
			puType += Gradient::DataSize(pData) / 4;
            break;

        case Component::eGateSwitch:
            uSizeInSamples += GateSwitch::Size();
			puType += GateSwitch::DataSize(pData) / 4;
            break;

        case Component::eInvert:
            uSizeInSamples += Invert::Size();
			puType += Invert::DataSize(pData) / 4;
            break;

        case Component::eLowPass:
            uSizeInSamples += LowPass::Size();
			puType += LowPass::DataSize(pData) / 4;
            break;

		case Component::eMax:
			uSizeInSamples += Max::Size();
			puType += Max::DataSize(pData) / 4;
			break;

		case Component::eMin:
			uSizeInSamples += Min::Size();
			puType += Min::DataSize(pData) / 4;
			break;

#ifndef AUDIO_SPU_MINIMAL
         case Component::eMonoAudioInput:
            uSizeInSamples += MonoAudioInput::Size();
			puType += MonoAudioInput::DataSize(pData) / 4;
            break;
#endif

        case Component::eMonoAudioOutput:
            uSizeInSamples += MonoAudioOutput::Size();
			puType += MonoAudioOutput::DataSize(pData) / 4;
            break;

#ifndef AUDIO_SPU_MINIMAL
        case Component::eMonoConvolution:
            uSizeInSamples += MonoConvolution::Size();
			puType += MonoConvolution::DataSize(pData) / 4;
            break;

        case Component::eMonoDelay:
            uSizeInSamples += MonoDelay::Size();
			puType += MonoDelay::DataSize(pData) / 4;
            break;

   //     case Component::eMonoFeedbackDelay:
   //         uSizeInSamples += MonoFeedbackDelay::Size();
			//puType += MonoFeedbackDelay::DataSize(pData) / 4;
   //         break;

   //     case Component::eMonoGain:
   //         uSizeInSamples += MonoGain::Size();
			//puType += MonoGain::DataSize(pData) / 4;
   //         break;

        case Component::eMonoSend:
            uSizeInSamples += MonoSend::Size();
			puType += MonoSend::DataSize(pData) / 4;
            break;

        case Component::eMonoReturn:
            uSizeInSamples += MonoReturn::Size();
			puType += MonoReturn::DataSize(pData) / 4;
            break;

        case Component::eStereoSend:
            uSizeInSamples += StereoSend::Size();
			puType += StereoSend::DataSize(pData) / 4;
            break;

        case Component::eStereoReturn:
            uSizeInSamples += StereoReturn::Size();
			puType += StereoReturn::DataSize(pData) / 4;
            break;

        case Component::eMonoLoopingSound:
            uSizeInSamples += MonoLoopingSound::Size();
			puType += MonoLoopingSound::DataSize(pData) / 4;
            break;

        case Component::eMonoLowPass:
            uSizeInSamples += MonoLowPass::Size();
			puType += MonoLowPass::DataSize(pData) / 4;
            break;

        case Component::eMonoHighPass:
            uSizeInSamples += MonoHighPass::Size();
			puType += MonoHighPass::DataSize(pData) / 4;
            break;

        case Component::eSurroundHighPass:
            uSizeInSamples += SurroundHighPass::Size();
			puType += SurroundHighPass::DataSize(pData) / 4;
            break;

   //     case Component::eMonoLowShelf:
   //         uSizeInSamples += MonoLowShelf::Size();
			//puType += MonoLowShelf::DataSize(pData) / 4;
   //         break;

   //     case Component::eMonoHighShelf:
   //         uSizeInSamples += MonoHighShelf::Size();
			//puType += MonoHighShelf::DataSize(pData) / 4;
   //         break;

        case Component::eMonoPeakEQ:
            uSizeInSamples += MonoPeakEQ::Size();
			puType += MonoPeakEQ::DataSize(pData) / 4;
            break;
#endif

        case Component::eMonoOneShotSound:
            uSizeInSamples += MonoOneShotSound::Size();
			puType += MonoOneShotSound::DataSize(pData) / 4;
            break;

#ifndef AUDIO_SPU_MINIMAL
        case Component::eMonoOneShotSoundTable:
			// reserve some memory for the array of wave handles
			uSizeInSamples += MonoOneShotSoundTable::Size() + MonoOneShotSoundTable::WaveHandleArraySize(pData);
			puType += MonoOneShotSoundTable::DataSize(pData) / 4;
            break;

        case Component::eMonoResonantLowPass:
            uSizeInSamples += MonoResonantLowPass::Size();
			puType += MonoResonantLowPass::DataSize(pData) / 4;
            break;

        case Component::eMono24dBResonantLowPass:
            uSizeInSamples += Mono24dBResonantLowPass::Size();
			puType += Mono24dBResonantLowPass::DataSize(pData) / 4;
            break;

        case Component::eSurround24dBResonantLowPass:
            uSizeInSamples += Surround24dBResonantLowPass::Size();
			puType += Surround24dBResonantLowPass::DataSize(pData) / 4;
            break;

        case Component::eMonoSaturator:
            uSizeInSamples += MonoSaturator::Size();
			puType += MonoSaturator::DataSize(pData) / 4;
            break;

        case Component::eMonoBitCrusher:
            uSizeInSamples += MonoBitCrusher::Size();
			puType += MonoBitCrusher::DataSize(pData) / 4;
            break;

        case Component::eSurroundBitCrusher:
            uSizeInSamples += SurroundBitCrusher::Size();
			puType += SurroundBitCrusher::DataSize(pData) / 4;
            break;

        case Component::eMonoRingModulator:
            uSizeInSamples += MonoRingModulator::Size();
			puType += MonoRingModulator::DataSize(pData) / 4;
            break;

        case Component::eSurroundRingModulator:
            uSizeInSamples += SurroundRingModulator::Size();
			puType += SurroundRingModulator::DataSize(pData) / 4;
            break;

        case Component::eMonoCompressor:
            uSizeInSamples += MonoCompressor::Size();
			puType += MonoCompressor::DataSize(pData) / 4;
            break;

   //     case Component::eMonoDistortion:
   //         uSizeInSamples += MonoDistortion::Size();
			//puType += MonoDistortion::DataSize(pData) / 4;
   //         break;

		//case Component::eMonoLipSync:
  //          uSizeInSamples += MonoLipSync::Size();
		//	puType += MonoLipSync::DataSize(pData) / 4;
  //          break;

		case Component::eMonoSpectralReverb:
            uSizeInSamples += MonoSpectralReverb::Size();
			puType += MonoSpectralReverb::DataSize(pData) / 4;
            break;

		//case Component::eMonoSpectralEnvelope:
  //          uSizeInSamples += MonoSpectralEnvelope::Size();
		//	puType += MonoSpectralEnvelope::DataSize(pData) / 4;
  //          break;

		//case Component::eStereoSpectralEnvelope:
  //          uSizeInSamples += StereoSpectralEnvelope::Size();
		//	puType += StereoSpectralEnvelope::DataSize(pData) / 4;
  //          break;

		//case Component::eMonoPeakLevel:
  //          uSizeInSamples += MonoPeakLevel::Size();
		//	puType += MonoPeakLevel::DataSize(pData) / 4;
  //          break;

		//case Component::eMonoPhaser:
  //          uSizeInSamples += MonoPhaser::Size();
		//	puType += MonoPhaser::DataSize(pData) / 4;
  //          break;

		case Component::eMonoFlanger:
            uSizeInSamples += MonoFlanger::Size();
			puType += MonoFlanger::DataSize(pData) / 4;
            break;

		case Component::eSurroundFlanger:
            uSizeInSamples += SurroundFlanger::Size();
			puType += SurroundFlanger::DataSize(pData) / 4;
            break;

		case Component::eMonoRMSLevel:
            uSizeInSamples += MonoRMSLevel::Size();
			puType += MonoRMSLevel::DataSize(pData) / 4;
            break;

		//case Component::eMonoOscilloscope:
  //          uSizeInSamples += MonoOscilloscope::Size();
		//	puType += MonoOscilloscope::DataSize(pData) / 4;
  //          break;
#endif

        case Component::eMonoToMonoMixer:
			uSizeInSamples += MonoToMonoMixer::Size();
			puType += MonoToMonoMixer::DataSize(pData) / 4;
            break;

   //     case Component::eMonoToMonoSpectralMixer:
   //         uSizeInSamples += MonoToMonoSpectralMixer::Size();
			//puType += MonoToMonoSpectralMixer::DataSize(pData) / 4;
   //         break;

        case Component::eMonoToStereoMixer:
            uSizeInSamples += MonoToStereoMixer::Size();
			puType += MonoToStereoMixer::DataSize(pData) / 4;
            break;

        case Component::eMonoToSurroundMixer:
            uSizeInSamples += MonoToSurroundMixer::Size();
			puType += MonoToSurroundMixer::DataSize(pData) / 4;
            break;

        case Component::eMonoToSurroundLFEMixer:
            uSizeInSamples += MonoToSurroundLFEMixer::Size();
			puType += MonoToSurroundLFEMixer::DataSize(pData) / 4;
            break;

        case Component::eMonoToAmbisonicsMixer:
            uSizeInSamples += MonoToAmbisonicsMixer::Size();
			puType += MonoToAmbisonicsMixer::DataSize(pData) / 4;
            break;

        case Component::eAmbisonicsToSurroundMixer:
            uSizeInSamples += AmbisonicsToSurroundMixer::Size();
			puType += AmbisonicsToSurroundMixer::DataSize(pData) / 4;
            break;

        case Component::eParameterIn:
            uSizeInSamples += ParameterIn::Size();
			puType += ParameterIn::DataSize(pData) / 4;
            break;

        case Component::eParameterOut:
            uSizeInSamples += ParameterOut::Size();
			puType += ParameterOut::DataSize(pData) / 4;
            break;

        case Component::eControlInput:
            uSizeInSamples += ControlInput::Size();
			puType += ControlInput::DataSize(pData) / 4;
            break;

        case Component::eControlOutput:
            uSizeInSamples += ControlOutput::Size();
			puType += ControlOutput::DataSize(pData) / 4;
            break;

        case Component::ePiecewiseLinearFunction:
            uSizeInSamples += PiecewiseLinearFunction::Size();
			puType += PiecewiseLinearFunction::DataSize(pData) / 4;
            break;

        case Component::eStateMachine:
            uSizeInSamples += StateMachine::Size();
			puType += StateMachine::DataSize(pData) / 4;
            break;

        case Component::eSequence:
            uSizeInSamples += Sequence::Size();
			puType += Sequence::DataSize(pData) / 4;
            break;

        case Component::eProduct:
            uSizeInSamples += Product::Size();
			puType += Product::DataSize(pData) / 4;
            break;

        case Component::eQuantize:
            uSizeInSamples += Quantize::Size();
			puType += Quantize::DataSize(pData) / 4;
            break;

        case Component::eRandom:
            uSizeInSamples += Random::Size();
			puType += Random::DataSize(pData) / 4;
            break;

        case Component::eRolloff:
            uSizeInSamples += Rolloff::Size();
			puType += Rolloff::DataSize(pData) / 4;
            break;

        case Component::eSampleAndHold:
            uSizeInSamples += SampleAndHold::Size();
			puType += SampleAndHold::DataSize(pData) / 4;
            break;

        case Component::eScript:
            uSizeInSamples += Script::Size();
			puType += Script::DataSize(pData) / 4;
            break;

		case Component::eSelector:
            uSizeInSamples += Selector::Size();
			puType += Selector::DataSize(pData) / 4;
            break;

        case Component::eSine:
            uSizeInSamples += Sine::Size();
			puType += Sine::DataSize(pData) / 4;
            break;

        case Component::eSource:
            uSizeInSamples += Source::Size();
			puType += Source::DataSize(pData) / 4;
            break;

        case Component::eStereoAudioInput:
            uSizeInSamples += StereoAudioInput::Size();
			puType += StereoAudioInput::DataSize(pData) / 4;
            break;

        case Component::eStereoAudioOutput:
            uSizeInSamples += StereoAudioOutput::Size();
			puType += StereoAudioOutput::DataSize(pData) / 4;
            break;

        case Component::eStereoHighPass:
            uSizeInSamples += StereoHighPass::Size();
			puType += StereoHighPass::DataSize(pData) / 4;
            break;

		case Component::eStereoPeakEQ:
            uSizeInSamples += StereoPeakEQ::Size();
			puType += StereoPeakEQ::DataSize(pData) / 4;
            break;

		case Component::eStereoRMSLevel:
            uSizeInSamples += StereoRMSLevel::Size();
			puType += StereoRMSLevel::DataSize(pData) / 4;
            break;

        case Component::eStereoLoopingSound:
            uSizeInSamples += StereoLoopingSound::Size();
			puType += StereoLoopingSound::DataSize(pData) / 4;
            break;

        case Component::eStereoLowPass:
            uSizeInSamples += StereoLowPass::Size();
			puType += StereoLowPass::DataSize(pData) / 4;
            break;

		case Component::eStereoHighShelf:
            uSizeInSamples += StereoHighShelf::Size();
			puType += StereoHighShelf::DataSize(pData) / 4;
            break;

        case Component::eStereoOneShotSound:
            uSizeInSamples += StereoOneShotSound::Size();
			puType += StereoOneShotSound::DataSize(pData) / 4;
            break;

        case Component::eStereoOneShotSoundTable:
			uSizeInSamples += StereoOneShotSoundTable::Size() + StereoOneShotSoundTable::WaveHandleArraySize(pData);
			puType += StereoOneShotSoundTable::DataSize(pData) / 4;
            break;

		case Component::eStereoResonantLowPass:
            uSizeInSamples += StereoResonantLowPass::Size();
			puType += StereoResonantLowPass::DataSize(pData) / 4;
            break;

        case Component::eStereoSaturator:
            uSizeInSamples += StereoSaturator::Size();
			puType += StereoSaturator::DataSize(pData) / 4;
            break;

        case Component::eStereoCompressor:
            uSizeInSamples += StereoCompressor::Size();
			puType += StereoCompressor::DataSize(pData) / 4;
            break;

   //     case Component::eStereoDistortion:
   //         uSizeInSamples += StereoDistortion::Size();
			//puType += StereoDistortion::DataSize(pData) / 4;
   //         break;

   //     case Component::eStereoFeedbackDelay:
   //         uSizeInSamples += StereoFeedbackDelay::Size();
			//puType += StereoFeedbackDelay::DataSize(pData) / 4;
   //         break;

		case Component::eStereoPitchShift:
            uSizeInSamples += StereoPitchShift::Size();
			puType += StereoPitchShift::DataSize(pData) / 4;
            break;

        case Component::eStereoToMonoMixer:
            uSizeInSamples += StereoToMonoMixer::Size();
			puType += StereoToMonoMixer::DataSize(pData) / 4;
            break;

        case Component::eStereoToStereoMixer:
            uSizeInSamples += StereoToStereoMixer::Size();
			puType += StereoToStereoMixer::DataSize(pData) / 4;
            break;

        case Component::eStereoToQuadMixer:
            uSizeInSamples += StereoToQuadMixer::Size();
			puType += StereoToQuadMixer::DataSize(pData) / 4;
            break;

        case Component::eQuadToSurroundMixer:
            uSizeInSamples += QuadToSurroundMixer::Size();
			puType += QuadToSurroundMixer::DataSize(pData) / 4;
            break;

        case Component::eStereoToSurroundMixer:
            uSizeInSamples += StereoToSurroundMixer::Size();
			puType += StereoToSurroundMixer::DataSize(pData) / 4;
            break;

        case Component::eSum:
            uSizeInSamples += Sum::Size();
			puType += Sum::DataSize(pData) / 4;
            break;

        case Component::eSurroundAudioInput:
            uSizeInSamples += SurroundAudioInput::Size();
			puType += SurroundAudioInput::DataSize(pData) / 4;
            break;

        case Component::eSurroundAudioOutput:
            uSizeInSamples += SurroundAudioOutput::Size();
			puType += SurroundAudioOutput::DataSize(pData) / 4;
            break;

        case Component::eSurroundLFEAudioInput:
            uSizeInSamples += SurroundLFEAudioInput::Size();
			puType += SurroundLFEAudioInput::DataSize(pData) / 4;
            break;

        case Component::eSurroundLFEAudioOutput:
            uSizeInSamples += SurroundLFEAudioOutput::Size();
			puType += SurroundLFEAudioOutput::DataSize(pData) / 4;
            break;

		case Component::eSurroundDelay:
            uSizeInSamples += SurroundDelay::Size();
            break;

		//case Component::eSurroundHighShelf:
  //          uSizeInSamples += SurroundHighShelf::Size();
		//	puType += SurroundHighShelf::DataSize(pData) / 4;
  //          break;

		//case Component::eSurroundLowShelf:
  //          uSizeInSamples += SurroundLowShelf::Size();
		//	puType += SurroundLowShelf::DataSize(pData) / 4;
  //          break;

        case Component::eSurroundLowPass:
            uSizeInSamples += SurroundLowPass::Size();
			puType += SurroundLowPass::DataSize(pData) / 4;
            break;

        case Component::eSurroundPeakEQ:
            uSizeInSamples += SurroundPeakEQ::Size();
			puType += SurroundPeakEQ::DataSize(pData) / 4;
            break;

		case Component::eSurroundRMSLevel:
            uSizeInSamples += SurroundRMSLevel::Size();
			puType += SurroundRMSLevel::DataSize(pData) / 4;
            break;

		case Component::eSurroundResonantLowPass:
            uSizeInSamples += SurroundResonantLowPass::Size();
			puType += SurroundResonantLowPass::DataSize(pData) / 4;
            break;

        case Component::eSurroundSaturator:
            uSizeInSamples += SurroundSaturator::Size();
			puType += SurroundSaturator::DataSize(pData) / 4;
            break;

        case Component::eSurroundCompressor:
            uSizeInSamples += SurroundCompressor::Size();
			puType += SurroundCompressor::DataSize(pData) / 4;
            break;

        case Component::eSurroundToSurroundMixer:
            uSizeInSamples += SurroundToSurroundMixer::Size();
			puType += SurroundToSurroundMixer::DataSize(pData) / 4;
            break;

        case Component::eSurroundToSurroundLFEMixer:
            uSizeInSamples += SurroundToSurroundLFEMixer::Size();
			puType += SurroundToSurroundLFEMixer::DataSize(pData) / 4;
            break;

        case Component::eSurroundLFEToSurroundLFEMixer:
            uSizeInSamples += SurroundLFEToSurroundLFEMixer::Size();
			puType += SurroundLFEToSurroundLFEMixer::DataSize(pData) / 4;
            break;

        case Component::eTimer:
            uSizeInSamples += Timer::Size();
			puType += Timer::DataSize(pData) / 4;
            break;

        case Component::eMetronome:
            uSizeInSamples += Metronome::Size();
			puType += Metronome::DataSize(pData) / 4;
            break;

        case Component::eVariable:
            uSizeInSamples += Variable::Size();
			puType += Variable::DataSize(pData) / 4;
            break;

        case Component::eVector:
            uSizeInSamples += Vector::Size();
			puType += Vector::DataSize(pData) / 4;
            break;

        case Component::eVectorIn:
            uSizeInSamples += VectorIn::Size();
			puType += VectorIn::DataSize(pData) / 4;
            break;

		case Component::eVectorNearestPointOnLine:
            uSizeInSamples += VectorNearestPointOnLine::Size();
			puType += VectorNearestPointOnLine::DataSize(pData) / 4;
            break;

        case Component::eImpactFilter:
            uSizeInSamples += ImpactFilter::Size();
			puType += ImpactFilter::DataSize(pData) / 4;
            break;

        case Component::eMessageDelay:
            uSizeInSamples += MessageDelay::Size();
			puType += MessageDelay::DataSize(pData) / 4;
            break;

        default:
            AUDIO_SYS_FATAL("Error: Unknown component type %d", *puType);
            break;
        };

		puType++;
		uSizeInSamples += sizeof(S32);	// now storing component type too

		//puType = (U32*)pCom->Init(pData);
	}

	m_u32ComponentFootprintSize = uSizeInSamples;

	return uSizeInSamples;
}
#endif	//	#ifndef PLATFORM_SPU

#ifndef PLATFORM_SPU
////////////////////////////////////////////////////////////////////////////////
//	CreateMasterInstance - only done once per object
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::CreateMasterInstance()
{
	m_lockInstance.Acquire();

	Instance* pInstance;

	AUDIO_SYS_DEBUG("Creating Master Instance of '%s'", m_pchStringTable);

	//System::Get()->PushCurrentMemoryAllocationId(3);
	MEMORY_TRACKER_PUSH("Audio.Objects.Instances");

    U8* pInstanceData = NULL;
    U8* pContext = NULL;
	U8* pCoreData = NULL;
	#ifndef CFG_GOLD
	U32 uInstanceMem = 0;
	#endif
	U32 uStateDataSize = 0;

    //  calculate memory required for each instance
    U32 uObjContextSize = 0;
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        U32 uAlignedSize = m_apComponents[uCount]->ContextSize();
        uAlignedSize += 0x0f;
        uAlignedSize &= 0xfffffff0;
        uObjContextSize += uAlignedSize;
    }

    if (uObjContextSize > 0)
    {
		U32 alignedSizeOfInstance = (sizeof(Instance) + 15) & ~0xf;

#if defined( PLATFORM_PS3 )
		U32 sizeOfCurrentValues = ((GetDefinition()->NumComponents() * sizeof(F32)) + 0x0f) & ~0x0f;

#else
		U32 sizeOfCurrentValues = 0;
#endif

		// calculate size of array of streams and adpcm info structures to append to core memory
		U32 u32StreamInfoSize = ((m_u32NumStreams * sizeof(Core::WaveStreamData)) + 4 + 0x0f) & ~0x0f;
		U32 u32AdpcmInfoSize = ((GetNumberOfAdpcmWaves() * sizeof(Core::AdpcmWaveInfo)) + 0x0f) & ~0x0f;

        //  allocate instance memory
        uStateDataSize = alignedSizeOfInstance + uObjContextSize + sizeOfCurrentValues;
//        uStateDataSize = alignedSizeOfInstance + uObjContextSize + sizeOfCurrentValues;

		#ifndef CFG_GOLD
		if (uStateDataSize > m_pManager->GetMaxInstanceStateDataSize())
		{
			m_pManager->SetMaxInstanceStateDataSize(uStateDataSize);
			m_pManager->SetPeakInstanceStateDataObjectName(m_pchStringTable);
		}
		#endif

		AUDIO_SYS_DEBUG("State data size %d bytes (%.2fk). MaxStateDataSize is %d bytes (%.2fk) '%s'", uStateDataSize, (F32)uStateDataSize / 1024.f, m_pManager->GetMaxInstanceStateDataSize(), (F32)m_pManager->GetMaxInstanceStateDataSize()/1024.f, m_pchStringTable);
		
		//if (uObjContextSize > static_cast<U32>(System::Get()->GetStateBufferSize()))
		//	AUDIO_SYS_FATAL("State data size for '%s' is too big for shared instance state data buffer. Instance needs %d bytes but buffer size is %d.", m_pchStringTable, uObjContextSize, System::Get()->GetStateBufferSize()); 

		pInstanceData = (U8*)System::Alloc(uStateDataSize, 16);

        #ifndef CFG_GOLD
		uInstanceMem += uStateDataSize;
		#endif
		Assert(pInstanceData != NULL);

	//	MEMORY_TRACKER_POP("oldtech.audio.Instance");

        F32* afCurrentVals = (F32*)(pInstanceData + alignedSizeOfInstance + uObjContextSize);
#if defined( PLATFORM_PS3 )
		memset(afCurrentVals, 0xff, sizeOfCurrentValues);
#endif

        pContext = pInstanceData + alignedSizeOfInstance;

		//pInstance = (Instance*)pInstanceData;
		pInstance = new(pInstanceData) Instance;
        #ifndef CFG_GOLD
		uInstanceMem += sizeof(Instance);
		#endif

		pInstance->SetObject(this);
		pInstance->Suspend(false);
		pInstance->Mute(false);
//		pInstance->ResetStateFlag(0xffffffff);	// reset all bits
//		pInstance->SetStateFlag(InstanceNotActivated);
		pInstance->SetContext((U8*)pContext);
		pInstance->SetContextSize(uObjContextSize);
		pInstance->SetStreamInfoSize(u32StreamInfoSize);
		pInstance->SetPrev(NULL);
		pInstance->SetNext(NULL);
		pInstance->SetPrevActive(NULL);
		pInstance->SetNextActive(NULL);
		pInstance->LinkedIntoActive(false);
#if defined( PLATFORM_PS3 )
		pInstance->SetCurrentValues(afCurrentVals);
#else
		pInstance->SetCurrentValues(NULL);
#endif

		pInstance->m_coreInfo.uFrameNum = 0;
		pInstance->m_coreInfo.uFrameMod = 0;
		pInstance->m_coreInfo.uFrameInt = 1;
		pInstance->m_coreInfo.fFraction = 0.f;

		pInstance->m_coreInfo.bMutePrev = true;
		pInstance->m_coreInfo.bMuteNext = false;
		pInstance->m_coreInfo.uFrameNum = System::Get()->GetFrameNum();
		pInstance->m_coreInfo.uFrameInt = (U16)pInstance->GetObject()->FrameSkip();
		//pInstance->m_coreInfo.uFrameMod = pInstance->m_coreInfo.uFrameNum % pInstance->m_coreInfo.uFrameInt;
		pInstance->m_coreInfo.uFrameMod++;
		if (pInstance->m_coreInfo.uFrameMod >= pInstance->m_coreInfo.uFrameInt)
			pInstance->m_coreInfo.uFrameMod = 0;
		pInstance->m_coreInfo.fFraction = (F32)pInstance->m_coreInfo.uFrameMod * Maths::RcpEst((F32)pInstance->m_coreInfo.uFrameInt);
		pInstance->m_coreInfo.fWidth = Maths::RcpEst((F32)pInstance->m_coreInfo.uFrameInt);

		pInstance->m_coreInfo.pCoreData = pInstance->GetCoreData();

		pInstance->m_coreInfo.pStaticBuffers = (F32*)pInstance->GetBufferData();
		pInstance->m_coreInfo.pDynamicBuffers = Audio::ObjectManager::m_pfMixerFrames;

		if (m_u32NumStreams)
		{
			pInstance->SetWaveServerOffset(m_u32CoreSize);
			m_u32CoreSize += u32StreamInfoSize;
		}

		if (GetNumberOfAdpcmWaves())
		{
			pInstance->SetAdpcmInfoOffset(m_u32CoreSize);
			m_u32CoreSize += u32AdpcmInfoSize + 4;
		}

		AUDIO_SYS_DEBUG("Core classes data size %d bytes (%.2fk)", m_u32CoreSize, (F32)m_u32CoreSize / 1024.f);
        #ifndef CFG_GOLD
		uInstanceMem += m_u32CoreSize;
		#endif
    }
    else
	{
		MEMORY_TRACKER_POP("Audio.Objects.Instances");
        return;
	}

	MEMORY_TRACKER_POP("Audio.Objects.Instances");

	m_lockInstance.Release();

	m_pMasterInstance = pInstance;
	m_uMasterInstanceSize = uStateDataSize;
}

////////////////////////////////////////////////////////////////////////////////
//	CreateInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::Object::CreateInstance(const char* pchGroupName, const char* pchEntityName)
{
	m_lockInstance.Acquire();

	Instance* pInstance;

	//AUDIO_SYS_DEBUG("Creating Instance of '%s'", m_pchStringTable);

	MEMORY_TRACKER_PUSH("Audio.Objects.Instances");

	U8* pCoreData = NULL;

	//	create instance from master copy
	U8* pInstanceData = (U8*)System::Alloc(m_uMasterInstanceSize, 16);
	memcpy(pInstanceData, m_pMasterInstance, m_uMasterInstanceSize);

	pInstance = (Instance*)pInstanceData;

	U32 alignedSizeOfInstance = (sizeof(Instance) + 15) & ~0xf;
	U8* pContext = pInstanceData + alignedSizeOfInstance;
	pInstance->SetContext((U8*)pContext);

	#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU) && !defined CFG_GOLD
	if (m_uNameHash == 0xffffffff)
		m_uNameHash = StringHash(Name()).GetValue();
	pInstance->SetEntityName(pchEntityName);
	pInstance->SetGroupName(pchGroupName);
	#endif

//MEMORY_TRACKER_PUSH("Audio.Objects.Instances.Core");
		pCoreData = (U8*)System::Alloc(m_u32CoreSize, 16);
//MEMORY_TRACKER_POP("Audio.Objects.Instances.Core");
		if (pCoreData)
		{
			pInstance->SetCoreData(pCoreData);	//	TODO: remove this dup
			pInstance->SetCoreDataSize(m_u32CoreSize);

			pInstance->m_coreInfo.pCoreData = pCoreData;
		}

		if (GetNumberOfAdpcmWaves())
		{
			pInstance->ResetNumAdpcmWavesNeedingUpdate();

			for (U32 i = 0; i < GetNumberOfAdpcmWaves(); i++)
				pInstance->GetAdpcmWaveInfo(i)->Init();
		}

		if (GetNumberOfStreams())
			pInstance->ResetNumStreamsNeedingUpdate();

        for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
            m_apComponents[uCount]->InitContext(pInstance);

    //  configure audio graph and effects : CORE_FRAME_BUFFER_MEMORY
    ConfigureAudio(pInstance);

#ifdef PLATFORM_PS3
    for (U32 uCount = 0; uCount < m_apTransmitters.Size(); uCount++)
    {
		Component* pCom = m_apTransmitters[uCount];

		Assert ( pCom->IsAudio( ) );

		AudioComponent* pAudioCom = (AudioComponent*)pCom;

		switch (pAudioCom->AudioOutType(pInstance))
		{
		case Core::eOutput:
			{
				U32 u32BufferSize = ((Core::Output*)&pInstance->m_coreInfo.pCoreData[pAudioCom->AudioOutOffset(pInstance)])->Core::Output::Channels() * Core::uFrameLength * sizeof(F32);
				if (u32BufferSize > m_u32LargestRequiredDmaBuffer)
					m_u32LargestRequiredDmaBuffer = u32BufferSize;
			}
			break;

		default:
			Assert(0);
		}
	}

	for (U32 uCount = 0; uCount < m_apReceivers.Size(); uCount++)
    {
		Component* pCom = m_apReceivers[uCount];

		Assert ( pCom->IsAudio( ) );

		AudioComponent* pAudioCom = (AudioComponent*)pCom;

		switch (pAudioCom->AudioOutType(pInstance))
		{
		case Core::eInput:
			{
				U32 u32BufferSize = ((Core::Input*)&pInstance->m_coreInfo.pCoreData[pAudioCom->AudioOutOffset(pInstance)])->Core::Input::Channels() * Core::uFrameLength * sizeof(F32);
				if (u32BufferSize > m_u32LargestRequiredDmaBuffer)
					m_u32LargestRequiredDmaBuffer = u32BufferSize;
			}
			break;

		default:
			Assert(0);
		}
	}
#endif


	U32 uMixerId = 0;
	U32 uSizeInSamples = 0;
	Core::BufferStack buffers;
    for (U32 uCount = 0; uCount < m_apTransmitters.Size(); uCount++)
    {
		Component* pCom = m_apTransmitters[uCount];

		Assert ( pCom->IsAudio( ) );

		AudioComponent* pAudioCom = (AudioComponent*)pCom;

		if(!pAudioCom)
		{

			AUDIO_SYS_ERROR("pAudioCom not valid! Entity: %s, ", pchEntityName);
			assert(pAudioCom);
			return NULL;
		}

		switch (pAudioCom->AudioOutType(pInstance))
		{
		case Core::eOutput:
			((Core::Output*)&pInstance->m_coreInfo.pCoreData[pAudioCom->AudioOutOffset(pInstance)])->DynamicBufferDepths(pInstance->m_coreInfo, buffers);
			break;

		default:
			Assert(0);
		}
	}

    for (U32 uCount = 0; uCount < m_apTransmitters.Size(); uCount++)
    {
		Assert ( ( (Component*)m_apTransmitters[ uCount ] )->IsAudio( ) );

		switch ( ( (AudioComponent*)m_apTransmitters[ uCount ] )->AudioOutType( pInstance ) )
		{
		case Core::eOutput:	//	TODO: helper func?
			( (Core::Output*)&pInstance->m_coreInfo.pCoreData[ ( (AudioComponent*)m_apTransmitters[ uCount ] )->AudioOutOffset( pInstance ) ] )->DynamicBufferOffsets( pInstance->m_coreInfo, buffers );
			break;

		default:
			Assert(0);
		}
	}

	uSizeInSamples = Core::uFrameLength * buffers.GetFootprint();

	//	instance state/context cannot be larger than scratch buffer mem, will break PS3
//	Assert(uObjContextSize < uSizeInSamples);

    if (uSizeInSamples > 0)
    {
		U32 uSizeInBytes = uSizeInSamples * sizeof(F32);
		AUDIO_SYS_DEBUG("Core dynamic frame buffers data size %d bytes (%.2fk)", uSizeInBytes, (F32)uSizeInBytes / 1024.f);

		#ifndef CFG_GOLD
		#ifndef AUDIO_USE_COMPONENT_ARRAY
			U32 uComponentClassesDataSize = CalculateComponentFootprint(m_iDefinition) + m_apComponents.GetIndexArraySize();
		#else
			U32 uComponentClassesDataSize = CalculateComponentFootprint(m_iDefinition);
		#endif
		U32 uTotSize = uSizeInBytes + m_iDefinition->GetDataSize() + uComponentClassesDataSize + m_uMasterInstanceSize + m_u32CoreSize;
		AUDIO_SYS_DEBUG("==== Total Instance Footprint for '%s' is %d bytes (%.2fk) ====", m_pchStringTable, uTotSize, uTotSize / 1024.f);
		#endif

		#ifndef CFG_GOLD
		if (uSizeInSamples > m_pManager->TempFrameBufferSizeInSamples())
			AUDIO_SYS_FATAL("'%s': dynamic frame buffer size of %d k (%d bytes) exceeds limit of %d k (%d bytes). Need to increase audio object instance temp frame buffer size", m_pchStringTable, uSizeInBytes/1024, uSizeInBytes, m_pManager->TempFrameBufferSizeInBytes()/1024, m_pManager->TempFrameBufferSizeInBytes());
		#endif

		#ifndef CFG_GOLD
		if (uSizeInBytes > m_pManager->GetPeakTempFrameBufferSize())
		{
			m_pManager->SetPeakTempFrameBufferSize(uSizeInBytes);
			m_pManager->SetPeakTempFrameBufferObjectName(m_pchStringTable);
		}
		#endif	

		AUDIO_SYS_DEBUG("Instance uses %d bytes out of %d of scratch frame buffer memory. Peak is %d bytes ('%s')", 
			uSizeInBytes, 
			m_pManager->TempFrameBufferSizeInBytes(), 
			m_pManager->GetPeakTempFrameBufferSize(), 
			m_pManager->GetPeakTempFrameBufferObjectName());

		m_u32DynamicBufferSizeInSamples = uSizeInSamples;
    }

	uSizeInSamples = 0;
    uMixerId = 0;
    for (U32 uCount = 0; uCount < m_apTransmitters.Size(); uCount++)
    {
		Component* pCom = m_apTransmitters[uCount];

		Assert ( pCom->IsAudio( ) );

		AudioComponent* pAudioCom = (AudioComponent*)pCom;

		switch (pAudioCom->AudioOutType(pInstance))
		{
		//case Core::DSP::Effect::eFeedbackDelay:
		//	uSizeInBytes = ((Core::DSP::FeedbackDelay*)&pInstance->m_coreInfo.pCoreData[m_apTransmitters[uCount]->DSPEffectUnit(pInstance->GetStateData(), 0)])->StaticMemoryUse(pInstance->m_coreInfo, uSizeInBytes);
		//	break;
		case Core::eOutput:
			uSizeInSamples = ((Core::Output*)&pInstance->m_coreInfo.pCoreData[pAudioCom->AudioOutOffset(pInstance)])->StaticBufferOffsets(pInstance->m_coreInfo, uSizeInSamples);
			break;

		}
	}

    if (uSizeInSamples > 0)
    {
		U32 uSizeInBytes = uSizeInSamples * sizeof(F32);
		AUDIO_SYS_DEBUG("Core static frame buffers data size 0x%x bytes (%.2fk)", uSizeInBytes, (F32)uSizeInBytes / 1024.f);
        
		System::Get()->PushCurrentMemoryAllocationId(4);

		MEMORY_TRACKER_PUSH("Audio.StaticFrameBuffers");
		void* pBufferData = Audio::System::Get()->Alloc(uSizeInBytes, 128);
		#if 0 // TODO : fix these
		uInstanceMem += uSizeInSamples;
		#endif
		MEMORY_TRACKER_POP("Audio.StaticFrameBuffers");

		pInstance->SetBufferData((U8*)pBufferData);
		pInstance->SetBufferDataSize(uSizeInBytes);

		System::Get()->PopCurrentMemoryAllocationId();
    }
	else
	{
		pInstance->SetBufferData(NULL);
		pInstance->SetBufferDataSize(0);
	}

	if (pInstance->GetNumberOfStreamsCreated())
		pInstance->m_coreInfo.decoderDataInfo.pWaveServerData = pInstance->GetWaveServerData();
	else
		pInstance->m_coreInfo.decoderDataInfo.pWaveServerData = 0;

	m_u32TotalInstanceCount++;

	//System::Get()->PopCurrentMemoryAllocationId();
	MEMORY_TRACKER_POP("Audio.Objects.Instances");

	// jkc- bug 29834- commented out the master build check- was causing loss of all audio on PS3 cos the reverb DSP is too big.
	// should probably comment back in once reverb is fixed.

	//#ifndef CFG_GOLD
	if (!m_u32InstanceFootprint)
		m_u32InstanceFootprint = m_uMasterInstanceSize + pInstance->GetCoreDataSize() + pInstance->GetBufferDataSize();
	m_u32TotalInstanceFootprint += m_u32InstanceFootprint;

	if (! m_u32SpuInstanceFootprint)
		m_u32SpuInstanceFootprint = m_u32InstanceFootprint + (m_u32DynamicBufferSizeInSamples * sizeof(F32)) + m_u32ComponentFootprintSize + GetDefinition()->GetDataSize() + GetDmaBufferSize();
	//#endif

#ifdef PLATFORM_PS3
	U32 u32DataSize = (m_u32SpuInstanceFootprint) / 1024;

	if (u32DataSize > 68)
	{
		AUDIO_SYS_ERROR("Size of instance %s is %.2dK\n", Name(), u32DataSize);
		return 0;
	}

#endif

	//if (pCoreData)
	//{
	//	for (U32 i = 0; i < m_u32NumStreams; i++)
	//	{
	//		Core::WaveStreamData* pInfo = &(((Core::WaveStreamData*)pInstance->GetWaveServerData())[i]);
	//		pInfo->Init();
	//	}
	//}

	m_aInstances.PushBack(pInstance);

	m_lockInstance.Release();

	return pInstance;
}

////////////////////////////////////////////////////////////////////////////////
//	DestroyInstance
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::DestroyInstance(Instance* pInstance)
{
	m_lockInstance.Acquire();

	for (int i=0; i<m_aInstances.Size(); i++)
	{
		if (m_aInstances[i] == pInstance)
		{
			m_aInstances.EraseByIndex(i);
			break;
		}
	}

	Assert (pInstance->StateFlagIsSet(InstanceReleasePending));
	
	System::Get()->PushCurrentMemoryAllocationId(3);
	MEMORY_TRACKER_PUSH("Audio.Objects.Instances");

//#if !defined(BUILD_CONFIG_SPEED) && !defined(BUILD_CONFIG_MASTER)
//	m_pManager->m_TimingData.RemoveTag(m_pchStringTable);
//#endif

    //  release state memory
    if (pInstance)
    {
		for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
			m_apComponents[uCount]->ReleaseContext(pInstance);

		for (U32 i = 0; i < pInstance->GetNumberOfStreamsCreated(); i++)
		{
			Core::WaveStreamData* pData = pInstance->GetWaveServerStreamData(i);
			Audio::System::Get()->GetWaveServer()->DestroyStream(pData->hWaveData);
		}

		//pInstance->~Instance();
        U8* pInstanceData = (U8*)pInstance;

		System::Free(pInstance->GetCoreData());

		if (pInstance->GetBufferData())
			System::Free(pInstance->GetBufferData());

		//AUDIO_SYS_DEBUG("Object::DestroyInstance: (0x%.8x) '%s'", pInstance, pInstance->GetObject()->Name());
		System::Free(pInstanceData);
    }


	m_u32TotalInstanceCount--;

	#ifndef CFG_GOLD
	m_u32TotalInstanceFootprint -= m_u32InstanceFootprint;
	#endif

	System::Get()->PopCurrentMemoryAllocationId();
	MEMORY_TRACKER_POP("Audio.Objects.Instances");

	m_lockInstance.Release();
}

////////////////////////////////////////////////////////////////////////////////
//	ConfigureAudio
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::ConfigureAudio(Instance* pInstance)
{
	AUDIO_SYS_DEBUG("Configuring audio graph for '%s'", pInstance->GetObject()->Name());

    //  configure audio graph and effects
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
	{
		Component* pCom = m_apComponents[uCount];

		if ( pCom->IsAudio( ) )
		{
			AudioComponent* pAudioCom = (AudioComponent*)pCom;

			pAudioCom->ConfigureAudio(pInstance);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	StopAllSounds
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::StopAllSounds(Instance* pInstance)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
	{
		switch (m_apComponents[uCount]->TypeId())
		{
			case Component::eMonoOneShotSound:
			((MonoOneShotSound*)m_apComponents[uCount])->DoMessage(pInstance, MonoOneShotSound::StopMsg);
			break;

			case Component::eMonoOneShotSoundTable:
			((MonoOneShotSoundTable*)m_apComponents[uCount])->DoMessage(pInstance, MonoOneShotSoundTable::StopMsg);
			break;

			case Component::eMonoLoopingSound:
			((MonoLoopingSound*)m_apComponents[uCount])->DoMessage(pInstance, MonoLoopingSound::StopMsg);
			break;

			case Component::eStereoOneShotSound:
			((StereoOneShotSound*)m_apComponents[uCount])->DoMessage(pInstance, StereoOneShotSound::StopMsg);
			break;

			case Component::eStereoOneShotSoundTable:
			((StereoOneShotSoundTable*)m_apComponents[uCount])->DoMessage(pInstance, StereoOneShotSoundTable::StopMsg);
			break;

			case Component::eStereoLoopingSound:
			((StereoLoopingSound*)m_apComponents[uCount])->DoMessage(pInstance, StereoLoopingSound::StopMsg);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::Object::Release()
{
	MEMORY_TRACKER_PUSH("Audio.Objects.Objects");

	// -------------------------------------------
	// REMOVE THIS???
#ifdef AUDIO_ERRORS_ARE_FATAL
//	Assert (! m_u32ActiveInstanceCount);
#endif
	// JASOND: POTENTIAL DEADLOCK HERE!
	#if 0 // JASOND: This keeps deadlocking on quitting out to section select...
	while (m_u32ActiveInstanceCount)
		Sleep(1);
    #else
    S32 waitCount = 1000;

	while ( m_u32ActiveInstanceCount && ( waitCount-- > 0 ) )
	{
#ifndef PLATFORM_PS3
		Sleep( 1 );
#else
		sys_timer_sleep( 1 );
#endif
	}
	if ( waitCount <= 0 )
	{
	    Assert( 0 && "Audio::Object::Release(): Deadlock force-broken!\n" );
	}
    #endif
	// -------------------------------------------
	

    //  delete components
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
		m_apComponents[uCount]->Release();

	if (m_pComponents)
	{
#ifdef AUDIO_USE_COMPONENT_ARRAY
		delete m_pComponents;
#endif
		m_pComponents = NULL;
	}

    m_apComponents.Clear();

	MEMORY_TRACKER_POP("Audio.Objects.Objects");

    return Success;
}
#endif	//	ifndef PLATFORM_SPU


////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::Update(Instance* pInstance)
{
	static U32 maxSize = 0;
	AUDIO_PERF_EVENT("Audio.Update.Object");

	OBJECT_CPU_COUNTER_BEGIN(pInstance);

	//	double buffer state mem and copy
	U32 uStateSize = pInstance->GetStateSize();
	U8* pState = pInstance->GetStateData();
	maxSize = Maths::Max(maxSize, uStateSize);
#ifndef PLATFORM_SPU
	U8* pNewState = (U8*)alloca(uStateSize);//new U8[ uStateSize ];
#else
	U8* pNewState = new U8[ uStateSize ];
#endif
	memcpy(pNewState, pState, uStateSize);
	pInstance->SetNewStateData(pNewState);

	// prepare object for update on spu..
#ifdef PLATFORM_SPU
	m_pInitData = (U8*)m_iDefinition->ComponentData();
#endif

#ifndef PLATFORM_SPU
    for (U32 uCount = 0; uCount < m_apUpdaters.Size(); uCount++)
        m_apUpdaters[uCount]->Update(pInstance);
#else
	for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
	{
		if (GetComponentFlags(uCount) & Component::flagUpdater)
			UpdateComponent(m_apComponents[uCount], pInstance);
	}
#endif

	//	copy state back
	memcpy(pState, pNewState, uStateSize);

#ifndef PLATFORM_SPU
#ifdef ENABLE_DEBUG_EXTENSIONS
	if (pInstance->m_bDumpStats)
	{
        Core::Internals internals;

        AUDIO_SYS_DEBUG("");

		for (U32 uCount = 0; uCount < m_apTransmitters.Size(); uCount++)
		{
			Core::IGenerator* pGen = (Core::IGenerator*)&pInstance->m_coreInfo.pCoreData[m_apTransmitters[uCount]->AudioOutOffset(pInstance)];
			pGen->DumpStats(pInstance->m_coreInfo, "-", &internals);
		}

        AUDIO_SYS_DEBUG("");
        AUDIO_SYS_DEBUG(" Active Generators = %d/%d", internals.uNumGeneratorsActive, internals.uTotalNumGenerators);
        AUDIO_SYS_DEBUG(" Active Waves = %d/%d", internals.uNumWavesActive, internals.uTotalNumWaves);
        AUDIO_SYS_DEBUG(" Active Mixers = %d/%d", internals.uNumMixersActive, internals.uTotalNumMixers);
        AUDIO_SYS_DEBUG("");

        pInstance->m_bDumpStats = false;
	}
#endif
#endif

	OBJECT_CPU_COUNTER_END(pInstance);
}

//extern bool bPrintGenerationInfo;
extern bool bShowGenerationInfo;

//Debug::Bool g_DebugAudioNewComUpdate( "Audio.System.Performance.NewComponentUpdate", true );
////////////////////////////////////////////////////////////////////////////////
//	ProcessConnections
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::Object::ProcessConnections(Instance* pInstance)
{
	OBJECT_CPU_COUNTER_BEGIN(pInstance);

	//	process audio connections
	for (U32 uCount = 0; uCount < m_apAudioConnections.Size(); uCount++)
		m_apAudioConnections[uCount]->ProcessConnection(pInstance);

	if( /*!g_DebugAudioNewComUpdate || */pInstance->m_coreInfo.uFrameMod == 0 )
	{
		//	instance updated this frame
		for (U32 uCount = 0; uCount < m_apConnections.Size(); uCount++)
			m_apConnections[uCount]->ProcessConnection(pInstance);
	}
	
	OBJECT_CPU_COUNTER_END(pInstance);
}
#endif	//	#ifndef PLATFORM_SPU

#ifndef CFG_GOLD
////////////////////////////////////////////////////////////////////////////////
//	DumpProcessFlags
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::DumpProcessFlags( Instance* pInstance )
{
	AUDIO_CORE_INFO( "\n'%s'", pInstance->GetObject()->Name() );

	for (U32 uCount = 0; uCount < m_apAudioConnections.Size(); uCount++)
		m_apAudioConnections[uCount]->DumpProcessFlags( pInstance );
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	ProcessActionOuts
////////////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_PS3
void Audio::Object::ProcessActionOuts(Instance* pInstance)
{
	for (U32 uCount = 0; uCount < m_apActionOuts.Size(); uCount++)
	{
		//if (m_apActionOuts[uCount]->TypeId() == Component::eActionOut)
		{
			ActionOut* pActionOut = (ActionOut*)(m_apActionOuts[uCount]);

			//for (U32 i = 0; i < pActionOut->GetCallCount(pInstance); i++)
			//	pActionOut->Execute(pInstance);

			//temp.. call count is ending up with silly values when getting set on spu, so the above loop takes ages
			if (pActionOut->GetCallCount(pInstance))
				pActionOut->Execute(pInstance);

			pActionOut->ResetCallCount(pInstance);
		}
	}
}
#endif


////////////////////////////////////////////////////////////////////////////////
//	Generate
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::Generate(Instance* pInstance, bool bOnFrameSkip)
{
	AUDIO_PERF_EVENT("Audio.Generate.Object");

#ifndef PLATFORM_SPU
	OBJECT_CPU_COUNTER_BEGIN(pInstance);

	pInstance->m_coreInfo.pDynamicBuffers = ObjectManager::GetDynamicBuffers();

#ifndef CFG_SPEED
	if (bShowGenerationInfo)
		printf("generating audio for object %s\n", m_pchStringTable);
#endif

	if (bOnFrameSkip)
	{
		for (U32 uCount = 0; uCount < m_apTransmitters.Size(); uCount++)
		{
			AudioComponent* pAudioCom = (AudioComponent*)m_apTransmitters[ uCount ];

			Assert( pAudioCom->IsAudio( ) );

			switch( pAudioCom->AudioOutType( pInstance ) )
			{
			case Core::eOutput:
				( (Core::Output*)&pInstance->m_coreInfo.pCoreData[ pAudioCom->AudioOutOffset( pInstance ) ] )->SetProcessFlag( pInstance->m_coreInfo );
				break;

			default:
				AUDIO_CORE_DEBUG("Transmitter not an output (%s)", m_apTransmitters[uCount]->Name());
				break;
			}
		}
	}

    for (U32 uCount = 0; uCount < m_apTransmitters.Size(); uCount++)
	{
	#ifndef CFG_GOLD
		if (bShowGenerationInfo)
			printf("\tgenerating audio for component %s\n", m_apTransmitters[uCount]->Name());
	#endif

        GenerateComponent(m_apTransmitters[uCount], pInstance);
	}
	
	OBJECT_CPU_COUNTER_END(pInstance);
#else

	m_pInitData = (U8*)m_iDefinition->ComponentData();

	{
		AUDIO_PERF_MICRO_EVENT("Audio.SetProcessFlags");

		for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
		{
			if (GetComponentFlags(uCount) & Component::flagTransmitter)
			{
				SetProcessFlagOnComponent(m_apComponents[uCount], pInstance);
			}
		}
	}

	for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
	{
		if (GetComponentFlags(uCount) & Component::flagTransmitter)
		{
			GenerateComponent(m_apComponents[uCount], pInstance);
		}
	}
#endif
}

#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	GetActionIn
////////////////////////////////////////////////////////////////////////////////
Audio::ActionIn* Audio::Object::GetActionIn(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if (m_apComponents[uCount]->Flags() & Component::flagActionIn)
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (ActionIn*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetActionOut
////////////////////////////////////////////////////////////////////////////////
Audio::ActionOut* Audio::Object::GetActionOut(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if (m_apComponents[uCount]->Flags() & Component::flagActionOut)
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (ActionOut*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetParameterIn
////////////////////////////////////////////////////////////////////////////////
Audio::ParameterIn* Audio::Object::GetParameterIn(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if (m_apComponents[uCount]->Flags() & Component::flagParameterIn)
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (ParameterIn*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetParameterOut
////////////////////////////////////////////////////////////////////////////////
Audio::ParameterOut* Audio::Object::GetParameterOut(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if (m_apComponents[uCount]->Flags() & Component::flagParameterOut)
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (ParameterOut*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetVectorIn
////////////////////////////////////////////////////////////////////////////////
Audio::VectorIn* Audio::Object::GetVectorIn(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if (m_apComponents[uCount]->Flags() & Component::flagVectorIn)
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (VectorIn*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetControlInput
////////////////////////////////////////////////////////////////////////////////
Audio::ControlInput* Audio::Object::GetControlInput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagControl) &&
            (m_apComponents[uCount]->Flags() & Component::flagInputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagControlConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (ControlInput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetEventInput
////////////////////////////////////////////////////////////////////////////////
Audio::EventInput* Audio::Object::GetEventInput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagEvent) &&
            (m_apComponents[uCount]->Flags() & Component::flagInputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagEventConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (EventInput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetMonoAudioInput
////////////////////////////////////////////////////////////////////////////////
Audio::MonoAudioInput* Audio::Object::GetMonoAudioInput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagMono) &&
            (m_apComponents[uCount]->Flags() & Component::flagInputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagAudioConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (MonoAudioInput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetStereoAudioInput
////////////////////////////////////////////////////////////////////////////////
Audio::StereoAudioInput* Audio::Object::GetStereoAudioInput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagStereo) &&
            (m_apComponents[uCount]->Flags() & Component::flagInputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagAudioConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (StereoAudioInput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetSurroundAudioInput
////////////////////////////////////////////////////////////////////////////////
Audio::SurroundAudioInput* Audio::Object::GetSurroundAudioInput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagSurround) &&
            (m_apComponents[uCount]->Flags() & Component::flagInputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagAudioConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (SurroundAudioInput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetSurroundLFEAudioInput
////////////////////////////////////////////////////////////////////////////////
Audio::SurroundLFEAudioInput* Audio::Object::GetSurroundLFEAudioInput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagSurroundLFE) &&
            (m_apComponents[uCount]->Flags() & Component::flagInputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagAudioConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (SurroundLFEAudioInput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetControlOutput
////////////////////////////////////////////////////////////////////////////////
Audio::ControlOutput* Audio::Object::GetControlOutput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagOutputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagControlConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (ControlOutput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetEventOutput
////////////////////////////////////////////////////////////////////////////////
Audio::EventOutput* Audio::Object::GetEventOutput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagOutputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagEventConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (EventOutput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetMonoAudioOutput
////////////////////////////////////////////////////////////////////////////////
Audio::MonoAudioOutput* Audio::Object::GetMonoAudioOutput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagMono) &&
            (m_apComponents[uCount]->Flags() & Component::flagOutputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagAudioConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (MonoAudioOutput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetStereoAudioOutput
////////////////////////////////////////////////////////////////////////////////
Audio::StereoAudioOutput* Audio::Object::GetStereoAudioOutput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagStereo) &&
            (m_apComponents[uCount]->Flags() & Component::flagOutputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagAudioConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (StereoAudioOutput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetSurroundAudioOutput
////////////////////////////////////////////////////////////////////////////////
Audio::SurroundAudioOutput* Audio::Object::GetSurroundAudioOutput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
        if ((m_apComponents[uCount]->Flags() & Component::flagSurround) &&
            (m_apComponents[uCount]->Flags() & Component::flagOutputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagAudioConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (SurroundAudioOutput*)m_apComponents[uCount];
        }
    }

    return NULL;
}


////////////////////////////////////////////////////////////////////////////////
//	GetSurroundLFEAudioOutput
////////////////////////////////////////////////////////////////////////////////
Audio::SurroundLFEAudioOutput* Audio::Object::GetSurroundLFEAudioOutput(const char* pchName)
{
    for (U32 uCount = 0; uCount < m_apComponents.Size(); uCount++)
    {
		if ((m_apComponents[uCount]->Flags() & Component::flagSurroundLFE) &&
            (m_apComponents[uCount]->Flags() & Component::flagOutputConnector) &&
            (m_apComponents[uCount]->Flags() & Component::flagAudioConnector))
        {
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
                return (SurroundLFEAudioOutput*)m_apComponents[uCount];
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//	GetComponentIndex
////////////////////////////////////////////////////////////////////////////////
U16 Audio::Object::GetComponentIndex(const char* pchName, U32 u32Flags)
{
	for (U16 uCount = 0; uCount < m_apComponents.Size(); uCount++)
	{
		if ((m_apComponents[uCount]->Flags() & u32Flags) == u32Flags)
		{
            const char* pchCom = m_apComponents[uCount]->Name();
            if (strcmp(pchCom, pchName) == 0)
				return uCount;
		}
	}

	return 0xffff;
}

#ifdef AUDIO_STRING_HASH_COMPONENTS
////////////////////////////////////////////////////////////////////////////////
//	GetComponentIndex - hash version
////////////////////////////////////////////////////////////////////////////////
// temp code to track down a crash on amax PS3
#ifdef GET_COMPONENT_FROM_HASH_DEBUG_CODE
volatile U32 u32GetCompHashCount = 0;
volatile U32 u32GetCompHashSize = 0;
volatile Audio::Object* u32GetCompHashObject = 0;
#endif
//end of temp code
U16 Audio::Object::GetComponentIndexFromHash(const StringHash nameHash, U32 u32Flags)
{
#ifdef GET_COMPONENT_FROM_HASH_DEBUG_CODE
u32GetCompHashObject = this;
u32GetCompHashSize = m_apComponents.Size();
#endif
	for (U16 uCount = 0; uCount < m_apComponents.Size(); uCount++)
	{
#ifdef GET_COMPONENT_FROM_HASH_DEBUG_CODE
u32GetCompHashCount = uCount;
#endif
		if ((m_apComponents[uCount]->Flags() & u32Flags) == u32Flags)
		{
            if (nameHash == m_apComponents[uCount]->NameHash())
				return uCount;
		}
	}

	return 0xffff;
}
#endif // #ifdef AUDIO_STRING_HASH_COMPONENTS

////////////////////////////////////////////////////////////////////////////////
//	SetFrameSkip
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::SetFrameSkip(U32 frameSkip)
{
	m_uFrameSkip = frameSkip;
	m_fFrameSkip = (F32) frameSkip;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInitData
////////////////////////////////////////////////////////////////////////////////
//U8* Object::GetInitData(U32 uOffset)
//{
//	U8* aData = (U8*)GetDefinition()->ComponentData();
//	return &aData[uOffset];
//}

////////////////////////////////////////////////////////////////////////////////
//	IncrementInstanceCounter
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::IncrementInstanceCounter()
{
#if defined(PLATFORM_PC)
	InterlockedIncrement((volatile long*)&m_u32ActiveInstanceCount);
#else
	cellAtomicIncr32((std::uint32_t*)&m_u32ActiveInstanceCount);
	//m_u32ActiveInstanceCount++;
#endif	//	XBOX_360
}

////////////////////////////////////////////////////////////////////////////////
//	DecrementInstanceCounter
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::DecrementInstanceCounter()
{
#if defined(PLATFORM_PC)
	InterlockedDecrement((volatile long*)&m_u32ActiveInstanceCount);
#else
	cellAtomicDecr32((std::uint32_t*)&m_u32ActiveInstanceCount);
	//m_u32ActiveInstanceCount--;
#endif	//	XBOX_360
}

////////////////////////////////////////////////////////////////////////////////
//	IncrementSuspendedInstanceCounter
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::IncrementSuspendedInstanceCounter()
{
#if defined(PLATFORM_PC)
	InterlockedIncrement((volatile long*)&m_u32SuspendedInstanceCount);
#else
	cellAtomicIncr32((std::uint32_t*)&m_u32SuspendedInstanceCount);
	//m_u32SuspendedInstanceCount++;
#endif	//	XBOX_360
}

////////////////////////////////////////////////////////////////////////////////
//	DecrementSuspendedInstanceCounter
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::DecrementSuspendedInstanceCounter()
{
#if defined(PLATFORM_PC)
	if (m_u32SuspendedInstanceCount)
		InterlockedDecrement((volatile long*)&m_u32SuspendedInstanceCount);
#else
	// TODO: InterlockedIncrement ps3 equivalent
	cellAtomicDecr32((std::uint32_t*)&m_u32SuspendedInstanceCount);
	//m_u32SuspendedInstanceCount--;
#endif	//	XBOX_360
}

///////////////////////////////////////////////////////////////////////////////
//	IncrementMutedInstanceCounter
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::IncrementMutedInstanceCounter()
{
#if defined(PLATFORM_PC)
	InterlockedIncrement((volatile long*)&m_u32MutedInstanceCount);
#else
	cellAtomicIncr32((std::uint32_t*)&m_u32MutedInstanceCount);
	//m_u32SuspendedInstanceCount++;
#endif	//	XBOX_360
}

////////////////////////////////////////////////////////////////////////////////
//	DecrementMutedInstanceCounter
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::DecrementMutedInstanceCounter()
{
#if defined(PLATFORM_PC)
	if (m_u32MutedInstanceCount)
		InterlockedDecrement((volatile long*)&m_u32MutedInstanceCount);
#else
	// TODO: InterlockedIncrement ps3 equivalent
	cellAtomicDecr32((std::uint32_t*)&m_u32MutedInstanceCount);
	//m_u32SuspendedInstanceCount--;
#endif	//	XBOX_360
}

#endif	//	PLATFORM_SPU

#ifdef PLATFORM_SPU
#define STATIC_FLAGS_USE_TABLE
#ifdef STATIC_FLAGS_USE_TABLE
U32 returnZero() { return 0; }

U32 (*staticFlags[])() = 
{
	Audio::ParameterIn::StaticFlags,					// eParameterIn,								0
	Audio::ParameterOut::StaticFlags,					// eParameterOut,								1
	Audio::ActionIn::StaticFlags,						// eActionIn,									2
	Audio::ActionOut::StaticFlags,						// eActionOut,									3
	Audio::VectorIn::StaticFlags,						// eVectorIn,									4
	returnZero,											// eVectorOut,									5
	Audio::ControlInput::StaticFlags,					// eControlInput,								6
	Audio::ControlOutput::StaticFlags,					// eControlOutput,								7
	Audio::EventInput::StaticFlags,						// eEventInput,									8
	Audio::EventOutput::StaticFlags,					// eEventOutput,								9
	Audio::CoordinateInput::StaticFlags,				// eCoordinateInput,							10	
	Audio::CoordinateOutput::StaticFlags,				// eCoordinateOutput,							11	
	Audio::MonoAudioInput::StaticFlags,					// eMonoAudioInput,								12
	Audio::MonoAudioOutput::StaticFlags,				// eMonoAudioOutput,							13	
	Audio::StereoAudioInput::StaticFlags,				// eStereoAudioInput,							14	
	Audio::StereoAudioOutput::StaticFlags,				// eStereoAudioOutput,							15
	Audio::SurroundAudioInput::StaticFlags,				// eSurroundAudioInput,							16
	Audio::SurroundAudioOutput::StaticFlags,			// eSurroundAudioOutput,						17		
	Audio::SurroundLFEAudioInput::StaticFlags,			// eSurroundLFEAudioInput,						18	
	Audio::SurroundLFEAudioOutput::StaticFlags,			// eSurroundLFEAudioOutput,						19
	Audio::Variable::StaticFlags,						// eVariable,									20
	Audio::PiecewiseLinearFunction::StaticFlags,		// ePiecewiseLinearFunction,					21			
	returnZero,											// ePiecewiseCubicFunction,						22
	Audio::Source::StaticFlags,							// eSource,										23
	Audio::Rolloff::StaticFlags,						// eRolloff,									24
	Audio::Cone::StaticFlags,							// eCone,										25
	Audio::LowPass::StaticFlags,						// eLowPass,									26
	returnZero,											// eHighPass,									27
	Audio::Timer::StaticFlags,							// eTimer,										28
	returnZero,											// eMetronome,									29
	returnZero,											// eRotarySwitch,								30	
	returnZero,											// eCrossFade,									31
	Audio::Random::StaticFlags,							// eRandom,										32
	Audio::Selector::StaticFlags,						// eSelector,									33
	returnZero,											// eStateMachine,								34
	returnZero,											// eQuantize,									35
	Audio::Sum::StaticFlags,							// eSum,										36
	Audio::Product::StaticFlags,						// eProduct,									37	
	returnZero,											// eAbsolute,									38
	returnZero,											// eNegate,										39
	Audio::Exponent::StaticFlags,						// eExponent,									40
	Audio::Max::StaticFlags,							// eMax,										41
	Audio::Min::StaticFlags,							// eMin,										42
	returnZero,											// eInvert,										43
	Audio::Gradient::StaticFlags,						// eGradient,									44
	Audio::Spring::StaticFlags,							// eSpring,										45
	returnZero,											// eCosine,										46
	returnZero,											// eSine,										47
	returnZero,											// eRelay,										48
	returnZero,											// eToggle,										49
	returnZero,											// ePeakHold,									50
	Audio::SampleAndHold::StaticFlags,					// eSampleAndHold,								51
	Audio::Envelope::StaticFlags,						// eEnvelope,									52
	returnZero,											// eLowFrequencyOscillator,						53
	Audio::GateSwitch::StaticFlags,						// eGateSwitch,									54
	Audio::Array::StaticFlags,							// eArray,										55
	returnZero,											// eFraction,									56
	returnZero,											// ePiecewiseLinearGraph,						57
	returnZero,											// ePiecewiseCubicGraph,						58
	Audio::MonoOneShotSound::StaticFlags,				// eMonoOneShotSound,							59
	Audio::StereoOneShotSound::StaticFlags,				// eStereoOneShotSound,							60
	Audio::MonoOneShotSoundTable::StaticFlags,			// eMonoOneShotSoundTable,						61
	Audio::StereoOneShotSoundTable::StaticFlags,		// eStereoOneShotSoundTable,					62	
	Audio::MonoLoopingSound::StaticFlags,				// eMonoLoopingSound,							63
	Audio::StereoLoopingSound::StaticFlags,				// eStereoLoopingSound,							64
	returnZero,											// eMonoStreamingSound,							65
	returnZero,											// eStereoStreamingSound,						66
	returnZero,											// eOscillator,									67
	Audio::MonoToMonoMixer::StaticFlags,				// eMonoToMonoMixer,							68
	Audio::MonoToStereoMixer::StaticFlags,				// eMonoToStereoMixer,							69
	Audio::MonoToSurroundMixer::StaticFlags,			// eMonoToSurroundMixer,						70
	Audio::MonoToSurroundLFEMixer::StaticFlags,			// eMonoToSurroundLFEMixer,						71
	Audio::StereoToMonoMixer::StaticFlags,				// eStereoToMonoMixer,							72
	Audio::StereoToStereoMixer::StaticFlags,			// eStereoToStereoMixer,						73
	Audio::StereoToQuadMixer::StaticFlags,				// eStereoToQuadMixer,							74
	Audio::StereoToSurroundMixer::StaticFlags,			// eStereoToSurroundMixer,						75
	Audio::QuadToSurroundMixer::StaticFlags,			// eQuadToSurroundMixer,						76
	returnZero,											// eSurroundToMonoMixer,						77
	returnZero,											// eSurroundToStereoMixer,						78
	Audio::SurroundToSurroundMixer::StaticFlags,		// eSurroundToSurroundMixer,					79
	Audio::SurroundToSurroundLFEMixer::StaticFlags,		// eSurroundToSurroundLFEMixer,					80
	Audio::SurroundLFEToSurroundLFEMixer::StaticFlags,	// eSurroundLFEToSurroundLFEMixer,				81	
	returnZero,											// eMonoToMonoSpectralMixer,					82
	returnZero,											// eMonoToStereoSpectralMixer,					83
	returnZero,											// eMonoToSurroundSpectralMixer,				84	
	returnZero,											// eStereoToMonoSpectralMixer,					85
	returnZero,											// eStereoToStereoSpectralMixer,				86	
	returnZero,											// eStereoToSurroundSpectralMixer,				87	
	returnZero,											// eSurroundToMonoSpectralMixer,				88	
	returnZero,											// eSurroundToStereoSpectralMixer,				89	
	returnZero,											// eSurroundToSurroundSpectralMixer,			90		
	returnZero,											// eMonoSpectralToMonoMixer,					91
	returnZero,											// eMonoSpectralToStereoMixer,					92
	returnZero,											// eMonoSpectralToSurroundMixer,				93	
	returnZero,											// eStereoSpectralToMonoMixer,					94
	returnZero,											// eStereoSpectralToStereoMixer,				95	
	returnZero,											// eStereoSpectralToSurroundMixer,				96	
	returnZero,											// eSurroundSpectralToMonoMixer,				97	
	returnZero,											// eSurroundSpectralToStereoMixer,				98	
	returnZero,											// eSurroundSpectralToSurroundMixer,			99		
	returnZero,											// eMonoSpectralToMonoSpectralMixer,			100
	returnZero,											// eMonoSpectralToStereoSpectralMixer,			101
	returnZero,											// eMonoSpectralToSurroundSpectralMixer,		102
	returnZero,											// eStereoSpectralToMonoSpectralMixer,			103
	returnZero,											// eStereoSpectralToStereoSpectralMixer,		104
	returnZero,											// eStereoSpectralToSurroundSpectralMixer,		105
	returnZero,											// eSurroundSpectralToMonoSpectralMixer,		106
	returnZero,											// eSurroundSpectralToStereoSpectralMixer,		107
	returnZero,											// eSurroundSpectralToSurroundSpectralMixer,	108
	returnZero,											// eMonoGain,									109
	returnZero,											// eStereoGain,									110
	returnZero,											// eSurroundGain,								111
	Audio::MonoDelay::StaticFlags,						// eMonoDelay,									112
	returnZero,											// eStereoDelay,								113
	returnZero,											// eSurroundDelay,								114
	returnZero,											// eMonoPeakLevel,								115
	returnZero,											// eStereoPeakLevel,							116
	returnZero,											// eSurroundPeakLevel,							117
	Audio::MonoRMSLevel::StaticFlags,					// eMonoRMSLevel,								118
	Audio::StereoRMSLevel::StaticFlags,					// eStereoRMSLevel,								119
	Audio::SurroundRMSLevel::StaticFlags,				// eSurroundRMSLevel,							120
	Audio::MonoSaturator::StaticFlags,					// eMonoSaturator,								121
	Audio::StereoSaturator::StaticFlags,				// eStereoSaturator,							122
	Audio::SurroundSaturator::StaticFlags,				// eSurroundSaturator,							123
	returnZero,											// eMonoDistortion,								124
	returnZero,											// eStereoDistortion,							125
	returnZero,											// eSurroundDistortion,							126
	returnZero,											// eMonoPhaser,									127
	returnZero,											// eStereoPhaser,								128
	returnZero,											// eSurroundPhaser,								129
	returnZero,											// eMonoConvolution,							130
	returnZero,											// eStereoConvolution,							131
	returnZero,											// eSurroundConvolution,						132
	returnZero,											// eMonoFeedbackDelay,							133
	returnZero,											// eStereoFeedbackDelay,						134
	returnZero,											// eSurroundFeedbackDelay,						135
	returnZero,											// eMonoVariableDelay,							136
	returnZero,											// eStereoVariableDelay,						137
	returnZero,											// eSurroundVariableDelay,						138
	returnZero,											// eMonoAllPass,								139
	returnZero,											// eStereoAllPass,								140
	returnZero,											// eSurroundAllPass,							141
	Audio::MonoLowPass::StaticFlags,					// eMonoLowPass,								142
	Audio::StereoLowPass::StaticFlags,					// eStereoLowPass,								143
	Audio::SurroundLowPass::StaticFlags,				// eSurroundLowPass,							144
	Audio::MonoHighPass::StaticFlags,					// eMonoHighPass,								145
	Audio::StereoHighPass::StaticFlags,					// eStereoHighPass,								146
	Audio::SurroundHighPass::StaticFlags,				// eSurroundHighPass,							147
	returnZero,											// eMonoResonantLowPass,						148
	returnZero,											// eStereoResonantLowPass,						149
	Audio::SurroundResonantLowPass::StaticFlags,		// eSurroundResonantLowPass,					150
	returnZero,											// eMonoResonantHighPass,						151
	returnZero,											// eStereoResonantHighPass,						152
	returnZero,											// eSurroundResonantHighPass,					153
	returnZero,											// eMono24dBResonantLowPass,					154
	returnZero,											// eStereo24dBResonantLowPass,					155
	returnZero,											// eSurround24dBResonantLowPass,				156
	returnZero,											// eMonoLowShelf,								157
	returnZero,											// eStereoLowShelf,								158
	returnZero,											// eSurroundLowShelf,							159
	returnZero,											// eMonoHighShelf,								160
	returnZero,											// eStereoHighShelf,							161
	returnZero,											// eSurroundHighShelf,							162
	returnZero,											// eMonoPeakEQ,									163
	returnZero,											// eStereoPeakEQ,								164
	returnZero,											// eSurroundPeakEQ,								165
	Audio::MonoCompressor::StaticFlags,					// eMonoCompressor,								166
	Audio::StereoCompressor::StaticFlags,				// eStereoCompressor,							167
	Audio::SurroundCompressor::StaticFlags,				// eSurroundCompressor,							168
	returnZero,											// eMonoLipSync,								169
	returnZero,											// eStereoPitchShift,							170
	returnZero,											// eMono2BandCompressor,						171
	returnZero,											// eStereo2BandCompressor,						172
	returnZero,											// eSurround2BandCompressor,					173
	returnZero,											// eMono3BandCompressor,						174
	returnZero,											// eStereo3BandCompressor,						175
	returnZero,											// eSurround3BandCompressor,					176
	returnZero,											// eMono4BandCompressor,						177
	returnZero,											// eStereo4BandCompressor,						178
	returnZero,											// eSurround4BandCompressor,					179
	returnZero,											// eMonoGraphicEQ,								180
	returnZero,											// eStereoGraphicEQ,							181
	returnZero,											// eSurroundGraphicEQ,							182
	returnZero,											// eMonoGraphicPhaseShift,						183
	returnZero,											// eStereoGraphicPhaseShift,					184
	returnZero,											// eSurroundGraphicPhaseShift,					185
	returnZero,											// eMonoSpectrumAnalyser,						186
	returnZero,											// eStereoSpectrumAnalyser,						187
	returnZero,											// eSurroundSpectrumAnalyser,					188
	Audio::Script::StaticFlags,							// eScript,										189
	Audio::Vector::StaticFlags,							// eVector,										190
	Audio::VectorNearestPointOnLine::StaticFlags,		// eVectorNearestPointOnLine,					191
	Audio::DopplerShift::StaticFlags,					// eDopplerShift,								192
	Audio::DebugPrint::StaticFlags,						// eDebugPrint,									193
	returnZero,											// eSequence,									194
	Audio::MonoSpectralReverb::StaticFlags,				// eMonoSpectralReverb,							195
	returnZero,											// eMonoToAmbisonicsMixer,						196
	returnZero,											// eAmbisonicsToSurroundMixer,					197
	Audio::ImpactFilter::StaticFlags,					// eImpactFilter,								198
	Audio::MessageDelay::StaticFlags,					// eMessageDelay,								199
	returnZero,											// eMonoSpectralEnvelope,						200
	returnZero,											// eStereoSpectralEnvelope,						201
	returnZero,											// eMonoFlanger,								202
	returnZero,											// eStereoFlanger,								203
	Audio::SurroundFlanger::StaticFlags,				// eSurroundFlanger,							204
	returnZero,											// eMonoBitCrusher,								205
	returnZero,											// eStereoBitCrusher,							206
	Audio::SurroundBitCrusher::StaticFlags,				// eSurroundBitCrusher,							207
	Audio::MonoRingModulator::StaticFlags,				// eMonoRingModulator,							208
	returnZero,											// eStereoRingModulator,						209
	Audio::SurroundRingModulator::StaticFlags,			// eSurroundRingModulator,						210
	returnZero,											// eMonoOscilloscope,							211
	returnZero,											// eStereoOscilloscope,							212
	returnZero,											// eSurroundOscilloscope,						213
	Audio::MonoSend::StaticFlags,						// eMonoSend,									214
	Audio::StereoSend::StaticFlags,						// eStereoSend,									215
	returnZero,											// eSurroundSend,								216
	Audio::MonoReturn::StaticFlags,						// eMonoReturn,									217
	Audio::StereoReturn::StaticFlags,					// eStereoReturn,								218
	returnZero,											// eSurroundReturn,								219
};																										
#endif																									
																										
U32 Audio::Object::GetComponentFlags(U32 Com)															
{	
	U32 result;
	U32 index = GetComponentTypeId(Com);																
																									
#ifdef STATIC_FLAGS_USE_TABLE																			
	result = (*staticFlags[ index ])();
#else
	result = 0;
	switch (index)
	{
		case Component::eParameterIn:									result = ParameterIn::StaticFlags();					break;
		case Component::eParameterOut: 									result = ParameterOut::StaticFlags();					break;
		case Component::eActionIn: 										result = ActionIn::StaticFlags();						break;
		case Component::eActionOut: 									result = ActionOut::StaticFlags();						break;
		case Component::eVectorIn: 										result = VectorIn::StaticFlags();						break;
		case Component::eVectorOut: 									result = VectorOut::StaticFlags();						break;
		case Component::eControlInput: 									result = ControlInput::StaticFlags();					break;
		case Component::eControlOutput: 								result = ControlOutput::StaticFlags();					break;
		case Component::eEventInput: 									result = EventInput::StaticFlags();						break;
		case Component::eEventOutput: 									result = EventOutput::StaticFlags();					break;
		case Component::eCoordinateInput: 								result = CoordinateInput::StaticFlags();				break;
		case Component::eCoordinateOutput: 								result = CoordinateOutput::StaticFlags();				break;
		case Component::eMonoAudioInput: 								result = MonoAudioInput::StaticFlags();					break;
		case Component::eMonoAudioOutput: 								result = MonoAudioOutput::StaticFlags();				break;
		case Component::eStereoAudioInput: 								result = StereoAudioInput::StaticFlags();				break;
		case Component::eStereoAudioOutput: 							result = StereoAudioOutput::StaticFlags();				break;
		case Component::eSurroundAudioInput: 							result = SurroundAudioInput::StaticFlags();				break;
		case Component::eSurroundAudioOutput: 							result = SurroundAudioOutput::StaticFlags();			break;
		case Component::eSurroundLFEAudioInput: 						result = SurroundLFEAudioInput::StaticFlags();			break;
		case Component::eSurroundLFEAudioOutput: 						result = SurroundLFEAudioOutput::StaticFlags();			break;
		case Component::eVariable: 										result = Variable::StaticFlags();						break;
		case Component::ePiecewiseLinearFunction: 						result = PiecewiseLinearFunction::StaticFlags();		break;
		case Component::eSource: 										result = Source::StaticFlags();							break;
		case Component::eRolloff: 										result = Rolloff::StaticFlags();						break;
		case Component::eCone: 											result = Cone::StaticFlags();							break;
		case Component::eLowPass: 										result = LowPass::StaticFlags();						break;
		case Component::eTimer: 										result = Timer::StaticFlags();							break;
		case Component::eMetronome: 									result = Metronome::StaticFlags();						break;
		case Component::eRandom: 										result = Random::StaticFlags();							break;
		case Component::eSelector: 										result = Selector::StaticFlags();						break;
		case Component::eStateMachine: 									result = StateMachine::StaticFlags();					break;
		case Component::eQuantize: 										result = Quantize::StaticFlags();						break;
		case Component::eSum: 											result = Sum::StaticFlags();							break;
		case Component::eProduct: 										result = Product::StaticFlags();						break;
		case Component::eExponent: 										result = Exponent::StaticFlags();						break;
		case Component::eMax: 											result = Max::StaticFlags();							break;
		case Component::eMin: 											result = Min::StaticFlags();							break;
		case Component::eInvert: 										result = Invert::StaticFlags();							break;
		case Component::eGradient: 										result = Gradient::StaticFlags();						break;
		case Component::eSpring: 										result = Spring::StaticFlags();							break;
		case Component::eSine: 											result = Sine::StaticFlags();							break;
		case Component::eSampleAndHold: 								result = SampleAndHold::StaticFlags();					break;
		case Component::eEnvelope: 										result = Envelope::StaticFlags();						break;
		case Component::eGateSwitch: 									result = GateSwitch::StaticFlags();						break;
		case Component::eArray: 										result = Array::StaticFlags();							break;
		case Component::eFraction: 										result = Fraction::StaticFlags();						break;
		case Component::eMonoOneShotSound: 								result = MonoOneShotSound::StaticFlags();				break;
		case Component::eStereoOneShotSound: 							result = StereoOneShotSound::StaticFlags();				break;
		case Component::eMonoOneShotSoundTable: 						result = MonoOneShotSoundTable::StaticFlags();			break;
		case Component::eStereoOneShotSoundTable: 						result = StereoOneShotSoundTable::StaticFlags();		break;
		case Component::eMonoLoopingSound: 								result = MonoLoopingSound::StaticFlags();				break;
		case Component::eStereoLoopingSound: 							result = StereoLoopingSound::StaticFlags();				break;
		case Component::eMonoToMonoMixer: 								result = MonoToMonoMixer::StaticFlags();				break;
		case Component::eMonoToStereoMixer: 							result = MonoToStereoMixer::StaticFlags();				break;
		case Component::eMonoToSurroundMixer: 							result = MonoToSurroundMixer::StaticFlags();			break;
		case Component::eMonoToSurroundLFEMixer: 						result = MonoToSurroundLFEMixer::StaticFlags();			break;
		case Component::eStereoToMonoMixer: 							result = StereoToMonoMixer::StaticFlags();				break;
		case Component::eStereoToStereoMixer: 							result = StereoToStereoMixer::StaticFlags();			break;
		case Component::eStereoToQuadMixer: 							result = StereoToQuadMixer::StaticFlags();				break;
		case Component::eStereoToSurroundMixer: 						result = StereoToSurroundMixer::StaticFlags();			break;
		case Component::eQuadToSurroundMixer: 							result = QuadToSurroundMixer::StaticFlags();			break;
		case Component::eSurroundToSurroundMixer: 						result = SurroundToSurroundMixer::StaticFlags();		break;
		case Component::eSurroundToSurroundLFEMixer: 					result = SurroundToSurroundLFEMixer::StaticFlags();		break;
		case Component::eSurroundLFEToSurroundLFEMixer: 				result = SurroundLFEToSurroundLFEMixer::StaticFlags();	break;
		case Component::eMonoGain: 										result = MonoGain::StaticFlags();						break;
		case Component::eMonoSend: 										result = MonoSend::StaticFlags();						break;
		case Component::eMonoReturn: 									result = MonoReturn::StaticFlags();						break;
		case Component::eStereoSend: 									result = StereoSend::StaticFlags();						break;
		case Component::eStereoReturn: 									result = StereoReturn::StaticFlags();					break;
		case Component::eMonoDelay: 									result = MonoDelay::StaticFlags();						break;
		case Component::eSurroundDelay: 								result = SurroundDelay::StaticFlags();					break;
		case Component::eMonoPeakLevel: 								result = MonoPeakLevel::StaticFlags();					break;
		case Component::eMonoRMSLevel: 									result = MonoRMSLevel::StaticFlags();					break;
		case Component::eMonoOscilloscope: 								result = MonoOscilloscope::StaticFlags();				break;
		case Component::eStereoRMSLevel: 								result = StereoRMSLevel::StaticFlags();					break;
		case Component::eSurroundRMSLevel: 								result = SurroundRMSLevel::StaticFlags();				break;
		case Component::eMonoSaturator: 								result = MonoSaturator::StaticFlags();					break;
		case Component::eStereoSaturator: 								result = StereoSaturator::StaticFlags();				break;
		case Component::eSurroundSaturator: 							result = SurroundSaturator::StaticFlags();				break;
		case Component::eMonoBitCrusher: 								result = MonoBitCrusher::StaticFlags();					break;
		case Component::eSurroundBitCrusher: 							result = SurroundBitCrusher::StaticFlags();				break;
		case Component::eMonoRingModulator: 							result = MonoRingModulator::StaticFlags();				break;
		case Component::eSurroundRingModulator: 						result = SurroundRingModulator::StaticFlags();			break;
		case Component::eMonoFeedbackDelay: 							result = MonoFeedbackDelay::StaticFlags();				break;
		case Component::eStereoFeedbackDelay: 							result = StereoFeedbackDelay::StaticFlags();			break;
		case Component::eMonoLowPass: 									result = MonoLowPass::StaticFlags();					break;
		case Component::eStereoLowPass: 								result = StereoLowPass::StaticFlags();					break;
		case Component::eSurroundLowPass: 								result = SurroundLowPass::StaticFlags();				break;
		case Component::eMonoHighPass: 									result = MonoHighPass::StaticFlags();					break;
		case Component::eStereoHighPass: 								result = StereoHighPass::StaticFlags();					break;
		case Component::eSurroundHighPass: 								result = SurroundHighPass::StaticFlags();				break;
		case Component::eSurroundResonantLowPass: 						result = SurroundResonantLowPass::StaticFlags();		break;
		case Component::eMonoLowShelf: 									result = MonoLowShelf::StaticFlags();					break;
		case Component::eSurroundLowShelf: 								result = SurroundLowShelf::StaticFlags();				break;
		case Component::eMonoHighShelf: 								result = MonoHighShelf::StaticFlags();					break;
		case Component::eSurroundHighShelf: 							result = SurroundHighShelf::StaticFlags();				break;
		case Component::eStereoPeakEQ: 									result = StereoPeakEQ::StaticFlags();					break;
		case Component::eSurroundPeakEQ: 								result = SurroundPeakEQ::StaticFlags();					break;
		case Component::eMonoCompressor: 								result = MonoCompressor::StaticFlags();					break;
		case Component::eStereoCompressor: 								result = StereoCompressor::StaticFlags();				break;
		case Component::eSurroundCompressor: 							result = SurroundCompressor::StaticFlags();				break;
//		case Component::eMonoLipSync: 									result = MonoLipSync::StaticFlags();					break;
		case Component::eStereoPitchShift: 								result = StereoPitchShift::StaticFlags();				break;
		case Component::eScript: 										result = Script::StaticFlags();							break;
		case Component::eVector: 										result = Vector::StaticFlags();							break;
		case Component::eVectorNearestPointOnLine: 						result = VectorNearestPointOnLine::StaticFlags();		break;
		case Component::eDopplerShift: 									result = DopplerShift::StaticFlags();					break;
		case Component::eDebugPrint: 									result = DebugPrint::StaticFlags();						break;
		case Component::eSequence: 										result = Sequence::StaticFlags();						break;
		case Component::eMonoSpectralReverb: 							result = MonoSpectralReverb::StaticFlags();				break;
		case Component::eMonoSpectralEnvelope: 							result = MonoSpectralEnvelope::StaticFlags();			break;
		case Component::eStereoSpectralEnvelope: 						result = StereoSpectralEnvelope::StaticFlags();			break;
		case Component::eMonoToAmbisonicsMixer: 						result = MonoToAmbisonicsMixer::StaticFlags();			break;
		case Component::eAmbisonicsToSurroundMixer: 					result = AmbisonicsToSurroundMixer::StaticFlags();		break;
		case Component::eImpactFilter: 									result = ImpactFilter::StaticFlags();					break;
		case Component::eMessageDelay: 									result = MessageDelay::StaticFlags();					break;
		case Component::eSurroundFlanger: 								result = SurroundFlanger::StaticFlags();				break;
	}

	if (result == 0) 
	{
		sLOGERROR(0, "couldn't look up component flags- invalid component %d", GetComponentTypeId(Com));
	}
}
#endif
	return result;
}

////////////////////////////////////////////////////////////////////////////////
//	InitComponent
////////////////////////////////////////////////////////////////////////////////
//void* Audio::Object::InitComponent(void* pCom, void* pDefData)
//{
//	U32 u32CompType = *((U32*)pCom - 1);
//	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
//	System::Get()->LoadCodeOverlayForComponent(u32CompType);
//
//	void* pRetValue = ((Component*)pCom)->Init(pDefData);
//
//	System::Get()->LoadCodeOverlay(u32CurrOverlayId);
//
//	return pRetValue;
//}

////////////////////////////////////////////////////////////////////////////////
//	UpdateComponent
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::UpdateComponent(void* pCom, Instance* pInst)
{
	U32 u32CompType = *((U32*)pCom - 1);
	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
	System::Get()->LoadCodeOverlayForComponent(u32CompType);

	((Component*)pCom)->SetObject(this);

	switch (u32CompType)
	{
		case Component::eActionIn:
			((ActionIn*)pCom)->ActionIn::Update(pInst);
			break;
		case Component::eDopplerShift:
			((DopplerShift*)pCom)->DopplerShift::Update(pInst);
			break;
		case Component::eEnvelope:
			((Envelope*)pCom)->Envelope::Update(pInst);
			break;
		case Component::eEventInput:
			((EventInput*)pCom)->EventInput::Update(pInst);
			break;
		case Component::eGateSwitch:
			((GateSwitch*)pCom)->GateSwitch::Update(pInst);
			break;
		case Component::eGradient:
			((Gradient*)pCom)->Gradient::Update(pInst);
			break;
		case Component::eImpactFilter:
			((ImpactFilter*)pCom)->ImpactFilter::Update(pInst);
			break;
		case Component::eLowPass:
			((LowPass*)pCom)->LowPass::Update(pInst);
			break;
		case Component::eMessageDelay:
			((MessageDelay*)pCom)->MessageDelay::Update(pInst);
			break;
		case Component::eMonoAudioInput:
			((MonoAudioInput*)pCom)->MonoAudioInput::Update(pInst);
			break;
		//case Component::eMonoBitCrusher:
		//	((MonoBitCrusher*)pCom)->MonoBitCrusher::Update(pInst);
		//	break;
		case Component::eMonoCompressor:
			((MonoCompressor*)pCom)->MonoCompressor::Update(pInst);
			break;
		//case Component::eMonoConvolution:
		//	((MonoConvolution*)pCom)->MonoConvolution::Update(pInst);
		//	break;
		case Component::eMonoDelay:
			((MonoDelay*)pCom)->MonoDelay::Update(pInst);
			break;
		//case Component::eMonoDistortion:
		//	((MonoDistortion*)pCom)->MonoDistortion::Update(pInst);
		//	break;
		//case Component::eMonoFeedbackDelay:
		//	((MonoFeedbackDelay*)pCom)->MonoFeedbackDelay::Update(pInst);
		//	break;
		//case Component::eMonoFlanger:
		//	((MonoFlanger*)pCom)->MonoFlanger::Update(pInst);
		//	break;
		//case Component::eMonoGain:
		//	((MonoGain*)pCom)->MonoGain::Update(pInst);
		//	break;
		case Component::eMonoSend:
			((MonoSend*)pCom)->MonoSend::Update(pInst);
			break;
		case Component::eMonoReturn:
			((MonoReturn*)pCom)->MonoReturn::Update(pInst);
			break;
		case Component::eStereoSend:
			((StereoSend*)pCom)->StereoSend::Update(pInst);
			break;
		case Component::eStereoReturn:
			((StereoReturn*)pCom)->StereoReturn::Update(pInst);
			break;
		case Component::eMonoHighPass:
			((MonoHighPass*)pCom)->MonoHighPass::Update(pInst);
			break;
		//case Component::eMonoHighShelf:
		//	((MonoHighShelf*)pCom)->MonoHighShelf::Update(pInst);
		//	break;
		//case Component::eMonoLipSync:
		//	((MonoLipSync*)pCom)->MonoLipSync::Update(pInst);
		//	break;
		case Component::eMonoLoopingSound:
			((MonoLoopingSound*)pCom)->MonoLoopingSound::Update(pInst);
			break;
		case Component::eMonoLowPass:
			((MonoLowPass*)pCom)->MonoLowPass::Update(pInst);
			break;
		//case Component::eMonoLowShelf:
		//	((MonoLowShelf*)pCom)->MonoLowShelf::Update(pInst);
		//	break;
		case Component::eMonoOneShotSound:
			((MonoOneShotSound*)pCom)->MonoOneShotSound::Update(pInst);
			break;
		case Component::eMonoOneShotSoundTable:
			((MonoOneShotSoundTable*)pCom)->MonoOneShotSoundTable::Update(pInst);
			break;
		case Component::eMonoPeakEQ:
			((MonoPeakEQ*)pCom)->MonoPeakEQ::Update(pInst);
			break;
		//case Component::eMonoPeakLevel:
		//	((MonoPeakLevel*)pCom)->MonoPeakLevel::Update(pInst);
		//	break;
		//case Component::eMonoPhaser:
		//	((MonoPhaser*)pCom)->MonoPhaser::Update(pInst);
		//	break;
		//case Component::eMonoResonantLowPass:
		//	((MonoResonantLowPass*)pCom)->MonoResonantLowPass::Update(pInst);
		//	break;
		case Component::eMonoRingModulator:
			((MonoRingModulator*)pCom)->MonoRingModulator::Update(pInst);
			break;
		case Component::eMonoRMSLevel:
			((MonoRMSLevel*)pCom)->MonoRMSLevel::Update(pInst);
			break;
		//case Component::eMonoOscilloscope:
		//	((MonoOscilloscope*)pCom)->MonoOscilloscope::Update(pInst);
		//	break;
		case Component::eMonoSaturator:
			((MonoSaturator*)pCom)->MonoSaturator::Update(pInst);
			break;
		//case Component::eMonoSpectralEnvelope:
		//	((MonoSpectralEnvelope*)pCom)->MonoSpectralEnvelope::Update(pInst);
		//	break;
		case Component::eMonoSpectralReverb:
			((MonoSpectralReverb*)pCom)->MonoSpectralReverb::Update(pInst);
			break;
		case Component::eMonoToMonoMixer:
			((MonoToMonoMixer*)pCom)->MonoToMonoMixer::Update(pInst);
			break;
		case Component::eMonoToStereoMixer:
			((MonoToStereoMixer*)pCom)->MonoToStereoMixer::Update(pInst);
			break;
		case Component::eMonoToSurroundMixer:
			((MonoToSurroundMixer*)pCom)->MonoToSurroundMixer::Update(pInst);
			break;
		case Component::eMonoToSurroundLFEMixer:
			((MonoToSurroundLFEMixer*)pCom)->MonoToSurroundLFEMixer::Update(pInst);
			break;
		case Component::eParameterOut:
			((ParameterOut*)pCom)->ParameterOut::Update(pInst);
			break;
		case Component::eQuadToSurroundMixer:
			((QuadToSurroundMixer*)pCom)->QuadToSurroundMixer::Update(pInst);
			break;
		case Component::eScript:
			((Script*)pCom)->Script::Update(pInst);
			break;
		case Component::eSource:
			((Source*)pCom)->Source::Update(pInst);
			break;
		case Component::eSpring:
			((Spring*)pCom)->Spring::Update(pInst);
			break;
		case Component::eStereoAudioInput:
			((StereoAudioInput*)pCom)->StereoAudioInput::Update(pInst);
			break;
		case Component::eStereoCompressor:
			((StereoCompressor*)pCom)->StereoCompressor::Update(pInst);
			break;
		//case Component::eStereoDistortion:
		//	((StereoDistortion*)pCom)->StereoDistortion::Update(pInst);
		//	break;
		//case Component::eStereoFeedbackDelay:
		//	((StereoFeedbackDelay*)pCom)->StereoFeedbackDelay::Update(pInst);
		//	break;
		case Component::eStereoHighPass:
			((StereoHighPass*)pCom)->StereoHighPass::Update(pInst);
			break;
		case Component::eSurroundHighPass:
			((SurroundHighPass*)pCom)->SurroundHighPass::Update(pInst);
			break;
		case Component::eStereoLoopingSound:
			((StereoLoopingSound*)pCom)->StereoLoopingSound::Update(pInst);
			break;
		case Component::eStereoLowPass:
			((StereoLowPass*)pCom)->StereoLowPass::Update(pInst);
			break;
		case Component::eStereoOneShotSound:
			((StereoOneShotSound*)pCom)->StereoOneShotSound::Update(pInst);
			break;
		case Component::eStereoOneShotSoundTable:
			((StereoOneShotSoundTable*)pCom)->StereoOneShotSoundTable::Update(pInst);
			break;
		case Component::eStereoRMSLevel:
			((StereoRMSLevel*)pCom)->StereoRMSLevel::Update(pInst);
			break;
		case Component::eStereoSaturator:
			((StereoSaturator*)pCom)->StereoSaturator::Update(pInst);
			break;
		//case Component::eStereoSpectralEnvelope:
		//	((StereoSpectralEnvelope*)pCom)->StereoSpectralEnvelope::Update(pInst);
		//	break;
		case Component::eStereoToMonoMixer:
			((StereoToMonoMixer*)pCom)->StereoToMonoMixer::Update(pInst);
			break;
		case Component::eStereoToQuadMixer:
			((StereoToQuadMixer*)pCom)->StereoToQuadMixer::Update(pInst);
			break;
		case Component::eStereoToStereoMixer:
			((StereoToStereoMixer*)pCom)->StereoToStereoMixer::Update(pInst);
			break;
		case Component::eStereoToSurroundMixer:
			((StereoToSurroundMixer*)pCom)->StereoToSurroundMixer::Update(pInst);
			break;
		case Component::eSurroundAudioInput:
			((SurroundAudioInput*)pCom)->SurroundAudioInput::Update(pInst);
			break;
		case Component::eSurroundBitCrusher:
			((SurroundBitCrusher*)pCom)->SurroundBitCrusher::Update(pInst);
			break;
		case Component::eSurroundCompressor:
			((SurroundCompressor*)pCom)->SurroundCompressor::Update(pInst);
			break;
		//case Component::eSurroundDelay:
		//	((SurroundDelay*)pCom)->SurroundDelay::Update(pInst);
		//	break;
		case Component::eSurroundFlanger:
			((SurroundFlanger*)pCom)->SurroundFlanger::Update(pInst);
			break;
		//case Component::eSurroundHighShelf:
		//	((SurroundHighShelf*)pCom)->SurroundHighShelf::Update(pInst);
		//	break;
		case Component::eSurroundLFEAudioInput:
			((SurroundLFEAudioInput*)pCom)->SurroundLFEAudioInput::Update(pInst);
			break;
		case Component::eSurroundLowPass:
			((SurroundLowPass*)pCom)->SurroundLowPass::Update(pInst);
			break;
		//case Component::eSurroundLowShelf:
		//	((SurroundLowShelf*)pCom)->SurroundLowShelf::Update(pInst);
		//	break;
		//case Component::eSurroundPeakEQ:
		//	((SurroundPeakEQ*)pCom)->SurroundPeakEQ::Update(pInst);
		//	break;
		case Component::eSurroundResonantLowPass:
			((SurroundResonantLowPass*)pCom)->SurroundResonantLowPass::Update(pInst);
			break;
		case Component::eSurroundRingModulator:
			((SurroundRingModulator*)pCom)->SurroundRingModulator::Update(pInst);
			break;
		case Component::eSurroundRMSLevel:
			((SurroundRMSLevel*)pCom)->SurroundRMSLevel::Update(pInst);
			break;
		case Component::eSurroundSaturator:
			((SurroundSaturator*)pCom)->SurroundSaturator::Update(pInst);
			break;
		case Component::eSurroundToSurroundLFEMixer:
			((SurroundToSurroundLFEMixer*)pCom)->SurroundToSurroundLFEMixer::Update(pInst);
			break;
		case Component::eSurroundToSurroundMixer:
			((SurroundToSurroundMixer*)pCom)->SurroundToSurroundMixer::Update(pInst);
			break;
		case Component::eSurroundLFEToSurroundLFEMixer:
			((SurroundLFEToSurroundLFEMixer*)pCom)->SurroundLFEToSurroundLFEMixer::Update(pInst);
			break;
		case Component::eTimer:
			((Timer*)pCom)->Timer::Update(pInst);
			break;
		case Component::eVector:
			((Vector*)pCom)->Vector::Update(pInst);
			break;
		case Component::eVectorNearestPointOnLine:
			((VectorNearestPointOnLine*)pCom)->VectorNearestPointOnLine::Update(pInst);
			break;
		default:
			sLOGERROR(0, "trying to update component of type %d\n", u32CompType);
			break;
	}
	System::Get()->LoadCodeOverlay(u32CurrOverlayId);
}

////////////////////////////////////////////////////////////////////////////////
//	GenerateComponent
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::GenerateComponent(void* pCom, Instance* pInst)
{
	U32 u32CompType = *((U32*)pCom - 1);
//	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
//	System::Get()->LoadCodeOverlayForComponent(u32CompType);

	((Component*)pCom)->SetObject(this);

	Core::Output* pOutput = 0;
	U8* pCoreData = pInst->GetCoreData();

	switch (u32CompType)
	{
		case Component::eMonoAudioOutput:
			pOutput = (Core::Output*)&pCoreData[((MonoAudioOutput*)pCom)->AudioOutOffset(pInst)];
			break;
		case Component::eStereoAudioOutput:
			pOutput = (Core::Output*)&pCoreData[((StereoAudioOutput*)pCom)->AudioOutOffset(pInst)];
			break;
		case Component::eSurroundAudioOutput:
			pOutput = (Core::Output*)&pCoreData[((SurroundAudioOutput*)pCom)->AudioOutOffset(pInst)];
			break;
		case Component::eSurroundLFEAudioOutput:
			pOutput = (Core::Output*)&pCoreData[((SurroundLFEAudioOutput*)pCom)->AudioOutOffset(pInst)];
			break;
		default:
			sLOGERROR(0, "can't generate component of type %d\n", u32CompType);
			break;
	}

	if (pOutput)
		pOutput->Process(pInst->m_coreInfo);

//	System::Get()->LoadCodeOverlay(u32CurrOverlayId);
}


void Audio::Object::SetProcessFlagOnComponent(void* pCom, Instance* pInst)
{
	U32 u32CompType = *((U32*)pCom - 1);
//	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
//	System::Get()->LoadCodeOverlayForComponent(u32CompType);

	((Component*)pCom)->SetObject(this);

	Core::Output* pOutput = 0;
	U8* pCoreData = pInst->GetCoreData();

	switch (u32CompType)
	{
		case Component::eMonoAudioOutput:
			pOutput = (Core::Output*)&pCoreData[((MonoAudioOutput*)pCom)->AudioOutOffset(pInst)];
			break;
		case Component::eStereoAudioOutput:
			pOutput = (Core::Output*)&pCoreData[((StereoAudioOutput*)pCom)->AudioOutOffset(pInst)];
			break;
		case Component::eSurroundAudioOutput:
			pOutput = (Core::Output*)&pCoreData[((SurroundAudioOutput*)pCom)->AudioOutOffset(pInst)];
			break;
		case Component::eSurroundLFEAudioOutput:
			pOutput = (Core::Output*)&pCoreData[((SurroundLFEAudioOutput*)pCom)->AudioOutOffset(pInst)];
			break;
		default:
			sLOGERROR(0, "can't set process flag on component of type %d\n", u32CompType);
			break;
	}

	if (pOutput)
		pOutput->SetProcessFlag(pInst->m_coreInfo);

//	System::Get()->LoadCodeOverlay(u32CurrOverlayId);
}


////////////////////////////////////////////////////////////////////////////////
//	GetComponentValue
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Object::GetComponentValue(S32 s32ComId, Instance* pInst, U32 u32InputOut)
{
#ifndef _DEBUG
	F32* afCurrentValues = pInst->GetCurrentValues();

	//	don't load overlay if value is known
	if (afCurrentValues[s32ComId] != Maths::fInvalid && (! u32InputOut))
		return afCurrentValues[s32ComId];

	U32 u32CompType = GetComponentTypeId(s32ComId);
	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
	System::Get()->LoadCodeOverlayForComponent(u32CompType);

	Component* pCom = GetComponent(s32ComId);
	pCom->SetObject(this);

	F32 f32RetValue = 0.f;

	switch (u32CompType)
	{
		case Component::eArray:
			f32RetValue = ((Array*)pCom)->Array::Value(pInst, u32InputOut);
			break;
		case Component::eCone:
			f32RetValue = ((Cone*)pCom)->Cone::Value(pInst, u32InputOut);
			break;
		case Component::eControlInput:
			f32RetValue = ((ControlInput*)pCom)->ControlInput::Value(pInst, u32InputOut);
			break;
		case Component::eControlOutput:
			f32RetValue = ((ControlOutput*)pCom)->ControlOutput::Value(pInst, u32InputOut);
			break;
		case Component::eDopplerShift:
			f32RetValue = ((DopplerShift*)pCom)->DopplerShift::Value(pInst, u32InputOut);
			break;
		case Component::eEnvelope:
			f32RetValue = ((Envelope*)pCom)->Envelope::Value(pInst, u32InputOut);
			break;
		case Component::eExponent:
			f32RetValue = ((Exponent*)pCom)->Exponent::Value(pInst, u32InputOut);
			break;
		case Component::eGateSwitch:
			f32RetValue = ((GateSwitch*)pCom)->GateSwitch::Value(pInst, u32InputOut);
			break;
		case Component::eGradient:
			f32RetValue = ((Gradient*)pCom)->Gradient::Value(pInst, u32InputOut);
			break;
		//case Component::eInvert:
		//	f32RetValue = ((Invert*)pCom)->Invert::Value(pInst, u32InputOut);
		//	break;
		case Component::eLowPass:
			f32RetValue = ((LowPass*)pCom)->LowPass::Value(pInst, u32InputOut);
			break;
		case Component::eMax:
			f32RetValue = ((Max*)pCom)->Max::Value(pInst, u32InputOut);
			break;
		case Component::eMin:
			f32RetValue = ((Min*)pCom)->Min::Value(pInst, u32InputOut);
			break;
		case Component::eMonoLoopingSound:
			f32RetValue = ((MonoLoopingSound*)pCom)->MonoLoopingSound::Value(pInst, u32InputOut);
			break;
		case Component::eMonoOneShotSound:
			f32RetValue = ((MonoOneShotSound*)pCom)->MonoOneShotSound::Value(pInst, u32InputOut);
			break;
		case Component::eMonoOneShotSoundTable:
			f32RetValue = ((MonoOneShotSoundTable*)pCom)->MonoOneShotSoundTable::Value(pInst, u32InputOut);
			break;
		//case Component::eMonoPeakLevel:
		//	f32RetValue = ((MonoPeakLevel*)pCom)->MonoPeakLevel::Value(pInst, u32InputOut);
		//	break;
		case Component::eMonoRMSLevel:
			f32RetValue = ((MonoRMSLevel*)pCom)->MonoRMSLevel::Value(pInst, u32InputOut);
			break;
		//case Component::eMonoOscilloscope:
		//	f32RetValue = ((MonoOscilloscope*)pCom)->MonoOscilloscope::Value(pInst, u32InputOut);
		//	break;
		case Component::eSpring:
			f32RetValue = ((Spring*)pCom)->Spring::Value(pInst, u32InputOut);
			break;
		case Component::eStereoRMSLevel:
			f32RetValue = ((StereoRMSLevel*)pCom)->StereoRMSLevel::Value(pInst, u32InputOut);
			break;
		case Component::eSurroundRMSLevel:
			f32RetValue = ((SurroundRMSLevel*)pCom)->SurroundRMSLevel::Value(pInst, u32InputOut);
			break;
		//case Component::eMonoLipSync:
		//	f32RetValue = ((MonoLipSync*)pCom)->MonoLipSync::Value(pInst, u32InputOut);
		//	break;
		case Component::eMonoSpectralReverb:
			f32RetValue = ((MonoSpectralReverb*)pCom)->MonoSpectralReverb::Value(pInst, u32InputOut);
			break;
		//case Component::eMonoSpectralEnvelope:
		//	f32RetValue = ((MonoSpectralEnvelope*)pCom)->MonoSpectralEnvelope::Value(pInst, u32InputOut);
		//	break;
		//case Component::eStereoSpectralEnvelope:
		//	f32RetValue = ((StereoSpectralEnvelope*)pCom)->StereoSpectralEnvelope::Value(pInst, u32InputOut);
		//	break;
		case Component::eParameterIn:
			f32RetValue = ((ParameterIn*)pCom)->ParameterIn::Value(pInst, u32InputOut);
			break;
		case Component::ePiecewiseLinearFunction:
			f32RetValue = ((PiecewiseLinearFunction*)pCom)->PiecewiseLinearFunction::Value(pInst, u32InputOut);
			break;
		//case Component::eStateMachine:
		//	f32RetValue = ((StateMachine*)pCom)->StateMachine::Value(pInst, u32InputOut);
		//	break;
		//case Component::eSequence:
		//	f32RetValue = ((Sequence*)pCom)->Sequence::Value(pInst, u32InputOut);
		//	break;
		case Component::eProduct:
			f32RetValue = ((Product*)pCom)->Product::Value(pInst, u32InputOut);
			break;
		//case Component::eQuantize:
		//	f32RetValue = ((Quantize*)pCom)->Quantize::Value(pInst, u32InputOut);
		//	break;
		case Component::eRandom:
			f32RetValue = ((Random*)pCom)->Random::Value(pInst, u32InputOut);
			break;
		case Component::eRolloff:
			f32RetValue = ((Rolloff*)pCom)->Rolloff::Value(pInst, u32InputOut);
			break;
		case Component::eSampleAndHold:
			f32RetValue = ((SampleAndHold*)pCom)->SampleAndHold::Value(pInst, u32InputOut);
			break;
		case Component::eSelector:
			f32RetValue = ((Selector*)pCom)->Selector::Value(pInst, u32InputOut);
			break;
		//case Component::eSine:
		//	f32RetValue = ((Sine*)pCom)->Sine::Value(pInst, u32InputOut);
		//	break;
		case Component::eSource:
			f32RetValue = ((Source*)pCom)->Source::Value(pInst, u32InputOut);
			break;
		case Component::eStereoLoopingSound:
			f32RetValue = ((StereoLoopingSound*)pCom)->StereoLoopingSound::Value(pInst, u32InputOut);
			break;
		case Component::eStereoOneShotSound:
			f32RetValue = ((StereoOneShotSound*)pCom)->StereoOneShotSound::Value(pInst, u32InputOut);
			break;
		case Component::eStereoOneShotSoundTable:
			f32RetValue = ((StereoOneShotSoundTable*)pCom)->StereoOneShotSoundTable::Value(pInst, u32InputOut);
			break;
		case Component::eSum:
			f32RetValue = ((Sum*)pCom)->Sum::Value(pInst, u32InputOut);
			break;
		case Component::eTimer:
			f32RetValue = ((Timer*)pCom)->Timer::Value(pInst, u32InputOut);
			break;
		case Component::eVariable:
			f32RetValue = ((Variable*)pCom)->Variable::Value(pInst, u32InputOut);
			break;
		case Component::eVector:
			f32RetValue = ((Vector*)pCom)->Vector::Value(pInst, u32InputOut);
			break;
		case Component::eVectorIn:
			f32RetValue = ((VectorIn*)pCom)->VectorIn::Value(pInst, u32InputOut);
			break;
		case Component::eVectorNearestPointOnLine:
			f32RetValue = ((VectorNearestPointOnLine*)pCom)->VectorNearestPointOnLine::Value(pInst, u32InputOut);
			break;
		case Component::eImpactFilter:
			f32RetValue = ((VectorNearestPointOnLine*)pCom)->VectorNearestPointOnLine::Value(pInst, u32InputOut);
			break;
		case Component::eMonoToMonoMixer:
			f32RetValue = ((MonoToMonoMixer*)pCom)->MonoToMonoMixer::Value(pInst, u32InputOut);
			break;
		case Component::eMonoToStereoMixer:
			f32RetValue = ((MonoToStereoMixer*)pCom)->MonoToStereoMixer::Value(pInst, u32InputOut);
			break;
		case Component::eMonoToSurroundMixer:
			f32RetValue = ((MonoToSurroundMixer*)pCom)->MonoToSurroundMixer::Value(pInst, u32InputOut);
			break;
		case Component::eMonoToSurroundLFEMixer:
			f32RetValue = ((MonoToSurroundLFEMixer*)pCom)->MonoToSurroundLFEMixer::Value(pInst, u32InputOut);
			break;
		case Component::eStereoToMonoMixer:
			f32RetValue = ((StereoToMonoMixer*)pCom)->StereoToMonoMixer::Value(pInst, u32InputOut);
			break;
		case Component::eStereoToStereoMixer:
			f32RetValue = ((StereoToStereoMixer*)pCom)->StereoToStereoMixer::Value(pInst, u32InputOut);
			break;
		case Component::eStereoToQuadMixer:
			f32RetValue = ((StereoToQuadMixer*)pCom)->StereoToQuadMixer::Value(pInst, u32InputOut);
			break;
		case Component::eStereoToSurroundMixer:
			f32RetValue = ((StereoToSurroundMixer*)pCom)->StereoToSurroundMixer::Value(pInst, u32InputOut);
			break;
		case Component::eSurroundToSurroundMixer:
			f32RetValue = ((SurroundToSurroundMixer*)pCom)->SurroundToSurroundMixer::Value(pInst, u32InputOut);
			break;
		case Component::eSurroundToSurroundLFEMixer:
			f32RetValue = ((SurroundToSurroundLFEMixer*)pCom)->SurroundToSurroundLFEMixer::Value(pInst, u32InputOut);
			break;
		//case Component::eSurroundLFEToSurroundLFEMixer:
		//	f32RetValue = ((SurroundLFEToSurroundLFEMixer*)pCom)->SurroundLFEToSurroundLFEMixer::Value(pInst, u32InputOut);
		//	break;
		case Component::eQuadToSurroundMixer:
			f32RetValue = ((QuadToSurroundMixer*)pCom)->QuadToSurroundMixer::Value(pInst, u32InputOut);
			break;

		default:
			sLOGERROR(0, "can't get value on component of type %d\n", u32CompType);
			break;
	}

	System::Get()->LoadCodeOverlay(u32CurrOverlayId);

	if (! u32InputOut)
		afCurrentValues[s32ComId] = f32RetValue;

	return f32RetValue;
#else
return 0.f;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	SetComponentValue
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::SetComponentValue(S32 s32Com, Instance* pInst, F32 fValue)
{
	U32 u32CompType = GetComponentTypeId(s32Com);
	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
	System::Get()->LoadCodeOverlayForComponent(u32CompType);

	Component* pCom = GetComponent(s32Com);
	pCom->SetObject(this);

	switch (u32CompType)
	{
		//case Component::eParameterIn:
		//	((ParameterIn*)pCom)->ParameterIn::SetValue(pInst, fValue);
		//	break;
		case Component::eVariable:
			((Variable*)pCom)->Variable::SetValueDirect(pInst, fValue);
			break;
		default:
			sLOGERROR(0, "can't set value on component of type %d\n", u32CompType);
			break;
	}

	System::Get()->LoadCodeOverlay(u32CurrOverlayId);
}

////////////////////////////////////////////////////////////////////////////////
//	ExecuteComponent
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::ExecuteComponent(S32 s32Com, Instance* pInst)
{
	U32 u32CompType = GetComponentTypeId(s32Com);
	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
	System::Get()->LoadCodeOverlayForComponent(u32CompType);

	Component* pCom = GetComponent(s32Com);
	pCom->SetObject(this);

	switch (u32CompType)
	{
		// doesn't get called from other components!
		//case Component::eActionIn:
		//	((ActionIn*)pCom)->ActionIn::Execute(pInst);
		//	break;
		case Component::eActionOut:
			((ActionOut*)pCom)->ActionOut::Execute(pInst);
			break;
		case Component::eEventInput:
			((EventInput*)pCom)->EventInput::Execute(pInst);
			break;
		case Component::eEventOutput:
			((EventOutput*)pCom)->EventOutput::Execute(pInst);
			break;
		case Component::eScript:
			((Script*)pCom)->Script::Execute(pInst);
			break;
		default:
			sLOGERROR(0, "can't execute component of type %d\n", u32CompType);
			break;
	}

	System::Get()->LoadCodeOverlay(u32CurrOverlayId);
}

////////////////////////////////////////////////////////////////////////////////
//	DoComponentMessage
////////////////////////////////////////////////////////////////////////////////
#ifndef AUDIO_SPU_MINIMAL
void Audio::Object::DoComponentMessage(S32 s32Com, Instance* pInstance, U32 uMessage)
{
	U32 u32CompType = GetComponentTypeId(s32Com);
	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
	System::Get()->LoadCodeOverlayForComponent(u32CompType);

	Component* pCom = GetComponent(s32Com);
	pCom->SetObject(this);

	switch (u32CompType)
	{
		case Component::eDebugPrint:
			((DebugPrint*)pCom)->DebugPrint::DoMessage(pInstance, uMessage);
			break;
		case Component::eEventOutput:
			((EventOutput*)pCom)->EventOutput::DoMessage(pInstance, uMessage);
			break;
		case Component::eImpactFilter:
			((ImpactFilter*)pCom)->ImpactFilter::DoMessage(pInstance, uMessage);
			break;
		//case Component::eMonoLipSync:
		//	((MonoLipSync*)pCom)->MonoLipSync::DoMessage(pInstance, uMessage);
		//	break;
		case Component::eMonoSpectralReverb:
			((MonoSpectralReverb*)pCom)->MonoSpectralReverb::DoMessage(pInstance, uMessage);
			break;
		//case Component::eMonoSpectralEnvelope:
		//	((MonoSpectralEnvelope*)pCom)->MonoSpectralEnvelope::DoMessage(pInstance, uMessage);
		//	break;
		//case Component::eStereoSpectralEnvelope:
		//	((StereoSpectralEnvelope*)pCom)->StereoSpectralEnvelope::DoMessage(pInstance, uMessage);
		//	break;
		case Component::eMonoLoopingSound:
			((MonoLoopingSound*)pCom)->MonoLoopingSound::DoMessage(pInstance, uMessage);
			break;
		case Component::eMonoOneShotSound:
			((MonoOneShotSound*)pCom)->MonoOneShotSound::DoMessage(pInstance, uMessage);
			break;
		case Component::eMonoOneShotSoundTable:
			((MonoOneShotSoundTable*)pCom)->MonoOneShotSoundTable::DoMessage(pInstance, uMessage);
			break;
		case Component::eRandom:
			((Random*)pCom)->Random::DoMessage(pInstance, uMessage);
			break;
		case Component::eSampleAndHold:
			((SampleAndHold*)pCom)->SampleAndHold::DoMessage(pInstance, uMessage);
			break;
		case Component::eScript:
			((Script*)pCom)->Script::DoMessage(pInstance, uMessage);
			break;
		case Component::eSelector:
			((Selector*)pCom)->Selector::DoMessage(pInstance, uMessage);
			break;
		case Component::eStereoLoopingSound:
			((StereoLoopingSound*)pCom)->StereoLoopingSound::DoMessage(pInstance, uMessage);
			break;
		case Component::eStereoOneShotSound:
			((StereoOneShotSound*)pCom)->StereoOneShotSound::DoMessage(pInstance, uMessage);
			break;
		case Component::eStereoOneShotSoundTable:
			((StereoOneShotSoundTable*)pCom)->StereoOneShotSoundTable::DoMessage(pInstance, uMessage);
			break;
		case Component::eTimer:
			((Timer*)pCom)->Timer::DoMessage(pInstance, uMessage);
			break;
		case Component::eSurroundBitCrusher:
			((SurroundBitCrusher*)pCom)->SurroundBitCrusher::DoMessage(pInstance, uMessage);
			break;
		case Component::eSurroundRingModulator:
			((SurroundRingModulator*)pCom)->SurroundRingModulator::DoMessage(pInstance, uMessage);
			break;
		case Component::eSurroundFlanger:
			((SurroundFlanger*)pCom)->SurroundFlanger::DoMessage(pInstance, uMessage);
			break;
		case Component::eMessageDelay:
			((MessageDelay*)pCom)->MessageDelay::DoMessage(pInstance, uMessage);
			break;
		case Component::eMonoDelay:
			((MonoDelay*)pCom)->MonoDelay::DoMessage(pInstance, uMessage);
			break;
		case Component::eMonoLowPass:
			((MonoLowPass*)pCom)->MonoLowPass::DoMessage(pInstance, uMessage);
			break;
		case Component::eStereoLowPass:
			((StereoLowPass*)pCom)->StereoLowPass::DoMessage(pInstance, uMessage);
			break;
		case Component::eSurroundLowPass:
			((SurroundLowPass*)pCom)->SurroundLowPass::DoMessage(pInstance, uMessage);
			break;
		case Component::eSurroundResonantLowPass:
			((SurroundResonantLowPass*)pCom)->SurroundResonantLowPass::DoMessage(pInstance, uMessage);
			break;
		case Component::eMonoHighPass:
			((MonoHighPass*)pCom)->MonoHighPass::DoMessage(pInstance, uMessage);
			break;
		case Component::eStereoHighPass:
			((StereoHighPass*)pCom)->StereoHighPass::DoMessage(pInstance, uMessage);
			break;
		case Component::eSurroundHighPass:
			((SurroundHighPass*)pCom)->SurroundHighPass::DoMessage(pInstance, uMessage);
			break;
		default:
			sLOGERROR(0, "couldn't run DoMessage- invalid component %d", u32CompType);
			break;
	}

	System::Get()->LoadCodeOverlay(u32CurrOverlayId);
}
#else
void Audio::Object::DoComponentMessage(S32 s32Com, Instance* pInstance, U32 uMessage)
{
	U32 u32CompType = GetComponentTypeId(s32Com);
	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
	System::Get()->LoadCodeOverlayForComponent(u32CompType);

	Component* pCom = GetComponent(s32Com);
	pCom->SetObject(this);

	switch (u32CompType)
	{
		case Component::eDebugPrint:
			((DebugPrint*)pCom)->DebugPrint::DoMessage(pInstance, uMessage);
			break;
		case Component::eMonoOneShotSound:
			((MonoOneShotSound*)pCom)->MonoOneShotSound::DoMessage(pInstance, uMessage);
			break;
		case Component::eScript:
			((Script*)pCom)->Script::DoMessage(pInstance, uMessage);
			break;
		case Component::eTimer:
			((Timer*)pCom)->Timer::DoMessage(pInstance, uMessage);
			break;
		default:
			break;
	}

	System::Get()->LoadCodeOverlay(u32CurrOverlayId);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	GetComponentCoords
////////////////////////////////////////////////////////////////////////////////
void Audio::Object::GetComponentCoords(S32 s32Com, Instance* pInstance, Maths::Vector* pVector)
{
	U32 u32CompType = GetComponentTypeId(s32Com);
	U32 u32CurrOverlayId = System::Get()->GetCurrentCodeOverlayId();
	System::Get()->LoadCodeOverlayForComponent(u32CompType);

	Component* pCom = GetComponent(s32Com);
	pCom->SetObject(this);
	switch (u32CompType)
	{
		case Component::eDopplerShift:
			((DopplerShift*)pCom)->DopplerShift::Coords(pInstance, pVector);
			break;
		case Component::eVector:
			((Vector*)pCom)->Vector::Coords(pInstance, pVector);
			break;
		case Component::eVectorIn:
			((VectorIn*)pCom)->VectorIn::Coords(pInstance, pVector);
			break;
		case Component::eVectorNearestPointOnLine:
			((VectorNearestPointOnLine*)pCom)->VectorNearestPointOnLine::Coords(pInstance, pVector);
			break;
		case Component::eCoordinateInput:
			((CoordinateInput*)pCom)->CoordinateInput::Coords(pInstance, pVector);
			break;
		default:
			//Assert("invalid component type in GetComponentCoords");
			//spu_printf("invalid component type %d in GetComponentCoords\n", u32CompType);
			break;
	}

	System::Get()->LoadCodeOverlay(u32CurrOverlayId);
}

#endif // PLATFORM_SPU

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
