#include "StdAfx.h"

#include "XsiIoMaterialExport.h"
#include "XsiIoSceneExport.h"

using namespace XsiIo;

//remove invalid characters to prevent xsi from falling over
std::string ValidateName(const std::string& inName)
{
	std::string fixedName = inName;
	std::replace(fixedName.begin(), fixedName.end(), ':', '_');

	return fixedName;
}

Aftk::C_Status C_MaterialExport::Export(
	C_SceneExport *pExporter,
	const Aftk::C_MaterialResource *pAftkMaterialResource,
	CSLBaseMaterial *pXsiBaseMaterial)
{
	AFTK_ASSERT( pExporter );
	AFTK_ASSERT( pAftkMaterialResource );
	AFTK_ASSERT( pXsiBaseMaterial );

	CSLTemplate::ETemplateType type = pXsiBaseMaterial->Type();

	// Create image library
	CSLScene* pXsiScene = pExporter->GetXsiScene();
	if( !pXsiScene )
		return Aftk::C_Status::E_Status_Failure;

	if( !pXsiScene->GetImageLibrary() )
	{
		if( !pXsiScene->CreateImageLibrary() )
			return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi image library");
	}

	if (type == CSLTemplate::XSI_MATERIAL)
	{
		Aftk::C_MaterialNode aftkMaterial = pAftkMaterialResource->GetContent();
		if( aftkMaterial.GetOrigin() == Aftk::E_MaterialOrigin_XSI ||
			aftkMaterial.GetOrigin() == Aftk::E_MaterialOrigin_SI)
		{
			return ExportXsiMaterial( pExporter, pAftkMaterialResource, (CSLXSIMaterial*) pXsiBaseMaterial);
		}
		else
		{
			return ExportDummyMaterial( pExporter, pAftkMaterialResource, (CSLXSIMaterial*) pXsiBaseMaterial);
		}
	}

	return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Exporting legacy SI_Material is unsupported");

}


