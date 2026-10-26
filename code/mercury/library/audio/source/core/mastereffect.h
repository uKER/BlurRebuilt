#pragma once

#ifdef USE_XAUDIO2

#include <crtdbg.h>
#include <xapobase.h>

namespace Audio { namespace Core { class Renderer; } }

//--------------------------------------------------------------------------------------
// CAPOBase
//
// Template class that provides a default class factory implementation and typesafe
// parameter passing for our sample xAPO classes
//--------------------------------------------------------------------------------------
template< typename APOClass, typename ParameterClass >
class CAPOBase : public CXAPOParametersBase
{
public:
    static HRESULT CreateInstance( void* pInitData, UINT32 cbInitData, APOClass** ppAPO );

    STDMETHOD( LockForProcess ) ( UINT32 InputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters, UINT32 OutputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters );
    STDMETHOD_( void, Process ) ( UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS *pInputProcessParameters, UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS *pOutputProcessParameters, BOOL IsEnabled);

protected:
    CAPOBase();
    ~CAPOBase( void );

    const WAVEFORMATEX& WaveFormat() { return m_wfx; }

    void OnSetParameters( const void* pParams, UINT32 cbParams )
    {
        _ASSERT( cbParams == sizeof( ParameterClass ) );
        cbParams;
        OnSetParameters( *(ParameterClass*)pParams );
    }

    virtual void DoProcess( const ParameterClass& params, FLOAT32* __restrict pData, UINT32 cFrames, UINT32 cChannels ) = 0;
    virtual void OnSetParameters( const ParameterClass& ) { }

private:
    // Ring buffer needed by CXAPOParametersBase
    ParameterClass m_parameters[ 3 ];

    // Format of the audio we're processing
    WAVEFORMATEX m_wfx;

    // Registration properties defining this xAPO class.
    static XAPO_REGISTRATION_PROPERTIES m_regProps;
};

//--------------------------------------------------------------------------------------
// CAPOBase::m_regProps
//
// Registration properties for the sample xAPO classes.
//--------------------------------------------------------------------------------------
template< typename APOClass, typename ParameterClass >
__declspec(selectany) XAPO_REGISTRATION_PROPERTIES CAPOBase< APOClass, ParameterClass> ::m_regProps =
{
	__uuidof( APOClass ),
	L"RendererAPO",
	L"Copyright (C)2009 Bizarre Creations",
	1,
	0,
	XAPO_FLAG_INPLACE_REQUIRED | XAPO_FLAG_CHANNELS_MUST_MATCH | XAPO_FLAG_FRAMERATE_MUST_MATCH | XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH | XAPO_FLAG_BUFFERCOUNT_MUST_MATCH | XAPO_FLAG_INPLACE_SUPPORTED,
	1, 1, 1, 1
};

//--------------------------------------------------------------------------------------
// Name: CAPOBase::CreateInstance
// Desc: Class factory for sample xAPO objects
//--------------------------------------------------------------------------------------
template< typename APOClass, typename ParameterClass >
HRESULT CAPOBase< APOClass, ParameterClass >::CreateInstance( void* pInitData, UINT32 cbInitData, APOClass** ppAPO )
{
    _ASSERT( ppAPO );
    HRESULT hr = S_OK;

    *ppAPO = new APOClass;

    hr = (*ppAPO)->Initialize( pInitData, cbInitData );
    return hr;
}


//--------------------------------------------------------------------------------------
// Name: CAPOBase::CAPOBase
// Desc: Constructor
//--------------------------------------------------------------------------------------
template< typename APOClass, typename ParameterClass >
CAPOBase< APOClass, ParameterClass >::CAPOBase() : CXAPOParametersBase( &m_regProps, (BYTE*)m_parameters, sizeof( ParameterClass ), FALSE )
{
    ZeroMemory( m_parameters, sizeof( m_parameters ) );
}

//--------------------------------------------------------------------------------------
// Name: CAPOBase::~CAPOBase
// Desc: Destructor
//--------------------------------------------------------------------------------------
template< typename APOClass, typename ParameterClass >
CAPOBase< APOClass, ParameterClass >::~CAPOBase()
{
}

