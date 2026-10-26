#ifndef BIZ_SYSTEM_IO_ASYNC_H
#define BIZ_SYSTEM_IO_ASYNC_H


#define IODEFAULT_NUM_BUFFERS 2

// Enums

enum IoPriority
{
	kIoPriority_High,	// Attempt to satify request in the next 1/4 second
	kIoPriority_Medium,	// Within the next 2 seconds
	kIoPriority_Low		// Within the next 8 seconds
};

enum IoAction
{
	kIoAction_None,
	kIoAction_Open,
	kIoAction_Read,
	kIoAction_Close
};

enum IoEvent
{
	kIoEvent_Failed,
	kIoEvent_Complete
};

enum IoError
{
	kIoError_Okay,
	kIoError_NullRequest,
	kIoError_InvalidParameters,
	kIoError_NoDeviceMapping,
	kIoError_PathNotFound,
	kIoError_FileNotFound,
	kIoError_AccessDenied,
	kIoError_MediaRemoved,
	kIoError_EndOfFile,
	kIoError_OutOfMemory,
	kIoError_Unknown,
	kIoError_FileNotYetOpen,
	kIoError_TooManyFilesOpen
};

enum IoReadType
{
	kIoReadType_Unknown,
	kIoReadType_Buffered,
	kIoReadType_Unbuffered,
	kIoReadType_Speculative,
	kIoReadType_Count
};

extern const char* g_ioReadTypeStr[kIoReadType_Count];

// IoFile

class IoFile
{
public:
	IoFile();
	IoFile(const IoFile& file);
	explicit IoFile(struct IoFileImpl* impl);
	~IoFile();

public:
	void operator=(const IoFile& file);

public:
	const Btl::String& Filename() const;
	U64 Size() const;
	U64 Handle() const { return (U64)_impl; }

private:
	struct IoFileImpl* _impl;

friend class IoService;
friend class IoDevice;
};

// IoRequest

class IoRequest
{
public:
	IoRequest();
	IoRequest(const IoRequest& req);
	explicit IoRequest(struct IoRequestImpl* impl);
	~IoRequest();

public:
	void operator=(const IoRequest& req);

public:
	void SetPriority(IoPriority priority);
	void Cancel();
	void Wait();

public:
	IoAction		Action() const;
	const IoFile&	File() const;
	void*			Buffer() const;
	U64				Offset() const;
	U32				RequestSize() const;
	U32				ActualSize() const;
	IoPriority		Priority() const;
	IoError			Error() const;

private:
	struct IoRequestImpl* _impl;

friend class IoService;
friend class IoDevice;
};

// IoCallback

class IoCallback
{
public:
	IoCallback();
	virtual ~IoCallback();

public:
	virtual void Event(IoEvent event, const IoRequest& request) = 0;
	virtual void Release() = 0;
};

// Interface

bool IoInitialise();
void IoShutdown();

void IoCachePrioritise(const char** fileList, int numFiles);

IoRequest IoFileOpen(IoFile& file, const char* filename, IoCallback* callback);
IoRequest IoFileRead(const IoFile& file, void* buffer, U64 offset, U32 size, IoPriority pri, IoCallback* callback);
IoRequest IoFileClose(const IoFile& file, IoCallback* callback);

#ifdef PLATFORM_PS3
void IoSetTitleIdPs3(const char* titleId);
#endif

void IoSuspendFileCaching();
void IoResumeFileCaching();

struct IoCacheStats
{
    U64 cachedTotal;
	U32 fileCount;
    U32 cacheHitOpens;
    U32 cacheMissOpens;
    U32 cacheHitReads;
    U32 cacheMissReads;
    S64 cacheHitBytesRequested;
    S64 cacheMissBytesRequested;
    S64 cacheCopyBytes;
    S64 cacheEvictBytes;
};

void IoGetCacheStats(IoCacheStats* pStats);

// I_File wrapper

#include <System/Artery/ArterySync.h>

class AsyncFile : public Sys::I_File, private IoCallback
{
public:
	AsyncFile();
	virtual ~AsyncFile();

public:
	bool Open(const char* filename, int bufferSize, bool isCritical, U32 maxBuffers = 0);

public:
	virtual const bool				IsWriting() const;
	virtual Btl::String				Name() const;
	virtual U32						GetVersionNumber() const;
	virtual void					SetVersionNumber(U32 iVersion);
	virtual bool					Underflow(UnderflowContext& ctx);
	virtual bool					Overflow();
	virtual const Sys::C_StreamSize	Length();
	virtual const Sys::C_StreamPos	GetPosition() const;
	virtual void					SetPosition(const Sys::C_StreamPos newPosition);
	virtual const bool				GetEndianSwapFlag() const;
	virtual void					SetEndianSwapFlag(const bool swizzle);
	virtual void					Flush();

private:
	virtual void Event(IoEvent event, const IoRequest& request);
	virtual void Release();

private:
	enum { kMaxBuffers = 10 };

private:
	enum State
	{
		kEmpty,
		kPending,
		kComplete,
		kFailed
	};

	struct Buffer : public IoCallback
	{
		AsyncFile*	file;
		State		state;
		U64			offset;
		U32			size;
		U8*			data;
		//just used for logging.
		IoReadType	readType;

	public:
		Buffer() : file(0), state(kEmpty), offset(0), size(0), data(0), readType(kIoReadType_Unknown) { }

	public:
		virtual void Event(IoEvent event, const IoRequest& request);
		virtual void Release();
	};

private:
	ArteryMutex		_mutex;
	ArteryCondVar	_condVar;

	IoFile	_file;
	State	_fileState;
	U64		_filePos;

	int		_bufferSize;
	Buffer	_buffers[kMaxBuffers];
	int		_currentBuffer;
	int		_pendingOps;

private:
	U32		_wtfVersion;
	U32		_maxBuffers;
	bool	_wtfCritical;
	bool	_wtfEndianSwap;
};

#endif // BIZ_SYSTEM_IO_ASYNC_H