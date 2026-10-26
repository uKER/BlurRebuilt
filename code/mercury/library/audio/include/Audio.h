#ifndef _AUDIO_H_
#define _AUDIO_H_


// temp code to track down a crash on amax PS3
#ifdef PLATFORM_PS3
#ifndef CFG_GOLD
#define GET_COMPONENT_FROM_HASH_DEBUG_CODE
#endif
#endif
// end of temp code

#if !defined(BUILD_CONFIG_SPEED) && !defined (BUILD_CONFIG_MASTER)
#if !defined(SN_TARGET_PS3_SPU) && !defined(PLATFORM_PS3)
// disabled on ppu as well as spu because we can't disable it on one and not the other
// because state offsets are calculated on both and need to match.. but don't because some component
// state data contains additional members with this defined
//#define ENABLE_DEBUG_EXTENSIONS
#else
#define ENABLE_PS3_DEBUG_EXTENSIONS
#endif
#endif


#ifdef PLATFORM_PS3
#ifdef PS3_OUTPUT_TO_MULTISTREAM
#include <cell/mstream.h>
#endif
//#include <cell/i3dl2.h>
#endif

#if defined(PLATFORM_PC)
#ifndef AUDIO_TOOLS
#include "al.h"
#include "alc.h"
#include "efx.h"
#include "xram.h"
#include "efx-creative.h"
#include "EFX-Util.h"
#endif
#endif

#if ! defined(PLATFORM_PS3) && ! defined(PLATFORM_SPU)
#define AUDIO_CPU_COUNTER_CODE
#endif


#include "../source/common/types.h"
#include <system/scheduler.h>
#include <system/serialise/filepath.h>
#include <btl/utility/string.h>

// frame size on console versions was 256.. we should multiply release queue sizes by whatever multiple of
// this original frame size the current frame size is
#define QUEUE_SIZE_MULTIPLIER	(Audio::Core::uFrameLength / 256)


#define DAT_FILE_VERSION_MAJOR	2
#define DAT_FILE_VERSION_MINOR	11

namespace Sys
{
	class I_FileSystem;
}

namespace Audio
{
	class Handle
	{
		template <class T, int N> friend class HandleCollection;
		friend class Profiler;
		friend class ObjectManager;
		friend class ObjectResource;

	public:
		Handle() { m_Value = GetInvalidHandle(); }
		Handle(U16 u16InitValue) { m_Value = u16InitValue; }

//		bool operator ==(Handle hComp) { return (m_Value == hComp.m_Value); }
		inline void operator =(const U16 value) { m_Value = value; }
		inline bool IsValid() { return (m_Value != GetInvalidHandle()); }
		inline void SetAsInvalid() { m_Value = GetInvalidHandle(); }
		inline bool operator < ( const Handle other ) const { return (other.Value() < Value()); }
		inline U32 Value() const { return m_Value; }

	protected:
		inline U16 GetInvalidHandle() { return 0xffff; }
		inline operator U16() { return m_Value; }

		U16			m_Value;
	};

	class GenericStandaloneAudioS32Handle
	{

	public:
		GenericStandaloneAudioS32Handle() { SetInvalid(); }
		GenericStandaloneAudioS32Handle(S32 s32Val) { m_s32Handle = s32Val; }

		inline bool IsValid() { return m_s32Handle != -1; }
		inline void SetInvalid() { m_s32Handle = -1; }
		inline operator U32() { return U32(m_s32Handle); }
		GenericStandaloneAudioS32Handle& operator =(GenericStandaloneAudioS32Handle hOther) { m_s32Handle = hOther.m_s32Handle; return *this; }

	protected:
		S32 m_s32Handle;
	};

	class PagedArrayHandle
	{

	public:
		PagedArrayHandle() { u32Handle = GetInvalidHandle(); }
		PagedArrayHandle(U16 u16UniqueId, U16 u16Index) { u32Handle = (((U32)u16UniqueId) << 16) | u16Index; }
		PagedArrayHandle(U32 u32Value) { u32Handle = u32Value; }

		bool operator ==(PagedArrayHandle hComp) { return (u32Handle == hComp.u32Handle); }
		bool operator !=(PagedArrayHandle hComp) { return (u32Handle != hComp.u32Handle); }
		void operator =(PagedArrayHandle hAssign) { u32Handle = hAssign.u32Handle; }

		inline U16 GetUniqueId() const { return (U16)(u32Handle >> 16); }
		inline U16 GetIndex() const { return (U16)(u32Handle & 0xffff); }
		inline U32 GetValue() const { return u32Handle; }

