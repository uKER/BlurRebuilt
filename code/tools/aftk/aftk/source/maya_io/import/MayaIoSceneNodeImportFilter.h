#ifndef MAYA_IO_SCENE_NODE_IMPORT_FILTER_H
#define MAYA_IO_SCENE_NODE_IMPORT_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_SceneNodeImportFilter
	{
	public:

		C_SceneNodeImportFilter()
		{
			//perform filter
			m_filterSet.PushBack(std::string("donotexport"));
			m_filterSet.PushBack(std::string("rotoscope"));
			m_filterSet.PushBack(std::string("persp"));
			m_filterSet.PushBack(std::string("top"));
			m_filterSet.PushBack(std::string("front"));
			m_filterSet.PushBack(std::string("side"));
			m_filterSet.PushBack(std::string("groundplane_transform"));
		}

		bool Find(const char* str) const
		{
			if (!strcmp(str, ""))
			{
				return false;
			}
			std::string search(str);
			std::transform (search.begin(),search.end(), search.begin(), tolower);

			for (U32 i = 0; i < (U32)m_filterSet.Size(); i++)
			{
				if (i < 2)
				{
					std::string::size_type pos = search.find(m_filterSet[i]);

					if (pos != std::string::npos)
					{
						return true;
					}
				}
				else
				{
					//perform exact matching
					const char* test = m_filterSet[i].c_str();
					if (!strcmp(test, search.c_str()))
					{
						return true;
					}
				}
			}
			return false;
		}

		private:
			Ut::UtVector< Ut::DynamicVector<std::string> > m_filterSet;

	};
}

#endif
