#include "GameDebugPch.h"
#include <game_debug/DebugResourceRender.h>

#include <core/containers/genesis_containers/Sort.h>
#include <framework/application/UserControls.h>
#include <game_debug/ModelResourceUsageUtils.h>
#include <game_resource/LevelResource.h>
#include <game_resource/LevelSceneManager.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/cycle/Cycle.h>
#include <animation/pose/PoseFieldTypes.h>
#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>
#include <input/GameInput.h>
#include <scenegraph/SceneGraph.h>
#include <system/debug/metadebug/MetaDebug.h>
#include <system/debug/metadebug/MetaDebugAttributeList.h>
#include <system/debug/metadebug/MetaDebugLog.h>
#include <system/misc/StringUtil.h>
#include <textureresource/TextureResource.h>
#include <world_render/EnvironmentSampler.h>
#include <world_render/ModelInstance.h>
#include <world_render/RenderModelData.h>
#include <world_render/RenderModelResourceUsage.h>

extern Debug::Bool g_DefaultMatrixParamsToIdentity; // nasty hack to resolve issues with skinned models


namespace GameDebug
{

static Debug::Bool g_DebugResourceRenderEnable("GameSys.DebugResourceRender.Enable", false);
static Debug::Float g_DebugResourceRenderTextSize("GameSys.DebugResourceRender.TextSize", 20.0f, Debug::Range(8.0f, 256.0f));

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

// There is an optional pane in the background that is configured by the 
// currently selected leaf to display context-sensitive information.

class I_DebugResourceRenderPane
{
public:
	virtual ~I_DebugResourceRenderPane() {}

	virtual void Update(const Maths::HMat43& mCameraToWorld) = 0;
	virtual void Render2d(DbUtil::I_DebugRender* pDebugRender) const = 0;
	virtual void Close() = 0;

};

static Debug::Float g_DebugResourceRenderPanePosX("GameSys.DebugResourceRender.Pane.PosX", 512.0f, Debug::Range(-1024.0f, 1024.0f));
static Debug::Float g_DebugResourceRenderPanePosY("GameSys.DebugResourceRender.Pane.PosY", 100.0f, Debug::Range(-768.0f, 768.0f));

//----------------------------------------------------------------------------

// This pane displays a texture.

class C_DebugResourceRenderPaneTexture : public I_DebugResourceRenderPane
{
public:
	C_DebugResourceRenderPaneTexture();
	virtual ~C_DebugResourceRenderPaneTexture() {}

	void Init(const Display::C_Texture2D* pTexture);
	virtual void Update(const Maths::HMat43& mCameraToWorld) {}
	virtual void Render2d(DbUtil::I_DebugRender* pDebugRender) const;
	virtual void Close();

private:
	const Display::C_Texture2D*	m_pTexture; ///< weak ref

	C_DebugResourceRenderPaneTexture(const C_DebugResourceRenderPaneTexture& copy); // can not copy
	C_DebugResourceRenderPaneTexture& operator = (const C_DebugResourceRenderPaneTexture& copy); // can not copy

	static const char* GetTextureFormatName(const Display2::E_TextureFormat eFormat);

};

static Debug::Float g_DebugResourceRenderPaneTextureScale("GameSys.DebugResourceRender.Pane.Texture.Scale", 512.0f, Debug::Range(1.0f, 1000.0f));

C_DebugResourceRenderPaneTexture::C_DebugResourceRenderPaneTexture()
	: I_DebugResourceRenderPane()
	, m_pTexture(NULL)
{
}

void C_DebugResourceRenderPaneTexture::Init(const Display::C_Texture2D* pTexture)
{
	m_pTexture = pTexture;
}

void C_DebugResourceRenderPaneTexture::Render2d(DbUtil::I_DebugRender* pDebugRender) const
{
	if (m_pTexture != NULL)
	{
		Maths::HVec2 pos(g_DebugResourceRenderPanePosX.Value(), g_DebugResourceRenderPanePosY.Value());

		const U32 width = m_pTexture->GetWidth();
		const U32 height = m_pTexture->GetHeight();

		{
			const F32 scale = g_DebugResourceRenderTextSize;

			Util::C_String text;
			text.Printf("%d Kb    %d x %d X %d    %s", (m_pTexture->GetSize() + 1023) / 1024, width, height, m_pTexture->GetLevels(), GetTextureFormatName(m_pTexture->GetFormat()));
			
			const Maths::RGBA colour(1.0f, 1.0f, 1.0f, 1.0f);
			pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);

			pos.Y(pos.Y() + (2.0f * scale));
		}

		F32 x = pos.X();
		F32 y = pos.Y();
		F32 w = g_DebugResourceRenderPaneTextureScale * (1024.0f / 1280.0f);
		F32 h = g_DebugResourceRenderPaneTextureScale;// * (768.0f / 720.0f);

		if (width > height)
		{
			const F32 ratio = static_cast<F32>(height) / static_cast<F32>(width);
			const F32 remove = w * (1.0f - ratio);
			x += 0.5f * remove;
			w -= remove;
		}
		else if (width < height)
		{
			const F32 ratio = static_cast<F32>(width) / static_cast<F32>(height);
			const F32 remove = h * (1.0f - ratio);
			y += 0.5f * remove;
			h -= remove;
		}

		const Maths::RGBA black(0.0f, 0.0f, 0.0f, 1.0f);
		const Maths::RGBA white(1.0f, 1.0f, 1.0f, 1.0f);

		Display::BasicRender()->DrawColoured2DRect(x, y, w, h, black);
		Display::BasicRender()->DrawTextured2DRect(x, y, w, h, white, m_pTexture);
	}
}

void C_DebugResourceRenderPaneTexture::Close()
{
	m_pTexture = NULL;
}

const char* C_DebugResourceRenderPaneTexture::GetTextureFormatName(const Display2::E_TextureFormat eFormat)
{
	switch (eFormat)
	{
	case Display2::E_TextureFormat_A8R8G8B8:	return "A8R8G8B8";
	case Display2::E_TextureFormat_DXT1:		return "DXT1";
	case Display2::E_TextureFormat_DXT3:		return "DXT3";
	case Display2::E_TextureFormat_DXT5:		return "DXT5";
	case Display2::E_TextureFormat_L8:			return "L8";
	default:									return "UNKNOWN";
	}
}

//----------------------------------------------------------------------------

// This pane shows a model or element.

class C_DebugResourceRenderPaneModel : public I_DebugResourceRenderPane
{
public:
	explicit C_DebugResourceRenderPaneModel(GameResource::C_LevelSceneManager* pLevelSceneManager);
	virtual ~C_DebugResourceRenderPaneModel();

	void Init(const GameResource::C_ModelData* pModelData, const WorldRender::C_RenderModelData* pRenderModelData, const S32 modelIndex, const S32 elementIndex);
	virtual void Update(const Maths::HMat43& mCameraToWorld);
	virtual void Render2d(DbUtil::I_DebugRender* pDebugRender) const {}
	virtual void Close();

private:
	GameResource::C_LevelSceneManager*		m_pLevelSceneManager; ///< weak ref
	Maths::HMat43							m_WorldMatrix; ///< world-space matrix of model instance
	const GameResource::C_Model*			m_pModel; ///< weak ref
	WorldRender::C_ModelInstance*			m_pModelInstance; ///< strong ref
	WorldRender::C_EnvironmentSampler*		m_pEnvSampler; ///< strong ref

	C_DebugResourceRenderPaneModel(const C_DebugResourceRenderPaneModel& copy); // can not copy
	C_DebugResourceRenderPaneModel& operator = (const C_DebugResourceRenderPaneModel& copy); // can not copy

};

static Debug::Bool g_DebugResourceRenderPaneModelPlace("GameSys.DebugResourceRender.Pane.Model.Place", false);

C_DebugResourceRenderPaneModel::C_DebugResourceRenderPaneModel(GameResource::C_LevelSceneManager* pLevelSceneManager)
	: I_DebugResourceRenderPane()
	, m_pLevelSceneManager(pLevelSceneManager)
	, m_pModel(NULL)
	, m_pModelInstance(NULL)
	, m_pEnvSampler(NULL)
{
}

C_DebugResourceRenderPaneModel::~C_DebugResourceRenderPaneModel()
{
	Close();
}

void C_DebugResourceRenderPaneModel::Init(const GameResource::C_ModelData* pModelData, const WorldRender::C_RenderModelData* pRenderModelData, const S32 modelIndex, const S32 elementIndex)
{
	Close();

	const bool bDefaultMatrixParamsToIdentity = g_DefaultMatrixParamsToIdentity;
	g_DefaultMatrixParamsToIdentity.SetValue(true);

	SceneGraph::C_SceneGraph* pSceneGraph = m_pLevelSceneManager->GetWorldRenderScene()->GetSceneGraph();

	m_pModel = pModelData->GetModel(modelIndex);
	m_pModelInstance = new WorldRender::C_ModelInstance(pRenderModelData, modelIndex, pSceneGraph);
	m_pEnvSampler = new WorldRender::C_EnvironmentSampler;

	SceneGraph::Insert(pSceneGraph, m_pModelInstance);
	m_pEnvSampler->Initialise(m_pLevelSceneManager->GetWorldRenderScene());

	g_DefaultMatrixParamsToIdentity.SetValue(bDefaultMatrixParamsToIdentity);

	if (elementIndex < 0)
	{
		for (S32 i = 0; i < m_pModelInstance->GetNumElements(); ++i)
		{
			m_pModelInstance->SetElementVisibility(i, true);
		}
	}
	else
	{
		for (S32 i = 0; i < m_pModelInstance->GetNumElements(); ++i)
		{
			m_pModelInstance->SetElementVisibility(i, false);
		}

		m_pModelInstance->SetElementVisibility(elementIndex, true);
	}

	m_pModelInstance->SetNearFarDistance(0.0f, 4000.0f);

	g_DebugResourceRenderPaneModelPlace.SetValue(true);
}

void C_DebugResourceRenderPaneModel::Update(const Maths::HMat43& mCameraToWorld)
{
	if (g_DebugResourceRenderPaneModelPlace)
	{
		g_DebugResourceRenderPaneModelPlace = false;

		if (m_pModel != NULL)
		{
			const Maths::C_AxisBox localBounds = m_pModel->GetLocalBounds();

			const Maths::HVec3 cameraPos = mCameraToWorld.AW();
			const Maths::HVec3 cameraDir = mCameraToWorld.AZ();
			const F32 distance = 1.5f * localBounds.Diagonal().Length();

			m_WorldMatrix.AW(cameraPos + (cameraDir * distance) - localBounds.Centre());
		}
	}

	if (m_pModelInstance != NULL)
	{
		if (m_pModelInstance->GetNumElements() > 1)
		{
			for (S32 i = 0; i < m_pModelInstance->GetNumElements(); ++i)
			{
				m_pModelInstance->SetElementMatrix(i, m_pModel->GetElement(i)->GetMatrix() * m_WorldMatrix);
			}
		}
		else
		{
			m_pModelInstance->SetElementMatrix(0, m_WorldMatrix);
		}

		m_pModelInstance->SetWorldSphere(Maths::C_Sphere(m_WorldMatrix.AW(), 100.0f));

		if (m_pEnvSampler != NULL)
		{
			m_pEnvSampler->SetPosition(m_WorldMatrix.AW());
			m_pEnvSampler->Update();
			m_pEnvSampler->ApplyToModelInstance(m_pModelInstance);
		}
	}
}

