#ifndef __FTMWRAPPER_H__
#define __FTMWRAPPER_H__

#include <map>

namespace FTM
{

// wrapper for quering a texture entry
class FTMTexInfo : public TexDatabase::DBRecord
{
public:
	template<typename T>
	inline bool GetAttribute(const std::string &name, T &outvalue) const
	{
		// look through attributes of this entry to find the right one
		for (U32 j = 0; j < NumAttributes(); j++)
		{
			const GenericAttribute& gAttr = Attribute(j);
			if (gAttr.Name() == name)
			{
				// found
				gAttr.Value(outvalue);
				return true;
			}
		}
		// not found
		return false;
	}
};

// singleton for keeping track of texture info databases
class FTMWrapper
{
public:
	const FTMTexInfo& LookupTexture(const std::string &path);
protected:
	struct _entry
	{
		FTM::TexDatabase	m_db;
	};

	int		GetDatabase(const std::string &path);

	// list of loaded .xml files
	Ut::UtVector<Ut::DynamicVector<_entry> >	m_entries;
	std::map<std::string, int>					m_map;

	FTMTexInfo m_DummyRecord;
};

}

#endif