		inline bool IsValid() const { return u32Handle != GetInvalidHandle(); }
		inline U32 GetInvalidHandle() const { return 0; }

		inline void SetInvalidHandle()  { u32Handle = GetInvalidHandle(); }

		U32 u32Handle;
	};


	typedef PagedArrayHandle				StreamHandle;
	typedef GenericStandaloneAudioS32Handle	StreamSourceHandle;

//	typedef U32		StreamSourceHandle;


	typedef Handle	ObjectHandle;
	typedef Handle	InstanceHandle;
	typedef Handle	ObjectComponentHandle;


	class InstanceComponentHandle
	{

	public:
		InstanceComponentHandle() { }
		InstanceComponentHandle(InstanceHandle hInst, ObjectComponentHandle hComp) { m_hInstance = hInst; m_hObjectComponent = hComp; }

		inline InstanceHandle			GetInstanceHandle() { return m_hInstance; }
		inline ObjectComponentHandle	GetObjectComponentHandle() { return m_hObjectComponent; }
		inline bool						IsValid() { return (m_hInstance.IsValid() && m_hObjectComponent.IsValid()); }

	private:
		InstanceHandle			m_hInstance;
		ObjectComponentHandle	m_hObjectComponent;
	};

	class ObjectResource
	{

	public:
		ObjectResource() { }
		ObjectResource(ObjectHandle h, U16 res) { m_h = h; m_u16 = res; }
		operator U32() { return ((U32)m_h.Value() << 16) | m_u16; }

	private:
		ObjectHandle			m_h;
		U16						m_u16;
	};

//	inline S32 PagedArrayInvalidHandle() { return 0; }
//	inline U16 GetInvalidHandle() { return 0xffff; }

	// callback for wavebankmanager callback to get mapped filename
	typedef Btl::String (*WbmMapFilenameFunc)(const Btl::String& strFname);

////////////////////////////////////////////////////////////////////////////////
//	forward class defs
////////////////////////////////////////////////////////////////////////////////
    class IInstanceManager;

    class ControlInput;
    class EventInput;
    class VectorInput;
    class GraphInput;
    class MonoAudioInput;
    class StereoAudioInput;
    class SurroundAudioInput;
    class SurroundLFEAudioInput;
    class ControlOutput;
    class EventOutput;
    class VectorOutput;
    class GraphOutput;
    class MonoAudioOutput;
    class StereoAudioOutput;
    class SurroundAudioOutput;
	class SurroundLFEAudioOutput;
	class File;
	class CircularBuffer;
	struct ScriptLineDef;

////////////////////////////////////////////////////////////////////////////////
//	Result handles
////////////////////////////////////////////////////////////////////////////////
    enum Result
    {
        Success,
        Failed,
        OutOfMemory,
        InstanceNotFoundError,
        DefinitionNotFoundError,
		ObjectNotFoundError,
        UnknownDefinitionType,
		WaveBankNotFound,
		WaveBankIncompatibleVersion,
		WaveBankCorrupt,
		WaveNotFound,
		InvalidHandle,
		VersionMismatch,
		EndianMismatch,
        NumResults
    };

////////////////////////////////////////////////////////////////////////////////
//	memory callbacks
////////////////////////////////////////////////////////////////////////////////
    typedef void* (*AllocateMemory)(size_t uSize, U32 uAlignment);
    typedef void (*FreeMemory)(void* pAddress);

////////////////////////////////////////////////////////////////////////////////
//	debug/ diagnostic message callbacks
////////////////////////////////////////////////////////////////////////////////
	typedef void (*PrintFunc)(F32 fX, F32 fY, U32 u32RGBA, F32 f32Size, char* strText);
	typedef void (*DrawBoxCallback)(F32 fXPos, F32 fYPos, F32 fXSize, F32 fYSize, U32 u32RGBA);

////////////////////////////////////////////////////////////////////////////////
//	file access callbacks
////////////////////////////////////////////////////////////////////////////////
	typedef U32 (*FileOpenFunc) (const char* pchFileName, bool bWrite);
	typedef bool (*FileReadFunc) (U32 hFile, void* pData, U32 uNumBytes, U32* puBytesRead);
	typedef bool (*FileSeekFunc) (U32 hFile, U32 pos);
	typedef bool (*FileWriteFunc) (U32 hFile, const void* pData, S32 sLength);
	typedef bool (*FileCloseFunc) (U32 hFile);
	typedef U32 (*FileSizeFunc) (U32 hFile);
	typedef void (*FilePauseFunc) (U32 hFile);
	typedef void (*FileResumeFunc) (void);

////////////////////////////////////////////////////////////////////////////////
//	wavebankmanager callbacks
////////////////////////////////////////////////////////////////////////////////
	typedef const char* (*GetMappedNameFunc) (const char* strBank, const char*& pchPath);
	
////////////////////////////////////////////////////////////////////////////////
//	engine communication callbacks
////////////////////////////////////////////////////////////////////////////////
	typedef bool (*AudioRequestDataCallback)(U32 hDataTracker, F32 f32Value);
	typedef void (*AudioActionOutCallback) (U32 u32AppData);

////////////////////////////////////////////////////////////////////////////////
//	objectmanager callbacks
////////////////////////////////////////////////////////////////////////////////
	typedef void (*DatPathCallback)(const char* pBuf, U32 uBufLen);

////////////////////////////////////////////////////////////////////////////////
//	Return queue structures
////////////////////////////////////////////////////////////////////////////////
	struct ReturnQueueMessage
	{
		bool bSuccess;
	};

