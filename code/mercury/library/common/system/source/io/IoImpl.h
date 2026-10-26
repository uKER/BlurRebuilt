#ifndef BIZ_SYSTEM_IO_IOIMPL_H
#define BIZ_SYSTEM_IO_IOIMPL_H

#include <Core/Core.h>
#include <Btl/Utility/String.h>
#include <System/Serialise/I_File.h>
#include <System/Io/ASync.h>

#include <System/Artery/ArterySync.h>
#include <System/Debug/Debug.h>
#include <Xt/Impl/XtCore.h>
#include <Xt/Impl/XtUt.h>

#if defined(PLATFORM_PS3)
#include <cell/fios.h>
#endif

struct IoFileImpl;
struct IoRequestImpl;
class IoDevice;

struct IoFileImpl
{
	enum State
	{
		kState_Opening,
		kState_Open,
		kState_Closing,
		kState_Closed
	};

public:
	IoFileImpl();
	~IoFileImpl();

public:
	static void Acquire(IoFileImpl* impl);
	static void Release(IoFileImpl* impl);

public:
	void Close();

public:
	U32			references;
	IoFile		self;
	State		state;
#if defined(PLATFORM_PC)
	HANDLE		handle;
	U32			diskOffset;
#elif defined(PLATFORM_PS3)
	cell::fios::filehandle* handle;
#endif
	Btl::String	filename;
	U64			fileSize;
	IoDevice*	device;
};

struct IoRequestImpl
{
public:
	U32			references;
	IoRequest	self;
	IoAction	action;
	IoFile		file;
	void*		buffer;
	U64			offset;
	U32			requestSize;
	U32			actualSize;
	IoPriority	priority;
	IoCallback*	callback;
	U32			creation;
	IoError		error;

#ifdef PLATFORM_PS3
	cell::fios::op* op;
#endif

public:
	XtUt::Link<IoRequestImpl> link;

public:
	IoRequestImpl(IoAction action_, const IoFile& file_, IoCallback* callback_);
	IoRequestImpl(IoError error_);
	~IoRequestImpl();

public:
	void Acquire();
	void Release();

public:
	void DoUserCallback(IoEvent event);
	void DoUserRelease();

public:
	U32 Deadline()
	{
		if (priority == kIoPriority_High)
			return creation + 500;

		if (priority == kIoPriority_Medium)
			return creation + 2000;

		return creation + 8000;
	}
};

class IoDevice
{
public:
	IoDevice(const char* name);
	~IoDevice();

public:
	bool Initialise();
	void Shutdown();

#if defined(PLATFORM_PC)
public:
	void Queue(IoRequestImpl* req);

private:
	void Process();

private:
	static void ThreadProc(void* context);

private:
	ArteryThread	_thread;
	ArteryLock		_lock;
	ArteryEvent		_wake;
	bool			_shutdown;

private:
	XtUt::LinkList<IoRequestImpl> _pending;
#elif defined(PLATFORM_PS3)
public:
	cell::fios::scheduler* _scheduler;
#endif

private:
	Btl::String		_name;

};

class IoService
{
public:
	IoService();
	~IoService();

public:
	bool Initialise();
	void Shutdown();

	void CachePrioritise(const char** fileList, int numFiles);

public:
	IoRequest Open(IoFile& file, const char* filename, IoCallback* callback);
	IoRequest Read(const IoFile& file, void* buffer, U64 offset, U32 size, IoPriority pri, IoCallback* callback);
	IoRequest Close(const IoFile& file, IoCallback* callback);

private:
	IoDevice* GetDeviceByPath(const char* path);
	IoDevice* CreateDevice(const char* name);
	bool InitialiseDevices();

private:
	bool					_initialised;
	XtUt::List<IoDevice>	_devices;

private:
	IoDevice* _dvd;
	IoDevice* _hdd;


#ifdef PLATFORM_PS3
private:
	cell::fios::media*		_appHomeMedia;
	cell::fios::media*		_dvdMedia;
	cell::fios::media*		_hddMedia;
	cell::fios::overlay*	_overlay;
	cell::fios::scheduler*	_scheduler;

private:
	static cell::fios::err_t OpCallbackOpen(void* context, cell::fios::op* op, cell::fios::opevent_t event, void* param);
	static cell::fios::err_t OpCallbackRead(void* context, cell::fios::op* op, cell::fios::opevent_t event, void* param);
	static cell::fios::err_t OpCallbackClose(void* context, cell::fios::op* op, cell::fios::opevent_t event, void* param);
#endif
};

extern IoRequestImpl gIoRequest_Null;
extern IoRequestImpl gIoRequest_InvalidParameters;
extern IoRequestImpl gIoRequest_NoDeviceMapping;
extern IoRequestImpl gIoRequest_OutOfMemory;
extern IoRequestImpl gIoRequest_FileNotYetOpen;

#endif // BIZ_SYSTEM_IO_IOIMPL_H