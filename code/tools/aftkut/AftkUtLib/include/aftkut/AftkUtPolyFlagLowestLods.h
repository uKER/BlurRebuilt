#ifndef AFTK_UT_POLY_FLAG_LOWEST_LODS_H
#define AFTK_UT_POLY_FLAG_LOWEST_LODS_H

#ifdef _MSCVER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <btl/utility/pair.h>

namespace AftkUt
{
	class C_PolyFlagLowestLods
	{
	public:
		static void PolyFlagLowestLod(
			Aftk::C_SceneLodGroupNode* pAftkLodGroupNode,
			const Btl::Vector<Btl::Pair<Util::C_String, Util::C_String> >& polyFlags
		);

		struct C_PolyFlagLowestLodsVisitor
		{
		public:
			C_PolyFlagLowestLodsVisitor(const Btl::Vector<Btl::Pair<Util::C_String, Util::C_String> >& polyFlags)
			{
				for (U32 i = 0, count = static_cast<U32>(polyFlags.Size()); i < count; ++i)
					m_PolyFlags.PushBack(polyFlags[i]);
			}

			static bool IsInteresting(Aftk::C_SceneNode* pSceneNode) 
			{
				if (pSceneNode->GetType() == Aftk::E_ObjType_SceneLodGroup)
					return true;

				return false;
			}

			Aftk::C_Status Visit(
				Aftk::C_SceneNode* pNode,
				C_NullMemoryManager& memoryManager)
			{
				if (pNode->GetType() != Aftk::E_ObjType_SceneLodGroup)
				{
					//Ignore this node.
					return Aftk::C_Status::E_Status_Success;
				}

				C_PolyFlagLowestLods::PolyFlagLowestLod(
					(Aftk::C_SceneLodGroupNode*)pNode,
					m_PolyFlags
				);

				return Aftk::C_Status::E_Status_Success;
			}

		private:
			Btl::Vector<Btl::Pair<Util::C_String, Util::C_String> >	m_PolyFlags;
		};

	private:
		static S16 CreateBlindDataResource(Aftk::C_SceneMeshNode& sceneMeshNode,
			const Btl::Vector<Btl::Pair<Util::C_String, Util::C_String> >& polyFlags, const S16 blindDataResourceIndex = -1);
	};
}

#endif // AFTK_UT_POLY_FLAG_LOWEST_LODS_H