void C_DebugResourceRenderPaneModel::Close()
{
	if (m_pModelInstance != NULL)
	{
		SceneGraph::Remove(m_pLevelSceneManager->GetWorldRenderScene()->GetSceneGraph(), m_pModelInstance);
		SAFE_DELETE_PTR(m_pModelInstance);
	}

	SAFE_DELETE_PTR(m_pEnvSampler);

	m_pModel = NULL;
}

//----------------------------------------------------------------------------

// This pane shows info for a set of textured model data.

class C_DebugResourceRenderPaneModelSet : public I_DebugResourceRenderPane
{
public:
	explicit C_DebugResourceRenderPaneModelSet(GameResource::C_LevelSceneManager* pLevelSceneManager, C_DebugResourceRenderPaneTexture* pTexturePane);
	virtual ~C_DebugResourceRenderPaneModelSet();

	void Init(const Util::C_String& fileName, const GameResource::C_ModelData* pModelData, const WorldRender::RenderModelResourceUsage* pResourceUsage);
	virtual void Update(const Maths::HMat43& mCameraToWorld) {}
	virtual void Render2d(DbUtil::I_DebugRender* pDebugRender) const;
	virtual void Close();

	C_DebugResourceRenderPaneModel* GetModelPane() {return m_pModelPane;}
	C_DebugResourceRenderPaneTexture* GetTexturePane() {return m_pTexturePane;}

private:
	C_DebugResourceRenderPaneModel*			m_pModelPane; ///< strong ref
	C_DebugResourceRenderPaneTexture*		m_pTexturePane; ///< weak ref
	Util::C_String							m_FileName; ///< name to display
	U32										m_NumModels; ///< number of models
	U32										m_NumElements; ///< number of elements
	ModelResourceUsage::C_TextureTotals		m_TextureTotals; ///< texture total sizes


	C_DebugResourceRenderPaneModelSet(const C_DebugResourceRenderPaneModelSet& copy); // can not copy
	C_DebugResourceRenderPaneModelSet& operator = (const C_DebugResourceRenderPaneModelSet& copy); // can not copy

	inline static U32 BytesToKb(const U32 bytes) {return (bytes + 1023) / 1024;}

};

C_DebugResourceRenderPaneModelSet::C_DebugResourceRenderPaneModelSet(GameResource::C_LevelSceneManager* pLevelSceneManager, C_DebugResourceRenderPaneTexture* pTexturePane)
	: I_DebugResourceRenderPane()
	, m_pModelPane(new C_DebugResourceRenderPaneModel(pLevelSceneManager))
	, m_pTexturePane(pTexturePane)
	, m_FileName()
	, m_NumModels(0)
	, m_NumElements(0)
{
}

C_DebugResourceRenderPaneModelSet::~C_DebugResourceRenderPaneModelSet()
{
	delete m_pModelPane;
}

void C_DebugResourceRenderPaneModelSet::Init(const Util::C_String& fileName, const GameResource::C_ModelData* pModelData, const WorldRender::RenderModelResourceUsage* pResourceUsage)
{
	m_FileName = fileName;
	m_NumModels = pModelData->GetModelCount();
	m_NumElements = 0;
	memset(m_TextureTotals, 0, sizeof(m_TextureTotals));

	for (S32 i = 0; i < pModelData->GetModelCount(); ++i)
	{
		m_NumElements += pModelData->GetModel(i)->GetElementCount();
	}

	for (WorldRender::RenderModelTextureInfoList::ConstIterator it = pResourceUsage->textureInfoList.Begin(); it != pResourceUsage->textureInfoList.End(); ++it)
	{
		const WorldRender::RenderModelTextureInfo& info = it->GetValue();
		ModelResourceUsage::AccumulateTotals(m_TextureTotals, info.textureName, info.uSizeInBytes, info.eFormat);
	}
}

void C_DebugResourceRenderPaneModelSet::Render2d(DbUtil::I_DebugRender* pDebugRender) const
{
	Maths::HVec2 pos(g_DebugResourceRenderPanePosX.Value(), g_DebugResourceRenderPanePosY.Value());
	const F32 scale = g_DebugResourceRenderTextSize;
	const Maths::RGBA colour(1.0f, 1.0f, 1.0f, 1.0f);

	// file info
	if (m_FileName.Length() > 0)
	{
		pDebugRender->Draw2dText(pos, m_FileName.Chars(), colour, scale);
		pos.Y(pos.Y() + (2.0f * scale));
	}

	// model info
	{
		Util::C_String text;

		text.Printf("%d models", m_NumModels);
		pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
		pos.Y(pos.Y() + scale);

		text.Printf("%d elements", m_NumElements);
		pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
		pos.Y(pos.Y() + (2.0f * scale));
	}

	// texture info
	{
		U32 totalBytes = 0;

		const Maths::HVec2 offset(6.0f * scale, 0.0f);

		Util::C_String text;

		for (U32 i = 0; i < ModelResourceUsage::E_TextureType_Count; ++i)
		{
			totalBytes += m_TextureTotals[i];
			text.Printf("%d Kb", BytesToKb(m_TextureTotals[i]));
			pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
			pDebugRender->Draw2dText(pos + offset, ModelResourceUsage::GetTextureTypeName(static_cast<ModelResourceUsage::E_TextureType>(i)), colour, scale);
			pos.Y(pos.Y() + scale);
		}

		text.Printf("%d Kb", BytesToKb(totalBytes));
		pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
		pDebugRender->Draw2dText(pos + offset, "Total", colour, scale);
		pos.Y(pos.Y() + (2.0f * scale));
	}
}

void C_DebugResourceRenderPaneModelSet::Close()
{
	m_FileName = "";
	m_NumModels = 0;
	m_NumElements = 0;
	memset(m_TextureTotals, 0, sizeof(m_TextureTotals));
}

//----------------------------------------------------------------------------

// This pane displays information about an animation cycle's field (data channel).


class C_DebugResourceRenderPaneAnimationField : public I_DebugResourceRenderPane
{
public:
	C_DebugResourceRenderPaneAnimationField();
	virtual ~C_DebugResourceRenderPaneAnimationField();

	void Init(const Animation::C_Cycle* pCycle, U32 fieldIndex);

	virtual void Update(const Maths::HMat43& mCameraToWorld) {}
	virtual void Render2d(DbUtil::I_DebugRender* pDebugRender) const;
	virtual void Close();

private:
	Util::C_String				m_Name;
	const Animation::C_Cycle*	m_pCycle;
	U32							m_Size;
	U32							m_TotalSize;
	U32							m_AvgSize;
	U32							m_FieldIndex;

	struct C_FieldInfo
	{
		U32		size;
		U32		index;
	};

	Btl::Vector<C_FieldInfo>	m_Fields;

	U32							m_MinSize;
	U32							m_MaxSize;

	C_DebugResourceRenderPaneAnimationField(const C_DebugResourceRenderPaneAnimationField& copy); // can not copy
	C_DebugResourceRenderPaneAnimationField& operator = (const C_DebugResourceRenderPaneAnimationField& copy); // can not copy

};

C_DebugResourceRenderPaneAnimationField::C_DebugResourceRenderPaneAnimationField()
{
}

C_DebugResourceRenderPaneAnimationField::~C_DebugResourceRenderPaneAnimationField()
{
}

void C_DebugResourceRenderPaneAnimationField::Init(const Animation::C_Cycle* pCycle, U32 fieldIndex)
{
	m_pCycle = pCycle;
	m_FieldIndex = fieldIndex;

	if (!pCycle) return;

	m_Size = pCycle->GetFieldMemoryUsage(fieldIndex);
	m_Name = pCycle->GetFieldName(fieldIndex).GetString();
	m_TotalSize = pCycle->GetMemoryUsage();
	m_AvgSize = m_TotalSize / pCycle->GetNumFields();

	m_MinSize = m_TotalSize;
	m_MaxSize = 0;
	m_Fields.Resize(pCycle->GetNumFields());
	for (U32 i = 0; i < pCycle->GetNumFields(); ++i)
	{
		U32 fieldSize = pCycle->GetFieldMemoryUsage(i);
		m_Fields[i].index = i;
		m_Fields[i].size = fieldSize;
		m_MinSize = Maths::Min(m_MinSize, fieldSize);
		m_MaxSize = Maths::Max(m_MaxSize, fieldSize);

	}

	// Sort by increasing size.
	bool swapped = true;
	while (swapped)
	{
		swapped = false;

		for (S32 i = 1; i < m_Fields.Size(); ++i)
		{
			if (m_Fields[i - 1].size > m_Fields[i].size)
			{
				swapped = true;
				C_FieldInfo tmp = m_Fields[i-1];
				m_Fields[i - 1] = m_Fields[i];
				m_Fields[i] = tmp;
			}
		}
	}
}

