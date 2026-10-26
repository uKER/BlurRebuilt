#include "SystemPch.h"
#include "IoImpl.h"

IoRequestImpl gIoRequest_Null(kIoError_NullRequest);
IoRequestImpl gIoRequest_InvalidParameters(kIoError_InvalidParameters);
IoRequestImpl gIoRequest_NoDeviceMapping(kIoError_NoDeviceMapping);
IoRequestImpl gIoRequest_OutOfMemory(kIoError_OutOfMemory);
IoRequestImpl gIoRequest_FileNotYetOpen(kIoError_FileNotYetOpen);

//
// IoCallback
//

IoCallback::IoCallback()
{
}

IoCallback::~IoCallback()
{
}

//
// IoRequest
//

IoRequest::IoRequest()						: _impl(&gIoRequest_Null)	{ _impl->Acquire(); }
IoRequest::IoRequest(const IoRequest& req)	: _impl(req._impl)			{ _impl->Acquire(); }
IoRequest::IoRequest(IoRequestImpl* impl)	: _impl(impl)				{ _impl->Acquire(); }

IoRequest::~IoRequest()
{
	_impl->Release();
}

void IoRequest::operator=(const IoRequest& req)
{
	IoRequestImpl* toRelease = _impl;
	req._impl->Acquire();
	_impl = req._impl;
	toRelease->Release();
}

void IoRequest::SetPriority(IoPriority priority)
{
	_impl->priority = priority;
}

void IoRequest::Cancel()
{
	// TODO
}

void IoRequest::Wait()
{
	// TODO
}

IoAction		IoRequest::Action() const		{ return _impl->action;			}
const IoFile&	IoRequest::File() const			{ return _impl->file;			}
void*			IoRequest::Buffer() const		{ return _impl->buffer;			}
U64				IoRequest::Offset() const		{ return _impl->offset;			}
U32				IoRequest::RequestSize() const	{ return _impl->requestSize;	}
U32				IoRequest::ActualSize() const	{ return _impl->actualSize;		}
IoPriority		IoRequest::Priority() const		{ return _impl->priority;		}
IoError			IoRequest::Error() const		{ return _impl->error;			}

//
// IoRequestImpl
//

IoRequestImpl::IoRequestImpl(IoAction action_, const IoFile& file_, IoCallback* callback_) :
	references(0),
	action(action_),
	file(file_),
	buffer(0),
	offset(0),
	requestSize(0),
	actualSize(0),
	priority(kIoPriority_Medium),
	callback(callback_),
	creation(0),
	error(kIoError_Okay),
#ifdef PLATFORM_PS3
	op(0),
#endif
	link(this)
{
	self = IoRequest(this);
}

IoRequestImpl::IoRequestImpl(IoError error_) :
	references(1),
	action(kIoAction_None),
	buffer(0),
	offset(0),
	requestSize(0),
	actualSize(0),
	priority(kIoPriority_Medium),
	callback(0),
	creation(0),
	error(error_),
#ifdef PLATFORM_PS3
	op(0),
#endif
	link(this)
{
	self = IoRequest(this);
}

IoRequestImpl::~IoRequestImpl()
{
	DoUserRelease();

	XtUt::Detach(&link);
}

void IoRequestImpl::Acquire()
{
#if defined(PLATFORM_PC)
	InterlockedIncrement((LONG*)&references);
#else
	cellAtomicIncr32((U32*)&references);
#endif
}

void IoRequestImpl::Release()
{
#if defined(PLATFORM_PC)
	if (InterlockedDecrement((LONG*)&references) <= 0)
#else
	if (cellAtomicDecr32((U32*)&references) <= 1)
#endif
	{
		delete this;
	}
}

void IoRequestImpl::DoUserCallback(IoEvent event)
{
	if (callback)
		callback->Event(event, IoRequest(this));
}

void IoRequestImpl::DoUserRelease()
{
	if (callback)
	{
		callback->Release();
		callback = 0;
	}
}