//--------------------------------------------------------------------------------------
// Name: CAPOBase::LockForProcess
// Desc: Overridden so that we can remember the wave format of the signal
//       we're supposed to be processing
//--------------------------------------------------------------------------------------
template< typename APOClass, typename ParameterClass >
HRESULT CAPOBase< APOClass, ParameterClass >::LockForProcess( UINT32 InputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters, UINT32 OutputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters )
{
    HRESULT hr = CXAPOParametersBase::LockForProcess( InputLockedParameterCount, pInputLockedParameters, OutputLockedParameterCount, pOutputLockedParameters );

    if( SUCCEEDED( hr ) )
    {
        // Copy the wave format. Note that we're using memcpy rather than XMemCpy
        // because XMemCpy is really only beneficial for larger blocks of memory.
        memcpy( &m_wfx, pInputLockedParameters[0].pFormat, sizeof( WAVEFORMATEX ) );
    }
    return hr;
}

//--------------------------------------------------------------------------------------
// Name: CAPOBase::Process
// Desc: Overridden to call this class's typesafe version
//--------------------------------------------------------------------------------------
template< typename APOClass, typename ParameterClass >
void CAPOBase< APOClass, ParameterClass >::Process( UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS *pInputProcessParameters, UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS *pOutputProcessParameters, BOOL IsEnabled )
{
    _ASSERT( IsLocked() );
    _ASSERT( InputProcessParameterCount == 1 );
    _ASSERT( OutputProcessParameterCount == 1 );
    _ASSERT( pInputProcessParameters[0].pBuffer == pOutputProcessParameters[0].pBuffer );

    UNREFERENCED_PARAMETER( OutputProcessParameterCount );
    UNREFERENCED_PARAMETER( InputProcessParameterCount );
    UNREFERENCED_PARAMETER( pOutputProcessParameters );
    UNREFERENCED_PARAMETER( IsEnabled );

    ParameterClass* pParams = (ParameterClass*)BeginProcess();

    if( pInputProcessParameters[0].BufferFlags & XAPO_BUFFER_VALID )
		DoProcess( *pParams, (FLOAT32* __restrict)pInputProcessParameters[0].pBuffer, pInputProcessParameters[0].ValidFrameCount, m_wfx.nChannels );

	EndProcess();
}

struct RendererAPOParams
{
	Audio::Core::Renderer* pRenderer;
};

class __declspec( uuid("{5EB8D611-FF96-429d-8365-2DDF89A7C1CD}")) CRendererAPO : public CAPOBase< CRendererAPO, RendererAPOParams >
{
public:
    CRendererAPO();
    ~CRendererAPO();

    void DoProcess( const RendererAPOParams&, FLOAT32* __restrict pData, UINT32 cFrames, UINT32 cChannels );
};

#else // !USE_XAUDIO2

//--------------------------------------------------------------------------------------
// Constants and defines
//--------------------------------------------------------------------------------------
// Effect IDs for all of our static effects.  These are passed as parameters to the 
// effect creation functions
#define MASTERINPUTEFFECT_ID  ( XAUDIOFXID_FIRSTCLIENT+0 )

// Channel count
const INT MASTERINPUTEFFECT_CHANNELCOUNT = 8;

// The only parameter available for this effect.
// It obtains the PeakMeter data associated with this effect.
const XAUDIOFXPARAMID MASTERINPUTEFFECT_GETDATA_PARAMID     = 0x0;

// Number of samples.
const WORD g_wSampleCount = XAUDIOFRAMESIZE_NATIVE;

const FLOAT CLIPPED_RANGE = 1.0f;

//[cian reverb] updated
//enum MASTER_EFFECT_BUFFER
//{
//	DRY_BUFFER,
//	REVERB_BUFFER,
//	REVERB_BUFFER_2
//};

#ifndef ASSERT
#define ASSERT assert       // For XAUDIO_USE_BATCHALLOC_NEWDELETE macro
#endif

//--------------------------------------------------------------------------------------
// Declarations
//--------------------------------------------------------------------------------------
// The MASTERINPUTEFFECTDATA structure is returned when you query this effect using its
// MASTERINPUTEFFECT_GETDATA_PARAMID parameter.  You'll get an array of floats in
// fPeakMeterLevels, fPeakLevels, fRMSMeterLevels, and fRMSLevels that correspond to each channel.
// 
// The samples that contribute to the PeakMeter and RMSMeter values will have been collected over
// the past X frames, as specified by the user upon initialization in dwNumFramesStored.
// 
// NOTE: A frame is 256 samples (5 1/3 ms), and the internal processing rate is 48kHz
typedef struct MASTERINPUTEFFECTDATA
{
    BOOL  bPrimed;        // Has the effect run long enough to have valid data?
    DWORD dwFrameIndex;   // A constantly-incremented number.  Rolls over near 0xFFFFFFFF;
    FLOAT fClippingRatio[ MASTERINPUTEFFECT_CHANNELCOUNT ];   // The PeakMeter levels
    FLOAT fPeakLevels[ MASTERINPUTEFFECT_CHANNELCOUNT ];      // The peak levels
	FLOAT fRMSPeakLevels[ MASTERINPUTEFFECT_CHANNELCOUNT ];	// The peak RMS levels
} MASTERINPUTEFFECTDATA, *LPMASTERINPUTEFFECTDATA;
typedef const MASTERINPUTEFFECTDATA *LPCMASTERINPUTEFFECTDATA;

