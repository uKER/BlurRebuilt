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

namespace LiveryTexcoordGenerator
{
// Name [polySetDropDown_LiveryProjection] Type  [string], Value [Right].

enum LiveryProjection
{
	LiveryProjection_None,
	LiveryProjection_Top,
	LiveryProjection_Left,
	LiveryProjection_Right,
	LiveryProjection_Front,
	LiveryProjection_Rear,
	LiveryProjection_Count
};

const char* LiveryProjectionNames[] = 
{
	"None",
	"Top",
	"Left",
	"Right",
	"Front",
	"Rear"
};

class Handler : public RenderDataConversion::DefaultVtxInputSourceSemantics
{
public:

	/// constructor
	Handler(RenderDataConversion::VtxInputAnnotationReader *pParent)
		: RenderDataConversion::DefaultVtxInputSourceSemantics(pParent)
	{
	}

	LiveryProjection GetLiveryProjection(const Aftk::C_Polygon *pPoly)
	{
		const Aftk::C_BlindDataResource* pBlindDataResource = pPoly->GetMeshNodePtr()->GetPolygonBlindDataMappingPtr(pPoly->GetBlindDataMappingIndex());
		if (pBlindDataResource)
		{
			const Aftk::C_BlindDataNode& blindDataNode = pBlindDataResource->GetContent();
			for (U32 i = 0; i < blindDataNode.GetNumAttributes(); i++) 
			{
				const Aftk::C_GenericAttribute& aftkAttribute = blindDataNode.GetAttribute(i); 
				const char* attrName = aftkAttribute.GetName().c_str();

				if( _stricmp( attrName, "polySetDropDown_LiveryProjection" ) == 0)
				{
					std::string enumValue;
					aftkAttribute.GetValue(enumValue);
					
					for (U32 j = 0; j < U32(LiveryProjection_Count); j++)
					{
						if (_stricmp( enumValue.c_str(), LiveryProjectionNames[j] ) == 0)
						{
							//Found
							return LiveryProjection(j);
						}
					}
				}
			}
		}

		//Not found
		return LiveryProjection_None;
	}


	/// returns true if the given input source semantic name can be handled by this class
	bool IdentifySourceSemantic(const char *pszSource)
	{
		if (!stricmp(pszSource, "Livery"))
		{
			return true;
		}
		
		return false;
	}
 
