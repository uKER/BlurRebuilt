#include "SystemPch.h"
#include <system/debuglog.h>

#include "IoImpl.h"

#include <system/serialise/FileSystemGlobals.h>
#include <system/serialise/FsPerfLogger.h>

extern Debug::Logger &g_SystemIoLog;

namespace { int MyMax(int a, int b) { return a > b ? a : b; } }
namespace { void DoCriticalError(bool isCritical, bool writing, const char* name) { FSERRORSPAM(g_SystemIoLog, "AsyncFile::DoCriticalError %s", name); if (isCritical) Sys::FileRwError(name, writing, isCritical, NULL); } }

AsyncFile::AsyncFile() :
	_fileState(kEmpty),
	_filePos(0),
	_bufferSize(0),
	_currentBuffer(-1),
	_pendingOps(0),
	_wtfCritical(false),
	_wtfVersion(0),
	_wtfEndianSwap(false),
	_maxBuffers(IODEFAULT_NUM_BUFFERS)
{
	for(int i = 0; i < kMaxBuffers; i++)
		_buffers[i].file = this;
}

AsyncFile::~AsyncFile()
{
	FSINFOSPAM(g_SystemIoLog, "AsyncFile::Closing %s", Name().Chars());

	if (_fileState != kEmpty)
	{
		_mutex.Wait();

		while(_pendingOps > 0)
			_condVar.Wait();

		_mutex.Release();
	}

	IoFileClose(_file, 0);

	_condVar.Destroy();
	_mutex.Destroy();

	for(int i = 0; i < _maxBuffers; i++)
	{
		delete [] _buffers[i].data;
		_buffers[i].data = 0;
	}
}

bool AsyncFile::Open(const char* filename, int bufferSize, bool isCritical, U32 maxBuffers)
{
	FSINFOSPAM(g_SystemIoLog, "AsyncFile::Open, %s, isCritical=%s bufferSize=%i", 
		filename, isCritical ? "true" : "false", bufferSize);

	_wtfCritical = isCritical;
	_maxBuffers = maxBuffers == 0 ? IODEFAULT_NUM_BUFFERS : maxBuffers;
	if (_maxBuffers > kMaxBuffers)
		_maxBuffers = kMaxBuffers;

	if (_fileState != kEmpty)
	{
		FSERROR(g_SystemIoLog, "AsyncFile::Open failure case 1 %s", filename);
		DoCriticalError(_wtfCritical, IsWriting(), filename);
		return false;
	}

	if (bufferSize > 0)
		_bufferSize = (((MyMax(bufferSize, 2048) - 1) / 2048) + 1) * 2048;
	else
		_bufferSize = 0;

	if (!_mutex.Create() || !_condVar.Create(&_mutex))
	{
		_mutex.Destroy();

		FSERROR(g_SystemIoLog, "AsyncFile::Open failure case 2 %s", filename);
		DoCriticalError(_wtfCritical, IsWriting(), filename);
		return false;
	}

	if (_bufferSize > 0)
	{
		for(int i = 0; i < _maxBuffers; i++)
		{
			if ((_buffers[i].data = new U8[_bufferSize]) == 0)
			{
				_condVar.Destroy();
				_mutex.Destroy();

				for(int j = 0; j < i; j++)
				{
					delete [] _buffers[j].data;
					_buffers[j].data = 0;
				}

				FSERROR(g_SystemIoLog, "AsyncFile::Open failure case 3 %s", filename);
				DoCriticalError(_wtfCritical, IsWriting(), filename);
				return false;
			}
		}
	}

	_mutex.Wait();

	_fileState = kPending;
	_pendingOps++;

	IoRequest req = IoFileOpen(_file, filename, this);

	while(_fileState == kPending)
		_condVar.Wait();

	_mutex.Release();

	if (_fileState != kComplete)
	{
		if ((req.Error() != kIoError_PathNotFound) &&
			(req.Error() != kIoError_FileNotFound) &&
			(req.Error() != kIoError_AccessDenied))
		{
			FSERROR(g_SystemIoLog, "AsyncFile::Open failure case 4, %s, error %i", filename, (int)req.Error());
			DoCriticalError(_wtfCritical, IsWriting(), filename);
		}
	}

	return _fileState == kComplete;
}

