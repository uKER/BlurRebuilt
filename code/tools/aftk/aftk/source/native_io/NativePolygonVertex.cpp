#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativePolygonVertex.h"
#include "NativeUtility.h"

using namespace Aftk;

C_Status C_NativePolygonVertex::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_PolygonVertex* pPolygonVertex, 
	const C_SceneMeshNode* pSceneMeshNode,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_POLYGON_VERTEX);
	}
	{
		//Write position index
		*pDiskWriter << pPolygonVertex->GetPositionIndex();

		//Write normal index
		*pDiskWriter << pPolygonVertex->GetNormalIndex();

		AFTK_ASSERT(pSceneMeshNode->GetNumVertexDataChannels() == 
			pPolygonVertex->GetNumData());

		//Write out vertex data channels
		*pDiskWriter << (U32)pPolygonVertex->GetNumData();
		for (U32 iChannel = 0; iChannel < pPolygonVertex->GetNumData(); iChannel++)
		{
			U32 dataSizeInBytes = GetDataTypeSize(
				pSceneMeshNode->GetVertexDataChannelInfo(iChannel).GetDataType());
			const U8* pData = (U8*)pPolygonVertex->GetDataPtr(iChannel);
			for (U32 jByte = 0; jByte < dataSizeInBytes; jByte++)
			{
				*pDiskWriter << pData[jByte];
			}
		}

		//Write texture coordinate indices
		//Store the number of texture coordinate indices.
		AFTK_ASSERT(
			pPolygonVertex->GetNumTextureCoordinateIndices() ==
			pSceneMeshNode->GetNumTextureCoordinateSets());
		
		U32 uvSetCount = (U32)pSceneMeshNode->GetNumTextureCoordinateSets();
		if (pPolygonVertex->GetNumTextureCoordinateIndices() !=
			uvSetCount)
		{
			LOGDEBUG(g_aftkMainLogger, "PV uv set count mismatch writing PV: %i Mesh: %i - clamping to count on mesh %s UniqueID %i.\n",
				pPolygonVertex->GetNumTextureCoordinateIndices(), uvSetCount, pSceneMeshNode->GetName().c_str(), pSceneMeshNode->GetUniqueId());
		}

		*pDiskWriter << (U32)uvSetCount;

		for (U32 i = 0; i < uvSetCount; i++)
		{
			//Write texture coordinate indices.
			*pDiskWriter << (S32)pPolygonVertex->GetTextureCoordinateIndex(i);

			//Write tangent index
			*pDiskWriter << pPolygonVertex->GetTangentIndex(i);

			//Write binormal index
			*pDiskWriter << pPolygonVertex->GetBinormalIndex(i);
		}

	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}

	return C_Status::E_Status_Success;
}

C_Status C_NativePolygonVertex::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_PolygonVertex* pPolygonVertex,
	const C_SceneMeshNode* pSceneMeshNode,
	bool useOuterBlockHeaders)
{
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_POLYGON_VERTEX))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find polygon vertex block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		//Read in position index
		{
			S32 index;
			*pDiskReader >> index;
				pPolygonVertex->SetPositionIndex(index);
		}

		//Read in normal index
		{
			S32 index;
			*pDiskReader >> index;
			pPolygonVertex->SetNormalIndex(index);
		}

		{
			U32 numChannels;
			if (pDiskReader->GetVersion() < 205)
			{
				U8 tmp;
				*pDiskReader >> tmp;
				numChannels = tmp;
			}
			else
			{
				*pDiskReader >> numChannels;
			}

			for (U32 iChannel = 0; iChannel < numChannels; iChannel++)
			{
				//Prior to v1.33 we were using 32bpp packed vertex colour information
				if (pDiskReader->GetVersion() < 133)
				{
					Vector4F32 colour;
					*pDiskReader >> colour;
					pPolygonVertex->SetData(iChannel, colour);
				}
				else
				{
					if (pDiskReader->GetVersion() >= 150)
					{
						U32 dataSizeInBytes = GetDataTypeSize(
							pSceneMeshNode->GetVertexDataChannelInfo(iChannel).GetDataType());
						U8* pTemp = new U8[dataSizeInBytes];
						
						pDiskReader->ReadBytes(pTemp, dataSizeInBytes);

						pPolygonVertex->SetData(iChannel,
							pSceneMeshNode->GetVertexDataChannelInfo(iChannel).GetDataType(), pTemp);
						delete[] pTemp;
					}
					else
					{
						U32 integer; 
						*pDiskReader >> integer;
						Vector4F32 temp(ConvertU32ToVector4F32(integer));
						pPolygonVertex->SetData(iChannel, temp);
					}
				}
			}
		}

		//Read in texture coordinate indices
		//Store the number of texture coordinate indices.
		U32 numTextureCoordinates;
		if (pDiskReader->GetVersion() < 205)
		{
			U8 tmp;
			*pDiskReader >> tmp;
			numTextureCoordinates = tmp;
		}
		else
		{
			*pDiskReader >> numTextureCoordinates;
		}

		AFTK_ASSERT(
			numTextureCoordinates ==
			pSceneMeshNode->GetNumTextureCoordinateSets());
		
		U32 uvSetCount = (U32)pSceneMeshNode->GetNumTextureCoordinateSets();
		if (numTextureCoordinates !=
			(U32)pSceneMeshNode->GetNumTextureCoordinateSets())
		{
			LOGFATAL(g_aftkMainLogger, "PV uv set count mismatch reading PV: %i Mesh: %i. This is very likley to trigger a crash.\n",
				numTextureCoordinates, 
				uvSetCount, 
				pSceneMeshNode->GetName().c_str(), 
				pSceneMeshNode->GetUniqueId());
		}

		for (U32 set = 0; set < numTextureCoordinates; set++)
		{
			S32 uvwxIndex;
			*pDiskReader >> uvwxIndex;

			if (pDiskReader->GetVersion() < 150)
			{
				pPolygonVertex->SetTextureCoordinateIndex(set, uvwxIndex);
			}
			else
			{
				//Read in tangent index
				S32 tangentIndex;
				*pDiskReader >> tangentIndex;

				//Read in binormal index
				S32 binormalIndex;
				*pDiskReader >> binormalIndex;
				
				pPolygonVertex->SetTextureCoordinateSetIndices(
					set,
					uvwxIndex, 
					tangentIndex, 
					binormalIndex);
			}
		}
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	
	return C_Status::E_Status_Success;
}