void C_DebugResourceRenderPaneAnimationField::Render2d(DbUtil::I_DebugRender* pDebugRender) const
{
	Maths::HVec2 pos(g_DebugResourceRenderPanePosX.Value(), g_DebugResourceRenderPanePosY.Value());
	const F32 scale = g_DebugResourceRenderTextSize;
	const Maths::RGBA colour(1.0f, 1.0f, 1.0f, 1.0f);

	// file info
	if (m_Name.Length() > 0)
	{
		pDebugRender->Draw2dText(pos, m_Name.Chars(), colour, scale);
		pos.Y(pos.Y() + (2.0f * scale));
	}

	Util::C_String text;
	text.Printf("Index: %d / %d", m_FieldIndex, m_pCycle->GetNumFields());
	pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
	pos.Y(pos.Y() + scale);

	text.Printf("Memory Used: %d bytes", m_Size);
	pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
	pos.Y(pos.Y() + scale);

	F32 chartHeight = scale * 10.f;
	F32 chartWidth = scale * 10.f;

	Maths::HVec2 chartOrigin = pos + Maths::HVecY(chartHeight);

	Maths::RGBA axisColour(0.6f, 0.6f, 0.6f, 1.f);
	Maths::RGBA lineColour(1.f, 1.f, 1.f, 1.f);
	Maths::RGBA currentColour(1.f, 1.f, 0.f, 1.f);
	Maths::RGBA avgColour(0.9f, 0.3f, 0.f, 1.f);

	pDebugRender->Draw2dLine(chartOrigin, pos, axisColour);
	pDebugRender->Draw2dLine(chartOrigin, chartOrigin + Maths::HVecX(chartWidth), axisColour);

	Maths::HVec2 offset(0.05f * scale, 0.f);

	Maths::HVec2 dx(chartWidth / m_Fields.Size(), 0.f);

	Maths::HVec2 chartPos = chartOrigin;
	F32 prevHeight = 0.f;
	for (S32 i = 0; i < m_Fields.Size(); ++i)
	{
		F32 height = chartHeight * (m_Fields[i].size - m_MinSize) / (m_MaxSize - m_MinSize);

		if (i > 1)
		{
			pDebugRender->Draw2dLine(chartPos - dx - Maths::HVecY(prevHeight), chartPos - Maths::HVecY(height), lineColour);
		}

		if (m_Fields[i].index == m_FieldIndex)
		{
			pDebugRender->Draw2dLine(chartPos, chartPos - Maths::HVecY(chartHeight), currentColour);
		}

		prevHeight = height;
		chartPos += dx;
	}
	F32 avg = chartHeight * (m_AvgSize - m_MinSize) / (m_MaxSize - m_MinSize);
	pDebugRender->Draw2dLine(chartOrigin + Maths::HVec2(0.f, -avg), chartOrigin + Maths::HVec2(chartWidth, -avg), avgColour);

	pos.Y(pos.Y() + scale);
}

void C_DebugResourceRenderPaneAnimationField::Close()
{
	m_Name.Clear();
	m_pCycle = 0;
	m_Size = 0;
	m_FieldIndex = 0;
}


//----------------------------------------------------------------------------

// This pane displays information about an animation cycle.


class C_DebugResourceRenderPaneAnimationCycle : public I_DebugResourceRenderPane
{
public:
	C_DebugResourceRenderPaneAnimationCycle();
	virtual ~C_DebugResourceRenderPaneAnimationCycle();

	void Init(const Util::C_String& filename, const Animation::C_Cycle* pCycle);

	virtual void Update(const Maths::HMat43& mCameraToWorld) {}
	virtual void Render2d(DbUtil::I_DebugRender* pDebugRender) const;
	virtual void Close();

	C_DebugResourceRenderPaneAnimationField*	GetFieldPane()	{ return m_pFieldPane; }

private:
	Util::C_String								m_Filename; ///< name to display
	U32											m_FieldCounts[Animation::E_PoseFieldType_Count];
	const Animation::C_Cycle*					m_pCycle;
	C_DebugResourceRenderPaneAnimationField*	m_pFieldPane;	// strong ref

	C_DebugResourceRenderPaneAnimationCycle(const C_DebugResourceRenderPaneModelSet& copy); // can not copy
	C_DebugResourceRenderPaneAnimationCycle& operator = (const C_DebugResourceRenderPaneModelSet& copy); // can not copy

	inline static U32 BytesToKb(const U32 bytes) {return (bytes + 1023) / 1024;}
};


const CHAR8* GetAnimationFieldTypeName(Animation::E_PoseFieldType fieldType)
{
	static const CHAR8* kFieldNames[Animation::E_PoseFieldType_Count] =
	{
		"Joint Transform", 
		"Trajectory Delta Transform",
		"3D Vector",
		"Scalar",
		"Trajectory Transform",
		"4D Vector"
	};
	return kFieldNames[fieldType];
}

C_DebugResourceRenderPaneAnimationCycle::C_DebugResourceRenderPaneAnimationCycle():
	m_pFieldPane(new C_DebugResourceRenderPaneAnimationField)
{
}

C_DebugResourceRenderPaneAnimationCycle::~C_DebugResourceRenderPaneAnimationCycle()
{
	delete m_pFieldPane;
}

void C_DebugResourceRenderPaneAnimationCycle::Init(const Util::C_String& filename, const Animation::C_Cycle* pCycle)
{
	m_Filename = filename;
	m_pCycle = pCycle;
	for (U32 i = 0; i < Animation::E_PoseFieldType_Count; ++i)
	{
		m_FieldCounts[i] = 0;
	}
		
	if (!pCycle)
	{
		return;
	}
	
	U32 numFields = m_pCycle->GetNumFields();
	for (U32 i = 0; i < numFields; ++i)
	{
		m_FieldCounts[m_pCycle->GetFieldType(i)]++;
	}
}

void C_DebugResourceRenderPaneAnimationCycle::Render2d(DbUtil::I_DebugRender* pDebugRender) const
{
	Maths::HVec2 pos(g_DebugResourceRenderPanePosX.Value(), g_DebugResourceRenderPanePosY.Value());
	const F32 scale = g_DebugResourceRenderTextSize;
	const Maths::RGBA colour(1.0f, 1.0f, 1.0f, 1.0f);

	// file info
	if (m_Filename.Length() > 0)
	{
		pDebugRender->Draw2dText(pos, m_Filename.Chars(), colour, scale);
		pos.Y(pos.Y() + (2.0f * scale));
	}

	// animation info
	{
		Util::C_String text;

		text.Printf("Duration: %.1f seconds", m_pCycle->GetDuration());
		pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
		pos.Y(pos.Y() + scale);

		text.Printf("No. frames: %d", (S32)m_pCycle->GetDurationFrames());
		pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
		pos.Y(pos.Y() + scale);

		text.Printf("Frame Rate: %.1f fps", m_pCycle->GetFrameRate());
		pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
		pos.Y(pos.Y() + (2.f * scale));

		const Maths::HVec2 offset(2.0f * scale, 0.0f);
		for (U32 i = 0; i < Animation::E_PoseFieldType_Count; ++i)
		{
			// Skip fields that are currently only ever used at runtime.
			if (i == Animation::E_PoseFieldType_TrajectoryDeltaTransform || i == Animation::E_PoseFieldType_TrajectoryTransform)
			{
				BIZ_ASSERT(m_FieldCounts[i] == 0);
				continue;
			}
			text.Printf("%d", m_FieldCounts[i]);
			pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);

			text.Printf("%s%s", GetAnimationFieldTypeName(static_cast<Animation::E_PoseFieldType>(i)), m_FieldCounts[i] != 1 ? "s" : "");
			pDebugRender->Draw2dText(pos + offset, text.Chars(), colour, scale);

			pos.Y(pos.Y() + scale);
		}
		pos.Y(pos.Y() + scale);

		text.Printf("Size: %d Kb", (m_pCycle->GetMemoryUsage() + 1023) / 1024);
		pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
		pos.Y(pos.Y() + (2.0f * scale));

		F32 xyz[3];
		Maths::Store(m_pCycle->GetLinearVelocity(), xyz);
		if (xyz[0] != 0.f || xyz[1] != 0.f || xyz[2] != 0.f)
		{
			text.Printf("Linear Velocity: %.3f, %.3f, %.3f", xyz[0], xyz[1], xyz[2]);
			pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
			pos.Y(pos.Y() + scale);
		}
		Maths::Store(m_pCycle->GetAngularVelocity(), xyz);
		if (xyz[0] != 0.f || xyz[1] != 0.f || xyz[2] != 0.f)
		{
			text.Printf("Angular Velocity: %.3f, %.3f, %.3f", xyz[0], xyz[1], xyz[2]);
			pDebugRender->Draw2dText(pos, text.Chars(), colour, scale);
			pos.Y(pos.Y() + scale);
		}
		pos.Y(pos.Y() + scale);
	}
}

void C_DebugResourceRenderPaneAnimationCycle::Close()
{
	m_Filename.Clear();
	for (U32 i = 0; i < Animation::E_PoseFieldType_Count; ++i)
	{
		m_FieldCounts[i] = 0;
	}
	m_pCycle = 0;
}



//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

// Leaves contain information for elements in the tree.

class I_DebugResourceRenderLeaf
{
public:
	virtual ~I_DebugResourceRenderLeaf() {}

	virtual Util::C_String GetName() const = 0;

	virtual I_DebugResourceRenderPane* InitPane() const {return NULL;}

	virtual bool HasChildren() const {return false;}
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const {}

	virtual bool QueryHighlight(Maths::RGBA& o_Colour) const {return false;}

};

//----------------------------------------------------------------------------

// This leaf represents a childless string.

class C_DebugResourceRenderLeafString : public I_DebugResourceRenderLeaf
{
public:
	explicit C_DebugResourceRenderLeafString(const Util::C_String& text, const Maths::RGBA* pHightlightColour = NULL);
	virtual ~C_DebugResourceRenderLeafString() {}

	virtual Util::C_String GetName() const;

	virtual bool QueryHighlight(Maths::RGBA& o_Colour) const;

private:
	Util::C_String		m_Text;
	const Maths::RGBA*	m_pHighlightColour; ///< weak ref

};

C_DebugResourceRenderLeafString::C_DebugResourceRenderLeafString(const Util::C_String& text, const Maths::RGBA* pHightlightColour)
	: I_DebugResourceRenderLeaf()
	, m_Text(text)
	, m_pHighlightColour(pHightlightColour)
{
}

Util::C_String C_DebugResourceRenderLeafString::GetName() const
{
	return m_Text;
}

