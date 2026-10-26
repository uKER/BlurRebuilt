#ifndef WORLD_RENDER_RENDER_MODEL_DATA_H
#define WORLD_RENDER_RENDER_MODEL_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Utility/String.h>
#include <renderingdata/MaterialIndex.h>
#include <renderingdata/DataTypes.h>
#include <world_render/materialparameter.h>

namespace RenderingData
{
namespace Target
{
class RenderListNode;
class Scene;
class MaterialList;
class MaterialListData;
class MaterialIndex;
}
class MaterialList;
class Node;
}

namespace WorldRender
{

struct C_ModelInternalData;
struct RenderModelResourceUsage;

// C_Model

class C_Model
{
public:
	C_Model();
	~C_Model();
	C_Model(const C_Model &t);

	// add a per-element parameter
	void ExposeParameter(S32 elementIndex, const Btl::String& name, RenderingData::MaterialElementType type, S32 count);
	// add a per-model parameter
	void ExposeParameter(const Btl::String& name, RenderingData::MaterialElementType type, S32 count);

	MaterialParameter			GetParameter(S32 elementIndex, const char* pName) const;
	MaterialParameter			GetParameter(const char* pName) const;
	const MaterialParameter*	GetWorldMatrixParameter(S32 elementIndex) const
	{
		const MaterialParameter* p = NULL;
		if (elementIndex >= 0 && elementIndex < m_WorldMatrices.Size())
		{
			p = &m_WorldMatrices[elementIndex];
		}
		return p;
	}

	const RenderingData::Target::RenderListNode*	GetRenderListNode() const { return m_pRenderListNode; }
	const RenderingData::MaterialIndex&				GetMaterialIndex() const { return m_MaterialIndex; }
	const RenderingData::Target::MaterialList&		GetMaterialList() const;
	S32												GetNumElements() const;

	const char *GetFriendlyName() const { return m_pszFriendlyName; }
	void		SetFriendlyName(const char *psz) { m_pszFriendlyName = psz; }

	friend class C_RenderModelData;
	friend class C_ModelInstance;

private:
	void	ReadMaterialInput();
	void	BuildDynamicMaterialList();
	void	SetDefaultValues(RenderingData::Target::MaterialListData *pModelInstanceMtlList) const;
	const	RenderingData::MaterialList *GetDynamicMaterialInput() const;

	Btl::Vector<MaterialParameter>			m_WorldMatrices;

	C_ModelInternalData						*m_pData;
	mutable const char						*m_pszFriendlyName; // for debugging

	RenderingData::Target::RenderListNode*	m_pRenderListNode;
	RenderingData::MaterialIndex			m_MaterialIndex;
};

// C_RenderModelData

class C_RenderModelData
{
public:
	C_RenderModelData();

	~C_RenderModelData();

	RenderingData::Target::Scene& GetScene();
	const RenderingData::Target::Scene& GetScene() const;

	void							BuildModelsStart();
	void							BuildModelsEnd();
	const C_Model&					GetModel(S32 index) const;
	C_Model&						GetModel(S32 index);
	S32								GetNumModels() const;

	const RenderingData::Target::MaterialList&		GetMaterialList() const;

	void							AccumulateResourceUsage(RenderModelResourceUsage& ioResourceUsage, S32 modelIndex = -1) const;

	MaterialParameter				GetParameter(const char* pName) const;

#ifndef CFG_GOLD
	void							DumpParameters() const;
#endif

protected:
	RenderingData::Target::Scene	*m_pScene;
	C_ModelInternalData				*m_pData;

	// extract render list nodes and create material outputs
	void BuildModels(RenderingData::Node& node, S32 modelIndex);
	void BuildSubModels(RenderingData::Node& node, S32 modelIndex);
	void BuildDynamicMaterialList();

	Btl::Vector<C_Model>			m_Models;
};

inline const C_Model& C_RenderModelData::GetModel(S32 index) const
{
	return m_Models[index];
}

inline C_Model& C_RenderModelData::GetModel(S32 index)
{
	return m_Models[index];
}

inline S32 C_RenderModelData::GetNumModels() const
{
	return m_Models.Size();
}

} // namespace WorldRender


#endif // WORLD_RENDER_RENDER_MODEL_DATA_H

