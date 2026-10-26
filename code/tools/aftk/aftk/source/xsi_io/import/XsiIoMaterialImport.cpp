#include "stdafx.h"

#include "../XsiIoUtility.h"

#include "XsiIoSceneImport.h"
#include "XsiIoSceneNodeImport.h"
#include "XsiIoSceneMeshImport.h"
#include "XsiIoMaterialImport.h"
#include "XsiIoImportOptions.h"
#include <core/utility/StringProc.h>

//#define DEBUG_OUTPUT
using namespace XsiIo;

Aftk::C_Status C_MaterialImport::Import(
	C_SceneImport* pSceneImport,
	CSLBaseMaterial* pXsiBaseMaterial, 
	Aftk::C_MaterialNode* pAftkMaterial)
{
	CSLTemplate::ETemplateType type = pXsiBaseMaterial->Type();
	
	if (type == CSLTemplate::XSI_MATERIAL) 
	{
		return ImportXsiMaterial(
			pSceneImport, 
			pXsiBaseMaterial,
			pAftkMaterial);
	}
	else if (type == CSLTemplate::SI_MATERIAL)
	{
		return ImportSiMaterial(
			pSceneImport, 
			pXsiBaseMaterial,
			pAftkMaterial);
	}
	
	return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Encountered unknown material type.");
}

