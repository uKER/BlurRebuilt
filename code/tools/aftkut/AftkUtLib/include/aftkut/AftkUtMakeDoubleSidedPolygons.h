#ifndef AFTK_UT_MAKE_DOUBLE_SIDED_POLYGONS_H
#define AFTK_UT_MAKE_DOUBLE_SIDED_POLYGONS_H

#ifdef _MSCVER
#pragma once
#endif

namespace AftkUt
{
	class C_MakeDoubleSidedPolygons
	{
	public:
		static void MakeDoubleSidedPolygons(
			Aftk::C_SceneMeshNode* pNode,
			const Util::C_String& attributeName);

		struct C_MakeDoubleSidedPolygonsVisitor
		{
			C_MakeDoubleSidedPolygonsVisitor()
			{
				m_AttributeName = "";
			}

			C_MakeDoubleSidedPolygonsVisitor( const Util::C_String& attributeName ) :
				m_AttributeName(attributeName)
			{
			}

			static bool IsInteresting(Aftk::C_SceneNode* pSceneNode) 
			{
				if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
					return true;

				return false;
			}

			Aftk::C_Status Visit(
				Aftk::C_SceneNode* pNode,
				C_NullMemoryManager& memoryManager)
			{
				if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
				{
					//Ignore this node.
					return Aftk::C_Status::E_Status_Success;
				}

				C_MakeDoubleSidedPolygons::MakeDoubleSidedPolygons(
					(Aftk::C_SceneMeshNode*)pNode,
					m_AttributeName);

				return Aftk::C_Status::E_Status_Success;
			}

			//Params
			Util::C_String m_AttributeName;
		};

	private:
		static const Aftk::C_MaterialResource* GetMaterialResourcePtr(const Aftk::C_Polygon* pPoly);

		static const Aftk::C_BlindDataResource* GetBlindDataResourcePtr(const Aftk::C_Polygon* pPoly);

		static void SetMaterialResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_MaterialResource* pRef);

		static void SetBlindDataResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_BlindDataResource* pRef);

		static void RemoveBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& attributeName);

		static bool HasBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& attributeName);
	};
}

#endif