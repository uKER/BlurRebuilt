#include "GameResourcePch.h"

#include "game_resource/ModelTypeManager.h"

namespace GameResource
{

ModelTypeManager g_ModelTypeManager;

/// destructor
ModelTypeManager::~ModelTypeManager()
{
	for (int i=0; i<m_Types.Size(); i++)
		delete m_Types[i].pType;
}


/// register a new model type
/// ownership of the pType pointer passes to the ModelTypeManager class
void ModelTypeManager::RegisterModelType(const char *pszName, I_ModelType *pType)
{
	int i = GetModelTypeIdx(pszName);
	if (i != -1)
	{
		// this model type was already registered
		delete pType;
		return;
	}

	Item item;
	item.sName = pszName;
	item.pType = pType;
	m_Types.PushBack(item);
}

/// run the callback for the specified modeltype
void ModelTypeManager::OnLoad(const char *pszType, C_ModelResource *pModel)
{
	int i = GetModelTypeIdx(pszType);
	if (i == -1)
		return;

	m_Types[i].pType->OnLoad(pModel);
}


/// return the index for the modeltype with the given name
int ModelTypeManager::GetModelTypeIdx(const char *pszType)
{
	for (int i=0; i<m_Types.Size(); i++)
	{
		if (!stricmp(m_Types[i].sName.Chars(), pszType))
			return i;
	}

	return -1;
}

} // namespace WorldRender

// BELOW IS AN EXAMPLE OF HOW TO USE THIS CLASS

/************************************************************************************************
#include <game_resource/ModelTypeManager.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <world_render/RenderModel.h>

class CarType : public GameResource::I_ModelType
{
public:
	void OnLoad(GameResource::C_ModelResource *pModelResource)
    {
		GameResource::C_ModelData *pModelDesc = pModelResource->GetModelData();
		WorldRender::C_RenderModelData *pGRModel = pModelResource->GetRenderModelData();

		// for each model inside the file
		for (int iModel=0; iModel<pModelDesc->GetModelCount(); iModel++)
		{
			GameResource::C_Model *pModel = pModelDesc->GetModel(iModel);
			WorldRender::C_Model *pWRModel = &pModelResource->GetRenderModelData()->GetModel(iModel);

			// add a per-model parameter
			WorldRender::MaterialParameter handle = pWRModel->ExposeParameter("g_mWorld", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);

			// for each element inside this model
			for (int iElement=0; iElement<pModel->GetElementCount(); iElement++)
			{
				GameResource::C_ModelElement *pElement = pModel->GetElement(iElement);

				// if this element has the world 'wheel' in it's name, expose a brake disc colour parameter
				if (strstr(pElement->GetName().Chars(), "wheel"))
				{
					// expose the parameter
					pWRModel->ExposeParameter(iElement, "g_vBrakeDiscColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);

					// this is how to get the handle to this parameter later on
					WorldRender::MaterialParameter handle = pWRModel->GetParameter(iElement, "g_vBrakeDiscColour");					
				}
			}
		}
    }
};

void RegisterCarModelType()
{
	GameResource::g_ModelTypeManager.RegisterModelType("car", new CarType);
}

***********************************************************************************************/
