#include "stdafx.h"

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtLarvaHelper.h>
#include <EditorLib/Xt/XtUserData.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>

#include <EditorApp/UserData/UserData.h>

namespace Editor
{
namespace UserData
{

void LoadUserData(XtId packId)
{
	Xt::DomComponent* levelData = EditorLib::C_Core::Instance().GetXtUserData().ObtainPackUserData(packId);

	const RenderViewportArray& viewports = C_RenderCore::Instance().GetRenderViewports();

	if (levelData)
	{
		//Frame scene.
		Xt::DomArray* viewportDataArray = Xt::AcquireArray(&levelData->data, "ViewportData");
		for (S32 i = 0; i < viewports.Size(); i++)
		{
			Btl::String viewportLabel = viewports[i]->GetLabel();
			
			Xt::DomBlock* viewportData = 0;

			for(Xt::DomArray::IndexIterator it(viewportDataArray); it; ++it)
			{
				Xt::DomComponent* pComponent = Xt::AsComponent(it);

				if (!pComponent)
					continue;

				Xt::DomBlock* pBlock = &pComponent->data;

				Xt::DomField* viewportIdField = AcquireField(pBlock, "ViewportId");
				Btl::String viewportId = Xt::AsText(viewportIdField);
				
				if (viewportLabel == viewportId)
				{
					viewportData = pBlock;
					break;
				}
			}

			if (viewportData) {
				EditorLib::Vector3 fieldVector;
				F32	fieldFloat;
				Xt::DomField* pField;
				Xt::DomBlock* pBlock;
				
				Maths::HVec3& cameraPosition = viewports[i]->GetCamera().GetPosition();
				pBlock = AcquireBlock(viewportData, "CameraPosition");
				if (pBlock) {
					EditorLib::XtFieldHelper::GetVector(fieldVector, pBlock);
					cameraPosition.X(fieldVector.X());
					cameraPosition.Y(fieldVector.Y());
					cameraPosition.Z(fieldVector.Z());
				}

				Maths::HVec3& cameraFocusPosition = viewports[i]->GetCamera().GetFocusPosition();
				pBlock = AcquireBlock(viewportData, "CameraFocusPosition");
				if (pBlock) {
					EditorLib::XtFieldHelper::GetVector(fieldVector, pBlock);
					cameraFocusPosition.X(fieldVector.X());
					cameraFocusPosition.Y(fieldVector.Y());
					cameraFocusPosition.Z(fieldVector.Z());
				}

				Maths::HVec3& cameraUpVector = viewports[i]->GetCamera().GetUpVector();
				pBlock = AcquireBlock(viewportData, "CameraUpVector");
				if (pBlock) {
					EditorLib::XtFieldHelper::GetVector(fieldVector, pBlock);
					cameraUpVector.X(fieldVector.X());
					cameraUpVector.Y(fieldVector.Y());
					cameraUpVector.Z(fieldVector.Z());
				}
				
				pField = AcquireField(viewportData, "CameraHeightExtent");
				if (pField) {
					EditorLib::XtFieldHelper::GetScalar(fieldFloat, pField);
					viewports[i]->GetCamera().SetHeightExtent(fieldFloat);
				}

			} else {
 				C_RenderCore::Instance().FrameScene(viewports[i]);
			}
		}
	} else {
		for (S32 i = 0; i < viewports.Size(); i++)
		{
			C_RenderCore::Instance().FrameScene(viewports[i]);
		}
	}
}

void SaveUserData(XtId packId)
{
	Xt::DomComponent* levelData = EditorLib::C_Core::Instance().GetXtUserData().ObtainPackUserData(packId);

	if (levelData)
	{
		Xt::DomArray* viewportDataArray = Xt::AcquireArray(&levelData->data, "ViewportData");

		const RenderViewportArray& viewports = C_RenderCore::Instance().GetRenderViewports();
		for (S32 i = 0; i < viewports.Size(); i++)
		{
			Btl::String viewportLabel = viewports[i]->GetLabel();

			Xt::DomComponent* viewportData = 0;

			for(Xt::DomArray::IndexIterator it(viewportDataArray); it; ++it)
			{
				Xt::DomComponent* pComponent = Xt::AsComponent(it);

				Xt::DomField* viewportIdField = AcquireField(&pComponent->data, "ViewportId");
				Btl::String viewportId = Xt::AsText(viewportIdField);
				
				if (viewportLabel == viewportId)
					viewportData = pComponent;
			}

			if (!viewportData)
			{
				viewportData = new Xt::DomComponent();
				if (!viewportData)
					return;

				LarvaSchema* pSchema = EditorLib::C_Core::Instance().GetXtUserData().GetSchema();
				LtStruct* pType = EditorLib::XtLarvaHelpers::FindLarvaType( EditorLib::C_Core::Instance().GetXtUserData().GetSchema(), "ActorStudioViewportData", ::EditorLib::E_XtSchemaType_Other);
				SetDefaults(pSchema, pType, viewportData);

				Append(viewportDataArray, viewportData);
			}

			Xt::DomField* viewportIdField = AcquireField(&viewportData->data, "ViewportId");

			Xt::Set(viewportIdField, viewports[i]->GetLabel());

			Xt::DomBlock* pBlock;
			Xt::DomField* pField;
			EditorLib::Vector3 fieldVector;
			F32	fieldFloat;
			
			Maths::HVec3& cameraPosition = viewports[i]->GetCamera().GetPosition();
			pBlock = AcquireBlock(&viewportData->data, "CameraPosition");
			fieldVector.X(cameraPosition.X());
			fieldVector.Y(cameraPosition.Y());
			fieldVector.Z(cameraPosition.Z());
			EditorLib::XtFieldHelper::SetVector(pBlock, fieldVector);

			Maths::HVec3& cameraFocusPosition = viewports[i]->GetCamera().GetFocusPosition();
			pBlock = AcquireBlock(&viewportData->data, "CameraFocusPosition");
			fieldVector.X(cameraFocusPosition.X());
			fieldVector.Y(cameraFocusPosition.Y());
			fieldVector.Z(cameraFocusPosition.Z());
			EditorLib::XtFieldHelper::SetVector(pBlock, fieldVector);

			Maths::HVec3& cameraUpVector = viewports[i]->GetCamera().GetUpVector();
			pBlock = AcquireBlock(&viewportData->data, "CameraUpVector");
			fieldVector.X(cameraUpVector.X());
			fieldVector.Y(cameraUpVector.Y());
			fieldVector.Z(cameraUpVector.Z());
			EditorLib::XtFieldHelper::SetVector(pBlock, fieldVector);

			fieldFloat = viewports[i]->GetCamera().GetHeightExtent();
			pField = AcquireField(&viewportData->data, "CameraHeightExtent");
			EditorLib::XtFieldHelper::SetScalar(pField, fieldFloat);
		}
	}
}

}
}
