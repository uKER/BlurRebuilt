#ifndef MAYA_IO_FILTER_H
#define MAYA_IO_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Filter
	{
	public:
		void Add(const std::string& str)
		{
			m_filterSet.insert(str);
		}

		bool Find(const std::string& str) const
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

		void Clear()
		{
			m_filterSet.Clear();
		}

	private:
		Ut::UtSet<std::string> m_filterSet;
	};
}
#endif
