#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeMaterialNode.h"
#include "NativeGenericAttribute.h"
#include "NativeUtility.h"
#include "NativeIPlug.h"
#include "NativeDynamicPlug.h"
#include "NativeShaderNode.h"
#include "NativeShaderImageNode.h"

using namespace Aftk;

C_Status C_NativeMaterialNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	S32 mtlIndex,
	const C_MaterialNode* pMaterialNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	// Material
	if (useOuterBlockHeaders)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_MATERIAL_NODE);
	}
	{
		//Write out the fragment of this material node that comes from C_Node
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeNode::Serialise(
			pDiskWriter, 
			pNativeFtk, 
			pMaterialNode, 
			false);
		if (!status) return status;

		//Padding, this used to be the material index but it is now out of use.
		*pDiskWriter << mtlIndex;

		//Only write out the proporties defined within AFTKMATERIALNODE.
		//Write out where this material came from;
		*pDiskWriter << pMaterialNode->GetOrigin();

		*pDiskWriter << pMaterialNode->GetMaterialTypeId();

		//Write out plugs
		{
			//num plugs
			U32 numPlugs = (U32)pMaterialNode->GetNumPlugs();
			//Store the number of plugs
			*pDiskWriter << numPlugs;
			//material attributes
			for (U32 i = 0; i < numPlugs; i++) 
			{
				const C_DynamicPlug* pPlug = pMaterialNode->GetPlugPtr(i);

				status = C_NativeDynamicPlug::Serialise(
					pDiskWriter,
					pNativeFtk,
					pPlug);
				if (!status) return status;
			}
		}
		
		//Write out shaders
		{
			U32 numShaders = (U32)pMaterialNode->GetNumShaderNodes();
			//Store the number of maps
			*pDiskWriter << numShaders;
			for (U32 i = 0; i < numShaders; i++) 
			{
				const C_ShaderNode* pShaderNode = 
					pMaterialNode->GetShaderNodePtr(i);

				switch (pShaderNode->GetType())
				{
					case E_ObjType_Shader:
					{
						status = C_NativeShaderNode::Serialise(
							pDiskWriter, pNativeFtk, 
							pShaderNode, 
							true);
						if (!status) return status;
						break;
					}
					case E_ObjType_ShaderImage:
					{
						status = C_NativeShaderImageNode::Serialise(
							pDiskWriter, 
							pNativeFtk, 
							(C_ShaderImageNode*)pShaderNode, 
							true);
						if (!status) return status;
						break;
					}
					default:
						//Unknown node type
						AFTK_ASSERT(false);
						break;
				};
			}
		}
	}
	if (useOuterBlockHeaders)
		pDiskWriter->EndBlock(); // end of MTL

	return C_Status::E_Status_Success;
}