bool C_DebugResourceRenderLeafString::QueryHighlight(Maths::RGBA& o_Colour) const
{
	if (m_pHighlightColour != NULL)
	{
		o_Colour = *m_pHighlightColour;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------

// This leaf represents a node in the meta-debug hierarchy

class C_DebugResourceRenderLeafMetaDebugNode : public I_DebugResourceRenderLeaf
{
public:
	explicit C_DebugResourceRenderLeafMetaDebugNode(const MetaDebug::C_MetaNode* pMetaDebug);
	virtual ~C_DebugResourceRenderLeafMetaDebugNode() {}

	virtual Util::C_String GetName() const;

	virtual bool HasChildren() const;
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

	virtual bool QueryHighlight(Maths::RGBA& o_Colour) const;

private:
	const MetaDebug::C_MetaNode* m_pMetaDebug; ///< weak ref

	static Util::C_String GetAttributeString(const MetaDebug::C_Attribute& attribute);

	static const Maths::RGBA* GetHighlightColourForDebugLevel(const DebugLevel::Level level);

	static const Maths::RGBA s_HighlightColourDebug;
	static const Maths::RGBA s_HighlightColourInfo;
	static const Maths::RGBA s_HighlightColourWarning;
	static const Maths::RGBA s_HighlightColourError;
	static const Maths::RGBA s_HighlightColourFatal;

};

const Maths::RGBA C_DebugResourceRenderLeafMetaDebugNode::s_HighlightColourDebug(0.0f, 0.4f, 0.8f, 1.0f);
const Maths::RGBA C_DebugResourceRenderLeafMetaDebugNode::s_HighlightColourInfo(0.8f, 0.8f, 0.8f, 1.0f);
const Maths::RGBA C_DebugResourceRenderLeafMetaDebugNode::s_HighlightColourWarning(0.8f, 0.5f, 0.0f, 1.0f);
const Maths::RGBA C_DebugResourceRenderLeafMetaDebugNode::s_HighlightColourError(0.8f, 0.0f, 0.0f, 1.0f);
const Maths::RGBA C_DebugResourceRenderLeafMetaDebugNode::s_HighlightColourFatal(1.0f, 0.0f, 0.2f, 1.0f);

C_DebugResourceRenderLeafMetaDebugNode::C_DebugResourceRenderLeafMetaDebugNode(const MetaDebug::C_MetaNode* pMetaDebug)
	: I_DebugResourceRenderLeaf()
	, m_pMetaDebug(pMetaDebug)
{
	BIZ_ASSERT(pMetaDebug != NULL);
}

Util::C_String C_DebugResourceRenderLeafMetaDebugNode::GetName() const
{
	return m_pMetaDebug->GetName();
}

bool C_DebugResourceRenderLeafMetaDebugNode::HasChildren() const
{
	if (m_pMetaDebug->GetChildCount() > 0)
	{
		return true;
	}

	switch (m_pMetaDebug->GetType())
	{
	case MetaDebug::E_Type_AttributeList:	return (static_cast<const MetaDebug::C_AttributeList*>(m_pMetaDebug)->GetAttributeCount() > 0);
	case MetaDebug::E_Type_Log:				return (static_cast<const MetaDebug::C_Log*>(m_pMetaDebug)->GetMessageCount() > 0);
	default:								return false;
	}
}

void C_DebugResourceRenderLeafMetaDebugNode::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	for (const MetaDebug::C_MetaNode* pChild = m_pMetaDebug->GetFirstChild(); pChild != NULL; pChild = pChild->GetNextSibling())
	{
		o_Children.PushBack(new C_DebugResourceRenderLeafMetaDebugNode(pChild));
	}

	switch (m_pMetaDebug->GetType())
	{
	case MetaDebug::E_Type_AttributeList:
		{
			const MetaDebug::C_AttributeList* pAttributeList = static_cast<const MetaDebug::C_AttributeList*>(m_pMetaDebug);

			for (S32 i = 0; i < pAttributeList->GetAttributeCount(); ++i)
			{
				const MetaDebug::C_Attribute* pAttribute = pAttributeList->GetAttributeByIndex(i);
				o_Children.PushBack(new C_DebugResourceRenderLeafString(GetAttributeString(*pAttribute)));
			}
		}
		break;

	case MetaDebug::E_Type_Log:
		{
			const MetaDebug::C_Log* pLog = static_cast<const MetaDebug::C_Log*>(m_pMetaDebug);

			for (S32 i = 0; i < pLog->GetMessageCount(); ++i)
			{
				const MetaDebug::C_LogMessage* pMessage = pLog->GetMessageByIndex(i);
				o_Children.PushBack(new C_DebugResourceRenderLeafString(pMessage->GetMessage(), GetHighlightColourForDebugLevel(pMessage->GetDebugLevel())));
			}
		}
		break;
	}
}

bool C_DebugResourceRenderLeafMetaDebugNode::QueryHighlight(Maths::RGBA& o_Colour) const
{
	const Maths::RGBA* pColour = GetHighlightColourForDebugLevel(m_pMetaDebug->GetDebugLevel());

	if (pColour != NULL)
	{
		o_Colour = *pColour;
		return true;
	}

	return false;
}

const Maths::RGBA* C_DebugResourceRenderLeafMetaDebugNode::GetHighlightColourForDebugLevel(const DebugLevel::Level level)
{
	switch (level)
	{
	case DebugLevel::Debug:	return &s_HighlightColourDebug;
	case DebugLevel::Info:	return &s_HighlightColourInfo;
	case DebugLevel::Warn:	return &s_HighlightColourWarning;
	case DebugLevel::Error:	return &s_HighlightColourError;
	case DebugLevel::Fatal:	return &s_HighlightColourFatal;
	default:				return NULL;
	}
}

Util::C_String C_DebugResourceRenderLeafMetaDebugNode::GetAttributeString(const MetaDebug::C_Attribute& attribute)
{
	Util::C_String text = attribute.GetName() + " = ";

	switch (attribute.GetType())
	{
	case MetaDebug::E_AttributeType_Void:
		{
			text += "<void>";
		}
		break;

	case MetaDebug::E_AttributeType_U32:
		{
			text += Util::C_String::Format("%d", attribute.GetValueU32());
		}
		break;

	case MetaDebug::E_AttributeType_S32:
		{
			text += Util::C_String::Format("%i", attribute.GetValueS32());
		}
		break;

	case MetaDebug::E_AttributeType_F32:
		{
			text += Util::C_String::Format("%f", attribute.GetValueF32());
		}
		break;

	case MetaDebug::E_AttributeType_String:
		{
			text += attribute.GetValueString();
		}
		break;

	case MetaDebug::E_AttributeType_Time:
		{
            const Time::C_DecomposedTime dt(attribute.GetValueTime(), true);
            text += Time::FormatLongDateString(dt) + " - " + Time::FormatTimeString(dt) + " GMT";
		}
		break;

	default:
		{
			text += "<unknown>";
		}
		break;
	}

	return text;
}

//----------------------------------------------------------------------------

// This leaf represents a meta-debug hierarchy

class C_DebugResourceRenderLeafMetaDebug : public I_DebugResourceRenderLeaf
{
public:
	explicit C_DebugResourceRenderLeafMetaDebug(const MetaDebug::C_MetaNode* pMetaDebugRoot);
	virtual ~C_DebugResourceRenderLeafMetaDebug() {}

	virtual Util::C_String GetName() const {return "Debug";}

	virtual bool HasChildren() const {return true;}
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	const MetaDebug::C_MetaNode* m_pMetaDebugRoot; ///< weak ref

};

C_DebugResourceRenderLeafMetaDebug::C_DebugResourceRenderLeafMetaDebug(const MetaDebug::C_MetaNode* pMetaDebugRoot)
	: I_DebugResourceRenderLeaf()
	, m_pMetaDebugRoot(pMetaDebugRoot)
{
	BIZ_ASSERT(pMetaDebugRoot != NULL);
}

void C_DebugResourceRenderLeafMetaDebug::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	for (const MetaDebug::C_MetaNode* pMetaDebug = m_pMetaDebugRoot; pMetaDebug != NULL; pMetaDebug = pMetaDebug->GetNextSibling())
	{
		o_Children.PushBack(new C_DebugResourceRenderLeafMetaDebugNode(pMetaDebug));
	}
}

//----------------------------------------------------------------------------

// This leaf represents a single texture.

class C_DebugResourceRenderLeafTexture : public I_DebugResourceRenderLeaf
{
public:
	C_DebugResourceRenderLeafTexture(C_DebugResourceRenderPaneTexture* pPane, const Display::C_Texture2D* pTexture, const Util::C_String& textureName);
	virtual ~C_DebugResourceRenderLeafTexture() {}

	virtual Util::C_String GetName() const {return m_TextureName;}

	virtual I_DebugResourceRenderPane* InitPane() const;

private:
	C_DebugResourceRenderPaneTexture*			m_pPane;
	const Display::C_Texture2D*					m_pTexture;
	Util::C_String								m_TextureName;

	C_DebugResourceRenderLeafTexture(const C_DebugResourceRenderLeafTexture& copy); // can not copy
	C_DebugResourceRenderLeafTexture& operator = (const C_DebugResourceRenderLeafTexture& copy); // can not copy

};

C_DebugResourceRenderLeafTexture::C_DebugResourceRenderLeafTexture(C_DebugResourceRenderPaneTexture* pPane, const Display::C_Texture2D* pTexture, const Util::C_String& textureName)
	: I_DebugResourceRenderLeaf()
	, m_pPane(pPane)
	, m_pTexture(pTexture)
	, m_TextureName(textureName)
{
}

I_DebugResourceRenderPane* C_DebugResourceRenderLeafTexture::InitPane() const
{
	m_pPane->Init(m_pTexture);
	return m_pPane;
}

//----------------------------------------------------------------------------

// This leaf represents a list of textures.

class C_DebugResourceRenderLeafTextureList : public I_DebugResourceRenderLeaf
{
public:
	C_DebugResourceRenderLeafTextureList(C_DebugResourceRenderPaneTexture* pPane, const WorldRender::RenderModelResourceUsage& resourceUsage);
	virtual ~C_DebugResourceRenderLeafTextureList() {}

	virtual Util::C_String GetName() const {return "Textures";}

	virtual bool HasChildren() const {return !m_ResourceUsage.textureInfoList.Empty();}
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	C_DebugResourceRenderPaneTexture*				m_pPane;
	const WorldRender::RenderModelResourceUsage&	m_ResourceUsage;

	C_DebugResourceRenderLeafTextureList(const C_DebugResourceRenderLeafTextureList& copy); // can not copy
	C_DebugResourceRenderLeafTextureList& operator = (const C_DebugResourceRenderLeafTextureList& copy); // can not copy

};

C_DebugResourceRenderLeafTextureList::C_DebugResourceRenderLeafTextureList(C_DebugResourceRenderPaneTexture* pPane, const WorldRender::RenderModelResourceUsage& resourceUsage)
	: I_DebugResourceRenderLeaf()
	, m_pPane(pPane)
	, m_ResourceUsage(resourceUsage)
{
}

void C_DebugResourceRenderLeafTextureList::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	o_Children.Reserve(m_ResourceUsage.textureInfoList.Size());

	for (WorldRender::RenderModelTextureInfoList::ConstIterator it = m_ResourceUsage.textureInfoList.Begin(); it != m_ResourceUsage.textureInfoList.End(); ++it)
	{
		o_Children.PushBack(new C_DebugResourceRenderLeafTexture(m_pPane, it->GetValue().pDisplayTexture, it->GetValue().textureName));
	}
}

//----------------------------------------------------------------------------

// This leaf represents a model or an element.

class C_DebugResourceRenderLeafModelElements : public I_DebugResourceRenderLeaf
{
public:
	C_DebugResourceRenderLeafModelElements(C_DebugResourceRenderPaneModel* pPane, const GameResource::C_ModelData* pModelData, const WorldRender::C_RenderModelData* pRenderModelData, const S32 modelIndex, const S32 elementIndex);
	virtual ~C_DebugResourceRenderLeafModelElements() {}

	virtual Util::C_String GetName() const;

