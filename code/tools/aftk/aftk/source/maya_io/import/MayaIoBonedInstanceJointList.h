#ifndef MAYA_IO_BONED_INSTANCE_JOINT_LIST_H
#define MAYA_IO_BONED_INSTANCE_JOINT_LIST_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_BonedInstanceJointList
	{
	public:
		void Add(const MDagPath& path)
		{
			MFnDagNode node(path);
			m_jointList[node.name().asChar()] = path;
		}

		bool Find(const MDagPath& path) const
		{
			MFnDagNode node(path);
			return Find(node.name().asChar());
		}

		bool Find(const std::string& str) const
		{
			Ut::UtMap<std::string, MDagPath>::ConstIterator it;
			it = m_jointList.Find(str);

			if (it == m_jointList.End())
			{
				// Not found;
				return false;
			}

			return true;
		}

		bool GetPath(const std::string& str, MDagPath& path) const
		{
			Ut::UtMap<std::string, MDagPath>::ConstIterator it;
			it = m_jointList.Find(str);

			if (it == m_jointList.End())
			{
				// Not found;
				return false;
			}

			path = it->GetValue();
			return true;
		}

		void Clear()
		{
			m_jointList.Clear();
		}

	private:
		Ut::UtMap<std::string, MDagPath> m_jointList;
	};
}

#endif
