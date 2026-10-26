#include "stdafx.h"

#include "../FbxIoUtility.h"

#include "FbxIoSceneImport.h"
#include "FbxIoSceneNodeImport.h"
#include "FbxIoImportOptions.h"
#include "FbxIoAnimationImport.h"
#include "FbxIoAnimationCurveImport.h"

using namespace FbxIo;

inline bool IsValidFbxCurve(KFCurve* pFbxCurve)
{
	if (pFbxCurve == NULL) return false;
	if (pFbxCurve->KeyGetCount() == 0) return false;
	return true;
}

Aftk::C_Status AnimationImport::Import(
	C_SceneImport* pSceneImport,
	KFbxNode* pFbxNode, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;

	std::string takeName	= pSceneImport->GetOptionsPtr()->takeName;
	
	//Obtain FBX animation curves (Translation, Rotation, Scaling)
	KFCurveNode* pCurveNodeT = NULL;
	KFCurveNode* pCurveNodeR = NULL;
	KFCurveNode* pCurveNodeS = NULL;

	pCurveNodeT = pFbxNode->LclTranslation.GetKFCurveNode(false, takeName.c_str());
	pCurveNodeR = pFbxNode->LclRotation.GetKFCurveNode(false, takeName.c_str());
	pCurveNodeS = pFbxNode->LclScaling.GetKFCurveNode(false, takeName.c_str());
    
	if(pCurveNodeT == NULL &&  pCurveNodeR == NULL &&  pCurveNodeS == NULL) {
		return Aftk::C_Status::E_Status_Success; 
	}

	//Convert TRS animation curves 
	KFCurve* pFbxCurve = NULL;
	Aftk::I_Plug* pPlugToConnect = NULL;
	
	if(pCurveNodeT) {
		pFbxCurve = pFbxNode->LclTranslation.GetKFCurve("X", takeName.c_str());
		if(IsValidFbxCurve(pFbxCurve))
		{
			Aftk::C_AnimationCurveNode* pAftkCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			std::string curveName = pAftkSceneNode->GetName();
			curveName += "_TranslateX";
			pAftkCurve->SetName(curveName);

			status = C_AnimationCurveImport::Import(
				pSceneImport,
				pFbxCurve,
				pAftkCurve);

			if (!status) return status;

			pPlugToConnect = pAftkSceneNode->GetTranslatePlug().GetChildPtr(0);
			if (!pPlugToConnect) {
				AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;
			}
			pPlugToConnect->SetSourceConnectionPtr(pAftkCurve);
		}

		pFbxCurve = pFbxNode->LclTranslation.GetKFCurve("Y", takeName.c_str());
		if(IsValidFbxCurve(pFbxCurve)) {
			Aftk::C_AnimationCurveNode* pAftkCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			std::string curveName = pAftkSceneNode->GetName();
			curveName += "_TranslateY";
			pAftkCurve->SetName(curveName);

			status = C_AnimationCurveImport::Import(
				pSceneImport,
				pFbxCurve,
				pAftkCurve);

			if (!status) return status;
			
			pPlugToConnect = pAftkSceneNode->GetTranslatePlug().GetChildPtr(1);
			if (!pPlugToConnect) {
				AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;
			}
			pPlugToConnect->SetSourceConnectionPtr(pAftkCurve);
		}

		pFbxCurve = pFbxNode->LclTranslation.GetKFCurve("Z", takeName.c_str());
		if(IsValidFbxCurve(pFbxCurve)) {
			Aftk::C_AnimationCurveNode* pAftkCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			std::string curveName = pAftkSceneNode->GetName();
			curveName += "_TranslateZ";
			pAftkCurve->SetName(curveName);

			status = C_AnimationCurveImport::Import(
				pSceneImport,
				pFbxCurve,
				pAftkCurve);

			if (!status) return status;

			
			pPlugToConnect = pAftkSceneNode->GetTranslatePlug().GetChildPtr(2);
			if (!pPlugToConnect) {
				AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;
			}
			pPlugToConnect->SetSourceConnectionPtr(pAftkCurve);
		}
	}

	if(pCurveNodeR) {
		pFbxCurve = pFbxNode->LclRotation.GetKFCurve("X", takeName.c_str());
		if(IsValidFbxCurve(pFbxCurve)) {
			Aftk::C_AnimationCurveNode* pAftkCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			std::string curveName = pAftkSceneNode->GetName();
			curveName += "_RotateX";
			pAftkCurve->SetName(curveName);

			status = C_AnimationCurveImport::Import(
				pSceneImport,
				pFbxCurve,
				pAftkCurve);

			if (!status) return status;

			//convert degrees angles to radians
			for (U32 i = 0; i < pAftkCurve->GetNumKeys(); i++)
			{
				Aftk::C_LinearKey key;
				pAftkCurve->GetKey(key, i);
				key.GetValue() = Maths::DegToRad(key.GetValue());
				pAftkCurve->SetKey(i, key);
			}

			pPlugToConnect = pAftkSceneNode->GetRotatePlug().GetChildPtr(0);
			if (!pPlugToConnect) {
				AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;
			}
			pPlugToConnect->SetSourceConnectionPtr(pAftkCurve);
		}

		pFbxCurve = pFbxNode->LclRotation.GetKFCurve("Y", takeName.c_str());
		if(IsValidFbxCurve(pFbxCurve)) {
			Aftk::C_AnimationCurveNode* pAftkCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			std::string curveName = pAftkSceneNode->GetName();
			curveName += "_RotateY";
			pAftkCurve->SetName(curveName);

			status = C_AnimationCurveImport::Import(
				pSceneImport,
				pFbxCurve,
				pAftkCurve);

			if (!status) return status;

			//convert degrees angles to radians
			for (U32 i = 0; i < pAftkCurve->GetNumKeys(); i++)
			{
				Aftk::C_LinearKey key;
				pAftkCurve->GetKey(key, i);
				key.GetValue() = Maths::DegToRad(key.GetValue());
				pAftkCurve->SetKey(i, key);
			}
			
			pPlugToConnect = pAftkSceneNode->GetRotatePlug().GetChildPtr(1);
			if (!pPlugToConnect) {
				AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;
			}
			pPlugToConnect->SetSourceConnectionPtr(pAftkCurve);
		}

		pFbxCurve = pFbxNode->LclRotation.GetKFCurve("Z", takeName.c_str());
		if(IsValidFbxCurve(pFbxCurve)) {
			Aftk::C_AnimationCurveNode* pAftkCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			std::string curveName = pAftkSceneNode->GetName();
			curveName += "_RotateZ";
			pAftkCurve->SetName(curveName);

			status = C_AnimationCurveImport::Import(
				pSceneImport,
				pFbxCurve,
				pAftkCurve);

			if (!status) return status;

			//convert degrees angles to radians
			for (U32 i = 0; i < pAftkCurve->GetNumKeys(); i++)
			{
				Aftk::C_LinearKey key;
				pAftkCurve->GetKey(key, i);
				key.GetValue() = Maths::DegToRad(key.GetValue());
				pAftkCurve->SetKey(i, key);
			}

			pPlugToConnect = pAftkSceneNode->GetRotatePlug().GetChildPtr(2);
			if (!pPlugToConnect) {
				AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;
			}
			pPlugToConnect->SetSourceConnectionPtr(pAftkCurve);
		}
	}

	if(pCurveNodeS) {
		pFbxCurve = pFbxNode->LclScaling.GetKFCurve("X", takeName.c_str());
		if(IsValidFbxCurve(pFbxCurve)) {
			Aftk::C_AnimationCurveNode* pAftkCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			std::string curveName = pAftkSceneNode->GetName();
			curveName += "_ScaleX";
			pAftkCurve->SetName(curveName);

			status = C_AnimationCurveImport::Import(
				pSceneImport,
				pFbxCurve,
				pAftkCurve);

			if (!status) return status;

			pPlugToConnect = pAftkSceneNode->GetScalePlug().GetChildPtr(0);
			if (!pPlugToConnect) {
				AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;
			}
			pPlugToConnect->SetSourceConnectionPtr(pAftkCurve);
		}

		pFbxCurve = pFbxNode->LclScaling.GetKFCurve("Y", takeName.c_str());
		if(IsValidFbxCurve(pFbxCurve)) {
			Aftk::C_AnimationCurveNode* pAftkCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			std::string curveName = pAftkSceneNode->GetName();
			curveName += "_ScaleY";
			pAftkCurve->SetName(curveName);

			status = C_AnimationCurveImport::Import(
				pSceneImport,
				pFbxCurve,
				pAftkCurve);

			if (!status) return status;
			
			pPlugToConnect = pAftkSceneNode->GetScalePlug().GetChildPtr(1);
			if (!pPlugToConnect) {
				AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;
			}
			pPlugToConnect->SetSourceConnectionPtr(pAftkCurve);
		}

		pFbxCurve = pFbxNode->LclScaling.GetKFCurve("Z", takeName.c_str());
		if(IsValidFbxCurve(pFbxCurve)) {
			Aftk::C_AnimationCurveNode* pAftkCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			std::string curveName = pAftkSceneNode->GetName();
			curveName += "_ScaleZ";
			pAftkCurve->SetName(curveName);

			status = C_AnimationCurveImport::Import(
				pSceneImport,
				pFbxCurve,
				pAftkCurve);

			if (!status) return status;

			
			pPlugToConnect = pAftkSceneNode->GetScalePlug().GetChildPtr(2);
			if (!pPlugToConnect) {
				AFTK_DO_BEGIN << "Failed to find plug to connect to." AFTK_DO_ERROR_RETURN_END_1;
			}
			pPlugToConnect->SetSourceConnectionPtr(pAftkCurve);
		}
	}
	   	   
	//Convert User Properties animation curves
	KFbxProperty lProperty = pFbxNode->GetFirstProperty();
	while (lProperty.IsValid())
	{
		if (lProperty.GetFlag(KFbxUserProperty::eUSER))
		{
			KString lFbxFCurveNodeName  = lProperty.GetName();

			KFCurveNode* lFbxFCurveNode = lProperty.GetKFCurveNode(false, takeName.c_str());

            if (!lFbxFCurveNode){
                lProperty = pFbxNode->GetNextProperty(lProperty);
                continue;
            }


			KFbxDataType lDataType = lProperty.GetPropertyDataType();
			KFbxUserProperty::EUserPropertyType lEUserPropertyType = DataTypeToEUserPropertyType(lDataType);
			switch (lEUserPropertyType)
			{
				case KFbxUserProperty::eBOOL:
				{
					pFbxCurve = lFbxFCurveNode->FCurveGet();

					if(IsValidFbxCurve(pFbxCurve)) 
					{
						Aftk::C_AnimationCurveNode* pAftkCurve = 
							new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());


						pAftkCurve->SetName((const char*)lProperty.GetName());

						status = C_AnimationCurveImport::Import(
							pSceneImport,
							pFbxCurve,
							pAftkCurve);

						if (!status) return status;

						Aftk::C_DynamicPlug* pAftkDynamicPlug = new Aftk::C_DynamicPlug();
						if (!pAftkDynamicPlug)
							return Aftk::C_Status::E_Status_OutOfMemory;

						pAftkSceneNode->AddPlugPtr(pAftkDynamicPlug);
						pAftkDynamicPlug->SetName((const char*)lProperty.GetName());
						pAftkDynamicPlug->SetSourceConnectionPtr(pAftkCurve);
						pAftkDynamicPlug->SetValue(true);
					}
				}
				break;
				case KFbxUserProperty::eREAL:
				{
					pFbxCurve = lFbxFCurveNode->FCurveGet();

					if(IsValidFbxCurve(pFbxCurve)) 
					{
						Aftk::C_AnimationCurveNode* pAftkCurve = 
							new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());


						pAftkCurve->SetName((const char*)lProperty.GetName());

						status = C_AnimationCurveImport::Import(
							pSceneImport,
							pFbxCurve,
							pAftkCurve);

						if (!status) return status;

						Aftk::C_DynamicPlug* pAftkDynamicPlug = new Aftk::C_DynamicPlug();
						if (!pAftkDynamicPlug)
							return Aftk::C_Status::E_Status_OutOfMemory;

						pAftkSceneNode->AddPlugPtr(pAftkDynamicPlug);
						pAftkDynamicPlug->SetName((const char*)lProperty.GetName());
						pAftkDynamicPlug->SetSourceConnectionPtr(pAftkCurve);
						pAftkDynamicPlug->SetValue(0.0f);
					}
				}
				break;
				case KFbxUserProperty::eINTEGER:			
				{
					pFbxCurve = lFbxFCurveNode->FCurveGet();

					if(IsValidFbxCurve(pFbxCurve)) 
					{
						Aftk::C_AnimationCurveNode* pAftkCurve = 
							new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());


						pAftkCurve->SetName((const char*)lProperty.GetName());

						status = C_AnimationCurveImport::Import(
							pSceneImport,
							pFbxCurve,
							pAftkCurve);

						if (!status) return status;

						Aftk::C_DynamicPlug* pAftkDynamicPlug = new Aftk::C_DynamicPlug();
						if (!pAftkDynamicPlug)
							return Aftk::C_Status::E_Status_OutOfMemory;

						pAftkSceneNode->AddPlugPtr(pAftkDynamicPlug);
						pAftkDynamicPlug->SetName((const char*)lProperty.GetName());
						pAftkDynamicPlug->SetSourceConnectionPtr(pAftkCurve);
						pAftkDynamicPlug->SetValue(0);
					}
				}
				break;
			} // switch
		}//if

		lProperty = pFbxNode->GetNextProperty(lProperty);
	} // while
	
	return Aftk::C_Status::E_Status_Success;
}