	virtual I_DebugResourceRenderPane* InitPane() const;

	virtual bool HasChildren() const;
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	C_DebugResourceRenderPaneModel*			m_pPane; ///< weak ref
	const GameResource::C_ModelData*		m_pModelData; ///< weak ref
	const WorldRender::C_RenderModelData*	m_pRenderModelData; ///< weak ref
	S32										m_ModelIndex; ///< index of root model, -1 for all
	S32										m_ElementIndex; ///< index of element, -1 for all

	C_DebugResourceRenderLeafModelElements(const C_DebugResourceRenderLeafModelElements& copy); // can not copy
	C_DebugResourceRenderLeafModelElements& operator = (const C_DebugResourceRenderLeafModelElements& copy); // can not copy

};

C_DebugResourceRenderLeafModelElements::C_DebugResourceRenderLeafModelElements(C_DebugResourceRenderPaneModel* pPane, const GameResource::C_ModelData* pModelData, const WorldRender::C_RenderModelData* pRenderModelData, const S32 modelIndex, const S32 elementIndex)
	: I_DebugResourceRenderLeaf()
	, m_pPane(pPane)
	, m_pModelData(pModelData)
	, m_pRenderModelData(pRenderModelData)
	, m_ModelIndex(modelIndex)
	, m_ElementIndex(elementIndex)
{
	BIZ_ASSERT((modelIndex >= 0) || (elementIndex < 0)); // element index must be <0 if model index is <0
}

Util::C_String C_DebugResourceRenderLeafModelElements::GetName() const
{
	const char* name = "Models";

	if (m_ModelIndex >= 0)
	{
		const GameResource::C_Model* pModel = m_pModelData->GetModel(m_ModelIndex);

		if (m_ElementIndex < 0)
		{
			return pModel->GetName();
		}
		else
		{
			return pModel->GetElement(m_ElementIndex)->GetName();
		}
	}
	else
	{
		return "Models";
	}
}

I_DebugResourceRenderPane* C_DebugResourceRenderLeafModelElements::InitPane() const
{
	if (m_ModelIndex >= 0)
	{
		m_pPane->Init(m_pModelData, m_pRenderModelData, m_ModelIndex, m_ElementIndex);
		return m_pPane;
	}

	return NULL;
}

bool C_DebugResourceRenderLeafModelElements::HasChildren() const
{
	if (m_ModelIndex < 0)
		return true;

	return (m_ElementIndex < 0) ? true : (m_pModelData->GetModel(m_ModelIndex)->GetElement(m_ElementIndex)->GetFirstChild() != NULL);
}

void C_DebugResourceRenderLeafModelElements::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	// add child models
	{
		const GameResource::C_Model* pModel = (m_ModelIndex < 0) ? m_pModelData->GetRootModel() : m_pModelData->GetModel(m_ModelIndex)->GetFirstChild();

		while (pModel != NULL)
		{
			const S32 modelIndex = pModel->GetIndex();

			if (modelIndex < m_pRenderModelData->GetNumModels())
			{
				o_Children.PushBack(new C_DebugResourceRenderLeafModelElements(m_pPane, m_pModelData, m_pRenderModelData, modelIndex, -1));
			}

			pModel = pModel->GetNextSibling();
		}
	}

	// add child elements
	if (m_ModelIndex >= 0)
	{
		const GameResource::C_Model* pModel = m_pModelData->GetModel(m_ModelIndex);
		const WorldRender::C_Model* pRenderModel = &m_pRenderModelData->GetModel(m_ModelIndex);

		const GameResource::C_ModelElement* pElement = (m_ElementIndex < 0) ? pModel->GetRootElement() : pModel->GetElement(m_ElementIndex)->GetFirstChild();

		while (pElement != NULL)
		{
			const S32 elementIndex = pElement->GetIndex();

			if (elementIndex < pRenderModel->GetNumElements())
			{
				o_Children.PushBack(new C_DebugResourceRenderLeafModelElements(m_pPane, m_pModelData, m_pRenderModelData, m_ModelIndex, elementIndex));
			}

			pElement = pElement->GetNextSibling();
		}
	}
}

//----------------------------------------------------------------------------

// This leaf represents a collection of textured models.

class C_DebugResourceRenderLeafModelSet : public I_DebugResourceRenderLeaf
{
public:
	C_DebugResourceRenderLeafModelSet(C_DebugResourceRenderPaneModelSet* pPane, const Util::C_String& fileName, const GameResource::C_ModelData* pModelData, const WorldRender::C_RenderModelData* pRenderModelData, const MetaDebug::C_MetaNode* pMetaDebugRoot);
	virtual ~C_DebugResourceRenderLeafModelSet() {}

	virtual Util::C_String GetName() const;

	virtual I_DebugResourceRenderPane* InitPane() const;

	virtual bool HasChildren() const {return true;}
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	C_DebugResourceRenderPaneModelSet*		m_pPane; ///< weak ref
	Util::C_String							m_FileName; ///< resource filename
	const GameResource::C_ModelData*		m_pModelData; ///< weak ref
	const WorldRender::C_RenderModelData*	m_pRenderModelData; ///< weak ref
	const MetaDebug::C_MetaNode*			m_pMetaDebugRoot; ///< weak ref
	WorldRender::RenderModelResourceUsage	m_ResourceUsage;

	C_DebugResourceRenderLeafModelSet(const C_DebugResourceRenderLeafModelSet& copy); // can not copy
	C_DebugResourceRenderLeafModelSet& operator = (const C_DebugResourceRenderLeafModelSet& copy); // can not copy

};

C_DebugResourceRenderLeafModelSet::C_DebugResourceRenderLeafModelSet(C_DebugResourceRenderPaneModelSet* pPane, const Util::C_String& fileName, const GameResource::C_ModelData* pModelData, const WorldRender::C_RenderModelData* pRenderModelData, const MetaDebug::C_MetaNode* pMetaDebugRoot)
	: I_DebugResourceRenderLeaf()
	, m_pPane(pPane)
	, m_FileName(fileName)
	, m_pModelData(pModelData)
	, m_pRenderModelData(pRenderModelData)
	, m_pMetaDebugRoot(pMetaDebugRoot)
	, m_ResourceUsage()
{
	pRenderModelData->AccumulateResourceUsage(m_ResourceUsage);
}

Util::C_String C_DebugResourceRenderLeafModelSet::GetName() const
{
	return Sys::RemoveFileExtension(Sys::ExtractFilename(m_FileName)).Chars();
}

I_DebugResourceRenderPane* C_DebugResourceRenderLeafModelSet::InitPane() const
{
	m_pPane->Init(m_FileName, m_pModelData, &m_ResourceUsage);
	return m_pPane;
}

void C_DebugResourceRenderLeafModelSet::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	o_Children.Resize(2);
	o_Children[0] = new C_DebugResourceRenderLeafModelElements(m_pPane->GetModelPane(), m_pModelData, m_pRenderModelData, -1, -1);
	o_Children[1] = new C_DebugResourceRenderLeafTextureList(m_pPane->GetTexturePane(), m_ResourceUsage);

	if (m_pMetaDebugRoot != NULL)
	{
		o_Children.PushBack(new C_DebugResourceRenderLeafMetaDebug(m_pMetaDebugRoot));
	}
}

//----------------------------------------------------------------------------

// This leaf represents a resource type.

class C_DebugResourceRenderLeafResourceBase : public I_DebugResourceRenderLeaf
{
public:
	explicit C_DebugResourceRenderLeafResourceBase(const Resource::C_ResourceTypeId resourceTypeId);
	virtual ~C_DebugResourceRenderLeafResourceBase() {}

	virtual Util::C_String GetName() const {return Resource::C_TypeRegistry::GetInstance()->GetResourceTypeName(m_ResourceTypeId);}

	virtual bool HasChildren() const {return !m_HandleList.Empty();}

protected:
	U32 GetNumResources() const {return m_HandleList.Size();}
	const Resource::C_HandleBase& GetResourceHandleBase(const U32 index) const {return m_HandleList[index];}

private:
	Resource::C_ResourceTypeId	m_ResourceTypeId;
	Resource::C_HandleList		m_HandleList;

	C_DebugResourceRenderLeafResourceBase(const C_DebugResourceRenderLeafResourceBase& copy); // can not copy
	C_DebugResourceRenderLeafResourceBase& operator = (const C_DebugResourceRenderLeafResourceBase& copy); // can not copy

};

C_DebugResourceRenderLeafResourceBase::C_DebugResourceRenderLeafResourceBase(const Resource::C_ResourceTypeId resourceTypeId)
	: I_DebugResourceRenderLeaf()
	, m_ResourceTypeId(resourceTypeId)
	, m_HandleList()
{
	Resource::C_Manager::GetInstance()->GetHandleList(m_HandleList, resourceTypeId);
}

//----------------------------------------------------------------------------

// This leaf represents a resource type that's interested in individual resources.

template <class T_ResourceType>
class C_DebugResourceRenderLeafResourceImpl : public C_DebugResourceRenderLeafResourceBase
{
public:
	C_DebugResourceRenderLeafResourceImpl();

protected:
	const T_ResourceType* GetResource(const U32 index) const;

};

template <class T_ResourceType>
C_DebugResourceRenderLeafResourceImpl<T_ResourceType>::C_DebugResourceRenderLeafResourceImpl()
	: C_DebugResourceRenderLeafResourceBase(T_ResourceType::StaticGetResourceTypeId()) 
{
}

template <class T_ResourceType>
const T_ResourceType* C_DebugResourceRenderLeafResourceImpl<T_ResourceType>::GetResource(const U32 index) const
{
	return typename Resource::C_Handle<T_ResourceType>(GetResourceHandleBase(index));
}

//----------------------------------------------------------------------------

// This leaf represents texture resources

class C_DebugResourceRenderLeafResourceTexture : public C_DebugResourceRenderLeafResourceImpl<TextureResource::C_TextureResource>
{
public:
	explicit C_DebugResourceRenderLeafResourceTexture(C_DebugResourceRenderPaneTexture* pPane);
	virtual ~C_DebugResourceRenderLeafResourceTexture() {}

	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	C_DebugResourceRenderPaneTexture*	m_pPane; ///< weak ref

	C_DebugResourceRenderLeafResourceTexture(const C_DebugResourceRenderLeafResourceTexture& copy); // can not copy
	C_DebugResourceRenderLeafResourceTexture& operator = (const C_DebugResourceRenderLeafResourceTexture& copy); // can not copy

};

C_DebugResourceRenderLeafResourceTexture::C_DebugResourceRenderLeafResourceTexture(C_DebugResourceRenderPaneTexture* pPane)
	: C_DebugResourceRenderLeafResourceImpl<TextureResource::C_TextureResource>()
	, m_pPane(pPane)
{
}