	struct GetValueQueueMessage : public ReturnQueueMessage
	{
		AudioRequestDataCallback pCallback;
		U32 hDataTracker;
		F32 f32Value;
	};

	struct CallbackQueueMessage : public ReturnQueueMessage
	{
		AudioActionOutCallback pCallback;
		U32 u32AppData;
	};

	struct DestroyObjectQueueMessage : public ReturnQueueMessage
	{
		ObjectHandle hObject;
		bool bReserveHandle;
	};

	namespace Core
    {
////////////////////////////////////////////////////////////////////////////////
//	WaveData
////////////////////////////////////////////////////////////////////////////////
        struct WaveData
        {
			WaveData()
			{
				uChannels = 0;
				uLength = 0;
				pData = NULL;
				uLoop = false;
			};
            U8 uChannels;
            U32 uLength;
            void* pData;
			bool uLoop;	//cian hack for adpcm loop
        };

		// forward decl
//		struct FileStreamInfoPS3;
    }

	typedef U32 WM_WAVE_HANDLE;
	typedef U32 WM_BANK_HANDLE;
	static const U32 WM_INVALID_HANDLE = 0;

////////////////////////////////////////////////////////////////////////////////
//	WaveExtendedInfo
////////////////////////////////////////////////////////////////////////////////
	struct WaveExtendedInfo
	{
		WaveExtendedInfo()
		{
			bCreatedStream = false;
			#ifdef PLATFORM_PS3
			pStreamInfo = NULL;
			#endif
		}
		bool bCreatedStream;
#ifdef PLATFORM_PS3
		void* pStreamInfo;
#endif
	};

////////////////////////////////////////////////////////////////////////////////
//	WaveStreamReadRequest
////////////////////////////////////////////////////////////////////////////////
	namespace Core
	{
		struct WaveStreamReadRequest;
	}
////////////////////////////////////////////////////////////////////////////////
//	IWaveServer
////////////////////////////////////////////////////////////////////////////////
	enum eWaveBankCreationType
	{
		eWaveBankCreationType_NotSet = 0,
		eWaveBankCreationType_Internal,			// created from within the audio system
		eWaveBankCreationType_External,			// created from outsied the audio system
	};

	namespace Core
	{
		typedef GenericStandaloneAudioS32Handle	WaveServerHandle;
		typedef GenericStandaloneAudioS32Handle	WaveServerLocalIndex;

		struct WaveStreamData;
		
/*		class WaveServerHandle
		{

		public:
			WaveServerHandle() { SetInvalid(); }
			WaveServerHandle(S32 s32Val) { m_s32Handle = s32Val; }

			bool IsValid() { return m_s32Handle != -1; }
			void SetInvalid() { m_s32Index = -1; }
			operator U32() { return m_s32Handle; }

		private:
			S32 m_s32Handle;
		};
//		typedef U32				WaveServerHandle;

		class WaveServerLocalIndex
		{

		public:
			WaveServerLocalIndex() { SetInvalid(); }
			WaveServerLocalIndex(S32 s32Val) { m_s32Index = s32Val; }

			bool IsValid() { return m_s32Index != -1; }
			void SetInvalid() { m_s32Index = -1; }
			operator U32() { return m_s32Index; }

		private:
			S32 m_s32Index;
		};*/