Aftk::C_Status C_MaterialExport::ExportDummyMaterial(
	XsiIo::C_SceneExport *pExporter,
	const Aftk::C_MaterialResource *pAftkMaterialResource,
	CSLXSIMaterial *pXsiMaterial)
{

	CSLXSIShader *pShaderToUse = 0;

	Aftk::C_MaterialNode aftkMaterial = pAftkMaterialResource->GetContent();
	pXsiMaterial->Name().SetText( ValidateName(aftkMaterial.GetName()).c_str() );

	Ut::UtMap<const Aftk::C_Node*, CSLXSIShader*> aftkXsiShaderNodeMapping;	

	// Create shader nodes
	{
		U32 nShaders = aftkMaterial.GetNumShaderNodes();
		for( U32 i=0; i<nShaders; ++i )
		{
			const Aftk::C_ShaderNode *pAftkShader = aftkMaterial.GetShaderNodePtr( i );
		
			CSLXSIShader *pXsiShader = pXsiMaterial->AddShader();
			if( !pXsiShader )
				return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi shader node");
				
			pXsiShader->SetOutputType( CSLXSIShader::SI_OT_COLOR );		// set shader output to colour
			aftkXsiShaderNodeMapping[ pAftkShader ] = pXsiShader;
			pXsiShader->Name().SetText( ValidateName(pAftkShader->GetName()).c_str() );

			{
				CSLVariantParameter *pShaderParameter = pXsiShader->AddParameter();
				if( !pShaderParameter )
					return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi shader parameter" );

				pShaderParameter->SetName( "Name" );
				SI_TinyVariant variantValue;
				variantValue.variantType = SI_VT_PCHAR;
				variantValue.p_cVal = pXsiShader->Name().GetText();
				pShaderParameter->SetValue( &variantValue );
			}

			if( pAftkShader->GetType() == Aftk::E_ObjType_ShaderImage )
			{
				// Add image to Xsi image library
				const Aftk::C_ShaderImageNode *pAftkImageShader = (const Aftk::C_ShaderImageNode*) pAftkShader;
				CSLScene* pXsiScene = pExporter->GetXsiScene();
				CSLImageLibrary *pImageLibrary = pXsiScene->GetImageLibrary();

				CSLImage *pImage = pImageLibrary->AddImage();
				pImage->SetName( (char*) ValidateName(pAftkShader->GetName()).c_str() );

				std::string imageSource;
				pAftkImageShader->GetFilePlug().GetValue( imageSource );
				pImage->SetSourceFile( (char*) imageSource.c_str() );

				pXsiShader->SetProgID( "Softimage.txt2d-image-explicit.1" );
				CSLShaderConnectionPoint *pTextureConnection = pXsiShader->AddConnectionPoint();
				if( !pTextureConnection )
					return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi shader connection" );

				pTextureConnection->SetName( "tex" );
				pTextureConnection->SetImage( (char*) ValidateName(pAftkShader->GetName()).c_str() );
			}
			else if( pAftkShader->GetType() == Aftk::E_ObjType_Shader )
			{
				pXsiShader->SetProgID( "Softimage.material-phong.1" );
				if( !pShaderToUse )
					pShaderToUse = pXsiShader;
			}
		}
	}

	// Find shader node connections to imageshader nodes
	{
		U32 nShaders = aftkMaterial.GetNumShaderNodes();
		for( U32 i=0; i<nShaders; ++i )
		{
			const Aftk::C_ShaderNode *pAftkShader = aftkMaterial.GetShaderNodePtr( i );
			if( pAftkShader->GetType() == Aftk::E_ObjType_Shader )
			{
				U32 nDynamicPlugs = pAftkShader->GetNumDynamicPlugs();
				for( U32 j=0; j<nDynamicPlugs; ++j )
				{
					const Aftk::C_DynamicPlug *pDynamicPlug = pAftkShader->GetDynamicPlugPtr( i );
					if( const Aftk::I_Plug* pSourceConnection = (const Aftk::I_Plug*) pDynamicPlug->GetSourceConnectionPtr() )
					{
						if( CSLXSIShader *pXsiShader = aftkXsiShaderNodeMapping[ pAftkShader ] )
						{
							CSLShaderConnectionPoint *pAmbientConnection = pXsiShader->AddConnectionPoint();
							if( !pAmbientConnection )
								return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi shader connection" );

							CSLXSIShader *pXsiImageShader = aftkXsiShaderNodeMapping[ (Aftk::C_ShaderNode*) pSourceConnection->GetParentPtr() ];
							if( pXsiImageShader )
							{
								pAmbientConnection->SetName( "ambient" );
								pAmbientConnection->SetShader( pXsiImageShader );

								CSLShaderConnectionPoint *pDiffuseConnection = pXsiShader->AddConnectionPoint();
								if( !pDiffuseConnection )
									return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi shader connection" );

								pDiffuseConnection->SetName( "diffuse" );
								pDiffuseConnection->SetShader( pXsiImageShader );

								break;
							}
						}	
					}
				}
			}
		}
	}

	// Set up material-shader connections
	{
		CSLConnectionPoint *pSurfaceShader = pXsiMaterial->AddConnectionPoint();
		if( !pSurfaceShader )
			return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi material connection point" );

		pSurfaceShader->SetName( "surface" );
		pSurfaceShader->SetShader( pShaderToUse );

		CSLConnectionPoint *pShadowShader = pXsiMaterial->AddConnectionPoint();
		if( !pShadowShader )
			return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi material connection point" );

		pShadowShader->SetName( "shadow" );
		pShadowShader->SetShader( pShaderToUse );

		CSLConnectionPoint *pPhotonShader = pXsiMaterial->AddConnectionPoint();
		if( !pPhotonShader )
			return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi material connection point" );

		pPhotonShader->SetName( "Photon" );
		pPhotonShader->SetShader( pShaderToUse );

	}
	#if 0 
	{
		CSLXSIMaterialInfo *pXsiMaterialInfo = pXsiMaterial->CreateMaterialInfo();
		pXsiMaterialInfo->SetUWrap( CSLXSIMaterialInfo::SI_REPEAT );
		pXsiMaterialInfo->SetVWrap( CSLXSIMaterialInfo::SI_REPEAT );
	}
	#endif
	return Aftk::C_Status::E_Status_Success;
}




