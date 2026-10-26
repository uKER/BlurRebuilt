#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include <aftk/Aftk.h>
#include <fx/Effect.h>

#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConstantAnnotationReader.h"
#include "converter_shared\renderdataconversion\VtxInputSourceSemantics.h"
#include "converter_shared/renderdataconversion/ConvertAftkMesh.h"

namespace TensionMapPlugin
{

Debug::Logger &g_Log = Debug::GetLogger("converter_shared.tensionmap_plugin");

//Vtx plugin for tension maps
class VtxHandler : public RenderDataConversion::DefaultVtxInputSourceSemantics
{
public:

	/// constructor
	VtxHandler(RenderDataConversion::VtxInputAnnotationReader *pParent)
		: RenderDataConversion::DefaultVtxInputSourceSemantics(pParent)
	{
	}

	void CalculateTensionMapNames(const Aftk::C_Scene* pScene)
	{
		m_NumTensionMapParameters = 0;

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
				const U32 numDataChannels = pMesh->GetNumVertexDataChannels();
				for (U32 i = 0; i < numDataChannels; i++)
				{
					if (pMesh->GetVertexDataChannelInfo(i).GetDataType() == Aftk::E_GenericAttributeType_Vector4F32)
					{
						const std::string& dataChannelName = pMesh->GetVertexDataChannelInfo(i).GetName();

						//Check if it is a normblend channel
						if (strncmp(dataChannelName.c_str(), "normblend_", 10) == 0)
						{
							bool found = false;
							//Check if it is already in array
							U32 j;
							for (j = 0; j < m_NumTensionMapParameters; j++)
							{
								if (strncmp(m_TensionMapParameters[j].m_sName, dataChannelName.c_str(), 16) == 0)
								{
									found = true;
								}
							}
							if (!found)
							{
								//Add
								strcpy_s(m_TensionMapParameters[j].m_sName, 16, dataChannelName.c_str());
								m_NumTensionMapParameters++;
								LOGERROR( g_Log, "More than 16 normblend_ channels for the tension maps\n");
							}
						}
					}
				}
			}
			++it;
		}
	}

	/// returns true if the given input source semantic name can be handled by this class
	bool IdentifySourceSemantic(const char *pszSource)
	{
		if (!stricmp(pszSource, "tensionweight"))
		{
			m_Type = E_Type_Weight;
			return true;
		}
		if (!stricmp(pszSource, "tensionindex"))
		{
			m_Type = E_Type_Index;
			return true;
		}

		return false;
	}


	struct WeightIndex
	{
		F32 m_Weight;
		U32 m_Index;

		WeightIndex()
		{
			m_Weight = 0.f;
			m_Index = 0;
		}
	};

	/// get the data for the given vertex
	bool GetData(const RenderDataConversion::AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, float *ppData)
	{
		if (!m_TensionMapParametersCalculated)
		{
			//First time

			CalculateTensionMapNames(pMeshConv->GetMeshNode()->GetScenePtr());

			m_TensionMapParametersCalculated = true;
		}

		const Aftk::C_SceneMeshNode *pMesh = pMeshConv->GetMeshNode();
		const Aftk::C_Polygon &polygon = pMesh->GetPolygon(0, iPolygon);
		//Aftk::Position3 pos = pMesh->GetPolygonVertex(polygon.GetPolygonVertex(iVtx).GetPositionIndex()).GetData;

		WeightIndex weightIndexTable[16];
		U32 numWeightIndex = 0;

		//Calculate more important tension values for this vertex
		const U32 numDataChannels = pMesh->GetNumVertexDataChannels();
		for (U32 i = 0; i < numDataChannels; i++)
		{
			if (pMesh->GetVertexDataChannelInfo(i).GetDataType() == Aftk::E_GenericAttributeType_Vector4F32)
			{

				const std::string& dataChannelName = pMesh->GetVertexDataChannelInfo(i).GetName();

				//Check if it is a normblend channel
				if (strncmp(dataChannelName.c_str(), "normblend_", 10) == 0)
				{
					bool found = false;
					//Check if it is already in array
					for (U32 j = 0; j < m_NumTensionMapParameters; j++)
					{
						if (strncmp(m_TensionMapParameters[j].m_sName, dataChannelName.c_str() + 10, 32) == 0)
						{
							//Add
							weightIndexTable[numWeightIndex].m_Index = j;
							//Get Data
							Aftk::Vector4F32 data;
							pMesh->GetPolygonVertex(polygon.GetPolygonVertex(iVtx).GetPositionIndex()).GetData(data, i);
							//Alpha
							weightIndexTable[numWeightIndex].m_Weight = data.w;
							numWeightIndex++;

							BIZ_ASSERT(numWeightIndex < 16);
						}
					}
				}
			}
		}

		//Sort weights and select first 4
		for (U32 i = 0; i < 4; i++)
		{
			for (U32 j = i + 1; j < numWeightIndex; j++)
			{
				if (weightIndexTable[i].m_Weight < weightIndexTable[i].m_Weight)
				{
					//Swap
					WeightIndex aux = weightIndexTable[i];
					weightIndexTable[i] = weightIndexTable[j];
					weightIndexTable[j] = aux;
				}
			}
		}

		//Output data depends of type
		switch (m_Type)
		{
		case E_Type_Weight:
			ppData[0] = weightIndexTable[0].m_Weight;
			ppData[1] = weightIndexTable[1].m_Weight;
			ppData[2] = weightIndexTable[2].m_Weight;
			ppData[3] = weightIndexTable[3].m_Weight;
			break;
		case E_Type_Index:
			ppData[0] = float(weightIndexTable[0].m_Index);
			ppData[1] = float(weightIndexTable[1].m_Index);
			ppData[2] = float(weightIndexTable[2].m_Index);
			ppData[3] = float(weightIndexTable[3].m_Index);
			break;
		}

		return true;
	}