		class IWaveServer
		{

		public:
			virtual bool		SendRequest(WaveStreamReadRequest& request) = 0;
		};

		class WaveServerComponent
		{
		public:
			WaveServerComponent(IWaveServer* pWaveServer) { m_iWaveServer = pWaveServer; }

			IWaveServer*		GetWaveServer() { return m_iWaveServer; }
			bool				LoadData(WaveStreamReadRequest& readRequest);

		protected:
			virtual bool		GetData(WaveServerHandle hWave, CircularBuffer* pDestCircularBuffer) = 0;
			virtual void		ReadRequestProcessed(WaveServerHandle hStream, U32 u32NumBytesRead, U32 u32RequestId = 0) = 0;

		private:
			IWaveServer*		m_iWaveServer;

		};
	}

////////////////////////////////////////////////////////////////////////////////
//	IWaveManager
////////////////////////////////////////////////////////////////////////////////
    class IWaveManager
    {
    public:
        virtual ~IWaveManager() {}

//        virtual void AddBankFilePath(const char* pchPath) = 0;
		virtual void SetMapFilenameFunc(WbmMapFilenameFunc func) = 0;

		virtual Audio::Result Prime(const char* pchBank, const char* pchName, WM_WAVE_HANDLE& hWave, WaveExtendedInfo* exInfo) = 0;
		virtual S32 GetFormat(const WM_WAVE_HANDLE hWave) const = 0;
		virtual Audio::Result GetWave(const WM_WAVE_HANDLE hWave, void* pWaveData, U32* pu32StreamIndex, Core::WaveStreamData* pStreamInfo = 0) const = 0;
		virtual Audio::Result Release(const WM_WAVE_HANDLE hWave) = 0;
		
		virtual WM_BANK_HANDLE LoadBank(const char* pchFullBankPath) = 0;
		virtual Audio::Result ReleaseBank(const WM_BANK_HANDLE hBank, const eWaveBankCreationType eCreation = eWaveBankCreationType_External) = 0;

		virtual void LogLoadedBanks() = 0;

        virtual void SetBankFilePath(const Sys::C_FilePath& pchPath) = 0;
		virtual void RenderStats() = 0;
	};


	class Definition;
	class DefinitionManager;
	class IObjectManager;
    class ObjectManager;
//    class IInstance;
    class Object;
#if defined(ENABLE_DEBUG_EXTENSIONS)
    class Component;
	namespace Core { class Lock; };
#endif // ENABLE_DEBUG_EXTENSIONS

////////////////////////////////////////////////////////////////////////////////
//	IControlScriptManager
////////////////////////////////////////////////////////////////////////////////
	//class IControlScriptManager
	//{

	//public:
	//	IControlScriptManager() { }
	//	virtual ~IControlScriptManager() { }

	//	virtual EnvironmentHandle			CreateEnvironment(ScriptDefinitionHandle hDef) = 0;
	//	virtual bool						Parse(EnvironmentHandle hEnvironment, IWaveManager* pWaveManager, U32 u32FunctionIndex) = 0;
	//	virtual bool						Parse(EnvironmentHandle hEnvironment, IWaveManager* pWaveManager, const char* strFunctionName) = 0;
	//	virtual bool						ReleaseEnvironment(EnvironmentHandle hEnvironment) = 0;
	//	virtual ScriptDefinitionHandle		LoadDefinition(const char* const strFilename) = 0;
	//	virtual bool						PlugInstanceIntoEnvironment(const char* pchIdentifier, EnvironmentHandle hEnvironment, InstanceHandle hInstance) = 0;
	//	virtual bool						PlugObjectIntoEnvironment(const char* pchIdentifier, EnvironmentHandle hEnvironment, ObjectHandle hObject) = 0;
	//	virtual bool						PlugObjectComponentIntoEnvironment(const char* pchIdentifier, EnvironmentHandle hEnvironment, ObjectComponentHandle hObjComp) = 0;
	//	virtual bool						PlugInstanceComponentIntoEnvironment(const char* pchIdentifier, EnvironmentHandle hEnvironment, InstanceComponentHandle hInstComp) = 0;
	//	virtual InstanceHandle				GetInstanceFromEnvironment(const char* pchIdentifier, EnvironmentHandle hEnvironment) = 0;
	//	virtual ObjectHandle				GetObjectFromEnvironment(const char* pchIdentifier, EnvironmentHandle hEnvironment) = 0;
	//	virtual InstanceComponentHandle		GetInstanceComponentFromEnvironment(const char* pchIdentifier, EnvironmentHandle hEnvironment) = 0;
	//	virtual ObjectComponentHandle		GetObjectComponentFromEnvironment(const char* pchIdentifier, EnvironmentHandle hEnvironment) = 0;
	//};

////////////////////////////////////////////////////////////////////////////////
//	IObjectManager
////////////////////////////////////////////////////////////////////////////////
    class IObjectManager
    {
    public:
        virtual ~IObjectManager() {}

