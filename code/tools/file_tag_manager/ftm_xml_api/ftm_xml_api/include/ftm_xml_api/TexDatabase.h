#ifndef TEX_DATA_READER_TEX_DATABASE_H
#define TEX_DATA_READER_TEX_DATABASE_H

namespace FTM
{

class TexDatabase
{
public:
	class DBRecord
	{
	public:
		U32 NumAttributes() const;

		const std::string& FileName() const;

		U32 AddAttribute(const GenericAttribute& attr = GenericAttribute());

		const GenericAttribute& Attribute(U32 index) const;

	private:
		std::string& FileName();

		void ReserveAttributeList(U32 size);

		GenericAttribute& Attribute(U32 index);

		std::string m_fileName;
		Ut::UtVector<Ut::DynamicVector<GenericAttribute> > m_attributes;

		friend class TexDatabase;
	};

	TexDatabase();

	~TexDatabase();

	U32 NumRecords() const;

	const DBRecord& Record(U32 index) const;

	DBRecord& Record(U32 index);

	S32 RecordIndex(const char* pTextureName) const;

	///\brief Populates a texture database from the given XML file.
	///
	///@param pFileName The name of the texture XML file to read.
	///@return status Value indicating the success or failure of the operation.
	Tu::Status Read(const char* pFileName);

	void Clear();

private:
	Tu::Status ParseXMLDOM(XERCES_CPP_NAMESPACE::DOMNode* pRootNode);

	Tu::Status ParseTextures(XERCES_CPP_NAMESPACE::DOMNode* pTextureRootNode);

	Tu::Status ParseTextureParams(DBRecord& record, XERCES_CPP_NAMESPACE::DOMNode* pTextureNode);

	Ut::UtVector< Ut::DynamicVector<DBRecord> > m_records;
};

}

#endif

