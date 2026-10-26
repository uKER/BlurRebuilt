#ifndef CS_GEN_DATABASE_H
#define CS_GEN_DATABASE_H

#include <tex_atlas/Definitions.h>
#include <tools_common/Status.h>
#include <tools_common/GenericDiskAccess.h>
#include <tools_common/GenericDiskWriter.h>
#include <tools_common/GenericDiskReader.h>
#include <tex_atlas/TextureKey.h>

namespace TexAtlas
{
	class Database
	{
	public:
		struct DBImageRecord
		{
			TextureKey key;
			U32 lowDate;
			U32 highDate;
			F32 uOffset;
			F32 vOffset;
			F32 uScale;
			F32 vScale;
			bool rotated;
			U32 atlasIndex;
		};

		struct DBAtlasRecord
		{
			std::string fileName;
		};

		inline Database();

		inline ~Database();

		inline U32 AddAtlasRecord(const DBAtlasRecord& record);

		inline U32 AddImageRecord(const DBImageRecord& record);
		
		inline const DBImageRecord& ImageRecord(U32 index) const;
		
		inline const DBAtlasRecord& AtlasRecord(U32 index) const;
		
		inline S32 ImageRecordIndex(const TextureKey& key) const;

		inline U32 NumImageRecords() const;

		inline U32 NumAtlasRecords() const;

		inline bool Read(const std::string& fileName);

		inline bool Write(const std::string& fileName);

		void Clear();

	private:
		Ut::UtVector<Ut::DynamicVector<DBAtlasRecord> >	m_atlasRecords;
		Ut::UtVector<Ut::DynamicVector<DBImageRecord> >	m_imageRecords;
		std::map<TextureKey, U32> m_imageIndexMap;
	};

inline Database::Database()
{
}

inline Database::~Database()
{
}

inline U32 Database::AddAtlasRecord(const DBAtlasRecord& record)
{
	m_atlasRecords.PushBack(record);
	return (U32)m_atlasRecords.Size()-1;
}

inline U32 Database::AddImageRecord(const DBImageRecord& record)
{
	m_imageRecords.PushBack(record);
	m_imageIndexMap[record.key] = (U32)m_imageRecords.Size()-1;
	return (U32)m_imageRecords.Size()-1;
}

inline const Database::DBAtlasRecord& Database::AtlasRecord(U32 index) const
{
	return m_atlasRecords[index];
}

inline const Database::DBImageRecord& Database::ImageRecord(U32 index) const
{
	return m_imageRecords[index];
}

inline S32 Database::ImageRecordIndex(const TextureKey& key) const
{
	std::map<TextureKey, U32>::const_iterator it;
	it = m_imageIndexMap.find(key);
	if (it == m_imageIndexMap.end())
	{
		return -1;
	}
	else
	{
		return it->second;
	}
}

inline U32 Database::NumAtlasRecords() const
{
	return (U32)m_atlasRecords.Size();
}

inline U32 Database::NumImageRecords() const
{
	return (U32)m_imageRecords.Size();
}

inline void Database::Clear()
{
	m_atlasRecords.Clear();
	m_imageRecords.Clear();
	m_imageIndexMap.clear();
}

inline bool Database::Read(const std::string& fileName)
{
	Clear();

	Tu::GenericDiskReader reader;
	try
	{
		if (!reader.Open(fileName, TU_GENERIC_DISK_ACCESS_ID('CSDB')))
			return false;
		reader.EnterBlock();
		{
			U32 numAtlasRecords;
			reader >> numAtlasRecords;
			m_atlasRecords.Resize(numAtlasRecords);

			for (U32 i = 0; i < numAtlasRecords; i++)
			{
				reader >> m_atlasRecords[i].fileName;
			}

			U32 numImageRecords;
			reader >> numImageRecords;
			m_imageRecords.Resize(numImageRecords);

			for (U32 i = 0; i < numImageRecords; i++)
			{
				m_imageRecords[i].key.Deserialise(reader);
				m_imageIndexMap[m_imageRecords[i].key] = i;
				reader >> m_imageRecords[i].lowDate;
				reader >> m_imageRecords[i].highDate;
				reader >> m_imageRecords[i].uOffset;
				reader >> m_imageRecords[i].vOffset;
				reader >> m_imageRecords[i].uScale;
				reader >> m_imageRecords[i].vScale;
				reader >> m_imageRecords[i].rotated;
				reader >> m_imageRecords[i].atlasIndex;
			}
		}

		reader.ExitBlock();
		reader.Close();
	}
	catch (Tu::GenericDiskReader::ReadException)
	{
		return false;
	}

	return true;
}

inline bool Database::Write(const std::string& fileName)
{
	Tu::GenericDiskWriter writer;
	try
	{
		if (!writer.Open(fileName, TU_GENERIC_DISK_ACCESS_ID('CSDB'), 100))
			return false;
		writer.BeginBlock(TU_GENERIC_DISK_ACCESS_ID('MAIN'));
		{
			U32 numAtlasRecords = (U32)m_atlasRecords.Size();
			writer << numAtlasRecords;
			for (U32 i = 0; i < (U32)m_atlasRecords.Size(); i++)
			{
				writer << m_atlasRecords[i].fileName;
			}

			U32 numImageRecords = (U32)m_imageRecords.Size();
			writer << numImageRecords;
			for (U32 i = 0; i < (U32)m_imageRecords.Size(); i++)
			{
				m_imageRecords[i].key.Serialise(writer);
				writer << m_imageRecords[i].lowDate;
				writer << m_imageRecords[i].highDate;
				writer << m_imageRecords[i].uOffset;
				writer << m_imageRecords[i].vOffset;
				writer << m_imageRecords[i].uScale;
				writer << m_imageRecords[i].vScale;		
				writer << m_imageRecords[i].rotated;
				writer << m_imageRecords[i].atlasIndex;
			}
		}
		writer.EndBlock();
		writer.Close();
	}
	catch (Tu::GenericDiskWriter::WriteException)
	{
		return false;
	}

	return true;
}
}

#endif