void C_DebugResourceRenderLeafResourceTexture::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	const U32 numResources = GetNumResources();

	o_Children.Resize(numResources);

	for (U32 i = 0; i < numResources; ++i)
	{
		const TextureResource::C_TextureResource* pResource = GetResource(i);
		const Util::C_String name(pResource->GetResourceFilename().Chars());
		o_Children[i] = new C_DebugResourceRenderLeafTexture(m_pPane, pResource->GetDisplayTexture(), name);
	}
}

//----------------------------------------------------------------------------

// This leaf represents a resource type with textured model data.

template <class T_ResourceType>
class C_DebugResourceRenderLeafResourceModelSet : public C_DebugResourceRenderLeafResourceImpl<T_ResourceType>
{
public:
	explicit C_DebugResourceRenderLeafResourceModelSet(C_DebugResourceRenderPaneModelSet* pPane);
	virtual ~C_DebugResourceRenderLeafResourceModelSet() {}

	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	C_DebugResourceRenderPaneModelSet*	m_pPane; ///< weak ref

	C_DebugResourceRenderLeafResourceModelSet(const C_DebugResourceRenderLeafResourceModelSet& copy); // can not copy
	C_DebugResourceRenderLeafResourceModelSet& operator = (const C_DebugResourceRenderLeafResourceModelSet& copy); // can not copy

};

template <class T_ResourceType>
C_DebugResourceRenderLeafResourceModelSet<T_ResourceType>::C_DebugResourceRenderLeafResourceModelSet(C_DebugResourceRenderPaneModelSet* pPane)
	: C_DebugResourceRenderLeafResourceImpl<T_ResourceType>()
	, m_pPane(pPane)
{
}

template <class T_ResourceType>
void C_DebugResourceRenderLeafResourceModelSet<T_ResourceType>::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	const U32 numResources = GetNumResources();

	o_Children.Resize(numResources);

	for (U32 i = 0; i < numResources; ++i)
	{
		const T_ResourceType* pResource = GetResource(i);
		o_Children[i] = new C_DebugResourceRenderLeafModelSet(m_pPane, pResource->GetResourceFilename().Chars(), pResource->GetModelData(), pResource->GetRenderModelData(), pResource->GetRootMetaNode());
	}
}

//----------------------------------------------------------------------------

// This leaf represents an animation cycle curve field (e.g. transforms, scalars, 3-d vectors...)

class C_DebugResourceRenderLeafAnimationCycleField: public I_DebugResourceRenderLeaf
{
public:
	C_DebugResourceRenderLeafAnimationCycleField(C_DebugResourceRenderPaneAnimationField* pFieldPane, const Animation::C_Cycle* pCycle, U32 fieldIndex);
	virtual ~C_DebugResourceRenderLeafAnimationCycleField() {}

	virtual Util::C_String	GetName() const		{ return m_Name; }

	virtual I_DebugResourceRenderPane* InitPane() const;

private:
	Util::C_String								m_Name;
	C_DebugResourceRenderPaneAnimationField*	m_pFieldPane;
	const Animation::C_Cycle*					m_pCycle;
	U32											m_FieldIndex;
};

C_DebugResourceRenderLeafAnimationCycleField::C_DebugResourceRenderLeafAnimationCycleField(
	C_DebugResourceRenderPaneAnimationField* pFieldPane, const Animation::C_Cycle* pCycle, U32 fieldIndex):
	m_pFieldPane(pFieldPane),
	m_pCycle(pCycle),
	m_FieldIndex(fieldIndex)
{
	m_Name = pCycle->GetFieldName(fieldIndex).GetString();
}

I_DebugResourceRenderPane* C_DebugResourceRenderLeafAnimationCycleField::InitPane() const
{
	m_pFieldPane->Init(m_pCycle, m_FieldIndex);
	return m_pFieldPane;
}


//----------------------------------------------------------------------------

// This leaf represents a list of animation cycle curve fields (e.g. transforms, scalars, 3-d vectors...)

class C_DebugResourceRenderLeafAnimationCycleFieldList: public I_DebugResourceRenderLeaf
{
public:
	C_DebugResourceRenderLeafAnimationCycleFieldList(C_DebugResourceRenderPaneAnimationField* pFieldPane, const Animation::C_Cycle* pCycle, Animation::E_PoseFieldType fieldType, U32 fieldCount, const Util::C_String& name);
	virtual ~C_DebugResourceRenderLeafAnimationCycleFieldList() {}

	virtual Util::C_String	GetName() const		{ return m_Name; }

	virtual bool HasChildren() const			{ return m_FieldCount > 0;}
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	Util::C_String								m_Name;
	const Animation::C_Cycle*					m_pCycle;
	C_DebugResourceRenderPaneAnimationField*	m_pFieldPane;
	U32											m_FieldCount;
	Animation::E_PoseFieldType					m_FieldType;
};

C_DebugResourceRenderLeafAnimationCycleFieldList::C_DebugResourceRenderLeafAnimationCycleFieldList(
		C_DebugResourceRenderPaneAnimationField* pFieldPane,
		const Animation::C_Cycle* pCycle, Animation::E_PoseFieldType fieldType, U32 fieldCount, const Util::C_String& name):
	m_pFieldPane(pFieldPane),
	m_Name(name),
	m_pCycle(pCycle),
	m_FieldCount(fieldCount),
	m_FieldType(fieldType)
{
}

void C_DebugResourceRenderLeafAnimationCycleFieldList::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	o_Children.Reserve(m_FieldCount);

	U32 numFields = m_pCycle->GetNumFields();

	Util::C_String name;
	U32 numChildren = 0;
	U32 cycleSize = m_pCycle->GetMemoryUsage();
	for (U32 i = 0; i < numFields; ++i)
	{
		if (m_pCycle->GetFieldType(i) == m_FieldType)
		{
			++numChildren;
			o_Children.PushBack(new C_DebugResourceRenderLeafAnimationCycleField(m_pFieldPane, m_pCycle, i));
		}

	}
	BIZ_ASSERT(numChildren == m_FieldCount);
}

//----------------------------------------------------------------------------

// This leaf represents an animation transform.
class C_DebugResourceRenderLeafAnimationTransform: public I_DebugResourceRenderLeaf
{
public:
	C_DebugResourceRenderLeafAnimationTransform(const Animation::C_JointTransform& transform, const Util::C_String& name);
	virtual ~C_DebugResourceRenderLeafAnimationTransform() {}

	virtual Util::C_String	GetName() const		{ return m_Name; }

	virtual bool HasChildren() const			{ return true;}
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	Animation::C_JointTransform		m_Transform;
	Util::C_String					m_Name;

	C_DebugResourceRenderLeafAnimationTransform(const C_DebugResourceRenderLeafAnimationTransform& copy); // can not copy
	C_DebugResourceRenderLeafAnimationTransform& operator = (const C_DebugResourceRenderLeafAnimationTransform& copy); // can not copy
};

C_DebugResourceRenderLeafAnimationTransform::C_DebugResourceRenderLeafAnimationTransform(
		const Animation::C_JointTransform& transform, const Util::C_String& name):
	m_Transform(transform),
	m_Name(name)
{
}

void C_DebugResourceRenderLeafAnimationTransform::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{

	Util::C_String str;
	o_Children.Resize(3);
	F32 xyzw[4];
	Maths::Store(m_Transform.GetRotation(), xyzw);
	str.Printf("Rotation: %.2f, %.2f, %.2f, %.2f", xyzw[0], xyzw[1], xyzw[2], xyzw[3]);
	o_Children[0] = new C_DebugResourceRenderLeafString(str);
	Maths::Store(m_Transform.GetTranslation(), xyzw);
	str.Printf("Translation: %.2f, %.2f, %.2f", xyzw[0], xyzw[1], xyzw[2]);
	o_Children[1] = new C_DebugResourceRenderLeafString(str);
	Maths::Store(m_Transform.GetScale(), xyzw);
	str.Printf("Scale: %.2f, %.2f, %.2f", xyzw[0], xyzw[1], xyzw[2]);
	o_Children[2] = new C_DebugResourceRenderLeafString(str);
}
//----------------------------------------------------------------------------

// This leaf represents a single animation cycle,

class C_DebugResourceRenderLeafAnimationCycle: public I_DebugResourceRenderLeaf
{
public:
	C_DebugResourceRenderLeafAnimationCycle(C_DebugResourceRenderPaneAnimationCycle* pPane, const Animation::C_Cycle* pCycle, const Util::C_String& filename);
	virtual ~C_DebugResourceRenderLeafAnimationCycle() {}

	virtual Util::C_String	GetName() const		{ return m_Name; }

	virtual bool HasChildren() const			{ return m_pCycle->GetNumFields() > 0;}
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

	virtual I_DebugResourceRenderPane* InitPane() const;

private:
	C_DebugResourceRenderPaneAnimationCycle*	m_pPane;
	const Animation::C_Cycle*					m_pCycle;
	Util::C_String								m_Name;
	Util::C_String								m_Filename;

	C_DebugResourceRenderLeafAnimationCycle(const C_DebugResourceRenderLeafAnimationCycle& copy); // can not copy
	C_DebugResourceRenderLeafAnimationCycle& operator = (const C_DebugResourceRenderLeafAnimationCycle& copy); // can not copy
};

C_DebugResourceRenderLeafAnimationCycle::C_DebugResourceRenderLeafAnimationCycle(C_DebugResourceRenderPaneAnimationCycle* pPane, const Animation::C_Cycle* pCycle, const Util::C_String& filename):
	m_pPane(pPane),
	m_pCycle(pCycle),
	m_Filename(filename)
{
	m_Name = Sys::RemoveFileExtension(Sys::ExtractFilename(filename)).Chars();
	if (m_pCycle->IsPartiallyLoaded())
	{
		m_Name += '*';
	}
}

