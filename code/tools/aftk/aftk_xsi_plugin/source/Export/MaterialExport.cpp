#include "XSIA3DExportPCH.h"
#include "MaterialExport.h"

namespace XSITrans
{

Aftk::C_ShaderImageNode* C_MaterialExport::CreateAftkShaderImageNode(
	Aftk::C_MaterialNode& aftkMaterial,
	XSI::ImageClip2 imageClip2,
	Ut::UtMap<
		Util::C_String, 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping)
{
	Aftk::C_ShaderImageNode* pAftkShaderImageNode = new Aftk::C_ShaderImageNode();
	if (!pAftkShaderImageNode)
		return 0;

	//Add new shader to material.
	aftkMaterial.AddShaderNodePtr(pAftkShaderImageNode);

	Util::C_String name = imageClip2.GetName().GetAsciiString();
	pAftkShaderImageNode->SetName(name.Chars());
	
	Util::C_String fileName = imageClip2.GetFileName().GetAsciiString();
	pAftkShaderImageNode->GetFilePlug().SetValue(fileName.Lower().Chars());

	//Add to shader mapping.
	xsiAftkShaderNodeMapping[name] = 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*>(imageClip2.GetRef(), pAftkShaderImageNode);

//	xsiAftkShaderNodeMapping[fileName] = 
//		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*>(imageClip2.GetRef(), pAftkShaderImageNode);

	return pAftkShaderImageNode;
}

Aftk::C_Status C_MaterialExport::BuildShaderRelationships(
	XSI::CRef shader,
	const Util::C_String& parameterName,
	XSI::CRef connectedObject,
	Aftk::C_MaterialNode& aftkMaterial,
	Ut::UtMap<
		Util::C_String, 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping)
{
	Ut::UtMap<
		Util::C_String, 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >::Iterator mappingIt;

	//Find destination node.
	XSI::Shader connectToShader(shader);
	Util::C_String connectToName = connectToShader.GetName().GetAsciiString();
	mappingIt = xsiAftkShaderNodeMapping.Find(connectToName);
	if (mappingIt == xsiAftkShaderNodeMapping.End())
		return Aftk::C_Status::E_Status_Failure;
	
	Aftk::C_ShaderNode* pAftkDestination = (*mappingIt).GetValue().second;

	//Find source node.
	XSI::SIObject connectedSiObject(connectedObject);
	Util::C_String connectedObjectName = connectedSiObject.GetName().GetAsciiString();

	mappingIt = xsiAftkShaderNodeMapping.Find(connectedObjectName);
	if (mappingIt == xsiAftkShaderNodeMapping.End())
		return Aftk::C_Status::E_Status_Failure;

	Aftk::C_ShaderNode* pAftkSource = (*mappingIt).GetValue().second;

	Aftk::I_Plug* pPlug = pAftkDestination->GetPlugPtr(
		parameterName.Chars());
	if (pPlug)
	{
		//Modify an existing plug to connect to
		pPlug->SetSourceConnectionPtr(pAftkSource);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_MaterialExport::CreateAndBuildImageRelationships(
	XSI::CRef shader,
	const Util::C_String& parameterName,
	XSI::CRef connectedObject,
	Aftk::C_MaterialNode& aftkMaterial,
	Ut::UtMap<
		Util::C_String, 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping)
{
	XSI::ImageClip2 imageClip2;
	if (connectedObject.GetClassID() == XSI::siTextureID)
	{
		XSI::Texture otherImage(connectedObject);
		imageClip2 = otherImage.GetImageClip();
	}
	else
	{
		imageClip2 = XSI::ImageClip2(connectedObject);
	}

	if (imageClip2.IsValid())
	{
		Util::C_String name = imageClip2.GetName().GetAsciiString();

		Aftk::C_ShaderNode* pAftkShaderImageNode;

		Ut::UtMap<
			Util::C_String, 
			Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >::Iterator mappingIt =
			xsiAftkShaderNodeMapping.Find(name);

		//If we've not encountered this image before, add it.
		if (mappingIt == xsiAftkShaderNodeMapping.End())
		{
			pAftkShaderImageNode = 
				CreateAftkShaderImageNode(
					aftkMaterial,
					imageClip2,
					xsiAftkShaderNodeMapping);
			if (!pAftkShaderImageNode)
				return Aftk::C_Status::E_Status_Failure;
		}
		else
		{
			pAftkShaderImageNode = (Aftk::C_ShaderImageNode*)((*mappingIt).GetValue().second);
			assert(pAftkShaderImageNode);
		}
	}

	return BuildShaderRelationships(
		shader,
		parameterName,
		connectedObject,
		aftkMaterial,
		xsiAftkShaderNodeMapping);
}

Aftk::C_Status C_MaterialExport::BuildRelationships(
	XSI::CRef shader,
	Aftk::C_MaterialNode& aftkMaterial,
	Ut::UtMap<
		Util::C_String, 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping)
{
	XSI::Shader xsiShader(shader);
	Util::C_String tmp = xsiShader.GetName().GetAsciiString();

	XSI::CParameterRefArray parameters = xsiShader.GetParameters();
	S32 nParameters = parameters.GetCount();

	for (S32 i = 0; i < nParameters; i++)
	{
		XSI::Parameter shaderParameter(parameters[i]);

		XSI::CRef connectedObject = shaderParameter.GetSource();
		if (!connectedObject.IsValid())
			continue;

		Util::C_String parameterName = shaderParameter.GetName().GetAsciiString();
		XSI::Shader otherShader(connectedObject);

		//Important: We may find the name of an existing plug when looking thru the connection list,
		//on the other hand we may not - nothing appears to be guaranteed. If we already have a plug
		//for the given name, fine, if not, then it must be created.

		XSI::siClassID classID = connectedObject.GetClassID();
	
		Aftk::C_Status status;
		switch (classID)
		{
			case XSI::siShaderID:
			{
				status = BuildShaderRelationships(
					shader,
					parameterName,
					connectedObject,
					aftkMaterial,
					xsiAftkShaderNodeMapping);
				if (!status) return status;
				break;
			}

			case XSI::siImageClipID:
			case XSI::siTextureID:
			{
				status = CreateAndBuildImageRelationships(
					shader,
					parameterName,
					connectedObject,
					aftkMaterial,
					xsiAftkShaderNodeMapping);
				 if (!status) return status;
				break;
			}

			default:
				break;
		}

		#ifdef DEBUG_OUTPUT
		std::cout << std::endl;
		#endif

		{
			XSI::CRefArray shaders = xsiShader.GetShaders();
			S32 nShaders = shaders.GetCount();

			for (S32 i = 0; i < nShaders; i++)
			{
				Aftk::C_Status status = BuildRelationships(
					otherShader, 
					aftkMaterial,
					xsiAftkShaderNodeMapping);
				if (!status)
					return status;
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status CreatePlug(
	Aftk::C_ShaderNode* pParentShader,
	Aftk::C_DynamicPlug* pParentPlug,
	XSI::CRef _parameter)
{
	XSI::Parameter parameter(_parameter);

	//If we get here we're not extracting some vector value.
	Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
	if (!pPlug) return Aftk::C_Status::E_Status_Failure;
	if (pParentShader)
	{
		assert(!pParentPlug);
		pParentShader->AddPlugPtr(pPlug);
	}
	else
	{
		assert(!pParentShader);
		pParentPlug->AddChildPtr(pPlug);
	}

	pPlug->SetName(parameter.GetName().GetAsciiString());

	XSI::CValue value = parameter.GetValue();
	switch (value.m_t)
	{
	case XSI::CValue::siBool:
		pPlug->SetValue((bool)value);
		break;
	case XSI::CValue::siUInt1:
		pPlug->SetValue((U8)value);
		break;
	case XSI::CValue::siUInt2:
		pPlug->SetValue((U16)value);
		break;
	case XSI::CValue::siUInt4:
		//XSI doesn't provide an unsigned 32-bit cast operator.
		pPlug->SetValue((U32)((S32)value));
		break;

	case XSI::CValue::siInt1:
		pPlug->SetValue((S8)value);
		break;
	case XSI::CValue::siInt2:
		pPlug->SetValue((S16)value);
		break;
	case XSI::CValue::siInt4:
		pPlug->SetValue((S32)value);
		break;

	case XSI::CValue::siDouble:
		pPlug->SetValue((F64)value);
		break;
	case XSI::CValue::siFloat:
		pPlug->SetValue((F32)value);
		break;

	case XSI::CValue::siString:
		pPlug->SetValue(std::string(((XSI::CString)value).GetAsciiString()));
		break;

	default:
		break;
	};

	XSI::CParameterRefArray parameters = parameter.GetParameters();
	S32 nParameters = parameters.GetCount();
	for (S32 i = 0; i < nParameters; i++)
	{
		Aftk::C_Status status = CreatePlug(
			0,
			pPlug,
			parameters[i]);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_MaterialExport::CreateAftkShaderNode(
	Aftk::C_MaterialNode& aftkMaterial,
	XSI::CRef _shader,
	Ut::UtMap<
		Util::C_String, 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >& xsiAftkShaderNodeMapping)
{
	XSI::Shader xsiShader(_shader);
	Util::C_String name = xsiShader.GetName().GetAsciiString();

	Ut::UtMap<
		Util::C_String, 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >::Iterator mappingIt =
		xsiAftkShaderNodeMapping.Find(name);
	if (mappingIt != xsiAftkShaderNodeMapping.End())
		//Node already exported
		return Aftk::C_Status::E_Status_Success;

	Aftk::C_Status status;

	Aftk::C_ShaderNode* pAftkShaderNode = new Aftk::C_ShaderNode();
	if (!pAftkShaderNode)
	{
		return Aftk::C_Status::E_Status_OutOfMemory;
	}

	//Add to shader mapping.
	xsiAftkShaderNodeMapping[name] = 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*>(xsiShader, pAftkShaderNode);

	//Add new shader to material.
	aftkMaterial.AddShaderNodePtr(pAftkShaderNode);

	// Xsi shader type
	Util::C_String shaderType = xsiShader.GetProgID().GetAsciiString();
	pAftkShaderNode->SetOriginalNodeTypeName( shaderType.Chars() );
	pAftkShaderNode->SetName(xsiShader.GetName().GetAsciiString());

	XSI::CParameterRefArray parameters = xsiShader.GetParameters();
	S32 nParameters = parameters.GetCount();
	for (S32 i = 0; i < nParameters; i++)
	{
		status = CreatePlug(
			pAftkShaderNode,
			0,
			parameters[i]);
		if (!status) return status;
	}

	XSI::CRefArray properties = xsiShader.GetProperties();
	S32 nProperties = properties.GetCount();
	for (S32 i = 0; i < nProperties; i++)
	{
		S32 plugStart = pAftkShaderNode->GetNumDynamicPlugs() - 1;

		XSI::CRef _property = properties[i];
		XSI::Property prop(_property);

		XSI::CParameterRefArray params = prop.GetParameters();
		S32 nParams = params.GetCount();
		for (int j = 0; j < nParams; j++)
		{
			status = CreatePlug(
				pAftkShaderNode,
				0,
				params[j]);
			if (!status) return status;
		}

		//Need to build my own compound types now... !HACK!HACK!HACK!
		if (plugStart >= 0)
		{
			//Get initial number of plugs
			S32 nPlugs = pAftkShaderNode->GetNumDynamicPlugs();
			for (S32 iPlug = plugStart; iPlug < nPlugs; iPlug++)
			{
				//Update number of plugs now that we may have added some
				nPlugs = pAftkShaderNode->GetNumDynamicPlugs();

				Aftk::C_DynamicPlug* pCurrentPlug = pAftkShaderNode->GetDynamicPlugPtr(iPlug);
				
				if (!pCurrentPlug)
					continue;

				std::string currentPlugName = pCurrentPlug->GetName();
				
				int strLength = currentPlugName.length();
				if (strLength == 0)
					continue;

				//First part of the compound plug is the 'X'
				char lastChar = currentPlugName.c_str()[strLength-1];
				char penChar = currentPlugName.c_str()[strLength-2];
				if (penChar != '_' || lastChar != 'X')
					continue;

				Aftk::C_DynamicPlug* pNewCompoundPlug = NULL;

				//Now we have the first part of the compound plug, find the rest (should be the next 3 in order)
				for (S32 iCmpPlug = plugStart; iCmpPlug < nPlugs; iCmpPlug++)
				{
					if (iCmpPlug == iPlug)
						continue;

					Aftk::C_DynamicPlug* pNextPlug = pAftkShaderNode->GetDynamicPlugPtr(iCmpPlug);

					if (!pNextPlug)
						continue;

					std::string nextPlugName = pNextPlug->GetName();
					
					int strLength2 = nextPlugName.length();
					if (strLength2 == 0)
						continue;

					//Strip the last character off so can identify if it's part of the compound plug
					char concatStr1[MAX_PATH];
					_snprintf_s(concatStr1, strLength-1, _TRUNCATE, "%s", currentPlugName.c_str());
					char concatStr2[MAX_PATH];
					_snprintf_s(concatStr2, strLength2-1, _TRUNCATE, "%s", nextPlugName.c_str());

					//Carry on if not part of the plug
					if (strcmp(concatStr1, concatStr2) != 0)
						continue;

					//If nothing been added yet
					if (!pNewCompoundPlug)
					{
						//Create the compound plug
						pNewCompoundPlug = new Aftk::C_DynamicPlug();
						pAftkShaderNode->AddPlugPtr(pNewCompoundPlug);
						pNewCompoundPlug->SetName(concatStr1);
						
						//Create a child, the first ('X') component of the plug
						Aftk::C_DynamicPlug* pNewChild = new Aftk::C_DynamicPlug(*pCurrentPlug);
						pNewCompoundPlug->AddChildPtr(pNewChild);						
					}

					//Make sure that the proceeding plugs are part of the compound type
					char lastChar2 = nextPlugName.c_str()[strLength2-1];
					char penChar2 = nextPlugName.c_str()[strLength2-2];
					if (lastChar2 != 'Y' && lastChar2 != 'Z' && lastChar2 != 'W' && penChar2 != '_')
						continue;

					//Add the next part of the compound type to the plug
					Aftk::C_DynamicPlug* pNewChild = new Aftk::C_DynamicPlug(*pNextPlug);
					pNewCompoundPlug->AddChildPtr(pNewChild);		
				}
			}
		}
	}

	//Export any connected shaders.
	{
		XSI::CRefArray shaders = xsiShader.GetShaders();
		S32 nShaders = shaders.GetCount();

		for (S32 i = 0; i < nShaders; i++)
		{
			status = CreateAftkShaderNode(
				aftkMaterial,
				shaders[i],
				xsiAftkShaderNodeMapping);
			if (!status) return status;
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_MaterialExport::Export(
	Aftk::C_MaterialNode& aftkMaterial, 
	XSI::Material& xsiMaterial)
{
	Aftk::C_Status status;
	XSI::CRefArray shaders = xsiMaterial.GetShaders();
	S32 nShaders = shaders.GetCount();

	// Set basic material parameters
	{
		aftkMaterial.SetName(xsiMaterial.GetName().GetAsciiString());
		aftkMaterial.SetMaterialTypeId(0);
		aftkMaterial.SetOrigin(Aftk::E_MaterialOrigin_XSI);
	}

	Ut::UtMap<
		Util::C_String, 
		Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> > xsiAftkShaderNodeMapping;
	// Copy shaders
	{
		for (S32 i = 0; i < nShaders; i++)
		{
			status = CreateAftkShaderNode(
				aftkMaterial,
				shaders[i],
				xsiAftkShaderNodeMapping);
			if (!status) return status;
		}
	}

	// Build relationships between shaders
	{
		for (S32 i = 0; i < nShaders; i++)
		{
			status = BuildRelationships(
				shaders[i],
				aftkMaterial,
				xsiAftkShaderNodeMapping);
		}
	}

	// Copy connections between material and shaders
	XSI::CParameterRefArray parameters = xsiMaterial.GetParameters();
	S32 nParameters = parameters.GetCount();

	//Copy plug names
	for (S32 i = 0; i < nParameters; i++)
	{
		XSI::Parameter shaderParameter(parameters[i]);
		Util::C_String parameterName = shaderParameter.GetName().GetAsciiString();

		Aftk::C_DynamicPlug* pAftkPlug = new Aftk::C_DynamicPlug();
		if (!pAftkPlug)
			return Aftk::C_Status::E_Status_OutOfMemory;
		aftkMaterial.AddPlugPtr(pAftkPlug);
		pAftkPlug->SetName(parameterName.Chars());

		//Connect plugs to shader nodes
		XSI::CRef connectedObject = shaderParameter.GetSource();
		if (!connectedObject.IsValid())
			continue;

		XSI::Shader otherShader(connectedObject);
		Ut::UtMap<
			Util::C_String, 
			Ut::Pair<XSI::CRef, Aftk::C_ShaderNode*> >::Iterator mappingIt;

		//Find source node.
		Util::C_String connectedObjectName = otherShader.GetName().GetAsciiString();
		mappingIt = xsiAftkShaderNodeMapping.Find(connectedObjectName);
		if (mappingIt == xsiAftkShaderNodeMapping.End())
			return Aftk::C_Status::E_Status_Failure;

		Aftk::C_ShaderNode* pAftkOtherShaderNode = (*mappingIt).GetValue().second;
		//We found that we had already created an AFTK shader to match that from XSI
		pAftkPlug->SetSourceConnectionPtr(pAftkOtherShaderNode);
	}

	return Aftk::C_Status::E_Status_Success;
}

}