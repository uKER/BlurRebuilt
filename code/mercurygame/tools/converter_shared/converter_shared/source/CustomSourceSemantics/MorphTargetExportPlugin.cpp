#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include <aftk/Aftk.h>
#include <fx/Effect.h>

#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConstantAnnotationReader.h"
#include "converter_shared\renderdataconversion\VtxInputSourceSemantics.h"
#include "converter_shared/CustomSourceSemantics/MorphTargetExportPlugin.h"
#include "converter_shared/CustomSourceSemantics/MorphTargetProcessor.h"
#include "converter_shared/renderdataconversion/ConvertAftkMesh.h"

namespace MeshDeform
{

namespace MorphTarget
{

/* 
===============================================================
					class VtxHandler

NOTES: Exporter plugin for exporting morph target data
===============================================================
*/
class VtxHandler : public RenderDataConversion::DefaultVtxInputSourceSemantics
{
public:
	//! Constructor
	VtxHandler(RenderDataConversion::VtxInputAnnotationReader *pParent) :
		RenderDataConversion::DefaultVtxInputSourceSemantics(pParent)
	{
	}

	//! Parse the given source semantic and return true if we recognise it
	bool IdentifySourceSemantic(const char *pszSource)
	{
		if (!stricmp(pszSource,"positionoffset"))
		{
			m_Type = E_Type_PositionOffset;
			return true;
		}
		if (!stricmp(pszSource,"normaloffset"))
		{
			m_Type = E_Type_NormalOffset;
			return true;
		}
		return false;
	}

	//! Get the data for the given vertex
	bool GetData(const RenderDataConversion::AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, float *ppData)
	{
		const Aftk::C_SceneMeshNode* pSourceMesh = pMeshConv->GetMeshNode();
		
		const Aftk::C_Polygon& poly = pSourceMesh->GetPolygon(0, iPolygon);
		const Aftk::C_PolygonVertex& pv = poly.GetPolygonVertex(iVtx);

		for (U32 iChannel = 0; iChannel < pSourceMesh->GetNumVertexDataChannels(); iChannel++)
		{
			const Aftk::C_VertexDataChannelInfo& info = pSourceMesh->GetVertexDataChannelInfo(iChannel);

			if (m_Type == E_Type_PositionOffset && info.GetName() == "positionOffset")
			{
				if (info.GetDataType() == Aftk::E_GenericAttributeType_Vector4F32)
				{
					Aftk::Vector4F32 data;
					pv.GetData(data, iChannel);

					ppData[0] = data.x;
					ppData[1] = data.y;
					ppData[2] = data.z;
					ppData[3] = 0.0f;
					ppData++;

					return true;
				}				
			}
			else if (m_Type == E_Type_NormalOffset && info.GetName() == "normalOffset")
			{
				if (info.GetDataType() == Aftk::E_GenericAttributeType_Vector4F32)
				{
					Aftk::Vector4F32 data;
					pv.GetData(data, iChannel);

					ppData[0] = data.x;
					ppData[1] = data.y;
					ppData[2] = data.z;
					ppData[3] = 0.0f;
					ppData++;

					return true;
				}
			}
		}

		ppData[0] = 0.0f;
		ppData[1] = 0.0f;
		ppData[2] = 0.0f;
		ppData[3] = 0.0f;
		ppData++;

		return false;
	}

private:
	enum E_Type
	{
		E_Type_PositionOffset,
		E_Type_NormalOffset,
	};

	E_Type m_Type;
};

void Register()
{
	RenderDataConversion::RegisterVtxInputAnnotationReader<VtxHandler>();
}

} // namespace MorphTarget

} // namespace MeshDeform