void C_DebugResourceRenderLeafAnimationCycle::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	U32 fieldCounts[Animation::E_PoseFieldType_Count];
	for (U32 i = 0; i < Animation::E_PoseFieldType_Count; ++i)
	{
		fieldCounts[i] = 0;
	}

	U32 numFields = m_pCycle->GetNumFields();
	for (U32 i = 0; i < numFields; ++i)
	{
		fieldCounts[m_pCycle->GetFieldType(i)]++;
	}

	o_Children.Reserve(numFields);

	Util::C_String name;
	for (U32 fieldType = 0; fieldType < Animation::E_PoseFieldType_Count; ++fieldType)
	{
		if (fieldCounts[fieldType] > 0)
		{
			name.Printf("%s%s (%d)", GetAnimationFieldTypeName(static_cast<Animation::E_PoseFieldType>(fieldType)), fieldCounts[fieldType] != 1 ? "s" : "", fieldCounts[fieldType]);
			o_Children.PushBack(new C_DebugResourceRenderLeafAnimationCycleFieldList(m_pPane->GetFieldPane(), m_pCycle, static_cast<Animation::E_PoseFieldType>(fieldType), fieldCounts[fieldType], name));
		}
	}

	if (!m_pCycle->GetReferenceTransform().IsIdentity())
	{
		o_Children.PushBack(new C_DebugResourceRenderLeafAnimationTransform(m_pCycle->GetReferenceTransform(), "Reference Transform"));
	}

	if (!(m_pCycle->GetFirstTrajectoryTransform().IsIdentity() && m_pCycle->GetFinalTrajectoryTransform().IsIdentity()))
	{
		o_Children.PushBack(new C_DebugResourceRenderLeafAnimationTransform(m_pCycle->GetFirstTrajectoryTransform(), "Trajectory Transform Start"));
		o_Children.PushBack(new C_DebugResourceRenderLeafAnimationTransform(m_pCycle->GetFinalTrajectoryTransform(), "Trajectory Transform Finish"));
	}
}

I_DebugResourceRenderPane* C_DebugResourceRenderLeafAnimationCycle::InitPane() const
{
	m_pPane->Init(m_Filename, m_pCycle);
	return m_pPane;
}

//----------------------------------------------------------------------------

// This leaf represents all animation cycle resources.

class C_DebugResourceRenderLeafResourceAnimationCycle: public C_DebugResourceRenderLeafResourceImpl<Animation::C_CycleResource>
{
public:
	explicit C_DebugResourceRenderLeafResourceAnimationCycle(C_DebugResourceRenderPaneAnimationCycle* pPane);
	virtual ~C_DebugResourceRenderLeafResourceAnimationCycle() {}

	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	C_DebugResourceRenderPaneAnimationCycle*	m_pPane; ///< weak ref

	C_DebugResourceRenderLeafResourceAnimationCycle(const C_DebugResourceRenderLeafResourceAnimationCycle& copy); // can not copy
	C_DebugResourceRenderLeafResourceAnimationCycle& operator = (const C_DebugResourceRenderLeafResourceAnimationCycle& copy); // can not copy

};

C_DebugResourceRenderLeafResourceAnimationCycle::C_DebugResourceRenderLeafResourceAnimationCycle(C_DebugResourceRenderPaneAnimationCycle* pPane)
	: C_DebugResourceRenderLeafResourceImpl<Animation::C_CycleResource>()
	, m_pPane(pPane)
{
}

void C_DebugResourceRenderLeafResourceAnimationCycle::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	const U32 numResources = GetNumResources();

	o_Children.Reserve(numResources);

	for (U32 i = 0; i < numResources; ++i)
	{
		const Animation::C_CycleResource* pCycleResource = GetResource(i);
		if (!pCycleResource)
		{
			BIZ_ASSERT_FAIL("Missing cycle resource");
			continue;
		}
		if (!pCycleResource->GetCycle())
		{
			// Cycle not loaded?
			BIZ_ASSERT(pCycleResource->GetNotifyType() != Resource::E_Notify_DataReady);
			LOGWARN(Debug::GetLogger("gamesys.debugrender"), "Cycle '%s' not loaded", pCycleResource->GetResourceFilename().Chars());
			continue;
		}
		o_Children.PushBack(new C_DebugResourceRenderLeafAnimationCycle(m_pPane, pCycleResource->GetCycle(), pCycleResource->GetResourceFilename().Chars()));
	}
}

//----------------------------------------------------------------------------

// This leaf represents the root of the resource tree.

class C_DebugResourceRenderLeafRoot : public I_DebugResourceRenderLeaf
{
public:
	explicit C_DebugResourceRenderLeafRoot(GameResource::C_LevelSceneManager* pLevelSceneManager);
	virtual ~C_DebugResourceRenderLeafRoot();

	virtual Util::C_String GetName() const {return "Resources";}

	virtual bool HasChildren() const {return true;}
	virtual void CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const;

private:
	C_DebugResourceRenderPaneModelSet*			m_pModelSetPane;
	C_DebugResourceRenderPaneTexture*			m_pTexturePane;
	C_DebugResourceRenderPaneAnimationCycle*	m_pAnimationCyclePane;

	C_DebugResourceRenderLeafRoot(const C_DebugResourceRenderLeafRoot& copy); // can not copy
	C_DebugResourceRenderLeafRoot& operator = (const C_DebugResourceRenderLeafRoot& copy); // can not copy

};

C_DebugResourceRenderLeafRoot::C_DebugResourceRenderLeafRoot(GameResource::C_LevelSceneManager* pLevelSceneManager)
	: I_DebugResourceRenderLeaf()
	, m_pModelSetPane(NULL)
	, m_pTexturePane(NULL)
	, m_pAnimationCyclePane(NULL)
{
	m_pTexturePane = new C_DebugResourceRenderPaneTexture();
	m_pModelSetPane = new C_DebugResourceRenderPaneModelSet(pLevelSceneManager, m_pTexturePane);
	m_pAnimationCyclePane = new C_DebugResourceRenderPaneAnimationCycle();
}

C_DebugResourceRenderLeafRoot::~C_DebugResourceRenderLeafRoot()
{
	delete m_pModelSetPane;
	delete m_pTexturePane;
	delete m_pAnimationCyclePane;
}

