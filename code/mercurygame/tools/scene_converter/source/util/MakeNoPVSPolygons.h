#ifndef MAKE_DO_NOT_PVS_POLYGONS_H
#define MAKE_DO_NOT_PVS_POLYGONS_H
#ifdef _MSCVER
#pragma once
#endif

#include <converter_shared/config/PolyflagConfig.h>
#include <converter_shared/config/ObjectflagConfig.h>


/// Helper class to set do not PVS polygon flag on invisible polygons.
class C_MakeNoPVSPolygons
{
public:

	/// Visitor type for do not PVS polygons.
	struct C_MakeNoPVSPolygonsVisitor
	{
	public:
		/// Default c'tor.
		C_MakeNoPVSPolygonsVisitor( const Objectflags::C_Config* pObjectFlagConfig )
			: m_pObjectFlagConfig( pObjectFlagConfig )
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

			C_MakeNoPVSPolygons::MakeNoPVSPolygons((Aftk::C_SceneMeshNode*)pNode, m_pObjectFlagConfig);
			return Aftk::C_Status::E_Status_Success;
		}

		const Objectflags::C_Config* m_pObjectFlagConfig;
	};

	/// Create invisible polygons on specified mesh node.
	static void		MakeNoPVSPolygons(Aftk::C_SceneMeshNode* pNode, const Objectflags::C_Config* pObjectFlagConfig);

private:
	/// Get blind data resource pointer from a polygon.
	static const	Aftk::C_BlindDataResource* GetBlindDataResourcePtr(const Aftk::C_Polygon* pPoly);
	/// Set the blind data resource pointer for a polygon.
	static void		SetBlindDataResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_BlindDataResource* pRef);
	/// Add some blind data to a polygon.
	static void		AddBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName);
	/// Return true if a particular blind data attribute is present on the polygon.
	static bool		HasBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName);
	/// Return true if mesh is a platform.
	static bool		IsPlatform( Aftk::C_SceneMeshNode* pAftkSceneMeshNode, const Objectflags::C_Config* pObjectFlagConfig );
};

#endif // MAKE_DO_NOT_PVS_POLYGONS_H
