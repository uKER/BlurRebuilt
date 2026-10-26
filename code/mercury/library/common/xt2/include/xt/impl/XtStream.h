#ifndef XT_TOOL_XTSTREAM_H
#define XT_TOOL_XTSTREAM_H

#include "Xt/Impl/XtUt.h"

namespace XtUt
{
	// StorageReader

	class StorageReader
	{
	public:
		virtual ~StorageReader() { }

	public:
		virtual bool	Read(void* data, size_t length) = 0;
		virtual bool	Seek(size_t position) = 0;
		virtual size_t	Position() = 0;
		virtual size_t	Length() = 0;
	};

	// StorageWriter

	class StorageWriter
	{
	public:
		virtual ~StorageWriter() { }

	public:
		virtual bool	Write(const void* data, size_t length) = 0;
		virtual bool	Seek(size_t position) = 0;
		virtual size_t	Position() = 0;
		virtual size_t	Length() = 0;
		virtual bool	Flush() = 0;
	};

	// BlockReader

	class BlockReader
	{
	public:
		virtual ~BlockReader() { }

	public:
		virtual XtUt::Block* Read(void* data, size_t length) = 0;
	};

	// BlockWriter

	class BlockWriter
	{
	public:
		virtual ~BlockWriter() { }

	public:
		virtual XtUt::Block*	Write(const void* data, size_t length) = 0;
		virtual bool			Flush(bool toStorage) = 0;
	};

	// StreamReader

	class StreamReader
	{
	public:
		inline StreamReader(BlockReader* reader) :
			_reader(reader),
			_mb(0)
		{
		}

	public:
		inline bool Read(void* data, size_t length)
		{
			if (!_mb || (length > Available(_mb)))
				return (_mb = _reader->Read(data, length)) != 0;

			memcpy(data, _mb->cursor, length);

			_mb->cursor += length;

			return true;
		}

		template<typename Type> inline bool Read(Type* data)
		{
			if (!_mb || (sizeof(Type) > Available(_mb)))
				return (_mb = _reader->Read(data, sizeof(Type))) != 0;

			*data = *(Type*)_mb->cursor; // TODO: May need unaligned attribute

			_mb->cursor += sizeof(Type);

			return true;
		}

	private:
		BlockReader* _reader;
		XtUt::Block* _mb;
	};

	// StreamWriter

	class StreamWriter
	{
	public:
		inline StreamWriter(BlockWriter* writer) :
			_writer(writer),
			_mb(0)
		{
		}

	public:
		inline bool Write(const void* data, size_t length)
		{
			if (!_mb || (length > Available(_mb)))
				return (_mb = _writer->Write(data, length)) != 0;

			memcpy(_mb->cursor, data, length);

			_mb->cursor += length;

			return true;
		}

		template<typename Type> inline bool Write(const Type* data)
		{
			if (!_mb || (sizeof(Type) > Available(_mb)))
				return (_mb = _writer->Write(data, sizeof(Type))) != 0;

			*(Type*)_mb->cursor = *data; // TODO: May need unaligned attribute

			_mb->cursor += sizeof(Type);

			return true;
		}

	private:
		BlockWriter* _writer;
		XtUt::Block* _mb;
	};

	// Serialisation operators

	template<typename Type>
	inline StreamReader& operator>>(StreamReader& reader, Type& data)
	{
		reader.Read(&data);

		return reader;
	}

	template<typename Type>
	inline StreamWriter& operator<<(StreamWriter& writer, const Type& data)
	{
		writer.Write(&data);

		return writer;
	}

	// ReadAllData

	bool ReadAllData(StorageReader* reader, XtUt::Block* mb);
}

namespace XtUt
{
	// FileStorageReader

	class FileStorageReader : public StorageReader
	{
	public:
		FileStorageReader(XtSimpleFile* file);

	public:
		virtual bool	Read(void* data, size_t length);
		virtual bool	Seek(size_t position);
		virtual size_t	Position();
		virtual size_t	Length();

	private:
		XtSimpleFile* _file;
	};

	// FileStorageWriter

	class FileStorageWriter : public StorageWriter
	{
	public:
		FileStorageWriter(XtSimpleFile* file);