void C_DebugResourceRenderLeafRoot::CreateChildren(Btl::Vector<I_DebugResourceRenderLeaf*>& o_Children) const
{
	o_Children.Resize(4);
	o_Children[0] = new C_DebugResourceRenderLeafResourceModelSet<GameResource::C_ModelResource>(m_pModelSetPane);
	o_Children[1] = new C_DebugResourceRenderLeafResourceModelSet<GameResource::C_LevelResource>(m_pModelSetPane);
	o_Children[2] = new C_DebugResourceRenderLeafResourceTexture(m_pTexturePane);
	o_Children[3] = new C_DebugResourceRenderLeafResourceAnimationCycle(m_pAnimationCyclePane);
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

// A branch holds a leaf, and when expanded also holds other branches.

class C_DebugResourceRenderBranch
{
public:
	explicit C_DebugResourceRenderBranch(I_DebugResourceRenderLeaf* pRoot);
	~C_DebugResourceRenderBranch();

	I_DebugResourceRenderPane* InitPane() const;

	bool CanExpand() const;
	bool IsExpanded() const;
	bool Expand();
	bool Collapse();

	U32 CalcSelectedHeight() const;
	U32 CalcTotalHeight() const;
	bool SelectedUp();
	bool SelectedDown();

	F32 Render2d(DbUtil::I_DebugRender* pDebugRender, const Maths::HVec2& origin, const F32 scale, const F32 limit, const U32 skipLeaves, const bool bSelected) const;

private:
	I_DebugResourceRenderLeaf*					m_pRoot; ///< strong ref
	Btl::Vector<C_DebugResourceRenderBranch*>	m_Children; ///< strong refs
	S32											m_Current; ///< current child index

	C_DebugResourceRenderBranch(const C_DebugResourceRenderBranch& copy); // can not copy
	C_DebugResourceRenderBranch& operator = (const C_DebugResourceRenderBranch& copy); // can not copy

};

struct C_NameSortStruct
{
	Util::C_String				m_Name;
	I_DebugResourceRenderLeaf*	m_pLeaf;

	inline bool operator > (const C_NameSortStruct& rhs) const {return stricmp(m_Name.Chars(), rhs.m_Name.Chars()) > 0;}
};

C_DebugResourceRenderBranch::C_DebugResourceRenderBranch(I_DebugResourceRenderLeaf* pRoot)
	: m_pRoot(pRoot)
	, m_Children()
	, m_Current(-1)
{
	BIZ_ASSERT(pRoot != NULL);
}

C_DebugResourceRenderBranch::~C_DebugResourceRenderBranch()
{
	while (IsExpanded())
	{
		Collapse();
	}

	BIZ_ASSERT(m_Children.Empty());
	BIZ_ASSERT(m_Current < 0);

	delete m_pRoot;
}

I_DebugResourceRenderPane* C_DebugResourceRenderBranch::InitPane() const
{
	return (m_Current < 0) ? m_pRoot->InitPane() : m_Children[m_Current]->InitPane();
}

bool C_DebugResourceRenderBranch::CanExpand() const
{
	return m_pRoot->HasChildren();
}

bool C_DebugResourceRenderBranch::IsExpanded() const
{
	return (m_Current >= 0);
}

bool C_DebugResourceRenderBranch::Expand()
{
	if (IsExpanded())
	{
		return m_Children[m_Current]->Expand();
	}
	else if (CanExpand())
	{
		BIZ_ASSERT(m_Children.Empty());

		Btl::Vector<I_DebugResourceRenderLeaf*> leaves;
		m_pRoot->CreateChildren(leaves);

		if (!leaves.Empty())
		{
			Btl::Vector<C_NameSortStruct> nameSort;
			nameSort.Resize(leaves.Size());

			for (S32 i = 0; i < leaves.Size(); ++i)
			{
				I_DebugResourceRenderLeaf* pSrc = leaves[i];
				C_NameSortStruct& dest = nameSort[i];

				dest.m_Name = pSrc->GetName();
				dest.m_pLeaf = pSrc;
			}

			Util::ShellSortArray<Btl::Vector<C_NameSortStruct>, C_NameSortStruct>(nameSort, nameSort.Size());

			m_Children.Resize(leaves.Size());

			for (S32 i = 0; i < leaves.Size(); ++i)
			{
				m_Children[i] = new C_DebugResourceRenderBranch(nameSort[i].m_pLeaf);
			}

			m_Current = 0;
			return true;
		}
	}

	return false;
}

bool C_DebugResourceRenderBranch::Collapse()
{
	if (IsExpanded())
	{
		C_DebugResourceRenderBranch* pSelectedChild = m_Children[m_Current];

		if (pSelectedChild->IsExpanded())
		{
			return pSelectedChild->Collapse();
		}
		else
		{
			BIZ_ASSERT(m_Children.Size() > 0);

			for (S32 i = 0; i < m_Children.Size(); ++i)
			{
				delete m_Children[i];
			}

			m_Children.Resize(0);
			m_Current = -1;
			return true;
		}
	}

	return false;
}

U32 C_DebugResourceRenderBranch::CalcSelectedHeight() const
{
	return (m_Current < 0) ? 0 : (1 + static_cast<U32>(m_Current) + m_Children[m_Current]->CalcSelectedHeight());
}

U32 C_DebugResourceRenderBranch::CalcTotalHeight() const
{
	return (m_Current < 0) ? 1 : (static_cast<U32>(m_Children.Size()) + m_Children[m_Current]->CalcTotalHeight());
}

bool C_DebugResourceRenderBranch::SelectedUp()
{
	if (IsExpanded())
	{
		C_DebugResourceRenderBranch* pSelectedChild = m_Children[m_Current];

		if (pSelectedChild->IsExpanded())
		{
			return pSelectedChild->SelectedUp();
		}
		else if (m_Current > 0)
		{
			--m_Current;
			return true;
		}
	}

	return false;
}

bool C_DebugResourceRenderBranch::SelectedDown()
{
	if (IsExpanded())
	{
		C_DebugResourceRenderBranch* pSelectedChild = m_Children[m_Current];

		if (pSelectedChild->IsExpanded())
		{
			return pSelectedChild->SelectedDown();
		}
		else if (m_Current < (m_Children.Size() - 1))
		{
			++m_Current;
			return true;
		}
	}

	return false;
}

F32 C_DebugResourceRenderBranch::Render2d(DbUtil::I_DebugRender* pDebugRender, const Maths::HVec2& origin, const F32 scale, const F32 limit, const U32 skipLeaves, const bool bSelected) const
{
	const F32 spacing = 1.0f * scale;
	U32 skipRemaining = skipLeaves;

	F32 bottom = origin.Y();
	BIZ_ASSERT(Maths::Abs(limit - bottom) > 1.0e-2f);

	// draw root
	if (skipRemaining > 0)
	{
		--skipRemaining;
	}
	else
	{
		const Maths::RGBA colour = bSelected ? Maths::RGBA(1.0f, 0.8f, 0.1f, 1.0f) : Maths::RGBA(1.0f, 1.0f, 1.0f, 1.0f);

		{
			Maths::RGBA symbolColour = colour;

			// draw highlight
			{
				Maths::RGBA highlightColour;

				if (m_pRoot->QueryHighlight(highlightColour))
				{
					const F32 barScale = 0.5f * scale;
					const F32 barOffset = 0.5f * (scale - barScale);

					const Maths::HVec2 barPos = origin + Maths::HVec2(-0.25 * (scale - barScale), 0.5f * (scale - barScale));
					const Maths::HVec2 barSize(barScale, barScale);

					pDebugRender->Draw2dBar(barPos, barSize, highlightColour);
					symbolColour = Maths::RGBA(0.0f, 0.0f, 0.0f, 1.0f);
				}
			}

			// draw +,-,etc.
			if (CanExpand())
			{
				pDebugRender->Draw2dText(origin, IsExpanded() ? "-" : "+", symbolColour, scale);
			}
		}

		// draw name
		{
			const Maths::HVec2 offset(spacing, 0.0f);
			const Util::C_String name = m_pRoot->GetName();

			pDebugRender->Draw2dText(origin + offset, name.Chars(), colour, scale);
		}

		// update position
		bottom += scale;
	}

	// draw children
	if (IsExpanded() && (bottom < limit))
	{
		for (S32 i = 0; i < m_Children.Size(); ++i)
		{
			const U32 totalLeaves = m_Children[i]->CalcTotalHeight();

			if (skipRemaining >= totalLeaves)
			{
				skipRemaining -= totalLeaves;
			}
			else
			{
				const Maths::HVec2 pos(origin.X() + spacing, bottom);
				bottom = m_Children[i]->Render2d(pDebugRender, pos, scale, limit, skipRemaining, (i == m_Current));
				
				skipRemaining = 0;

				if (bottom >= limit)
					break;
			}
		}
	}

	return bottom;
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

// The tree shows a hierarchical view of data.

class C_DebugResourceRenderTree
{
public:
	C_DebugResourceRenderTree(GameResource::C_LevelSceneManager* pLevelSceneManager, Input::C_GameInput* pGameInput);
	~C_DebugResourceRenderTree();

	void Update(const Maths::HMat43& mCameraToWorld);
	void Render2d(DbUtil::I_DebugRender* pDebugRender) const;

private:
	C_DebugResourceRenderBranch			m_Root; ///< root of tree
	Input::C_GameInput*					m_pGameInput; ///< weak ref
	U32									m_RenderSkip; ///< number of leaves to skip when rendering
	I_DebugResourceRenderPane*			m_pCurrentPane; ////< weak ref

	C_DebugResourceRenderTree(const C_DebugResourceRenderTree& copy); // can not copy
	C_DebugResourceRenderTree& operator = (const C_DebugResourceRenderTree& copy); // can not copy

};

static Debug::Float g_DebugResourceRenderTreeX("GameSys.DebugResourceRender.Tree.PosX", 100.0f, Debug::Range(-1024.0f, 1024.0f));
static Debug::Float g_DebugResourceRenderTreeY("GameSys.DebugResourceRender.Tree.PosY", 100.0f, Debug::Range(-768.0f, 768.0f));
static Debug::Float g_DebugResourceRenderTreeScale("GameSys.DebugResourceRender.Tree.Scale", 568.0f, Debug::Range(1.0f, 1000.0f));
static Debug::Bool g_DebugResourceRenderTreeHide("GameSys.DebugResourceRender.Tree.Hide", false);

C_DebugResourceRenderTree::C_DebugResourceRenderTree(GameResource::C_LevelSceneManager* pLevelSceneManager, Input::C_GameInput* pGameInput)
	: m_Root(new C_DebugResourceRenderLeafRoot(pLevelSceneManager))
	, m_pGameInput(pGameInput)
	, m_RenderSkip(0)
	, m_pCurrentPane(NULL)
{
	BIZ_ASSERT(pLevelSceneManager != NULL);
}

C_DebugResourceRenderTree::~C_DebugResourceRenderTree()
{
	if (m_pCurrentPane != NULL)
	{
		m_pCurrentPane->Close();
		m_pCurrentPane = NULL;
	}
}

void C_DebugResourceRenderTree::Update(const Maths::HMat43& mCameraToWorld)
{
	bool bSelectionChanged = false;

	if (m_pGameInput->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadUp, 16, 0))
	{
		if (m_Root.SelectedUp())
		{
			bSelectionChanged = true;
		}
		else if (m_RenderSkip > 0)
		{
			--m_RenderSkip;
		}
	}
	else if (m_pGameInput->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadDown, 16, 0))
	{
		if (m_Root.SelectedDown())
		{
			bSelectionChanged = true;
		}
		else
		{
			++m_RenderSkip;
		}
	}
	else if (m_pGameInput->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadLeft, 16, 0))
	{
		bSelectionChanged |= m_Root.Collapse();
	}
	else if (m_pGameInput->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadRight, 16, 0))
	{
		bSelectionChanged |= m_Root.Expand();
	}

	{
		const U32 selectedHeight = m_Root.CalcSelectedHeight();
		const U32 totalHeight = m_Root.CalcTotalHeight();
		const U32 rangeHeight = static_cast<U32>(ceilf(g_DebugResourceRenderTreeScale / g_DebugResourceRenderTextSize));

		if (totalHeight <= rangeHeight)
		{
			m_RenderSkip = 0;
		}
		else if ((m_RenderSkip + rangeHeight) > totalHeight)
		{
			m_RenderSkip = totalHeight - rangeHeight;
		}

		if (m_RenderSkip > selectedHeight)
		{
			m_RenderSkip = selectedHeight;
		}
		else if ((m_RenderSkip + rangeHeight) <= selectedHeight)
		{
			m_RenderSkip = (selectedHeight - rangeHeight) + 1;
		}
	}

	if (bSelectionChanged)
	{
		if (m_pCurrentPane != NULL)
		{
			m_pCurrentPane->Close();
			m_pCurrentPane = NULL;
		}

		m_pCurrentPane = m_Root.InitPane();
	}

	if (m_pCurrentPane != NULL)
	{
		m_pCurrentPane->Update(mCameraToWorld);
	}
}

void C_DebugResourceRenderTree::Render2d(DbUtil::I_DebugRender* pDebugRender) const
{
	BIZ_ASSERT(pDebugRender != NULL);

	if (m_pCurrentPane != NULL)
	{
		m_pCurrentPane->Render2d(pDebugRender);
	}

	if (g_DebugResourceRenderTreeHide)
		return;

	const Maths::HVec2 origin(g_DebugResourceRenderTreeX.Value(), g_DebugResourceRenderTreeY.Value());
	const F32 scale = g_DebugResourceRenderTextSize;
	const F32 height = g_DebugResourceRenderTreeScale;
	const F32 limit = origin.Y() + height;
	
	const F32 bottom = m_Root.Render2d(pDebugRender, origin, scale, limit, m_RenderSkip, true);

	const Maths::RGBA colour(0.8f, 0.2f, 1.0f, 1.0f);
	const char* text = "-------- More --------";

	if (m_RenderSkip > 0)
	{
		const Maths::HVec2 pos(origin.X(), origin.Y() - scale);
		pDebugRender->Draw2dText(pos, text, colour, scale);
	}

	const U32 totalHeight = m_Root.CalcTotalHeight();
	const U32 rangeHeight = static_cast<U32>(ceilf(g_DebugResourceRenderTreeScale / scale));

	if ((m_RenderSkip + rangeHeight) < totalHeight)
	{
		const Maths::HVec2 pos(origin.X(), bottom);
		pDebugRender->Draw2dText(pos, text, colour, scale);
	}
}

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

C_DebugResourceRender::C_DebugResourceRender()
	: m_pLevelSceneManager(NULL)
	, m_pGameInput(NULL)
	, m_pTree(NULL)
{
}

C_DebugResourceRender::~C_DebugResourceRender()
{
	BIZ_ASSERT(m_pTree == NULL);
}

void C_DebugResourceRender::Init(GameResource::C_LevelSceneManager* pLevelSceneManager, Input::C_GameInput* pGameInput)
{
	BIZ_ASSERT(m_pTree == NULL);
	m_pLevelSceneManager = pLevelSceneManager;
	m_pGameInput = pGameInput;
}

void C_DebugResourceRender::Update(const Maths::HMat43& mCameraToWorld)
{
	if (!g_DebugResourceRenderEnable)
	{
		SAFE_DELETE_PTR(m_pTree);
	}
	else
	{
		if (m_pTree == NULL)
		{
			m_pTree = new C_DebugResourceRenderTree(m_pLevelSceneManager, m_pGameInput);
		}

		m_pTree->Update(mCameraToWorld);
	}
}

void C_DebugResourceRender::Render2d() const
{
	if (!g_DebugResourceRenderEnable)
		return;

	if (m_pTree != NULL)
		m_pTree->Render2d(DbUtil::DefaultDebugRenderSys()->Immediate());
}

void C_DebugResourceRender::Close()
{
	m_pLevelSceneManager = NULL;
	m_pGameInput = NULL;
	SAFE_DELETE_PTR(m_pTree);
}

//////////////////////////////////////////////////////////////////////////////

} // namespace GameDebug