private:

	static bool m_TensionMapParametersCalculated;

	struct TensionMapParameter
	{
		char m_sName[32];
	};
	static U32					m_NumTensionMapParameters;
	static TensionMapParameter	m_TensionMapParameters[16];

	enum E_Type
	{
		E_Type_Weight,
		E_Type_Index
	};

	E_Type m_Type;

	friend class Handler;
};

//Static variables
bool							VtxHandler::m_TensionMapParametersCalculated = false;
U32								VtxHandler::m_NumTensionMapParameters = 0;
VtxHandler::TensionMapParameter	VtxHandler::m_TensionMapParameters[16];


/// exporter plugin for exporting skinning data
class Handler : public RenderDataConversion::ConstantAnnotationReader
{
public:
	/// constructor
	Handler(FX::Parameter *pParam, const char *pszFXName) :
	  RenderDataConversion::ConstantAnnotationReader(pParam, pszFXName)
	  {
	  }

	  /// parse the given source semantic and return true if we recognise it
	  bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
	  {
		  if (type != RenderDataConversion::E_CustomSemanticType_Constant)
			  return false;

		  return !stricmp(pszSource,"<TensionMap>"); //Array of values
	  }

	  /// read out data + MaterialElement for this parameter, given an Aftk Material
	  /// automatically advances the pData pointer
	  /// returns false if this parameter is not supposed to be stored in the renderingdata material
	  bool GetMaterialData(RenderDataConversion::AftkMaterialConverter *pMtl, RenderingData::MaterialElement *pMtlElement, U8 **ppData)
	  {
		  RenderingData::Build::MaterialList *pMaterialList = &pMtl->GetSceneExport()->GetMaterialList();


		  pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;
		  pMtlElement->count = 0;


		  int nTensionMapsParamters = VtxHandler::m_NumTensionMapParameters;
		  

		  // set the static material element
		  // this contains an index into a dynamic matrix-material for each matrix in the palette
		  pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_DATA_INDEX;
		  pMtlElement->count = nTensionMapsParamters;
		  pMtlElement->name = m_pParam->Name.Chars();

		  if (!ppData)
			  return true;

		  // create the dynamic material and store it's index in the static material data
		  for (int i=0; i<nTensionMapsParamters; i++)
		  {
			  const char *pszName = VtxHandler::m_TensionMapParameters[i].m_sName;

			  // create a new dynamic material type with the name of the matrix that needs to be mapped into this palette entry
			  RenderingData::MaterialElement dynElement;			

			  dynElement.count = 1; // the element stores the required type and count of the output material
			  dynElement.name = pszName;
			  dynElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;

			  // create the dynamic material type
			  RenderingData::MaterialType materialType;
			  materialType.SetName("AnimatedParam");
			  materialType.SetDynamicListIndex(0);
			  materialType.AddElement(dynElement);
			  int iDynamicMtlType = pMaterialList->InsertMaterialType(materialType);

			  // create a F32-material using the new material type
			  RenderingData::Material material;
			  pMaterialList->CreateMaterial(material, iDynamicMtlType);
			  material.SetData(0, RenderingData::MATERIAL_ELEMENT_TYPE_F32);
			  material.SetData(4, 4);

			  // store the index of this new material in the static material output
			  *((int*)*ppData) =  pMaterialList->InsertMaterial(material);
			  (*ppData) += 4;
		  }
		  return true;
	  }

};

void Register()
{
	RenderDataConversion::RegisterAnnotationReader<Handler>();
	RenderDataConversion::RegisterVtxInputAnnotationReader<VtxHandler>();
}

};

