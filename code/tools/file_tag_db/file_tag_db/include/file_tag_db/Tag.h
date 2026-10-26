#ifndef FILE_TAG_DB_TAG_H
#define FILE_TAG_DB_TAG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace FileTag
{
	class C_Tag
	{
	public:
		U32 GetNumAttributes() const;

		const Util::C_String& GetFileName() const;

		const Ut::Vector<C_GenericAttribute>& GetAttributes() const;

		Ut::Vector<C_GenericAttribute>& GetAttributes();

		const C_GenericAttribute* GetAttributePtr(const Util::C_String &name) const;

		C_GenericAttribute* GetAttributePtr(const Util::C_String &name);

		template<typename T>
		bool GetAttributeValue(const Util::C_String& name, T& outvalue) const;

	private:
		Util::C_String& GetFileName();

		Util::C_String					m_fileName;
		Ut::Vector<C_GenericAttribute>	m_attributes;

		friend class C_TagDB;
	};

	template<typename T>
	bool C_Tag::GetAttributeValue(const Util::C_String &name, T &outvalue) const
	{
		// look through attributes of this entry to find the right one
		for (S32 j = 0; j < m_attributes.Size(); j++)
		{
			const C_GenericAttribute& gAttr = m_attributes[j];
			if (gAttr.GetName() == name)
			{
				// found
				gAttr.GetValue(outvalue);
				return true;
			}
		}
		// not found
		return false;
	}
}

#endif
