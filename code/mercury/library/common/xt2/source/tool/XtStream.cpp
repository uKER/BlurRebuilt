#include "Common/XtPch.h"
#include <Xt/XtTool.h>

// ReadAllData

bool XtUt::ReadAllData(StorageReader* reader, XtUt::Block* mb)
{
	size_t length = reader->Length();

	if (!Allocate(mb, length))
		return false;

	return reader->Read(mb->start, length);
}

// FileStorageReader

XtUt::FileStorageReader::FileStorageReader(XtSimpleFile* file) :
	_file(file)
{
}

bool XtUt::FileStorageReader::Read(void* data, size_t length)
{
	return _file->Read(data, length);
}

bool XtUt::FileStorageReader::Seek(size_t position)
{
	return _file->Seek(position);
}

size_t XtUt::FileStorageReader::Position()
{
	return _file->Position();
}

size_t XtUt::FileStorageReader::Length()
{
	return _file->Length();
}

// FileStorageWriter

XtUt::FileStorageWriter::FileStorageWriter(XtSimpleFile* file) :
	_file(file)
{
}

bool XtUt::FileStorageWriter::Write(const void* data, size_t length)
{
	return _file->Write(data, length);
}

bool XtUt::FileStorageWriter::Seek(size_t position)
{
	return _file->Seek(position);
}

size_t XtUt::FileStorageWriter::Position()
{
	return _file->Position();
}

size_t XtUt::FileStorageWriter::Length()
{
	return _file->Length();
}

bool XtUt::FileStorageWriter::Flush()
{
	return _file->Flush();
}

// MemoryStorageReader

XtUt::MemoryStorageReader::MemoryStorageReader(XtUt::Block* mb) :
	_mb(mb)
{
}

bool XtUt::MemoryStorageReader::Read(void* data, size_t length)
{
	return XtUt::Read(_mb, data, length) == length;
}

bool XtUt::MemoryStorageReader::Seek(size_t position)
{
	if (position > Capacity(_mb))
	{
		_mb->cursor = _mb->end;

		return false;
	}

	_mb->cursor = _mb->start + position;

	return true;
}

size_t XtUt::MemoryStorageReader::Position()
{
	return Used(_mb);
}

size_t XtUt::MemoryStorageReader::Length()
{
	return Capacity(_mb);
}

// MemoryStorageWriter

XtUt::MemoryStorageWriter::MemoryStorageWriter(XtUt::Block* mb) :
	_mb(mb),
	_position(Used(mb))
{
}

bool XtUt::MemoryStorageWriter::Write(const void* data, size_t length)
{
	if ((_position + length) > Capacity(_mb))
	{
		size_t newCapacity =_position + length;

		XtUt::Block mb;

		if (!Allocate(&mb, newCapacity))
			return false;

		if (mb.start)
			memcpy(mb.start, _mb->start, Used(_mb));

		ReplaceData(_mb, &mb);
	}

	if (XtUt::Write(_mb, _position, data, length) != length)
		return false;

	_position += length;

	if (Used(_mb) < _position)
		_mb->cursor = _mb->start + _position;

	return true;
}

bool XtUt::MemoryStorageWriter::Seek(size_t position)
{
	_position = position;

	return true;
}

size_t XtUt::MemoryStorageWriter::Position()
{
	return _position;
}

size_t XtUt::MemoryStorageWriter::Length()
{
	return Used(_mb);
}

bool XtUt::MemoryStorageWriter::Flush()
{
	return true;
}

#ifdef PLATFORM_PC
// MappedFileStorageReader

XtUt::MappedFileStorageReader::MappedFileStorageReader(XtSimpleFile* file) :
	_file(file),
	_mm(0),
	_view(0),
	_ptr(0),
	_size(0)
{
	if (!Init())
		ShutDown();
}

XtUt::MappedFileStorageReader::~MappedFileStorageReader()
{
	ShutDown();
}

bool XtUt::MappedFileStorageReader::Read(void* data, size_t length)
{
	if (!_view)
		return false;

	if (_ptr + length > _size)
		return false;

	memcpy(data, _view + _ptr, length);

	_ptr += length;

	return true;
}

bool XtUt::MappedFileStorageReader::Seek(size_t position)
{
	if (!_view || (position > _size))
		return false;

	_ptr = position;

	return true;
}

size_t XtUt::MappedFileStorageReader::Position()
{
	return _ptr;
}

size_t XtUt::MappedFileStorageReader::Length()
{
	return _size;
}

bool XtUt::MappedFileStorageReader::IsMapped()
{
	return _view != 0;
}

XtUt::WeakBlock XtUt::MappedFileStorageReader::GetBlock()
{
	if (!_view)
		return XtUt::WeakBlock();

	return XtUt::WeakBlock(_view, _size);
}

bool XtUt::MappedFileStorageReader::Init()
{
	if (!_file || !*_file)
		return false;

	if ((_mm = CreateFileMapping(_file->Handle(), 0, PAGE_READONLY, 0, 0, 0)) == 0)
		return false;

	if ((_view = (XtU8*)MapViewOfFile(_mm, FILE_MAP_READ, 0, 0, 0)) == 0)
	{
		return false;
	}

	_size = _file->Length();

	return true;
}

void XtUt::MappedFileStorageReader::ShutDown()
{
	if (_view)
	{
		UnmapViewOfFile(_view);
		_view = 0;
	}

	if (_mm)
	{
		CloseHandle(_mm);
		_mm = 0;
	}

	_ptr	= 0;
	_size	= 0;
}
#endif

// UnbufferedFileReader