	public:
		virtual bool	Write(const void* data, size_t length);
		virtual bool	Seek(size_t position);
		virtual size_t	Position();
		virtual size_t	Length();
		virtual bool	Flush();

	private:
		XtSimpleFile* _file;
	};

	// MemoryStorageReader

	class MemoryStorageReader : public StorageReader
	{
	public:
		MemoryStorageReader(XtUt::Block* mb);

	public:
		virtual bool	Read(void* data, size_t length);
		virtual bool	Seek(size_t position);
		virtual size_t	Position();
		virtual size_t	Length();

	private:
		XtUt::Block* _mb;
	};

	// MemoryStorageWriter

	class MemoryStorageWriter : public StorageWriter
	{
	public:
		MemoryStorageWriter(XtUt::Block* mb);

	public:
		virtual bool	Write(const void* data, size_t length);
		virtual bool	Seek(size_t position);
		virtual size_t	Position();
		virtual size_t	Length();
		virtual bool	Flush();

	private:
		XtUt::Block*	_mb;
		size_t			_position;
	};

#ifdef PLATFORM_PC
	// MappedFileStorageReader

	class MappedFileStorageReader : public StorageReader
	{
	public:
		MappedFileStorageReader(XtSimpleFile* file);
		virtual ~MappedFileStorageReader();

	public:
		virtual bool	Read(void* data, size_t length);
		virtual bool	Seek(size_t position);
		virtual size_t	Position();
		virtual size_t	Length();

	public:
		bool IsMapped();
		XtUt::WeakBlock GetBlock();

	private:
		bool Init();
		void ShutDown();

	private:
		XtSimpleFile*	_file;
		HANDLE			_mm;
		XtU8*			_view;
		size_t			_ptr;
		size_t			_size;
	};
#endif

	// UnbufferedBlockReader

	class UnbufferedBlockReader : public BlockReader
	{
	public:
		UnbufferedBlockReader(StorageReader* reader);

	public:
		virtual XtUt::Block* Read(void* data, size_t length);

	private:
		StorageReader*	_reader;
		XtUt::Block		_mb;
	};

	// UnbufferedBlockWriter

	class UnbufferedBlockWriter : public BlockWriter
	{
	public:
		UnbufferedBlockWriter(StorageWriter* writer);

	public:
		virtual XtUt::Block*	Write(const void* data, size_t length);
		virtual bool			Flush(bool toStorage);

	private:
		StorageWriter*	_writer;
		XtUt::Block		_mb;
	};

	// WholeBlockReader

	class WholeBlockReader : public BlockReader
	{
	public:
		WholeBlockReader(StorageReader* reader);
		virtual ~WholeBlockReader();

	public:
		virtual XtUt::Block* Read(void* data, size_t length);

	private:
		StorageReader*	_reader;
		XtUt::Block		_mb;
	};

	// WholeBlockWriter

	class WholeBlockWriter : public BlockWriter
	{
	public:
		WholeBlockWriter(StorageWriter* writer, size_t capacityIncrease = 0);
		virtual ~WholeBlockWriter();

	public:
		virtual XtUt::Block*	Write(const void* data, size_t length);
		virtual bool			Flush(bool toStorage);

	private:
		StorageWriter*	_writer;
		XtUt::Block		_mb;
		size_t			_capacityIncrease;
	};

	// ChunkedBlockReader

	class ChunkedBlockReader : public BlockReader
	{
	public:
		ChunkedBlockReader(StorageReader* reader, size_t chunkSize);
		virtual ~ChunkedBlockReader();

	public:
		virtual XtUt::Block* Read(void* data, size_t length);

	private:
		StorageReader*	_reader;
		size_t			_chunkSize;
		XtUt::Block		_mb;
	};

	// ChunkedBlockWriter

	class ChunkedBlockWriter : public BlockWriter
	{
	public:
		ChunkedBlockWriter(StorageWriter* writer, size_t chunkSize);
		virtual ~ChunkedBlockWriter();

	public:
		virtual XtUt::Block*	Write(const void* data, size_t length);
		virtual bool			Flush(bool toStorage);

	private:
		StorageWriter*	_writer;
		size_t			_chunkSize;
		XtUt::Block		_mb;
	};
}

#endif // XT_TOOL_XTSTREAM_H