		virtual ObjectHandle Create(const char* pchFilename, IWaveManager* pWaveManager) = 0;
		virtual ObjectHandle CreateImmediate(const char* pchFilename, IWaveManager* pWaveManager) = 0;
		virtual bool CreateUsingHandle(const char* pchFilename, IWaveManager* pWaveManager, ObjectHandle hObject) = 0;
        virtual Result Destroy(ObjectHandle pObject, bool bReserveHandle = false) = 0;

        virtual bool Suspend(InstanceHandle hInstance, bool bSuspend) = 0;
        virtual bool IsSuspended(InstanceHandle hInstance) = 0;
		virtual bool Mute(InstanceHandle hInstance, bool bMute) = 0;
        virtual bool IsMuted(InstanceHandle hInstance) = 0;
		virtual bool ConnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn) = 0;
		virtual bool DisconnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn) = 0;
		virtual bool RegisterActionOutCallback(InstanceComponentHandle hActionOut, AudioActionOutCallback pCallback, U32 u32AppData) = 0;
		virtual U32 GetNumMixersActive() = 0;
		virtual U32 GetNumInstancesActive() = 0;
		virtual U32 GetNumInstancesMuted() = 0;
		virtual U32 GetNumTotalInstances() = 0;
		// functions that used to be in IObject..
        virtual InstanceHandle CreateObjectInstance(ObjectHandle hObject, const char* pchGroupName = 0, const char* pchEntityName = 0) = 0;
		virtual bool CreateObjectInstanceUsingHandle(ObjectHandle hObject, InstanceHandle hInstance, const char* pchGroupName = 0, const char* pchEntityName = 0) = 0;
        virtual void DestroyObjectInstance(InstanceHandle hInstance, bool bReserveHandle = false) = 0;

		virtual bool ReleaseObjectComponentHandle(ObjectComponentHandle hObjComp, bool bReserveHandle = false) = 0;

		virtual InstanceComponentHandle GetInstanceComponentHandle(ObjectComponentHandle hObjComp, InstanceHandle hInstance) = 0;

		virtual bool IsInstanceValid(InstanceHandle hInstance) = 0;
		virtual bool IsObjectValid(ObjectHandle hObject) = 0;
		virtual bool IsObjectComponentValid(ObjectComponentHandle hComponent) = 0;
		virtual bool IsInstanceComponentValid(InstanceComponentHandle hComponent) = 0;

		virtual ObjectComponentHandle GetObjectActionIn(ObjectHandle hObject, const char* pchName) = 0;				
		virtual ObjectComponentHandle GetObjectActionOut(ObjectHandle hObject, const char* pchName) = 0;				
		virtual ObjectComponentHandle GetObjectParameterIn(ObjectHandle hObject, const char* pchName) = 0;			
		virtual ObjectComponentHandle GetObjectParameterOut(ObjectHandle hObject, const char* pchName) = 0;			
		virtual ObjectComponentHandle GetObjectVectorIn(ObjectHandle hObject, const char* pchName) = 0;				
		virtual ObjectComponentHandle GetObjectControlInput(ObjectHandle hObject, const char* pchName) = 0;			
		virtual ObjectComponentHandle GetObjectEventInput(ObjectHandle hObject, const char* pchName) = 0;				
		virtual ObjectComponentHandle GetObjectVectorInput(ObjectHandle hObject, const char* pchName) = 0;			
		virtual ObjectComponentHandle GetObjectGraphInput(ObjectHandle hObject, const char* pchName) = 0;				
		virtual ObjectComponentHandle GetObjectMonoAudioInput(ObjectHandle hObject, const char* pchName) = 0;			
		virtual ObjectComponentHandle GetObjectStereoAudioInput(ObjectHandle hObject, const char* pchName) = 0;		
		virtual ObjectComponentHandle GetObjectSurroundAudioInput(ObjectHandle hObject, const char* pchName) = 0;		
		virtual ObjectComponentHandle GetObjectSurroundLFEAudioInput(ObjectHandle hObject, const char* pchName) = 0;
		virtual ObjectComponentHandle GetObjectControlOutput(ObjectHandle hObject, const char* pchName) = 0;
		virtual ObjectComponentHandle GetObjectEventOutput(ObjectHandle hObject, const char* pchName) = 0;
		virtual ObjectComponentHandle GetObjectVectorOutput(ObjectHandle hObject, const char* pchName) = 0;
		virtual ObjectComponentHandle GetObjectGraphOutput(ObjectHandle hObject, const char* pchName) = 0;
		virtual ObjectComponentHandle GetObjectMonoAudioOutput(ObjectHandle hObject, const char* pchName) = 0;
		virtual ObjectComponentHandle GetObjectStereoAudioOutput(ObjectHandle hObject, const char* pchName) = 0;
		virtual ObjectComponentHandle GetObjectSurroundAudioOutput(ObjectHandle hObject, const char* pchName) = 0;
		virtual ObjectComponentHandle GetObjectSurroundLFEAudioOutput(ObjectHandle hObject, const char* pchName) = 0;
		virtual ObjectComponentHandle GetObjectComponent(ObjectHandle hObject, const char* pchName) = 0;
		virtual void SetObjectFrameSkip(ObjectHandle hObject, U32 frameSkip) = 0;

		virtual bool ReuseObjectComponent(ObjectHandle hObject, ObjectComponentHandle hObjComp, const char* pchName) = 0;
	};

