#ifndef XT_TOOL_XTSIMPLEFILE_H
#define XT_TOOL_XTSIMPLEFILE_H

#ifdef _MSC_VER
#pragma once
#endif

// XtSimpleFile

class XtSimpleFile
{
public:
	XtSimpleFile();
	~XtSimpleFile();

public:
	operator bool() const;

public:
	bool Open(const char* fileName, bool forWriting = false);
	void Close();

public:
	bool Read(void* data, int length);
	bool Write(const void* data, int length);
	bool Seek(int offset);
	bool Flush();

public:
	size_t Length();
	size_t Position();
	HANDLE Handle();
	XtFileTime CreationTime();
	XtFileTime LastAccessTime();
	XtFileTime LastWriteTime();

private:
	HANDLE _handle;
};

// Serialisation operators

template<typename Type>
XtSimpleFile& operator>>(XtSimpleFile& file, Type& value)
{
	file.Read(&value, sizeof(value));

	return file;
}

template<typename Type>
XtSimpleFile& operator<<(XtSimpleFile& file, const Type& value)
{
	file.Write(&value, sizeof(value));

	return file;
}

#endif // XT_TOOL_XTSIMPLEFILE_H