XtUt::UnbufferedBlockReader::UnbufferedBlockReader(StorageReader* reader) :
	_reader(reader)
{
}

XtUt::Block* XtUt::UnbufferedBlockReader::Read(void* data, size_t length)
{
	return _reader->Read(data, length) ? &_mb : 0;
}

// UnbufferedBlockWriter

XtUt::UnbufferedBlockWriter::UnbufferedBlockWriter(StorageWriter* writer) :
	_writer(writer)
{
}

XtUt::Block* XtUt::UnbufferedBlockWriter::Write(const void* data, size_t length)
{
	return _writer->Write(data, length) ? &_mb : 0;
}

bool XtUt::UnbufferedBlockWriter::Flush(bool toStorage)
{
	return toStorage ? _writer->Flush() : true;
}

// WholeBlockReader

XtUt::WholeBlockReader::WholeBlockReader(StorageReader* reader) :
	_reader(reader)
{
}

XtUt::WholeBlockReader::~WholeBlockReader()
{
}

XtUt::Block* XtUt::WholeBlockReader::Read(void* data, size_t length)
{
	for(;;)
	{
		size_t advance = XtUt::Read(&_mb, data, length);

		(XtU8*&)data	+= advance;
		length			-= advance;

		if (length == 0)
			return &_mb;

		size_t available = _reader->Length() - _reader->Position();

		if ((available <= 0) || (length > available))
			return 0;

		if (length == available)
			return _reader->Read(data, length) ? &_mb : 0;

		if (!Allocate(&_mb, available))
			return 0;

		if (!_reader->Read(_mb.start, available))
			return 0;
	}
}

// WholeBlockWriter

XtUt::WholeBlockWriter::WholeBlockWriter(StorageWriter* writer, size_t capacityIncrease) :
	_writer(writer),
	_capacityIncrease(capacityIncrease)
{
	if (capacityIncrease <= 0)
		capacityIncrease = 16384;
}

XtUt::WholeBlockWriter::~WholeBlockWriter()
{
	if (Used(&_mb) > 0)
		_writer->Write(_mb.start, Used(&_mb));
}

XtUt::Block* XtUt::WholeBlockWriter::Write(const void* data, size_t length)
{
	size_t usedCapacity		= Used(&_mb);
	size_t totalCapacity	= Capacity(&_mb);
	size_t requiredCapacity	= usedCapacity + length;

	if (requiredCapacity > totalCapacity)
	{
		requiredCapacity = AlignUpper(Max(requiredCapacity, totalCapacity + _capacityIncrease), (size_t)128);

		XtUt::Block mb;

		if (!Allocate(&mb, requiredCapacity))
			return 0;

		if (mb.start)
			memcpy(mb.start, _mb.start, Used(&_mb));

		ReplaceData(&_mb, &mb);
	}

	XtUt::Write(&_mb, data, length);

	return &_mb;
}

bool XtUt::WholeBlockWriter::Flush(bool toStorage)
{
	if ((Used(&_mb) > 0) && !_writer->Write(_mb.start, Used(&_mb)))
		return false;

	Free(&_mb);

	return toStorage ? _writer->Flush() : true;
}

// ChunkedBlockReader

XtUt::ChunkedBlockReader::ChunkedBlockReader(StorageReader* reader, size_t chunkSize) :
	_reader(reader),
	_chunkSize(chunkSize)
{
}

XtUt::ChunkedBlockReader::~ChunkedBlockReader()
{
}

XtUt::Block* XtUt::ChunkedBlockReader::Read(void* data, size_t length)
{
	for(;;)
	{
		size_t advance = XtUt::Read(&_mb, data, length);

		(XtU8*&)data	+= advance;
		length			-= advance;

		if (length == 0)
			return &_mb;

		size_t available = _reader->Length() - _reader->Position();

		if ((available <= 0) || (length > available))
			return 0;

		if ((length == available) || (length > (_chunkSize / 2)))
			return _reader->Read(data, length) ? &_mb : 0;

		if (!Allocate(&_mb, _chunkSize))
			return 0;

		available = XtUt::Min(available, _chunkSize);

		_mb.cursor = _mb.end - available;

		if (!_reader->Read(_mb.cursor, available))
			return 0;
	}
}

// ChunkedBlockReader

XtUt::ChunkedBlockWriter::ChunkedBlockWriter(StorageWriter* writer, size_t chunkSize) :
	_writer(writer),
	_chunkSize(chunkSize)
{
}

XtUt::ChunkedBlockWriter::~ChunkedBlockWriter()
{
	if (Used(&_mb) > 0)
		_writer->Write(_mb.start, Used(&_mb));
}

XtUt::Block* XtUt::ChunkedBlockWriter::Write(const void* data, size_t length)
{
	for(;;)
	{
		size_t advance = XtUt::Write(&_mb, data, length);

		(XtU8*&)data	+= advance;
		length			-= advance;

		if (length == 0)
			return &_mb;

		if ((Used(&_mb) > 0) && !_writer->Write(_mb.start, Used(&_mb)))
			return 0;

		if (!Allocate(&_mb, _chunkSize))
			return 0;

		_mb.cursor = _mb.start;

		if (length > (_chunkSize / 2))
			return _writer->Write(data, length) ? &_mb : 0;
	}
}

bool XtUt::ChunkedBlockWriter::Flush(bool toStorage)
{
	if ((Used(&_mb) > 0) && !_writer->Write(_mb.start, Used(&_mb)))
		return false;

	_mb.cursor = _mb.start;

	return toStorage ? _writer->Flush() : true;
}