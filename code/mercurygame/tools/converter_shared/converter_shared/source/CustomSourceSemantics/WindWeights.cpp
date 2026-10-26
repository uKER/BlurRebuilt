#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include "converter_shared\renderdataconversion\Effect.h"
#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include <display/device/DeviceTypes.h>

#include <fx/effect.h>
#include <Aftk/Aftk.h>

#include <renderingdata/vertexbuffer.h>
#include "converter_shared\renderdataconversion\VtxInputAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConvertAftkMesh.h"
#include "converter_shared\renderdataconversion\VtxInputSourceSemantics.h"
#include "tools_shared\ToolsApplication.h"

namespace WindWeights
{

	class Handler : public RenderDataConversion::DefaultVtxInputSourceSemantics
	{
	public:

		/// constructor
		Handler(RenderDataConversion::VtxInputAnnotationReader *pParent)
			: RenderDataConversion::DefaultVtxInputSourceSemantics(pParent)
		{
		}

		/// returns true if the given input source semantic name can be handled by this class
		bool IdentifySourceSemantic(const char *pszSource)
		{
			if (!stricmp(pszSource, "windweight"))
			{
				m_bWindWeight = true;
				m_bWindIndex = false;
				return true;
			}
			else if (!stricmp(pszSource, "windindex"))
			{
				m_bWindWeight = false;
				m_bWindIndex = true;
				return true;
			}
			else
			{
				m_bWindWeight = false;
				m_bWindIndex = false;
				return false;
			}
		}

		/// get the data for the given vertex
		bool GetData(const RenderDataConversion::AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, float *ppData)
		{
			const Aftk::C_SceneMeshNode *pMesh = pMeshConv->GetMeshNode();
			const Aftk::C_Polygon &polygon = pMesh->GetPolygon(0, iPolygon);

			F32 data[4];
			pMeshConv->GetUV(&polygon, iVtx, 2, data);

			if (m_bWindIndex)
			{
				ppData[0] = data[0];
			}
			else if (m_bWindWeight)
			{
				ppData[0] = data[1];
			}

			return true;
		}

	private:
		
		bool m_bWindWeight;
		bool m_bWindIndex;

	};

	void Register()
	{
		RenderDataConversion::RegisterVtxInputAnnotationReader<Handler>();
	}

};