// Initialization structure.  You must pass an LPVOID to an instance of this when 
// you create this effect.
typedef struct MASTER_INPUT_EFFECT_INIT
{
    // The required effects init data.
    XAUDIOFXINIT EffectHeader;

    // Number of channels we expect to be analyzing.  Used to allocate appropriate 
    // amount of storage.
    DWORD dwNumChannels;
    
    // A frame is 256 samples (5 1/3 ms).  Taking the PeakMeter of an audio sample 
    // this small results in results that fluctuate.  Taking a larger sample will 
    // "calm" these fluctuations.
    DWORD dwNumFramesStored;    
} MASTERINPUTEFFECT_INIT, *LPMASTERINPUTEFFECT_INIT;


struct MasterEffectInit
{
	Audio::Core::Renderer* pRenderer;
	//F32* pFrameData;
	S32 sIndex;
	//bool bUseReverbBuffer;
	//U8	u32UseBuffer;
	//bool bUseTunnelBuffer;
//[cian reverb] updated
	//MasterEffectInit()
	//{
	//	pRenderer = 0;
	//	//bUseReverbBuffer = false;
	////	u32UseBuffer = DRY_BUFFER;
	//	//bUseTunnelBuffer = false;
	//}
};

//--------------------------------------------------------------------------------------
// Class definition
//--------------------------------------------------------------------------------------


// Size function.
HRESULT __stdcall QuerySizeMasterInputEffect( LPCXAUDIOFXINIT pInit, LPDWORD pEffectSize );
HRESULT QuerySizeMasterInputEffectBuffers( LPCXAUDIOFXINIT pInit, LPDWORD pEffectSize );

// Effect creation function.  Part of the data needed by XAudioInitialize().
HRESULT __stdcall CreateMasterInputEffect( const XAUDIOFXINIT* pInit, 
                               IXAudioBatchAllocator* pAllocator, 
                               IXAudioEffect** ppEffect );

class CMasterInputEffect: public IXAudioEffect
{
    XAUDIO_USE_BATCHALLOC_NEWDELETE();

    public:
        // IXAudioRefCount
        STDMETHOD_( ULONG, AddRef )( void );
        STDMETHOD_( ULONG, Release )( void );

        // IXAudioEffect    
        STDMETHOD( GetInfo )    ( XAUDIOFXINFO* pInfo );
        STDMETHOD( GetParam )   ( XAUDIOFXPARAMID nParamId, XAUDIOFXPARAMTYPE nParamType, XAUDIOFXPARAM* pParam );
        STDMETHOD( SetParam )   ( XAUDIOFXPARAMID nParamId, XAUDIOFXPARAMTYPE nParamType, const XAUDIOFXPARAM* pParam );
        STDMETHOD( GetContext ) ( LPVOID* ppvContext );
        STDMETHOD( Process )    ( IXAudioFrameBuffer* pInputBuffer, IXAudioFrameBuffer* pOutputBuffer );

        // CUserEffect
        CMasterInputEffect( void );
        STDMETHOD( Initialize ) ( const XAUDIOFXINIT* pInit, 
                                  IXAudioBatchAllocator* pAllocator );

    private:
        DWORD                   m_dwRefCount;        
        MASTERINPUTEFFECT_INIT  m_MasterInputEffectInit;
        DWORD                   m_dwFrameIndex;
        FLOAT*                  m_pfClipped;
        FLOAT*                  m_pfPeaks;
		FLOAT*					m_pfRMS;
        
        // The user must have supplied our effect ID via SetMasterInputEffectID();
        BYTE            m_byEffectID;
        
        // Has enough data passed through the pipe to allow correct calulation 
        // of the PeakMeter data?
        BOOL            m_bPrimed; 

        // Batch allocator.        
        IXAudioBatchAllocator *m_pAllocator;

        DWORD                   m_uFrame;
};

#endif // USE_XAUDIO2

