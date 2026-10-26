#ifndef AFTK_LOD_LEVEL_H
#define AFTK_LOD_LEVEL_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_LodLevel
	{
	public:
		C_LodLevel();
		
		C_LodLevel(const C_LodLevel& rhs);
		
		~C_LodLevel();

		const C_LodLevel& operator= (const C_LodLevel& rhs);

		bool operator== (const C_LodLevel& rhs) const;

		virtual void Clear();

	private:
		S32 AddPolygon(const C_Polygon& polygon);

		void DeletePolygon(U32 index);

		void SetInfo(const C_MeshLodLevelInfo& info);

		void SetPolygonInitialCapacity(U32 capacity);

		C_MeshLodLevelInfo& GetInfo();

		const C_MeshLodLevelInfo& GetInfo() const;

		C_Polygon& GetPolygon(U32 polyIndex);

		const C_Polygon& GetPolygon(U32 polyIndex) const;

		PolygonArray& GetPolygonArray();

		const PolygonArray& GetPolygonArray() const;

		U32 GetNumPolygons() const;

		void SetPolygon(U32 instance, const C_Polygon& polygon);

		C_MeshLodLevelInfo	m_info;
		PolygonArray	m_polygons;

		friend class C_SceneMeshNode;
	};
}

#endif