const bool AsyncFile::IsWriting() const
{
	return false;
}

Btl::String AsyncFile::Name() const
{
	return _file.Filename();
}

U32 AsyncFile::GetVersionNumber() const
{
	return _wtfVersion;
}

void AsyncFile::SetVersionNumber(U32 iVersion)
{
	_wtfVersion = iVersion;
}

bool AsyncFile::Underflow(UnderflowContext& ctx)
{
	_mutex.Wait();

	_currentBuffer	= -1;
	m_pCurBuffer	= 0;
	m_pBufferEnd	= 0;

	bool result = false;
	bool readIssued = false;

	bool readPastEof = false;
	if (_filePos + ctx.readSize >= _file.Size())
	{
		readPastEof = true;
		//If at end of file already.
		if (_filePos >= _file.Size())
			goto finished;
		//Otherwise a partial read will be performed.
	}

	if (_bufferSize <= 0)
	{
		// No buffer requested, lets do unbuffered io, caller is responsible for making sure data is correctly aligned in memory and on disk

		if ((ctx.readSize > 0) && (ctx.readTarget != 0))
		{
			Buffer* buf = &_buffers[0];

			buf->state	= kPending;
			buf->size	= 0;
			buf->readType = kIoReadType_Unbuffered;

			_pendingOps++;

			IoFileRead(_file, ctx.readTarget, _filePos, ctx.readSize, kIoPriority_Medium, buf);
			
			while(buf->state == kPending)
				_condVar.Wait();

			if (buf->state == kComplete)
			{
				_filePos += ctx.readSize;
				ctx.bytesRead	= buf->size;
				result			= true;
			}
		}
		//Make ourselves consistent with buffered reading, c_file and Windows API behaviour here.
		else if ((ctx.readSize == 0) && (ctx.readTarget != 0))
		{
			Buffer* buf		= &_buffers[0];
			buf->state		= kComplete;
			ctx.bytesRead	= 0;
			result			= true;
		}

		goto finished;
	}

	while(_fileState == kComplete)
	{
		State order[] = { kEmpty, kFailed, kComplete };

		for(int i = 0; i < _maxBuffers; i++)
		{
			Buffer* buf = &_buffers[i];

			if (buf->state == kPending)
			{
				S64 delta = _filePos - buf->offset;

				if ((delta >= 0) && (delta < _bufferSize))
					goto wait;
			}
			else if (buf->state == kComplete)
			{
				S64 delta = _filePos - buf->offset;

				if ((delta >= 0) && (delta < buf->size))
				{
					m_pCurBuffer	= buf->data + delta;
					m_pBufferEnd	= buf->data + buf->size;
					_filePos		= buf->offset + buf->size;
					_currentBuffer	= i;
					result			= true;

					goto done;
				}
			}
			else if (buf->state == kFailed)
			{
				S64 delta = _filePos - buf->offset;

				if ((delta >= 0) && (delta < _bufferSize))
					goto done;
			}
		}

		for(int j = 0; j < lengthof(order); j++)
		{
			for(int i = 0; i < _maxBuffers; i++)
			{
				Buffer* buf = &_buffers[i];

				if (buf->state == order[j])
				{
					// TODO: Align offset to cluster/sector size

					buf->state	= kPending;
					buf->offset	= _filePos & ~2047;
					buf->size	= 0;
					//The first buffer we fill is to satisfy our immediate demand, any subsequent buffer filling
					//is classed as speculative.
					buf->readType = readIssued ? kIoReadType_Speculative : kIoReadType_Buffered;

					_pendingOps++;

					IoFileRead(_file, buf->data, buf->offset, _bufferSize, kIoPriority_Medium, buf);
					readIssued = true;
					goto repeat;
				}
			}
		}

		for(int i = 0; i < _maxBuffers; i++)
		{
			Buffer* buf = &_buffers[i];

			if (buf->state == kPending)
				goto wait;
		}

		break;

wait:
		_condVar.Wait();

repeat:;
	}

done:
	if ((_currentBuffer >= 0) && (_filePos < _file.Size()) && (_maxBuffers > 1))
	{
		// See if speculative read is already scheduled

		for(int i = 0; i < _maxBuffers; i++)
		{
			Buffer* buf = &_buffers[i];

			if (buf->state != kEmpty)
			{
				S64 delta = _filePos - buf->offset;

				if ((delta >= 0) && (delta < _bufferSize))
					goto finished;
			}
		}

		// Kick off speculative read

		State order[] = { kEmpty, kFailed, kComplete };

		for(int j = 0; j < lengthof(order); j++)
		{
			for(int i = 0; i < _maxBuffers; i++)
			{
				if (i == _currentBuffer)
					continue;

				Buffer* buf = &_buffers[i];

				if (buf->state == order[j])
				{
					// TODO: Align offset to cluster/sector size

					buf->state	= kPending;
					buf->offset	= _filePos & ~2047;
					buf->size	= 0;
					buf->readType = kIoReadType_Speculative;

					_pendingOps++;

					IoFileRead(_file, buf->data, buf->offset, _bufferSize, kIoPriority_Medium, buf);
					readIssued = true;
					goto finished;
				}
			}
		}
	}

finished:
	_mutex.Release();

	if (!result)
	{
		if (!readPastEof)
		{
			FSERROR(g_SystemIoLog, "AsyncFile::Underflow failure %s readSize=%i, bytesRead=%i ", Name().Chars(), ctx.readSize, ctx.bytesRead);
			DoCriticalError(_wtfCritical, IsWriting(), Name().Chars());
		}
		else
		{
			//Sadly buffered io relies on reading past file end.
			if (_bufferSize <= 0)
			{
				FSASSERT(g_SystemIoLog, "AsyncFile::Underflow EOF failure %s readSize=%i, bytesRead=%i ", Name().Chars(), ctx.readSize, ctx.bytesRead);
			}
			//drop out...
		}
	}
	return result;
}