Aftk::C_Status C_MaterialImport::ImportSiMaterial(
	C_SceneImport* pSceneImport,
	CSLBaseMaterial* pXsiBaseMaterial, 
	Aftk::C_MaterialNode* pAftkMaterial)
{
	//We are converting a legacy material
	CSLMaterial* pMaterial = (CSLMaterial*) pXsiBaseMaterial;
	
	pAftkMaterial->SetName(pMaterial->GetName());
	pAftkMaterial->SetMaterialTypeId(0);
	pAftkMaterial->SetOrigin(Aftk::E_MaterialOrigin_SI);

	CSLTexture2D* texture = pMaterial->Texture2D();
	if (texture)
	{
		Aftk::C_Status status;
		//Create a new image shader node
		Aftk::C_ShaderImageNode* pAftkImage = new Aftk::C_ShaderImageNode(&status);
		if (!pAftkImage)
			return Aftk::C_Status::E_Status_OutOfMemory;
		if (!status)
			return status;

		pAftkImage->SetName(texture->GetName());
		std::string fileName = texture->GetImageFileName();
		Tu::ToLower(fileName);
		pAftkImage->GetFilePlug().SetValue(fileName);
		U32 shaderIndex = pAftkMaterial->AddShaderNodePtr(pAftkImage);
		Aftk::I_Object* pShaderObj = pAftkMaterial->GetShaderNodePtr(shaderIndex);

		//Add a plug to our material, to link it to the new texture

		Aftk::C_DynamicPlug* pAftkPlug = new Aftk::C_DynamicPlug();
		if (!pAftkPlug)
			return Aftk::C_Status::E_Status_OutOfMemory;
		pAftkMaterial->AddPlugPtr(pAftkPlug);
		
		pAftkPlug->SetName("siTexture2d");
		//Connect the plug to our new shader node
		pAftkPlug->SetSourceConnectionPtr(pShaderObj);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_MaterialImport::ImportXsiMaterial(
	C_SceneImport* pSceneImport,
	CSLBaseMaterial* pXsiBaseMaterial, 
	Aftk::C_MaterialNode* pAftkMaterial)
{
	Aftk::C_Status status;

	CSLXSIMaterial* pXsiMaterial = (CSLXSIMaterial*) pXsiBaseMaterial;

	//Stage 1 set basic material proporties
	#ifdef DEBUG_OUTPUT
	std::cout << "Stage 1 - set basic material proporties." << std::endl;
	#endif
	{
		#ifdef DEBUG_OUTPUT
		std::cout << "*** [Material] = " << pXsiMaterial->Name().GetText() << "***" << std::endl;
		#endif
		pAftkMaterial->SetName(pXsiMaterial->Name().GetText());
		pAftkMaterial->SetMaterialTypeId(0);
		pAftkMaterial->SetOrigin(Aftk::E_MaterialOrigin_XSI);
	}

	if (pXsiMaterial->GetShaderCount() == 0) 
	{
		Aftk::C_Status::E_Status_Success;
	}

	Ut::UtMap<CSLXSIShader*, Aftk::C_ShaderNode*> xsiAftkShaderNodeMapping;
	// Stage 2 copy shaders
	#ifdef DEBUG_OUTPUT
	std::cout << "Stage 2 - copy shaders." << std::endl;
	#endif
	{
		CSLXSIShader** pShaderList = pXsiMaterial->GetShaderList();
		for (int i = 0; i < pXsiMaterial->GetShaderCount(); i++)
		{
			Aftk::C_ShaderNode* pAftkShaderNode = CreateAftkShaderNode(
				pShaderList[i],
				xsiAftkShaderNodeMapping,
				status);
			AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
			xsiAftkShaderNodeMapping[pShaderList[i]] = pAftkShaderNode;
			pAftkMaterial->AddShaderNodePtr(pAftkShaderNode);
		}
	}

	// Stage 3 build relationships between shaders
	#ifdef DEBUG_OUTPUT
	std::cout << "Stage 3 - build relationships between shaders." << std::endl;
	#endif
	{
		CSLXSIShader** pShaderList = pXsiMaterial->GetShaderList();
		for (int i = 0; i < pXsiMaterial->GetShaderCount(); i++)
		{
			status = BuildRelationships(
				pSceneImport,
				pShaderList[i],
				pAftkMaterial,
				xsiAftkShaderNodeMapping);
			AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
		}
	}

	// Stage 4 copy connections between material and shaders
	#ifdef DEBUG_OUTPUT
	std::cout << "Stage 4 - copy connections between material and shaders." << std::endl;
	#endif
	U32 numConnectionPoints = pXsiMaterial->GetConnectionPointCount();
	if (pXsiMaterial->GetConnectionPointCount() > 0)
	{
		CSLConnectionPoint** pConnectionPointList = pXsiMaterial->GetConnectionPointList();

		//Copy plug names
		for (U32 i = 0; i < numConnectionPoints; i++)
		{
			CSLConnectionPoint* pConnectionPoint = pConnectionPointList[i];
			#ifdef DEBUG_OUTPUT
			std::cout << pConnectionPoint->GetName() << " -> ";
			#endif

			Aftk::C_DynamicPlug* pAftkPlug = new Aftk::C_DynamicPlug();
			if (!pAftkPlug)
				return Aftk::C_Status::E_Status_OutOfMemory;
			pAftkMaterial->AddPlugPtr(pAftkPlug);
			pAftkPlug->SetName(pConnectionPoint->GetName());

			//Connect plugs to shader nodes
			CSLXSIShader* pShader = pConnectionPoint->GetShader();
			if (pShader == 0)
			{
				#ifdef DEBUG_OUTPUT
				std::cout << "NULL" << std::endl;
				#endif
				continue;
			}

			//Attempt to find the shader attached at the end of the connection point. We look in our
			//mappping of XSI shaders -> AFTK shaders we previously created.
			Ut::UtMap<CSLXSIShader*, Aftk::C_ShaderNode*>::Iterator it;
			it = xsiAftkShaderNodeMapping.Find(pShader);
			if (it != xsiAftkShaderNodeMapping.End())
			{
				//We found that we had already created an AFTK shader to match that from XSI
				Aftk::C_ShaderNode* pAftkShaderNode = it->GetValue();
				#ifdef DEBUG_OUTPUT
				std::cout << pConnectionPoint->GetShader()->Name().GetText() << std::endl;
				#endif
				pAftkPlug->SetSourceConnectionPtr(pAftkShaderNode);
			}
			else
			{
				//We could not find an AFTK shader to match that from XSI. I'm not sure if we should allow this
				//to happen. It is probably the result of a prior failure during shader conversion.
				#ifdef DEBUG_OUTPUT
				std::cout << "No connection source" << std::endl;
				#endif
			}
		}
	}

	#ifdef DEBUG_OUTPUT
	std::cout << "Connection points finished." << std::endl;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_ShaderNode* C_MaterialImport::ConvertAftkImageShaderNode(
	const std::string& nodeName, 
	std::string& imageName, 
	Aftk::C_Status& status)
{
	Aftk::C_ShaderImageNode* pAftkShaderImageNode = new Aftk::C_ShaderImageNode(&status);
	if (!pAftkShaderImageNode)
	{
		status.Set(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create shader image node.", 1);
		return 0;
	}
	if (!status)
		return 0;

	pAftkShaderImageNode->SetName(nodeName);
	Tu::ToLower(imageName);
	pAftkShaderImageNode->GetFilePlug().SetValue(imageName);

	status = Aftk::C_Status::E_Status_Success;
	return pAftkShaderImageNode;
}

Aftk::C_ShaderNode* C_MaterialImport::CreateAftkShaderNode(
	CSLXSIShader* pShader,
	Ut::UtMap<CSLXSIShader*, Aftk::C_ShaderNode*>& xsiAftkShaderNodeMapping,
	Aftk::C_Status& status)
{
	Aftk::C_ShaderNode* pAftkShaderNode = new Aftk::C_ShaderNode();
	if (!pAftkShaderNode)
	{
		status = Aftk::C_Status::E_Status_OutOfMemory;
		return 0;
	}

	if (pShader->GetParameterCount() == 0) 
	{
		return pAftkShaderNode;
	}

	if( pShader->GetProgID() )
	{
		// Xsi shader type
		std::string shaderType( pShader->GetProgID() );
		pAftkShaderNode->SetOriginalNodeTypeName( shaderType );
	}

	CSLVariantParameter** pParameterList = pShader->GetParameterList();
	#ifdef DEBUG_OUTPUT
	std::cout << "Displaying parameters for shader [" << pShader->Name().GetText() << "]" << std::endl;
	#endif
	pAftkShaderNode->SetName(pShader->Name().GetText());

	for (int i = 0; i < pShader->GetParameterCount(); i++)
	{
		//The name of our current parameter
		std::string name0(pParameterList[i]->GetName());
		if (pShader->GetParameterCount() - i >= 3)
		{
			//Get addition parameters - look into the future to see whether we need to read on a
			//compound attribute.
			std::string name1(pParameterList[i+1]->GetName());
			std::string name2(pParameterList[i+2]->GetName());
			std::string name3;
			if (pShader->GetParameterCount() - i >= 4)
			{
				//Get addition parameters - look into the future to see whether we need to read on a
				//compound attribute.
				name3 = pParameterList[i+3]->GetName();
			}

			//Check for colours
			if (
				(name0.size() >= 4)
				&&
				(name1.size() >= 6)
				&&
				(name2.size() >= 5)
				&&
				(name3.size() >= 6)
				)
			{
				if (
					(name0.substr(name0.size() - 4, 4) == ".red")
					&&
					(name1.substr(name1.size() - 6, 6) == ".green")
					&&
					(name2.substr(name2.size() - 5, 5) == ".blue")
					&&
					(name3.substr(name3.size() - 6, 6) == ".alpha")
				)
				{
					//We have a 4 element colour, convert it.
					Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
					if (!pPlug)
					{
						status = Aftk::C_Status::E_Status_OutOfMemory;
						return 0;
					}

					pAftkShaderNode->AddPlugPtr(pPlug);
					std::string name = name0.substr(0, name0.size()-4);
					pPlug->SetName(name);

					Aftk::C_DynamicPlug* pRAftkPlug = new Aftk::C_DynamicPlug();
					if (!pRAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }

					Aftk::C_DynamicPlug* pGAftkPlug = new Aftk::C_DynamicPlug();
					if (!pGAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }

					Aftk::C_DynamicPlug* pBAftkPlug = new Aftk::C_DynamicPlug();
					if (!pBAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }

					Aftk::C_DynamicPlug* pAAftkPlug = new Aftk::C_DynamicPlug();
					if (!pAAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }

					pPlug->AddChildPtr(pRAftkPlug);
					pPlug->AddChildPtr(pGAftkPlug);
					pPlug->AddChildPtr(pBAftkPlug);
					pPlug->AddChildPtr(pAAftkPlug);

					pRAftkPlug->SetName("r");
					pGAftkPlug->SetName("g");
					pBAftkPlug->SetName("b");
					pAAftkPlug->SetName("a");
		
					pRAftkPlug->SetValue(pParameterList[i]->GetFloatValue());
					pGAftkPlug->SetValue(pParameterList[i+1]->GetFloatValue());
					pBAftkPlug->SetValue(pParameterList[i+2]->GetFloatValue());
					pAAftkPlug->SetValue(pParameterList[i+3]->GetFloatValue());

					i+=3;
					continue;
				}
			}
			if (
					(name0.size() >= 4)
					&&
					(name1.size() >= 6)
					&&
					(name2.size() >= 5)
					)
			{			
				if (
					(name0.substr(name0.size() - 4, 4) == ".red")
					&&
					(name1.substr(name1.size() - 6, 6) == ".green")
					&&
					(name2.substr(name2.size() - 5, 5) == ".blue")
					)
				{
					//We have a 3 element colour, convert it.
					Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug;
					if (!pPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }

					pAftkShaderNode->AddPlugPtr(pPlug);
					std::string name = name0.substr(0, name.size()-4);
					pPlug->SetName(name);

					Aftk::C_DynamicPlug* pRAftkPlug = new Aftk::C_DynamicPlug;
					if (!pRAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					Aftk::C_DynamicPlug* pGAftkPlug = new Aftk::C_DynamicPlug;
					if (!pGAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					Aftk::C_DynamicPlug* pBAftkPlug = new Aftk::C_DynamicPlug;
					if (!pBAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }

					pPlug->AddChildPtr(pRAftkPlug);
					pPlug->AddChildPtr(pGAftkPlug);
					pPlug->AddChildPtr(pBAftkPlug);

					pRAftkPlug->SetName("r");
					pGAftkPlug->SetName("g");
					pBAftkPlug->SetName("b");

					pRAftkPlug->SetValue(pParameterList[i]->GetFloatValue());
					pGAftkPlug->SetValue(pParameterList[i+1]->GetFloatValue());
					pBAftkPlug->SetValue(pParameterList[i+2]->GetFloatValue());

					i+=2;
					continue;
				}
			}

			//Check for other vectors
			if ((name0.size() >= 2) &&	(name1.size() >= 2)	&&(name2.size() >= 2)&&	(name3.size() >= 2))
			{
				if (
					(name0.substr(name0.size() - 2, 2) == ".x")
					&&
					(name1.substr(name1.size() - 2, 2) == ".y")
					&&
					(name2.substr(name2.size() - 2, 2) == ".z")
					&&
					(name3.substr(name3.size() - 2, 2) == ".w")
				)
				{
					//We have a 4 element colour, convert it.
					Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
					if (!pPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					pAftkShaderNode->AddPlugPtr(pPlug);
					std::string name = name0.substr(0, name0.size()-2);
					pPlug->SetName(name);
					
					Aftk::C_DynamicPlug* pXAftkPlug = new Aftk::C_DynamicPlug;
					if (!pXAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					pPlug->AddChildPtr(pXAftkPlug);

					Aftk::C_DynamicPlug* pYAftkPlug = new Aftk::C_DynamicPlug;
					if (!pYAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					pPlug->AddChildPtr(pYAftkPlug);

					Aftk::C_DynamicPlug* pZAftkPlug = new Aftk::C_DynamicPlug;
					if (!pZAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					pPlug->AddChildPtr(pZAftkPlug);

					Aftk::C_DynamicPlug* pWAftkPlug = new Aftk::C_DynamicPlug;
					if (!pWAftkPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					pPlug->AddChildPtr(pWAftkPlug);

					pXAftkPlug->SetName("x");
					pYAftkPlug->SetName("y");
					pZAftkPlug->SetName("z");
					pWAftkPlug->SetName("w");
		
					pXAftkPlug->SetValue(pParameterList[i]->GetFloatValue());
					pYAftkPlug->SetValue(pParameterList[i+1]->GetFloatValue());
					pZAftkPlug->SetValue(pParameterList[i+2]->GetFloatValue());
					pWAftkPlug->SetValue(pParameterList[i+3]->GetFloatValue());

					i+=3;
					continue;
				}
			}
			if ( (name0.size() >= 2) && (name1.size() >= 2) && (name2.size() >= 2) )
			{			
				if (
					(name0.substr(name0.size() - 2, 2) == ".x")
					&&
					(name1.substr(name1.size() - 2, 2) == ".y")
					&&
					(name2.substr(name2.size() - 2, 2) == ".z")
					)
				{
					//We have a 3 element colour, convert it.
					Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
					if (!pPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					pAftkShaderNode->AddPlugPtr(pPlug);

					std::string name = name0.substr(0, name0.size()-2);
					pPlug->SetName(name);

					Aftk::C_DynamicPlug* pXAftkPlug = new Aftk::C_DynamicPlug();
					if (!pPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					pPlug->AddChildPtr(pXAftkPlug);
					
					Aftk::C_DynamicPlug* pYAftkPlug = new Aftk::C_DynamicPlug();
					if (!pPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					pPlug->AddChildPtr(pYAftkPlug);
					
					Aftk::C_DynamicPlug* pZAftkPlug = new Aftk::C_DynamicPlug();
					if (!pPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }
					pPlug->AddChildPtr(pZAftkPlug);

					pXAftkPlug->SetName("x");
					pYAftkPlug->SetName("y");
					pZAftkPlug->SetName("z");

					pXAftkPlug->SetValue(pParameterList[i]->GetFloatValue());
					pYAftkPlug->SetValue(pParameterList[i+1]->GetFloatValue());
					pZAftkPlug->SetValue(pParameterList[i+2]->GetFloatValue());

					i+=2;
					continue;
				}
			}

		}

		//If we get here we're not extracting some vector value.
		Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
		if (!pPlug) { status = Aftk::C_Status::E_Status_OutOfMemory; return 0; }

		pAftkShaderNode->AddPlugPtr(pPlug);
		pPlug->SetName(name0);

		CSLAnimatableType::EElementType valueType = pParameterList[i]->Type();
		SI_TinyVariant* variant = pParameterList[i]->GetValue();

		switch (variant->variantType)
		{
			case SI_VT_BOOL:
				pPlug->SetValue((variant->boolVal>0));
				break;
			case SI_VT_BYTE:
				pPlug->SetValue(variant->bVal);
				break;
			case SI_VT_DOUBLE:
				pPlug->SetValue(variant->dVal);
				break;
			case SI_VT_FLOAT:
				pPlug->SetValue(variant->fVal);
				break;
			case SI_VT_INT:
				pPlug->SetValue(variant->nVal);
				break;
			case SI_VT_LONG:
				pPlug->SetValue(variant->lVal);
				break;
			case SI_VT_SHORT:
				pPlug->SetValue(variant->sVal);
				break;
			case SI_VT_PCHAR:
			{
				std::string stringTest = variant->p_cVal;
				pPlug->SetValue(stringTest);
				break;
			}
			case SI_VT_UBYTE:
				pPlug->SetValue(variant->ubVal);
				break;
			case SI_VT_ULONG:
				pPlug->SetValue(U32(variant->ulVal));
				break;
			default:
				break;
		};

		#ifdef DEBUG_OUTPUT
		std::cout << "XSI Material Param: " << name0 << std::endl;
		#endif
	}

	status = Aftk::C_Status::E_Status_Success;
	return pAftkShaderNode;
}

Aftk::C_Status C_MaterialImport::BuildRelationships(
	C_SceneImport* pSceneImport,
	CSLXSIShader* pShader,
	Aftk::C_MaterialNode* pAftkMaterial,
	Ut::UtMap<CSLXSIShader*, Aftk::C_ShaderNode*>& xsiAftkShaderNodeMapping)
{
	#ifdef DEBUG_OUTPUT
	std::cout << "Displaying shader connection points:" << std::endl;
	#endif
	
	if (pShader->GetConnectionPointCount() > 0)
	{
		CSLShaderConnectionPoint** pConnectionPointList = pShader->GetConnectionPointList();

		for (int i = 0; i < pShader->GetConnectionPointCount(); i++)
		{
			CSLShaderConnectionPoint* pConnectionPoint = pConnectionPointList[i];
			CSLShaderConnectionPoint::EConnectionType connectedShaderNodeType =
				pConnectionPoint->GetConnectionType();

			if (connectedShaderNodeType == CSLShaderConnectionPoint::SI_NONE)
			{
				//Null connection point
				continue;
			}

			//Important: We may find the name of an existing plug when looking thru the connection list,
			//on the other hand we may not - nothing appears to be guaranteed. If we already have a plug
			//for the given name, fine, if not, then it must be created.

			switch (connectedShaderNodeType)
			{
				case CSLShaderConnectionPoint::SI_SHADER:
				{
					CSLXSIShader* pOtherShader = pConnectionPoint->GetShader();
					AFTK_ASSERT(pShader != 0);
					#ifdef DEBUG_OUTPUT
					std::cout << pConnectionPoint->GetName() << " -> ";
					std::cout << pOtherShader->Name().GetText() << " ";
					std::cout << "Type == SHADER";
					#endif

					Aftk::C_ShaderNode* pAftkOtherShaderNode = xsiAftkShaderNodeMapping[pOtherShader];
					Aftk::C_ShaderNode* pAftkShaderNodeToConnectTo = xsiAftkShaderNodeMapping[pShader];

					Aftk::I_Plug* pPlug = pAftkShaderNodeToConnectTo->GetPlugPtr(pConnectionPoint->GetName());
					if (pPlug)
					{
						//Modify an existing plug to connect to
						pPlug->SetSourceConnectionPtr(pAftkOtherShaderNode);
					}
					else
					{
						//We need to create a new plug on our shader to connect to
						Aftk::C_DynamicPlug* pNewPlug = new Aftk::C_DynamicPlug();
						if (!pNewPlug) { return Aftk::C_Status::E_Status_OutOfMemory;}
						pAftkShaderNodeToConnectTo->AddPlugPtr(pNewPlug);
						pNewPlug->SetName(pConnectionPoint->GetName());
						pNewPlug->SetSourceConnectionPtr(pAftkOtherShaderNode);
					}
					break;
				}

				case CSLShaderConnectionPoint::SI_IMAGE:
				{
					CSLImageLibrary* pImageLibrary = pSceneImport->GetXsiScenePtr()->GetImageLibrary();
					if (!pImageLibrary)
					{
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Could not get XSI image library.");		
					}

					CSLImage* pImage = pImageLibrary->FindImage(pConnectionPoint->GetImage());
					std::string fileName;
					if (pImage)
					{
						fileName = pImage->GetSourceFile();
					}
					#ifdef DEBUG_OUTPUT
					std::cout << pConnectionPoint->GetName() << " -> ";
					std::cout << fileName << " ";
					std::cout << "Type == IMAGE";
					#endif

					std::string imageNodeName = pConnectionPoint->GetImage();
					imageNodeName += "_image";
					
					Aftk::C_Status status;
					Aftk::C_ShaderNode* pAftkShaderImageNode = 
						ConvertAftkImageShaderNode(imageNodeName, fileName, status);
					AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

					pAftkMaterial->AddShaderNodePtr(pAftkShaderImageNode);

					Aftk::C_ShaderNode* pAftkShaderNodeToConnectTo = xsiAftkShaderNodeMapping[pShader];

					Aftk::I_Plug* pPlug = pAftkShaderNodeToConnectTo->GetPlugPtr(pConnectionPoint->GetName());
					if (pPlug)
					{
						//Modify an existing plug to connect to our image
						pPlug->SetSourceConnectionPtr(pAftkShaderImageNode);
					}
					else
					{
						//We need to create a new plug on our shader to connect to
						{
							Aftk::C_DynamicPlug* pNewPlug = new Aftk::C_DynamicPlug();
							if (!pNewPlug) { return Aftk::C_Status::E_Status_OutOfMemory;}
							pAftkShaderNodeToConnectTo->AddPlugPtr(pNewPlug);

							pNewPlug->SetName(pConnectionPoint->GetName());
							pNewPlug->SetSourceConnectionPtr(pAftkShaderImageNode);
						}
					}
					break;
				}

				default:
					break;
			}

			#ifdef DEBUG_OUTPUT
			std::cout << std::endl;
			#endif
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

