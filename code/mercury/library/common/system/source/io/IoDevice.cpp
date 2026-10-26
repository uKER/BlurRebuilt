#include "SystemPch.h"
#include "IoImpl.h"
#include <core/threads/atomic.h>
#include <system/serialise/FsPerfLogger.h>
#include <system/serialise/FileSystemGlobals.h>

extern Debug::Logger &g_SystemIoLog;
bool IoDebugShouldReadFail();

//
// IoDevice
//

IoDevice::IoDevice(const char* name) :
	_name(name)
#if defined(PLATFORM_PC)
	,_shutdown(false)
#elif defined(PLATFORM_PS3)
	,_scheduler(0)
#endif
{
}

IoDevice::~IoDevice()
{
}

bool IoDevice::Initialise()
{
#if defined(PLATFORM_PC)
	return _wake.Create() && _thread.Create(ThreadProc, this, 0);
#elif defined(PLATFORM_PS3)
	return true;
#endif
}

void IoDevice::Shutdown()
{
#if defined(PLATFORM_PC)
	_lock.Wait();
	_shutdown = true;
	_lock.Release();
	_wake.Raise();

	while(IoRequestImpl* impl = XtUt::PopFront(&_pending))
	{
		impl->DoUserCallback(kIoEvent_Failed);
		impl->self = IoRequest();
	}

	_thread.Close();
	_wake.Destroy();
#endif
}

#if defined(PLATFORM_PC)
void IoDevice::Queue(IoRequestImpl* req)
{
	BIZ_ASSERT(req != 0);
	BIZ_ASSERT(req->file._impl != 0);
	BIZ_ASSERT(req->file._impl->device == this);

	req->creation = GetTickCount();

	_lock.Wait();

	XtUt::PushBack(&_pending, &req->link);

	_lock.Release();
	_wake.Raise();
}

