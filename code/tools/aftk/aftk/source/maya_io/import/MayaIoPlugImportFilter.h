#ifndef MAYA_IO_PLUG_IMPORT_FILTER_H
#define MAYA_IO_PLUG_IMPORT_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_PlugImportFilter
	{
	public:

		C_PlugImportFilter()
		{
			//perform filter
			m_filterSet.Insert(std::string("message"));
			m_filterSet.Insert(std::string("binMembership"));
			m_filterSet.Insert(std::string("caching"));
			m_filterSet.Insert(std::string("isHistoricallyInteresting"));
			m_filterSet.Insert(std::string("nodeState"));
			m_filterSet.Insert(std::string("outputMesh"));
			m_filterSet.Insert(std::string("lockInfluenceWeights"));
		}

		bool Find(const char* str) const
		{
			Ut::UtSet<std::string>::ConstIterator it;
			it = m_filterSet.Find(str);

			if (it == m_filterSet.End())
			{
				// Not found;
				return false;
			}

			return true;
		}

		bool Find(const std::string& str) const
		{
			return Find(str.c_str());
		}

		private:
			Ut::UtSet<std::string> m_filterSet;

	};
}
#endif