bool AsyncFile::Overflow()
{
	return false;
}

const Sys::C_StreamSize AsyncFile::Length()
{
	return _file.Size();
}

const Sys::C_StreamPos AsyncFile::GetPosition() const
{
	if (_currentBuffer < 0)
		return _filePos;

	const Buffer* buf = &_buffers[_currentBuffer];

	return buf->offset + (U64)(m_pCurBuffer - buf->data);
}

void AsyncFile::SetPosition(const Sys::C_StreamPos newPosition)
{
	_mutex.Wait();

	_filePos		= newPosition;
	_currentBuffer	= -1;
	m_pCurBuffer	= 0;
	m_pBufferEnd	= 0;

	_mutex.Release();
}

const bool AsyncFile::GetEndianSwapFlag() const
{
	return _wtfEndianSwap;
}

void AsyncFile::SetEndianSwapFlag(const bool swizzle)
{
	_wtfEndianSwap = swizzle;
}

void AsyncFile::Flush()
{
}

// Open callback

void AsyncFile::Event(IoEvent event, const IoRequest& request)
{
	_mutex.Wait();

	_fileState = (event == kIoEvent_Complete) ? kComplete : kFailed;

	_condVar.Broadcast();
	_mutex.Release();
}

void AsyncFile::Release()
{
	_mutex.Wait();

	_pendingOps--;

	_condVar.Broadcast();
	_mutex.Release();
}

// Read callback

void AsyncFile::Buffer::Event(IoEvent event, const IoRequest& request)
{
	file->_mutex.Wait();

	state	= (event == kIoEvent_Complete) ? kComplete : kFailed;
	size	= request.ActualSize();

	FSPERF_READ_PHYSICAL("IoDevice", request.File().Handle(), file->GetPosition(), size, g_ioReadTypeStr[readType]);

	file->_condVar.Broadcast();
	file->_mutex.Release();
}

void AsyncFile::Buffer::Release()
{
	file->_mutex.Wait();

	file->_pendingOps--;

	file->_condVar.Broadcast();
	file->_mutex.Release();
}