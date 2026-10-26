#include "stdafx.h"

#include <EditorLib/Collision/Collision.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/Collision/CollisionRenderer.h>

namespace Editor
{

C_CollisionRenderer::C_CollisionRenderer() : Display::I_Renderer("collisionRenderer")
{
}

C_CollisionRenderer::~C_CollisionRenderer()
{
}

//U32 count = 0;

void C_CollisionRenderer::Render()
{
#if defined(USER_KMACLEOD) && defined(BUILD_CONFIG_DEBUG)
	const Btl::List<EditorLib::C_CollisionScene*>& collisionScenes = 
		EditorLib::C_Core::Instance().GetCollisionManager().GetCollisionScenes();

	for (Btl::List<EditorLib::C_CollisionScene*>::ConstIterator it = collisionScenes.Begin(); 
		it != collisionScenes.End(); ++it)
	{
		RenderCollisionScene(*it);
	}
#endif

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
	DbUtil::DefaultDebugRenderSys()->RenderPrimitive();
/*
	char buf[512];
	sprintf(buf, "%i\n", count);
	OutputDebugString(buf);
*/
//	count++;
}

void C_CollisionRenderer::RenderCollisionScene(const EditorLib::C_CollisionScene* pCollisionScene)
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	const EditorLib::C_CollisionScene::CollisionInstanceArray& instances = 
		pCollisionScene->GetCollisionInstanceArray();
	for (S32 i = 0; i < instances.Size(); i++)
	{
		const EditorLib::C_CollisionInstance& r = instances[i];

		//Ignore objects not selected.
		if (!selection.Find(r.pDgNode->GetId()))
			continue;

		Maths::HMat44 m = Maths::Load<Maths::HMat44>(
			(const MathsStorage::Float44*)&r.pInstance->toWorldSpace);

		if (r.pInstance->pSource->GetType() == EditorLib::E_CollisionSourceType_Proxy)
		{
			EditorLib::C_CollisionProxy* pProxy = 
				(EditorLib::C_CollisionProxy*)r.pInstance->pSource;
			Maths::HVec3 p =  pProxy->boundingSphere.Centre()*m;

			DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dSphere(
				p, 
				pProxy->boundingSphere.Radius(), Maths::RGBA(0, 0, 255));
		}
		else if (r.pInstance->pSource->GetType() == EditorLib::E_CollisionSourceType_Mesh)
		{
			EditorLib::C_CollisionMesh* pMesh = 
				(EditorLib::C_CollisionMesh*)r.pInstance->pSource;

			DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dBox(
				-pMesh->obb.extents, pMesh->obb.extents, pMesh->obb.transform * Maths::HMat43(m), Maths::RGBA(0, 255, 0));
		}
		else if (r.pInstance->pSource->GetType() == EditorLib::E_CollisionSourceType_Spline)
		{
			EditorLib::C_CollisionSpline* pSpline = 
				(EditorLib::C_CollisionSpline*)r.pInstance->pSource;

			DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dBox(
				-pSpline->obb.extents, pSpline->obb.extents, pSpline->obb.transform * Maths::HMat43(m), Maths::RGBA(255, 0, 0));
		}
	}
}

}