	void CalculateSize(const Aftk::C_Scene* pScene)
	{
		//Fill projection info
		m_LiveryProjectionInfo[LiveryProjection_Top].m_UProjection[0] = 1.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_UProjection[1] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_UProjection[2] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_VProjection[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_VProjection[1] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_VProjection[2] = -1.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_MinRect[0] = 2.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_MinRect[1] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_MaxRect[0] = 1.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_MaxRect[1] = 2.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_Flip[0] = false;
		m_LiveryProjectionInfo[LiveryProjection_Top].m_Flip[1] = true;

		m_LiveryProjectionInfo[LiveryProjection_Left].m_UProjection[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_UProjection[1] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_UProjection[2] = -1.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_VProjection[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_VProjection[1] = 1.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_VProjection[2] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_MinRect[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_MinRect[1] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_MaxRect[0] = 2.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_MaxRect[1] = 1.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_Flip[0] = true;
		m_LiveryProjectionInfo[LiveryProjection_Left].m_Flip[1] = true;

		m_LiveryProjectionInfo[LiveryProjection_Right].m_UProjection[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_UProjection[1] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_UProjection[2] = -1.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_VProjection[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_VProjection[1] = 1.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_VProjection[2] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_MinRect[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_MinRect[1] = 1.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_MaxRect[0] = 2.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_MaxRect[1] = 2.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_Flip[0] = false;
		m_LiveryProjectionInfo[LiveryProjection_Right].m_Flip[1] = true;

		m_LiveryProjectionInfo[LiveryProjection_Front].m_UProjection[0] = 1.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_UProjection[1] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_UProjection[2] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_VProjection[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_VProjection[1] = 1.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_VProjection[2] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_MinRect[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_MinRect[1] = 2.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_MaxRect[0] = 1.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_MaxRect[1] = 3.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_Flip[0] = true;
		m_LiveryProjectionInfo[LiveryProjection_Front].m_Flip[1] = true;

		m_LiveryProjectionInfo[LiveryProjection_Rear].m_UProjection[0] = 1.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_UProjection[1] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_UProjection[2] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_VProjection[0] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_VProjection[1] = 1.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_VProjection[2] = 0.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_MinRect[0] = 1.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_MinRect[1] = 2.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_MaxRect[0] = 2.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_MaxRect[1] = 3.f/3.f;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_Flip[0] = false;
		m_LiveryProjectionInfo[LiveryProjection_Rear].m_Flip[1] = true;


		//Check all the nodes

		Aftk::C_SceneNodeConstIterator it(pScene);
		//List all nodes
		while(it)
		{
			const Aftk::C_SceneNode* pSceneNode = it;
			if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				//Looks for polyflags
				const Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pSceneNode;
				const U32 numPolygons = pMesh->GetNumPolygons(0);
				for (U32 i = 0; i < numPolygons; i++)
				{
					const Aftk::C_Polygon& polygon = pMesh->GetPolygon(0, i);
					LiveryProjection liveryProjection = GetLiveryProjection(&polygon);
					//Extend AxisBox
					for (U32 j = 0; j < polygon.GetNumPolygonVertices(); j++)
					{
						Aftk::Position3 pos = pMesh->GetVertexPosition(polygon.GetPolygonVertex(j).GetPositionIndex());
						Aftk::Vector4F32 vertexLocalPosition(pos.x,pos.y,pos.z,1.f);
						Aftk::Vector4F32 vertexPosition = vertexLocalPosition * pMesh->GetWorldMatrix();

						//Extend AxisBox
						m_LiveryProjectionAxisBox[U32(liveryProjection)].UpdateExtents(Maths::HVec3(vertexPosition.x, vertexPosition.y, vertexPosition.z));
					}
				}
			}
			++it;
		}

		//Calculate the fit rect
		for (U32 i = LiveryProjection_Top; i < LiveryProjection_Count; i++)
		{
			const Maths::C_AxisBox& axisBox = m_LiveryProjectionAxisBox[LiveryProjection(i)];
			LiveryProjectionInfo& liveryProjectionInfo = m_LiveryProjectionInfo[LiveryProjection(i)];	

			//Calculate max side (wide or height)
			Maths::HVec3 axisBoxSize = axisBox.m_Max - axisBox.m_Min;
			
			//Projection size
			float uProjectionAxisSize = liveryProjectionInfo.m_UProjection[0] * axisBoxSize.X() +
										liveryProjectionInfo.m_UProjection[1] * axisBoxSize.Y() +
										liveryProjectionInfo.m_UProjection[2] * axisBoxSize.Z();

			float vProjectionAxisSize = liveryProjectionInfo.m_VProjection[0] * axisBoxSize.X() +
										liveryProjectionInfo.m_VProjection[1] * axisBoxSize.Y() +
										liveryProjectionInfo.m_VProjection[2] * axisBoxSize.Z();

			uProjectionAxisSize = fabs(uProjectionAxisSize);
			vProjectionAxisSize = fabs(vProjectionAxisSize);

			
			//Texture Size
			float uTextureAxisSize = (liveryProjectionInfo.m_MaxRect[0] - liveryProjectionInfo.m_MinRect[0]) * uProjectionAxisSize;
			float vTextureAxisSize = (liveryProjectionInfo.m_MaxRect[1] - liveryProjectionInfo.m_MinRect[1]) * vProjectionAxisSize;

			//Calculate max size and fit quad
			if (uTextureAxisSize > vTextureAxisSize)
			{
				//Fill wide
				liveryProjectionInfo.m_FitMinRect[0] = liveryProjectionInfo.m_MinRect[0];
				liveryProjectionInfo.m_FitMaxRect[0] = liveryProjectionInfo.m_MaxRect[0];
				//Fit height
				float ratio = vProjectionAxisSize / uProjectionAxisSize;
				float ratio2 = (liveryProjectionInfo.m_MaxRect[1] - liveryProjectionInfo.m_MinRect[1]) / (liveryProjectionInfo.m_MaxRect[0] - liveryProjectionInfo.m_MinRect[0]);
				float lenght = (liveryProjectionInfo.m_MaxRect[1] - liveryProjectionInfo.m_MinRect[1]) / 2.f;
				float center = (liveryProjectionInfo.m_MaxRect[1] + liveryProjectionInfo.m_MinRect[1]) / 2.f;
			
				liveryProjectionInfo.m_FitMinRect[1] = center - lenght * ratio / ratio2;
				liveryProjectionInfo.m_FitMaxRect[1] = center + lenght * ratio / ratio2;
				
			}
			else
			{
				//Fill height
				liveryProjectionInfo.m_FitMinRect[1] = liveryProjectionInfo.m_MinRect[1];
				liveryProjectionInfo.m_FitMaxRect[1] = liveryProjectionInfo.m_MaxRect[1];
				//Fit wide
				float ratio = uProjectionAxisSize / vProjectionAxisSize;
				float ratio2 = (liveryProjectionInfo.m_MaxRect[0] - liveryProjectionInfo.m_MinRect[0]) / (liveryProjectionInfo.m_MaxRect[1] - liveryProjectionInfo.m_MinRect[1]);
				float lenght = (liveryProjectionInfo.m_MaxRect[0] - liveryProjectionInfo.m_MinRect[0]) / 2.f;
				float center = (liveryProjectionInfo.m_MaxRect[0] + liveryProjectionInfo.m_MinRect[0]) / 2.f;

				liveryProjectionInfo.m_FitMinRect[0] = center - lenght * ratio / ratio2;
				liveryProjectionInfo.m_FitMaxRect[0] = center + lenght * ratio / ratio2;
			}
			
// 			liveryProjectionInfo.m_FitMinRect[0] = liveryProjectionInfo.m_MinRect[0];
// 			liveryProjectionInfo.m_FitMaxRect[0] = liveryProjectionInfo.m_MaxRect[0];
// 			liveryProjectionInfo.m_FitMinRect[1] = liveryProjectionInfo.m_MinRect[1];
// 			liveryProjectionInfo.m_FitMaxRect[1] = liveryProjectionInfo.m_MaxRect[1];

			LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "Livery projection %s: BoxSize(%f, %f, %f), TextureRect((%f, %f) - (%f %f)\n", LiveryProjectionNames[i], F32(axisBoxSize.X()), F32(axisBoxSize.Y()), F32(axisBoxSize.Z()), liveryProjectionInfo.m_FitMinRect[0], liveryProjectionInfo.m_FitMinRect[1], liveryProjectionInfo.m_FitMaxRect[0], liveryProjectionInfo.m_FitMaxRect[1]);

		}
	}


	/// get the data for the given vertex
	bool GetData(const RenderDataConversion::AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, float *ppData)
	{
		if (!m_SizeCalculated)
		{
			//First time
			
			CalculateSize(pMeshConv->GetMeshNode()->GetScenePtr());

			m_SizeCalculated = true;
		}

		const Aftk::C_SceneMeshNode *pMesh = pMeshConv->GetMeshNode();
		const Aftk::C_Polygon &polygon = pMesh->GetPolygon(0, iPolygon);
		Aftk::Position3 pos = pMesh->GetVertexPosition(polygon.GetPolygonVertex(iVtx).GetPositionIndex());
		Aftk::Vector4F32 vertexLocalPosition(pos.x,pos.y,pos.z,1.f);
		Aftk::Vector4F32 vertexPosition = vertexLocalPosition * pMesh->GetWorldMatrix();

		Maths::HVec2 liveryTexcoords;

		//Check Polyflag
		LiveryProjection liveryProjection = GetLiveryProjection(&polygon);
		
		if (liveryProjection == LiveryProjection_None)
		{
			//not a livery polygon, defaut texcoords (1,1)

			ppData[0] = 1.f;
			ppData[1] = 1.f;
		}
		else
		{
			//Use projection Info
			const Maths::C_AxisBox& axisBox = m_LiveryProjectionAxisBox[liveryProjection];
			const LiveryProjectionInfo& liveryProjectionInfo = m_LiveryProjectionInfo[liveryProjection];
			
			//Convert vertex to axisBox space
			Maths::HVec3 worldBoxPosition(vertexPosition.x, vertexPosition.y, vertexPosition.z);
			Maths::HVec3 localBoxPosition = (worldBoxPosition - axisBox.m_Min) / (axisBox.m_Max - axisBox.m_Min);

			//Project localBox position
			float uProjectionTexCoord =	liveryProjectionInfo.m_UProjection[0] * localBoxPosition.X() + 
										liveryProjectionInfo.m_UProjection[1] * localBoxPosition.Y() +
										liveryProjectionInfo.m_UProjection[2] * localBoxPosition.Z();

			float vProjectionTexCoord =	liveryProjectionInfo.m_VProjection[0] * localBoxPosition.X() + 
										liveryProjectionInfo.m_VProjection[1] * localBoxPosition.Y() +
										liveryProjectionInfo.m_VProjection[2] * localBoxPosition.Z();


			if (liveryProjectionInfo.m_Flip[0])
				uProjectionTexCoord = 1.f - fabs(uProjectionTexCoord);
			else
				uProjectionTexCoord = fabs(uProjectionTexCoord);

			if (liveryProjectionInfo.m_Flip[1])
				vProjectionTexCoord = 1.f - fabs(vProjectionTexCoord);
			else
				vProjectionTexCoord = fabs(vProjectionTexCoord);

			//Fit to texture space
			float uTextureSpace = liveryProjectionInfo.m_FitMaxRect[0] - liveryProjectionInfo.m_FitMinRect[0];
			float vTextureSpace = liveryProjectionInfo.m_FitMaxRect[1] - liveryProjectionInfo.m_FitMinRect[1];

			float uTexCoord = liveryProjectionInfo.m_FitMinRect[0] +  uProjectionTexCoord * (uTextureSpace);
			float vTexCoord = liveryProjectionInfo.m_FitMinRect[1] +  vProjectionTexCoord * (vTextureSpace);

			//Copy to output data
			ppData[0] = uTexCoord;
			ppData[1] = vTexCoord;
		}


		return true;
	}
private:
	
	static bool m_SizeCalculated;

	static Maths::C_AxisBox m_LiveryProjectionAxisBox[LiveryProjection_Count];

	struct LiveryProjectionInfo
	{
		float m_UProjection[3];
		float m_VProjection[3];
		float m_MinRect[2];
		float m_MaxRect[2];
		float m_FitMinRect[2];
		float m_FitMaxRect[2];
		bool  m_Flip[2];
	};

	static LiveryProjectionInfo m_LiveryProjectionInfo[LiveryProjection_Count];
};

//Static variables
bool Handler::m_SizeCalculated = false;
Handler::LiveryProjectionInfo Handler::m_LiveryProjectionInfo[LiveryProjection_Count];
Maths::C_AxisBox Handler::m_LiveryProjectionAxisBox[LiveryProjection_Count];


void Register()
{
	RenderDataConversion::RegisterVtxInputAnnotationReader<Handler>();
}

};