////////////////////////////////////////////////////////////////////////////////
//	IDefinitionManager
////////////////////////////////////////////////////////////////////////////////
	class IDefinitionManager
    {
    public:
		IDefinitionManager() {}
        virtual ~IDefinitionManager() {}

		virtual Definition* Create(const char* pchFilename) = 0;
		virtual Result Release(Definition*& pDefinition) = 0;
	};
////////////////////////////////////////////////////////////////////////////////
//	IDebug
////////////////////////////////////////////////////////////////////////////////
	class IDebug
	{
    public:
		virtual ~IDebug() {}
		virtual void DrawText2D(const char* pchText, F32 fX, F32 fY) = 0;
		virtual void DrawText3D(const char* pchText, F32 fX, F32 fY, F32 fZ) = 0;
	};

////////////////////////////////////////////////////////////////////////////////
//	ISystem
////////////////////////////////////////////////////////////////////////////////
	typedef void (*SystemCallback)(void* pContext);

    class ISystem
    {
    public:
		enum AllocType
		{
			eVirtual,
			ePhysical,

			eNumAllocTypes
		};
		enum eDistributedUpdateSetting
		{
			eDistribUpdate_UseDefaultSetting=0,
			eDistribUpdate_Disabled,
			eDistribUpdate_Enabled,
		};
		struct InitParams
		{
			InitParams()
			{
				uMessageQueueNumBytes = 0;
				uReturnMessageQueueNumBytes = 0;
				uNumReverbs = 0;
				uTempFrameBufferNumBytes = 0;
				uInstanceStateDataBufferSize = 0;
				bStereoReverb = false;
				pfnAllocate = NULL;
				pfnFree = NULL;
				pfnPhyAllocate = NULL;
				pfnPhyFree = NULL;
				pfnGetDatPath = NULL;
				hWnd = NULL;
				hPolicy = 0;
				hGroup = 0;
				eDistributedUpdate = eDistribUpdate_UseDefaultSetting;
			}
			U32 uMessageQueueNumBytes;
			U32 uReturnMessageQueueNumBytes;
			U32 uNumReverbs;
			U32 uTempFrameBufferNumBytes;
			U32 uInstanceStateDataBufferSize;
			bool bStereoReverb;
			AllocateMemory pfnAllocate;
			FreeMemory pfnFree;
			AllocateMemory pfnPhyAllocate;
			FreeMemory pfnPhyFree;
			DatPathCallback pfnGetDatPath;
			eDistributedUpdateSetting eDistributedUpdate;
			Scheduler::PolicyHandle hPolicy;
			Scheduler::PolicyHandle hPolicyB;
			Scheduler::GroupHandle hGroup;
			void* hWnd;							// PC only. needs casting to HWND
		};

#ifdef AUDIO_CPU_COUNTER_CODE
		#define MAX_CPU_NAMED_EVENTS 128
		#define CPU_EVENT_NAME_LENGTH 16

		struct NamedCpuEvent
		{
			char pchName[CPU_EVENT_NAME_LENGTH];
			F32 fTime;
		};
#endif

		struct Info
		{
			Info()
			{
				uTotalGenerators = 0;
				uActiveGenerators = 0;
				uTotalWaves = 0;
				uActiveWaves = 0;
				uTotalMixers = 0;
				uActiveMixers = 0;
				uTotalPCMDecoders = 0;
				uActivePCMDecoders = 0;
				uTotalADPCMDecoders = 0;
				uActiveADPCMDecoders = 0;
				uTotalXMADecoders = 0;
				uActiveXMADecoders = 0;
				uCurrentMessageQueueBytes = 0;
				uPeakMessageQueueBytes = 0;
				uNumObjectDefinitions = 0;
				fTotalCpuTime = 0;
				fMsgQueueCpuTime = 0;
				fObjUpdateAndGenerateCpuTime = 0;
				fStreamerCpuTime = 0;
				
#ifdef AUDIO_CPU_COUNTER_CODE
				uNumNamedCpuEvents = 0;
				for (S32 i=0; i<MAX_CPU_NAMED_EVENTS; i++)
				{
					namedCpuEvents[i].fTime = 0.f;
					namedCpuEvents[i].pchName[0] = 0;
				}
#endif
			}
			U32 uTotalGenerators;
			U32 uActiveGenerators;
			U32 uTotalWaves;
			U32 uActiveWaves;
			U32 uTotalMixers;
			U32 uActiveMixers;
			U32 uTotalPCMDecoders;
			U32 uActivePCMDecoders;
			U32 uTotalADPCMDecoders;
			U32 uActiveADPCMDecoders;
			U32 uTotalXMADecoders;
			U32 uActiveXMADecoders;
			U32 uCurrentMessageQueueBytes;
			U32 uPeakMessageQueueBytes;
			U32 uNumObjectDefinitions;
				
			//F32 fPreUpdateTime;
			F32 fTotalCpuTime;
			F32 fMsgQueueCpuTime;
			F32 fObjUpdateAndGenerateCpuTime;
			//F32 fObjUpdateTime;		
			//F32 fObjGenerateTime;	
			//F32 fPostUpdateTime;
			F32 fStreamerCpuTime;
			
#ifdef AUDIO_CPU_COUNTER_CODE
			U32 uNumNamedCpuEvents;
			NamedCpuEvent namedCpuEvents[MAX_CPU_NAMED_EVENTS];
#endif
		};

		enum SpeakerOption
		{
			eHeadphone,
			eStereoHiFi,
			eSurround,

			eNumSpeakerOptions
		};

		enum SpeakerConfig
		{
			eStereo,
			eFivePointOne,
			eSevenPointOne,

			eNumSpeakerModes
		};

    public:
        virtual ~ISystem() {}

        virtual bool Init() = 0;
        virtual bool Shutdown() = 0;

		virtual bool IsPaused() const = 0;
		virtual void Pause(bool pause) = 0;

		virtual IDefinitionManager* GetIDefinitionManager() = 0;
		virtual IObjectManager* GetIObjectManager() = 0;
//		virtual IControlScriptManager* GetControlScriptManager() = 0;
		//virtual IInstanceManager* GetInstanceManager() = 0;

		virtual void SetMaster(InstanceComponentHandle hComponent) = 0;
        virtual void SetMonoMaster(InstanceComponentHandle hComponent) = 0;
        virtual void SetStereoMaster(InstanceComponentHandle hComponent) = 0;
        virtual void SetSurroundMaster(InstanceComponentHandle hComponent) = 0;
        virtual void SetSurroundLFEMaster(InstanceComponentHandle hComponent) = 0;

		virtual void SetAllocator(AllocateMemory pfnAlloc, FreeMemory pfnFree) = 0;
		virtual void SetPhysicalAllocator(AllocateMemory pfnAlloc, FreeMemory pfnFree) = 0;

		virtual void SetPrintAtCallback(PrintFunc printFunc) = 0;
		virtual void SetDrawBoxCallback(DrawBoxCallback pCallback) = 0;

		virtual void SetFileOpenCallback(FileOpenFunc openFunc) = 0;
		virtual void SetFileReadCallback(FileReadFunc readFunc) = 0;
		virtual void SetFileSeekCallback(FileSeekFunc seekFunc) = 0;
		virtual void SetFileWriteCallback(FileWriteFunc writeFunc) = 0;
		virtual void SetFileCloseCallback(FileCloseFunc closeFunc) = 0;
		virtual void SetFileSizeCallback(FileSizeFunc sizeFunc) = 0;
		virtual void SetFilePauseCallback(FilePauseFunc sizeFunc) = 0;
		virtual void SetFileResumeCallback(FileResumeFunc sizeFunc) = 0;

		virtual void DebugRender() = 0;

		virtual void SignalEvent() = 0;
//#ifdef PLATFORM_PS3
//		virtual void *GetSpuScheduler() = 0;
//#endif

		virtual void SetValue(InstanceComponentHandle hParamIn, F32 fValue) = 0;
		virtual void Value(InstanceComponentHandle hParamOut, U32 hDataTracker, AudioRequestDataCallback pCallback) = 0;
		virtual void Execute(InstanceComponentHandle hActionIn) = 0;
//		virtual void Count(InstanceComponentHandle hActionOut, bool bReset, AudioRequestDataCallback pCallback) = 0;
		virtual void SetDimensions(InstanceComponentHandle hVectorIn, F32* afDimensions) = 0;

		// Engine muting functions
		virtual void MuteOutput() = 0;
		virtual void UnmuteOutput() = 0;

		// Debug functions
		#ifdef ENABLE_DEBUG_EXTENSIONS
		virtual const char* GetName(ObjectComponentHandle hActionIn) = 0;
		#endif

		virtual U32 QueryCurrentMemoryAllocationId() = 0;

		virtual const void GetInfo(Info* pInfo, bool bAverage = true) /*const*/ = 0;

		virtual void SetPreUpdateCallback(SystemCallback pCallback) = 0;
		virtual void SetPostUpdateCallback(SystemCallback pCallback) = 0;

		virtual void SetDebugger(IDebug* pDebugger) = 0;

		virtual U32 GetFramesDropped() = 0;

		virtual bool StaggerInstanceCreation() = 0;
		virtual void SetStaggeredInstanceCreation(bool bEnable) = 0;

#if defined(PLATFORM_PC)
#ifndef AUDIO_TOOLS
		virtual void SetReverb(EAXREVERBPROPERTIES* pSettings, U32 uIndex) = 0;
#endif
#endif
		
		virtual void SetHighQuality(bool bHighQuality) = 0;

#ifdef PLATFORM_PS3
#ifdef PS3_OUTPUT_TO_MULTISTREAM
#ifdef AUDIO_USE_OLD_REVERB
		virtual void SetReverb(CellMSFXReverbParams* pSettings, U32 uIndex) = 0;
#else
		virtual void SetReverb(CellMSFXI3DL2Params* pSettings, U32 uIndex) = 0;
#endif // AUDIO_USE_OLD_REVERB
#endif	//	PS3_OUTPUT_TO_MULTISTREAM
#endif	//	PLATFORM_PS3
		virtual void SetSpeakerDistance( SpeakerOption eOption, F32 fDistance ) = 0;
		virtual void SetSpeakerAngles( SpeakerConfig eMode, F32 fFrontLeft, F32 fFrontRight, F32 fSurroundLeft = 0.f, F32 fSurroundRight = 0.f, F32 fBackLeft = 0.f, F32 fBackRight = 0.f ) = 0;
		virtual void SetSpeakerOption( SpeakerOption eOption ) = 0;

		virtual bool TitleHasControl() = 0;


#if defined(ENABLE_DEBUG_EXTENSIONS)
		virtual Core::Lock* GetLock() = 0;
#endif

		virtual void DoWork() = 0;
};

////////////////////////////////////////////////////////////////////////////////
//	IDebugMessage
////////////////////////////////////////////////////////////////////////////////
    class IDebugMessage
    {
    public:
        virtual ~IDebugMessage() {}

		virtual bool SetMessageData(void* pData, U32 u32Size) = 0;
		virtual bool Despatch() = 0;
	};

////////////////////////////////////////////////////////////////////////////////
//	Functions
////////////////////////////////////////////////////////////////////////////////
	ISystem* CreateSystem(ISystem::InitParams* pInitParams);
    void ReleaseSystem();
	ISystem* GetSystem();

    IWaveManager* CreateWaveManager();
    void ReleaseWaveManager(IWaveManager* pWaveManager);

////////////////////////////////////////////////////////////////////////////////
//	Movie Capture
////////////////////////////////////////////////////////////////////////////////
	typedef int (*CaptureFunc)(F32 *audio, int channels, int len);
	void CaptureFrames(CaptureFunc callback, F32 seconds);
	void CaptureStop();

}


#endif // _AUDIO_H_
