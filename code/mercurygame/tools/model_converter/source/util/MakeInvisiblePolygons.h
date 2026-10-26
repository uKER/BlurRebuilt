#ifndef AFTK_UT_MAKE_INVISIBLE_POLYGONS_H
#define AFTK_UT_MAKE_INVISIBLE_POLYGONS_H
#ifdef _MSCVER
#pragma once
#endif

#include <converter_shared/config/PolyflagConfig.h>
#include <converter_shared/config/ObjectflagConfig.h>


/// Helper class to set invisible polygon flag on portals, navmesh and non-NULL collision types.
class C_MakeInvisiblePolygons
{
public:

	/// Visitor type for invisible polygons.
	struct C_MakeInvisiblePolygonsVisitor
	{
	public:
		/// Default c'tor.
		C_MakeInvisiblePolygonsVisitor() :
			m_pPolyConfig(NULL),
			m_pObjectConfig(NULL)
		{ return; }

		/// Specified c'tor.
		C_MakeInvisiblePolygonsVisitor(const Polyflags::C_Config* pPolyConfig, const Objectflags::C_Config* pObjectConfig) :
			m_pPolyConfig(pPolyConfig),
			m_pObjectConfig(pObjectConfig)
		{ return; }

		/// Return true if this node is a mesh node.
		static bool IsInteresting(Aftk::C_SceneNode* pSceneNode) 
		{
			if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
				return true;

			return false;
		}

		/// Visit specified node.
		Aftk::C_Status Visit(Aftk::C_SceneNode* pNode, C_NullMemoryManager& /*rkMemoryManager*/)
		{
			if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
				return Aftk::C_Status::E_Status_Success;	// Ignore this node.

			C_MakeInvisiblePolygons::MakeInvisiblePolygons((Aftk::C_SceneMeshNode*)pNode, m_pPolyConfig, m_pObjectConfig);
			return Aftk::C_Status::E_Status_Success;
		}

	private:
		const Polyflags::C_Config*	m_pPolyConfig;			// Polygon config pointer.
		const Objectflags::C_Config*	m_pObjectConfig;		// Object config pointer.
	};

	/// Create invisible polygons on specified mesh node.
	static void		MakeInvisiblePolygons(Aftk::C_SceneMeshNode* pNode, const Polyflags::C_Config* pPolyConfig, const Objectflags::C_Config* pObjectConfig);

private:
	/// Get blind data resource pointer from a polygon.
	static const	Aftk::C_BlindDataResource* GetBlindDataResourcePtr(const Aftk::C_Polygon* pPoly);
	/// Set the blind data resource pointer for a polygon.
	static void		SetBlindDataResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_BlindDataResource* pRef);
	/// Add some blind data to a polygon.
	static void		AddBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName);
	/// Return true if a particular blind data attribute is present on the polygon.
	static bool		HasBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName);
};

#endif // AFTK_UT_MAKE_INVISIBLE_POLYGONS_H