C_Status C_NativeMaterialNode::DeserialiseLegacy(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk,
	C_MaterialNode* pMaterialNode)
{
	pMaterialNode->SetOrigin(E_MaterialOrigin_Maya);
	//Only write out the proporties defined within AFTK MATERIAL NODE.
	if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_GENERIC_ATTRIBUTE_LIST))
	{
		return C_Status(C_Status::E_Status_Failure, "Failed to find generic attribute list block.");
	}
	pDiskReader->EnterBlock();
	pDiskReader->ExitBlock();

	if (!pDiskReader->FindBlock(C_NativeUtility::GetBlockIdInt("MAP ")))
	{
		return C_Status(C_Status::E_Status_Failure, "Failed to find map block.");
	}
	pDiskReader->EnterBlock();
	{
		U32 numMaps;
		//Store the number of maps
		*pDiskReader >> numMaps;

		for (U32 i = 0; i < numMaps; i++) 
		{
			pDiskReader->EnterBlock();
			{
				C_Status status;
				C_ShaderImageNode* pImageNode = new C_ShaderImageNode(&status);
				if (!pImageNode)
					return C_Status::E_Status_OutOfMemory;
				if (!status)
					return status;
				pMaterialNode->AddShaderNodePtr(pImageNode);
				{
					std::string temp;
					*pDiskReader >> temp;
					pImageNode->SetName(temp);
				}
				{
					std::string temp;
					*pDiskReader >> temp;
					pImageNode->GetFilePlug().SetValue(temp);
				}
				{
					U32 type;
					*pDiskReader >> type;
					if (type != 0)
					{
						pImageNode->GetImageTypePlug().SetValue("lightmap0");
					}
				}
				{
					C_ShaderNode* pLambert = new C_ShaderNode;
					pMaterialNode->AddShaderNodePtr(pLambert);
					pLambert->SetName("lambert1");
					
					{
						C_DynamicPlug* pColourPlug = new C_DynamicPlug();
						if (!pColourPlug)
							return C_Status::E_Status_OutOfMemory;
						pImageNode->AddPlugPtr(pColourPlug);

						std::string shaderPlugName = "outColor";
						pColourPlug->SetName(shaderPlugName);

						C_DynamicPlug* pPlug = new C_DynamicPlug();
						if (!pPlug)
							return C_Status::E_Status_OutOfMemory;
						pLambert->AddPlugPtr(pPlug);

						pPlug->SetName("color");
						pPlug->SetSourceConnectionPtr(pImageNode); 
					}
					
					{
						C_DynamicPlug* pSurfacePlug = new C_DynamicPlug();
						if (!pSurfacePlug)
							return C_Status::E_Status_OutOfMemory;
						pMaterialNode->AddPlugPtr(pSurfacePlug);

						pSurfacePlug->SetName("mayaSurfaceShader");
						pSurfacePlug->SetSourceConnectionPtr(pLambert); 
					}
				}
			}			
			pDiskReader->ExitBlock();
		}
	}
	pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeMaterialNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk,
	C_MaterialNode* pMaterialNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_MATERIAL_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find material node block.");
		pDiskReader->EnterBlock();
	}
	{
		//Read the fragment of this material node that comes from C_Node
		status = C_NativeNode::Deserialise(
			pDiskReader, 
			pNativeFtk, 
			pMaterialNode, 
			false);
		if (!status) return status;

		if (pDiskReader->GetVersion() < 124)
		{
			//Read legacy materials
			status = DeserialiseLegacy(
				pDiskReader,
				pNativeFtk,
				pMaterialNode);
			if (!status) return status;
		}
		else
		{
			if (pDiskReader->GetVersion() > 126)
			{
				S32 index;
				*pDiskReader >> index;
			}

			U32 origin;
			*pDiskReader >> origin;
			pMaterialNode->SetOrigin((E_MaterialOrigin)origin);

			U32 id;
			*pDiskReader >> id;
			pMaterialNode->SetMaterialTypeId(id);

			if (pDiskReader->GetVersion() < 200)
			{
				if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_PLUG_LIST))
					return C_Status(C_Status::E_Status_Failure, "Failed to find plug list block.");
				pDiskReader->EnterBlock();
			}
			//Read plug list
			{
				//num plugs
				U32 numPlugs;
				//Store the number of plugs
				*pDiskReader >> numPlugs;
				pMaterialNode->SetPlugInitialCapacity(numPlugs);
				//material attributes
				for (U32 i = 0; i < numPlugs; i++) 
				{
					C_DynamicPlug* pPlug = new C_DynamicPlug();
					if (!pPlug)
						return C_Status::E_Status_OutOfMemory;
					pMaterialNode->AddPlugPtr(pPlug);

					status = C_NativeDynamicPlug::Deserialise(
						pDiskReader, 
						pNativeFtk, 
						pPlug);

					if (!status) return status;
				}
			}
			if (pDiskReader->GetVersion() < 200)
				pDiskReader->ExitBlock();

			if (pDiskReader->GetVersion() < 200)
			{
				if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_SHADER_LIST))
					return C_Status(C_Status::E_Status_Failure, "Failed to find shader list block.");
				pDiskReader->EnterBlock();
			}
			{
				U32 numShaders;
				*pDiskReader >> numShaders;
				pMaterialNode->SetShaderNodeInitialCapacity(numShaders);
				for (U32 i = 0; i < numShaders; i++) 
				{
					U32 blockId = pDiskReader->GetNextBlockInfo();
					if (blockId == NATIVE_BLOCK_ID_SHADER_NODE)
					{
						C_ShaderNode* pShaderNode = new C_ShaderNode();
						if (!pShaderNode)
							return C_Status::E_Status_OutOfMemory;
						pMaterialNode->AddShaderNodePtr(pShaderNode);
						status = C_NativeShaderNode::Deserialise(
							pDiskReader, 
							pNativeFtk, 
							pShaderNode, 
							true);
						if (!status) return status;
					}
					else if (blockId == NATIVE_BLOCK_ID_SHADER_IMAGE_NODE)
					{
						C_ShaderImageNode* pShaderImageNode = new C_ShaderImageNode(&status);
						if (!pShaderImageNode)
							return C_Status::E_Status_OutOfMemory;
						if (!status)
							return status;
						pMaterialNode->AddShaderNodePtr(pShaderImageNode);
						status = C_NativeShaderImageNode::Deserialise(
							pDiskReader, 
							pNativeFtk, 
							pShaderImageNode, 
							true);
						if (!status) return status;
					}
					else
					{
						//Unknown node type
						AFTK_ASSERT(false);
						break;
					};
				}
			}
			if (pDiskReader->GetVersion() < 200)
				pDiskReader->ExitBlock();
		}
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock(); // end of MTL
	}

	return C_Status::E_Status_Success;
}