Aftk::C_Status C_MaterialExport::ExportXsiMaterial(
	XsiIo::C_SceneExport *pExporter,
	const Aftk::C_MaterialResource *pAftkMaterialResource,
	CSLXSIMaterial *pXsiMaterial)
{
	// Set material name
	Aftk::C_MaterialNode aftkMaterial = pAftkMaterialResource->GetContent();
	pXsiMaterial->Name().SetText( ValidateName(aftkMaterial.GetName()).c_str() );

	Ut::UtMap<const Aftk::C_Node*, CSLXSIShader*> aftkXsiShaderNodeMapping;	

	// 1st pass: create shader nodes so we can set up connections between them
	{
		U32 nShaders = aftkMaterial.GetNumShaderNodes();

		for( U32 i=0; i<nShaders; ++i )
		{
			const Aftk::C_ShaderNode *pAftkShader = aftkMaterial.GetShaderNodePtr( i );
			if( pAftkShader->GetType() == Aftk::E_ObjType_ShaderImage )
			{
			}
			else if( pAftkShader->GetType() == Aftk::E_ObjType_Shader )
			{
				CSLXSIShader *pXsiShader = pXsiMaterial->AddShader();
				if( !pXsiShader )
					return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi shader node");
				
				pXsiShader->SetOutputType( CSLXSIShader::SI_OT_COLOR );		// set shader output to colour
				aftkXsiShaderNodeMapping[ pAftkShader ] = pXsiShader;

				pXsiShader->Name().SetText( ValidateName(pAftkShader->GetName()).c_str() );
				pXsiShader->SetProgID( (char*) pAftkShader->GetOriginalNodeTypeName().c_str() );
			}
		}
	}

	// 2nd pass: fill shader data
	{
		U32 nShaders = aftkMaterial.GetNumShaderNodes();

		for( U32 i=0; i<nShaders; ++i )
		{
			const Aftk::C_ShaderNode *pAftkShader = aftkMaterial.GetShaderNodePtr( i );
			if( pAftkShader->GetType() == Aftk::E_ObjType_ShaderImage )
			{
				const Aftk::C_ShaderImageNode *pAftkImageShader = (const Aftk::C_ShaderImageNode*) pAftkShader;
				CSLScene* pXsiScene = pExporter->GetXsiScene();
				CSLImageLibrary *pImageLibrary = pXsiScene->GetImageLibrary();

				CSLImage *pImage = pImageLibrary->AddImage();
				pImage->SetName( (char*) ValidateName(pAftkShader->GetName()).c_str() );

				std::string imageSource;
				pAftkImageShader->GetFilePlug().GetValue( imageSource );
				pImage->SetSourceFile( (char*) imageSource.c_str() );
			}
			else if( pAftkShader->GetType() == Aftk::E_ObjType_Shader )
			{
				Aftk::C_Status status = ExportXsiShader( aftkXsiShaderNodeMapping[ pAftkShader ], pAftkShader, aftkXsiShaderNodeMapping);
				AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
			}
		}
	}

	// Connect shaders to material
	U32 nDynamicMaterialPlugs = aftkMaterial.GetNumPlugs();
	for( U32 i=0; i<nDynamicMaterialPlugs; ++i )
	{
		const Aftk::C_DynamicPlug *pMaterialPlug = aftkMaterial.GetPlugPtr( i );
		CSLConnectionPoint *pConnectionPoint = pXsiMaterial->AddConnectionPoint();
		if( !pConnectionPoint )
			return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi material connection point" );
		
		pConnectionPoint->SetName( (char*) ValidateName(pMaterialPlug->GetName()).c_str() );
		const Aftk::C_Node *pShader = (const Aftk::C_Node*) pMaterialPlug->GetSourceConnectionPtr();
		if( pShader )
			pConnectionPoint->SetShader( aftkXsiShaderNodeMapping[ pShader ] );
	}

	return Aftk::C_Status::E_Status_Success;
}



