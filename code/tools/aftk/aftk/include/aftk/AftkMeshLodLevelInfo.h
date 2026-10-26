#ifndef AFTK_MESH_LOD_LEVEL_INFO_H
#define AFTK_MESH_LOD_LEVEL_INFO_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_MeshLodLevelInfo
	{
	public:
		C_MeshLodLevelInfo()
		{
			m_threshold = 0;
		}

		C_MeshLodLevelInfo(const C_MeshLodLevelInfo& rhs)
		{
			*this = rhs;
		}
		
		~C_MeshLodLevelInfo()
		{
		}

		const C_MeshLodLevelInfo& operator=(const C_MeshLodLevelInfo& rhs)
		{
			m_threshold = rhs.m_threshold;
			m_name = rhs.m_name;
			return *this;
		}

		void SetName(const std::string& name)
		{
			m_name = name;
		}

		void SetThreshold(F32 threshold)
		{
			m_threshold = threshold;
		}

		const std::string& GetName() const
		{
			return m_name;
		}

		std::string& GetName()
		{
			return m_name;
		}

		F32 GetThreshold() const
		{
			return m_threshold;
		}

		bool operator== (const C_MeshLodLevelInfo& rhs) const
		{
			if ((m_threshold == rhs.m_threshold) && (m_name == rhs.m_name))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	private:
		F32			m_threshold;
		std::string m_name;
	};
}
#endif
