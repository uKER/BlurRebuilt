#ifndef MAYA_IO_LAYER_IMPORT_FILTER_H
#define MAYA_IO_LAYER_IMPORT_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace MayaIo
{
	class C_LayerImportFilter
	{
	public:

		C_LayerImportFilter()
		{
			//perform filter
			m_filterSet.PushBack(std::string("rotoscope"));
			m_filterSet.PushBack(std::string("donotexport"));
		}

		bool Find(const char* str) const
		{
			if (!strcmp(str, ""))
			{
				return false;
			}
			std::string search(str);

			for (U32 i = 0; i < (U32)m_filterSet.Size(); i++)
			{
				std::transform (search.begin(),search.end(), search.begin(), tolower);
				std::string::size_type pos = search.find(m_filterSet[i]);

				if (pos != std::string::npos)
				{
					return true;
				}
			}
			return false;
		}

		private:
			Ut::UtVector< Ut::DynamicVector<std::string> > m_filterSet;

	};
}

#endif
