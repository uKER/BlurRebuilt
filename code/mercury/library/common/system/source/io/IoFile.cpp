#include "SystemPch.h"
#include "IoImpl.h"

//
// IoFile
//

IoFile::IoFile()					: _impl(0)				{ }
IoFile::IoFile(const IoFile& req)	: _impl(req._impl)		{ IoFileImpl::Acquire(_impl); }
IoFile::IoFile(IoFileImpl* impl)	: _impl(impl)			{ IoFileImpl::Acquire(_impl); }

IoFile::~IoFile()
{
	IoFileImpl::Release(_impl);
}

void IoFile::operator=(const IoFile& file)
{
	IoFileImpl* toRelease = _impl;
	IoFileImpl::Acquire(file._impl);
	_impl = file._impl;
	IoFileImpl::Release(toRelease);
}

const Btl::String& IoFile::Filename() const
{
	static Btl::String empty;

	return _impl ? _impl->filename : empty;
}

U64 IoFile::Size() const
{
	return _impl ? _impl->fileSize : 0;
}

//
// IoFileImpl
//

IoFileImpl::IoFileImpl() :
	references(0),
	state(kState_Opening),
#if defined(PLATFORM_PC)
	handle(INVALID_HANDLE_VALUE),
	diskOffset(0),
#elif defined(PLATFORM_PS3)
	handle(0),
#endif
	fileSize(0),
	device(0)
{
	self = IoFile(this);
}

IoFileImpl::~IoFileImpl()
{
}

void IoFileImpl::Acquire(IoFileImpl* impl)
{
	if (impl)
	{
#if defined(PLATFORM_PC)
		InterlockedIncrement((LONG*)&impl->references);
#elif defined(PLATFORM_PS3)
		cellAtomicIncr32((U32*)&impl->references);
#endif
	}
}

void IoFileImpl::Release(IoFileImpl* impl)
{
	if (impl)
	{
#if defined(PLATFORM_PC)
		if (InterlockedDecrement((LONG*)&impl->references) <= 0)
#elif defined(PLATFORM_PS3)
		if (cellAtomicDecr32((U32*)&impl->references) <= 1)
#endif
		{
			delete impl;
		}
	}
}