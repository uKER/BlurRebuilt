#include "Common/XtPch.h"
#include <Xt/XtTool.h>

//
// XtSimpleFile
//

XtSimpleFile::XtSimpleFile() :
	_handle(INVALID_HANDLE_VALUE)
{
}

XtSimpleFile::~XtSimpleFile()
{
	Close();
}

XtSimpleFile::operator bool() const
{
	return _handle != INVALID_HANDLE_VALUE;
}

bool XtSimpleFile::Open(const char* fileName, bool forWriting)
{
	Close();

	if (!forWriting)
		_handle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	else
		_handle = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

	return _handle != INVALID_HANDLE_VALUE;
}

void XtSimpleFile::Close()
{
	if (_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_handle);
		_handle = INVALID_HANDLE_VALUE;
	}
}

bool XtSimpleFile::Read(void* data, int length)
{
	if (_handle == INVALID_HANDLE_VALUE)
		return false;

	DWORD bytesRead;

	if (!ReadFile(_handle, data, length, &bytesRead, 0))
		return false;

	if ((int)bytesRead != length)
		return false;

	return true;
}

bool XtSimpleFile::Write(const void* data, int length)
{
	if (_handle == INVALID_HANDLE_VALUE)
		return false;

	DWORD bytesWritten;

	if (!WriteFile(_handle, data, length, &bytesWritten, 0))
		return false;

	if ((int)bytesWritten != length)
		return false;

	return true;
}

bool XtSimpleFile::Seek(int offset)
{
	return SetFilePointer(_handle, offset, 0, FILE_BEGIN) != 0xFFFFFFFF;
}

bool XtSimpleFile::Flush()
{
	return FlushFileBuffers(_handle) != FALSE;
}

size_t XtSimpleFile::Length()
{
	return GetFileSize(_handle, 0);
}

size_t XtSimpleFile::Position()
{
	return (size_t)SetFilePointer(_handle, 0, 0, FILE_CURRENT);
}

HANDLE XtSimpleFile::Handle()
{
	return _handle;
}

XtFileTime XtSimpleFile::CreationTime()
{
	XtFileTime ft;

	if (_handle != INVALID_HANDLE_VALUE)
		GetFileTime(_handle, ft, 0, 0);

	return ft;
}

XtFileTime XtSimpleFile::LastAccessTime()
{
	XtFileTime ft;

	if (_handle != INVALID_HANDLE_VALUE)
		GetFileTime(_handle, 0, ft, 0);

	return ft;
}

XtFileTime XtSimpleFile::LastWriteTime()
{
	XtFileTime ft;

	if (_handle != INVALID_HANDLE_VALUE)
		GetFileTime(_handle, 0, 0, ft);

	return ft;
}