Aftk::C_Status C_MaterialExport::ExportXsiShader(
	CSLXSIShader *pXsiShader,
	const Aftk::C_ShaderNode *pAftkShader,
	Ut::UtMap<const Aftk::C_Node*, CSLXSIShader*>& aftkXsiShaderNodeMapping)
{
	Aftk::C_Status status;

	U32 nDynamicPlugs = pAftkShader->GetNumDynamicPlugs();

	for( U32 i=0; i<nDynamicPlugs; ++i )
	{
		const Aftk::C_DynamicPlug *pDynamicPlug = pAftkShader->GetDynamicPlugPtr( i );

		Aftk::E_GenericAttributeType type = pDynamicPlug->GetDataType();

		if( pDynamicPlug->GetSourceConnectionPtr() )
		{
			// Shader connection
			CSLShaderConnectionPoint *pShaderConnection = pXsiShader->AddConnectionPoint();
			if( !pShaderConnection )
				return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi shader connection" );

			status = ExportXsiShaderConnection( pShaderConnection, pDynamicPlug, aftkXsiShaderNodeMapping);
			AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
		}
		else
		{
			// Shader attribute
			U32 nChildren = pDynamicPlug->GetNumChildren();
			if( nChildren > 0 )
			{
				// Compound parameter
				const std::string parentName = pDynamicPlug->GetName();
				for( U32 j=0; j<nChildren; ++j )
				{
					CSLVariantParameter *pShaderParameter = pXsiShader->AddParameter();
					if( !pShaderParameter )
						return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi shader parameter" );

					status = ExportXsiShaderParameter( pShaderParameter, pDynamicPlug->GetChildPtr( j ) );
					AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
				}
			}
			else
			{
				CSLVariantParameter *pShaderParameter = pXsiShader->AddParameter();
				if( !pShaderParameter )
					return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi shader parameter" );

				status = ExportXsiShaderParameter( pShaderParameter, pDynamicPlug );
				AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}



Aftk::C_Status C_MaterialExport::ExportXsiShaderConnection(
	CSLShaderConnectionPoint *pShaderConnection,
	const Aftk::I_Plug *pPlug,
	Ut::UtMap<const Aftk::C_Node*, CSLXSIShader*>& aftkXsiShaderNodeMapping)
{

	pShaderConnection->SetName( (char*)ValidateName(pPlug->GetName()).c_str() );
	const Aftk::C_Node *connectedPlug = (const Aftk::C_Node*) pPlug->GetSourceConnectionPtr();

	switch( connectedPlug->GetType() )
	{
		case Aftk::E_ObjType_Shader:
			pShaderConnection->SetShader( aftkXsiShaderNodeMapping[ (const Aftk::C_Node*) connectedPlug ] );
			break;
		case Aftk::E_ObjType_ShaderImage:
			pShaderConnection->SetImage( (char*) ValidateName(connectedPlug->GetName()).c_str() );
			break;
		default:
			break;
	}

	return Aftk::C_Status::E_Status_Success;
}



Aftk::C_Status C_MaterialExport::ExportXsiShaderParameter(
	CSLVariantParameter *pShaderParameter,
	const Aftk::I_Plug *pPlug)
{
	// Build hierarchical parameter name
	{
		std::string parameterName = pPlug->GetName();
		const Aftk::I_Plug* pParentPlug = (const Aftk::I_Plug*) pPlug->GetParentPtr();
		
		if( parameterName.size() == 1 )
		{
			if( parameterName == "r")
				parameterName = "red";
			else if ( parameterName == "g" )
				parameterName = "green";
			else if ( parameterName == "b" )
				parameterName = "blue";
			else if ( parameterName == "a" )
				parameterName = "alpha";
		}

		/*
		while( pParentPlug )
		{
			parameterName = pParentPlug->GetName() + "." + parameterName;
			pPlug = (const Aftk::I_Plug*) pPlug->GetParentPtr();
		}
		*/
		// Only need name of direct parent for compound types
		if( pParentPlug && pParentPlug->GetParentPtr() )
		{
			parameterName = pParentPlug->GetName() + "." + parameterName;
		}

		pShaderParameter->SetName( (char*) parameterName.c_str() );
	}

	SI_TinyVariant variantValue;
	switch( pPlug->GetDataType() )
	{
		case Aftk::E_GenericAttributeType_Boolean:			//SI_VT_BOOL
			variantValue.variantType = SI_VT_BOOL;
			pPlug->GetValue( *((bool*)(&variantValue.boolVal)) );
			pShaderParameter->SetValue( &variantValue );
			break;
		case Aftk::E_GenericAttributeType_S8:				//SI_VT_BYTE
			variantValue.variantType = SI_VT_BYTE;
			pPlug->GetValue( *((S8*)(&variantValue.bVal)) );	// char == signed __int8 ?
			pShaderParameter->SetValue( &variantValue );
			break;
		case Aftk::E_GenericAttributeType_F64:				//SI_VT_DOUBLE
			variantValue.variantType = SI_VT_DOUBLE;
			pPlug->GetValue( variantValue.dVal );
			pShaderParameter->SetValue( &variantValue );
			break;
		case Aftk::E_GenericAttributeType_F32:				//SI_VT_FLOAT
			variantValue.variantType = SI_VT_FLOAT;
			pPlug->GetValue( variantValue.fVal );
			pShaderParameter->SetValue( &variantValue );
			break;
		case Aftk::E_GenericAttributeType_U16:
		case Aftk::E_GenericAttributeType_S32:				//SI_VT_INT
			variantValue.variantType = SI_VT_INT;
			pPlug->GetValue( variantValue.nVal );
			pShaderParameter->SetValue( &variantValue );
			break;
		case Aftk::E_GenericAttributeType_U32:
		case Aftk::E_GenericAttributeType_S64:				//SI_VT_LONG
			variantValue.variantType = SI_VT_LONG;
			pPlug->GetValue( variantValue.lVal );
			pShaderParameter->SetValue( &variantValue );
			break;
		case Aftk::E_GenericAttributeType_S16:				//SI_VT_SHORT
			variantValue.variantType = SI_VT_SHORT;
			pPlug->GetValue( variantValue.sVal );
			pShaderParameter->SetValue( &variantValue );
			break;
		case Aftk::E_GenericAttributeType_U64:				//SI_VT_ULONG
			variantValue.variantType = SI_VT_ULONG;
			pPlug->GetValue( variantValue.ulVal );
			pShaderParameter->SetValue( &variantValue );
			break;
		case Aftk::E_GenericAttributeType_U8:				//SI_VT_UBYTE
			variantValue.variantType = SI_VT_UBYTE;
			pPlug->GetValue( variantValue.ubVal );
			pShaderParameter->SetValue( &variantValue );
			break;
		case Aftk::E_GenericAttributeType_String:			//SI_VT_PCHAR
		{
			variantValue.variantType = SI_VT_PCHAR;
			std::string sValue;
			pPlug->GetValue( sValue );
			variantValue.p_cVal = (char *) sValue.c_str();
			pShaderParameter->SetValue( &variantValue );
			break;
		}
		default:
			//return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Encountered unknown shader parameter");
			break;
	}

	return Aftk::C_Status::E_Status_Success;
}