void IoDevice::Process()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);


	while(!_shutdown)
	{
		for(;;)
		{
			IoRequestImpl*	active			= 0;
			DWORD			activeDeadline	= 0;
			IoPriority		activePriority	= kIoPriority_Low;
			DWORD			time			= GetTickCount();

			_lock.Wait();

			if (_shutdown)
			{
				_lock.Release();
				return;
			}

			for(XtUt::LinkList<IoRequestImpl>::Iterator it(&_pending); it; ++it)
			{
				IoFileImpl*		fi			= it->file._impl;
				DWORD			deadline	= it->Deadline();
				DWORD			duration	= it->requestSize / ((6 * 1024 * 1024) / 1000); // TODO: Accurate duration estimate

				deadline -= duration;

				bool accept = false;

				if (!active)
					accept = true;
				else if (deadline < time)
				{
					if (it->priority < activePriority)
						accept = true;
					else if (deadline < activeDeadline)
						accept = true;
				}
				else
				{
					if (deadline < activeDeadline)
						accept = true;
					else if (it->priority < activePriority)
						accept = true;
				}

				if (accept)
				{
					if ((it->action != kIoAction_Open) && (fi->state == IoFileImpl::kState_Opening))
						accept = false;
				}

				// TODO: Proper scheduling algorithm.
				//
				// Correct solution requires exhaustive search so a no go unless dealing with <5 requests
				// Possibly sort by file and offset, sort higher priority and out of deadline items infront of queue?
				//

				if (accept)
				{
					active			= it;
					activeDeadline	= deadline;
					activePriority	= it->priority;
				}
			}

			if (active)
				XtUt::Detach(&active->link);

			_lock.Release();

			if (!active)
				break;

			IoFileImpl* fi = active->file._impl;

			switch(active->action)
			{
				case kIoAction_Open:
					BIZ_ASSERT(fi->state == IoFileImpl::kState_Opening);
					BIZ_ASSERT(fi->handle == INVALID_HANDLE_VALUE);

					if ((fi->handle = CreateFile(
							fi->filename.Chars(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, 0
						)) != INVALID_HANDLE_VALUE)
					{
						fi->state		= IoFileImpl::kState_Open;

						fi->diskOffset	= 0;

						DWORD fileHigh	= 0;
						DWORD fileLow	= GetFileSize(fi->handle, &fileHigh);

						fi->fileSize	= (((U64)fileHigh) << 32) | (U64)fileLow;
						FSPERF_OPENED("IoDevice", fi->filename.Chars(), active->file.Handle(), Sys::E_FileAccess_Read);
						active->DoUserCallback(kIoEvent_Complete);
					}
					else
					{
						fi->state = IoFileImpl::kState_Closed;

						DWORD osErr = GetLastError();

						switch(osErr)
						{
							case ERROR_PATH_NOT_FOUND:
							case ERROR_INVALID_DRIVE:
								FSERRORSPAM(g_SystemIoLog, "IoDevice::kIoError_PathNotFound, %s, osErr %i", fi->filename.Chars(), osErr);
								active->error = kIoError_PathNotFound;
							break;

							case ERROR_INVALID_NAME:
							case ERROR_FILE_NOT_FOUND:
								FSERRORSPAM(g_SystemIoLog, "IoDevice::kIoError_FileNotFound, %s, osErr %i", fi->filename.Chars(), osErr);
								active->error = kIoError_FileNotFound;
							break;

							case ERROR_ACCESS_DENIED:
								FSERRORSPAM(g_SystemIoLog, "IoDevice::kIoError_AccessDenied, %s, osErr %i", fi->filename.Chars(), osErr);
								active->error = kIoError_AccessDenied;
							break;

							case ERROR_IO_DEVICE:
							case ERROR_NO_MEDIA_IN_DRIVE:
							case ERROR_MEDIA_CHANGED:
								FSERRORSPAM(g_SystemIoLog, "IoDevice::kIoError_MediaRemoved, %s, osErr %i", fi->filename.Chars(), osErr);
								active->error = kIoError_MediaRemoved;
							break;

							default:
								FSERROR(g_SystemIoLog, "IoDevice::kIoError_Unknown, %s, osErr %i", fi->filename.Chars(), osErr);
								active->error = kIoError_Unknown;
							break;
						}

						active->DoUserCallback(kIoEvent_Failed);
					}
				break;

				case kIoAction_Read:
					if (fi->handle != INVALID_HANDLE_VALUE)
					{
						if (active->buffer && (active->requestSize > 0))
						{
							OVERLAPPED ov = { 0 };

							ov.Offset		= (U32)active->offset;
							ov.OffsetHigh	= (U32)(active->offset >> 32);

							DWORD read = 0;

							if (!IoDebugShouldReadFail() && ReadFile(fi->handle, active->buffer, active->requestSize, &read, &ov))
							{
								active->actualSize = read;

								active->DoUserCallback(kIoEvent_Complete);
							}
							else
							{
								active->error = kIoError_Unknown;

								active->DoUserCallback(kIoEvent_Failed);
							}
						}
						else
						{
							active->error = kIoError_InvalidParameters;

							active->DoUserCallback(kIoEvent_Failed);
						}
					}
					else
					{
						active->error = kIoError_FileNotYetOpen;

						active->DoUserCallback(kIoEvent_Failed);
					}
				break;

				case kIoAction_Close:
					if (fi->handle != INVALID_HANDLE_VALUE)
						CloseHandle(fi->handle);

					if (fi->state != IoFileImpl::kState_Closed)
					{
						FSPERF_CLOSED("IoDevice", active->file.Handle());
					}
					fi->state		= IoFileImpl::kState_Closed;
					fi->handle		= INVALID_HANDLE_VALUE;
					fi->fileSize	= 0;

					active->DoUserCallback(kIoEvent_Complete);

					fi->self = IoFile();
				break;
			}

			active->DoUserRelease();

			active->self = IoRequest();
		}

		_wake.Wait();
	}
}

void IoDevice::ThreadProc(void* context)
{
	((IoDevice*)context)->Process();
}
#endif
