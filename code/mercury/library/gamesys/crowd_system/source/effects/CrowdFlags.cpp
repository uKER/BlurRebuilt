#include "CrowdSystemPch.h"

#include <crowd_system/effects/CrowdFlags.h>
#include <crowd_system/CrowdDebug.h>
#include <crowd_system/CrowdTextureCache.h>

#include <SceneGraph/View.h>
#include <SceneGraph/RenderGroupSettings.h>

#define SCENEGRAPH_ALLOW_INCLUDE // only for CrowdFlags.cpp
#include <SceneGraph/SceneGraphImpl.h>

#ifdef PLATFORM_PS3
#include <cell/cgb.h>
#endif

#include <System/debug/I_DebugRender.h>

#include <dynamic/cloth/Cloth.h>

#include <core/maths/hvec/new/IncludeMinimal.h>
#include <core/maths/hvec/new/IncludeMinimal.inl>

EXTERN_SHADER(vsCrowdFlag);
EXTERN_SHADER(vsCrowdFlagPrepass);
EXTERN_SHADER(psCrowdFlag);
EXTERN_SHADER(psCrowdFlagPrepass);

namespace AnimatedCrowd
{

Debug::Bool	g_DrawCrowdFlags("GameSys.CrowdSystem.Effects.Flags.Draw", true);
extern Debug::Float	g_FlagOffset;

Debug::Float g_LodFadeX("GameSys.CrowdSystem.Effects.Flags.LodFadeX", 0.3125f);
Debug::Float g_LodFadeY("GameSys.CrowdSystem.Effects.Flags.LodFadeY", -(1.f / 32.f));
Debug::Float g_LodFadeZ("GameSys.CrowdSystem.Effects.Flags.LodFadeZ", 0.f);
Debug::Float g_LodFadeW("GameSys.CrowdSystem.Effects.Flags.LodFadeW", 5.f);

Display2::C_VertexElement CrowdFlagVertex::m_Elements[CrowdFlagVertex::m_ElementCount] =
{
	Display2::C_VertexElement( 0, 0, Display2::E_VertexElementType_F32_3, Display2::E_VertexElementUsage_Position, 0), 
	Display2::C_VertexElement( 0, 12, Display2::E_VertexElementType_F32_2, Display2::E_VertexElementUsage_TexCoord, 0),
	Display2::C_VertexElement( 0, 20, Display2::E_VertexElementType_F32_3, Display2::E_VertexElementUsage_Normal, 0),
	VERTEXELEMENT_END()
};

C_CrowdFlags::C_CrowdFlags(SceneGraph::C_SceneGraph* pSceneGraph, const U32 prepassSubtechniqueHandle)
	: Display::I_Renderer("CrowdFlags")
	, SceneGraph::I_Renderer(pSceneGraph)
	, m_PrepassSubtechniqueHandle(prepassSubtechniqueHandle)
	, m_pLightBufferAddress(NULL)
	, m_HalfPixelOffset(0.f)
	, m_Luminance(1.f, 1.f, 1.f, 1.f)
	, m_FogColour(0.25f, 0.25f, 0.25f, 1.f)
	, m_DepthFogSettings(0.0001f, 25.f, 1.f, 0.f)
	, m_pFlags(NULL)
	, m_NumFlags(0)
	, m_ViewFlags(0)
	, m_NumSides(2)
	, m_ClothSimulationGroupHandle(-1)
	, m_pTextureCache(NULL)
{
	m_NumBaseFlags = 0;
	m_BufferIndex = 0;
	m_Created = false;
	m_Ready = false;
	
	m_SamplerState.SetDefaultStates();
	m_SamplerState.SetTextureFilter(Display2::E_TextureFilter_Linear);
	m_SamplerState.SetTextureAddress(Display2::E_TextureAddress_Wrap);

	m_LightingSamplerState.SetDefaultStates();
	m_LightingSamplerState.SetTextureAddress(Display2::E_TextureAddress_Clamp);
	m_LightingSamplerState.SetTextureFilter(Display2::E_TextureFilter_Point);

	m_CreationParams.Width = 0.f;
	m_CreationParams.Height = 0.f;
	m_CreationParams.NumColumns = 0;
	m_CreationParams.NumRows = 0;

	m_CreationParams.StretchFactor = 0.95f;
	m_CreationParams.ParticleMass = 0.01f;
	m_CreationParams.WindCoeff = 0.01f;

	m_NumBaseFlagColumns = 0;
	m_NumBaseFlagRows = 0;

	m_NumBaseFlagPoints = 0;
	m_NumBaseFlagIndices = 0;
}

C_CrowdFlags::~C_CrowdFlags()
{
	DestroyRenderer();
}

void C_CrowdFlags::SetViewFlags(const U32 viewFlags)
{
	m_ViewFlags = viewFlags;
}

bool C_CrowdFlags::OnCreate()
{
	if (m_Created)
		return true;

	RESOURCE_MEMORY_ID("CrowdFlags");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsCrowdFlag", 0, &m_FlagVertexShader, REF_SHADER(vsCrowdFlag)))
		return false;

	m_FlagTransformParameter		= m_FlagVertexShader.GetParameter(200, "g_FlagTransform");
	m_FlagColourParameter			= m_FlagVertexShader.GetParameter(204, "g_FlagColour");
	m_FlagLodFadeParameter			= m_FlagVertexShader.GetParameter(205, "g_LodFade");
	m_FlagLodDistanceScaleParameter	= m_FlagVertexShader.GetParameter(206, "g_LodDistanceScale");
	m_FlagSHParameter				= m_FlagVertexShader.GetParameter(207, "g_FlagSH");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsCrowdFlagPrepass", 0, &m_FlagPrepassVertexShader, REF_SHADER(vsCrowdFlagPrepass)))
		return false;

	m_FlagPrepassTransformParameter	= m_FlagPrepassVertexShader.GetParameter(200, "g_FlagTransform");
	m_FlagPrepassNearFarParameter	= m_FlagPrepassVertexShader.GetParameter(204, "g_NearFar");

	if (!Display::CreatePixelShaderFromFunc("Shader/psCrowdFlag", 0, &m_FlagPixelShader, REF_SHADER(psCrowdFlag)))
		return false;

	m_FlagHalfPixelOffsetParameter	= m_FlagPixelShader.GetParameter(200, "g_HalfPixelOffset");
	m_FlagLuminanceParameter		= m_FlagPixelShader.GetParameter(201, "g_Luminance");
	m_FlagFogColourParameter		= m_FlagPixelShader.GetParameter(202, "g_FogColour");
	m_FlagDepthFogParameter			= m_FlagPixelShader.GetParameter(203, "g_DepthFog");

	if (!Display::CreatePixelShaderFromFunc("Shader/psCrowdFlagPrepass", 0, &m_FlagPrepassPixelShader, REF_SHADER(psCrowdFlagPrepass)))
		return false;

	Display2::Device()->CreateVertexDeclaration(&CrowdFlagVertex::m_Elements[0], &m_VertexDeclaration);

	m_ClothSimulationGroupHandle = Scheduler::CreateGroup();

	m_EnabledFlags.ClearBits();

	if (m_NumBaseFlags)
	{
		BuildFlagMesh(m_CreationParams.Width, m_CreationParams.Height, m_CreationParams.NumColumns, m_CreationParams.NumRows, m_NumBaseFlags,
			m_CreationParams.StretchFactor, m_CreationParams.ParticleMass, m_CreationParams.WindCoeff);
	}

	m_Created = true;

	return true;
}

bool C_CrowdFlags::BuildFlagMesh(const F32 width, const F32 height, const U32 numColumns, const U32 numRows, const U32 numBaseFlags,
								 const F32 stretchFactor, const F32 particleMass, const F32 windCoeff)
{
	BIZ_ASSERT(numColumns > 1 && numRows > 1);
	BIZ_ASSERT(numBaseFlags < 32); // TODO: [rthomas]	Could increase if REALLY needed

	m_CreationParams.Width = width;
	m_CreationParams.Height = height;
	m_CreationParams.NumColumns = numColumns;
	m_CreationParams.NumRows = numRows;

	m_CreationParams.StretchFactor = stretchFactor;
	m_CreationParams.ParticleMass = particleMass;
	m_CreationParams.WindCoeff = windCoeff;

	m_NumBaseFlags = numBaseFlags;

	if (!m_NumBaseFlags)
		return false;

	DynamicEffect::Cloth::SheetParameters sheetParams;

	sheetParams.size.x = numColumns;
	sheetParams.size.y = numRows;

	sheetParams.stretchFactor = stretchFactor;
	sheetParams.stretchNaturalLength = Maths::Min<F32>(width / (float)(numColumns - 1), height / (float)(numRows - 1));

	sheetParams.windResistanceCoeff = windCoeff;

	sheetParams.worldMatrix.SetIdentity();

	sheetParams.particleMass = particleMass;
	sheetParams.gravity = 9.8f;

	for (U32 i = 0, count = static_cast<U32>(sheetParams.size.y); i < count; ++i)
		sheetParams.pinnedPositions.PushBack(MathsStorage::Int2(0, i));

	m_ClothSheets.Resize(m_NumBaseFlags);

	for (U32 i = 0, count = m_NumBaseFlags; i < count; ++i)
	{
		m_ClothSheets[i].Initialise(sheetParams);
		m_ClothSheets[i].Update(1.f / 30.f);
	}

	m_ParticleSeperation = sheetParams.stretchNaturalLength;

	m_NumBaseFlagColumns = sheetParams.size.x;
	m_NumBaseFlagRows = sheetParams.size.y;

	m_NumBaseFlagPoints = m_NumBaseFlagColumns * m_NumBaseFlagRows;
	m_NumBaseFlagIndices = (m_NumBaseFlagColumns - 1) * (m_NumBaseFlagRows - 1) * 6;

	m_DeltaU = 1.f / (float)(m_NumBaseFlagColumns - 1);
	m_DeltaV = 1.f / (float)(m_NumBaseFlagRows - 1);

	for (U32 iVb = 0; iVb < 2; ++iVb)
	{
		if (!Display2::Device()->CreateVertexBuffer(sizeof(CrowdFlagVertex) * m_NumBaseFlagPoints * m_NumSides * m_NumBaseFlags, 0, &m_VertexBuffer[iVb]))
			return false;

		UpdateVertexBuffer(iVb);
	}

	if (!Display2::Device()->CreateIndexBuffer(sizeof(U16) * m_NumBaseFlagIndices * m_NumSides * m_NumBaseFlags, 0, Display2::E_IndexFormat_16, &m_IndexBuffer))
		return false;

	// Fill index buffer as triangle list
	volatile U16* pIndices = (U16*)m_IndexBuffer.Lock();
	if (pIndices)
	{
		U32 iIndex = 0;
		for (U32 iFlag = 0; iFlag < m_NumBaseFlags; ++iFlag)
		{
			for (U32 iSide = 0; iSide < m_NumSides; ++iSide)
			{
				for (U32 j = 0, rowCount = (m_NumBaseFlagRows - 1); j < rowCount; ++j)
				{
					for (U32 i = 0, colCount = (m_NumBaseFlagColumns - 1); i < colCount; ++i)
					{
						if (iSide == 0)
						{
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j) * m_NumBaseFlagColumns) + (i)));
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j) * m_NumBaseFlagColumns) + (i + 1)));
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j + 1) * m_NumBaseFlagColumns) + (i)));

							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j + 1) * m_NumBaseFlagColumns) + (i)));
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j) * m_NumBaseFlagColumns) + (i + 1)));
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j + 1) * m_NumBaseFlagColumns) + (i + 1)));
						}
						else
						{
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j) * m_NumBaseFlagColumns) + (i)));
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j + 1) * m_NumBaseFlagColumns) + (i)));
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j) * m_NumBaseFlagColumns) + (i + 1)));

							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j) * m_NumBaseFlagColumns) + (i + 1)));
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j + 1) * m_NumBaseFlagColumns) + (i)));
							pIndices[iIndex++] = (U16)((iFlag * m_NumBaseFlagPoints * m_NumSides) + (iSide * m_NumBaseFlagPoints) + (((j + 1) * m_NumBaseFlagColumns) + (i + 1)));
						}
					}
				}
			}
		}
		m_IndexBuffer.Unlock();
	}

	m_Ready = true;

	return true;
}

void C_CrowdFlags::UpdateVertexBuffer(const U32 bufferIndex)
{
	BIZ_ASSERT(bufferIndex < 2);

	CrowdFlagVertex* __restrict pVertices = (CrowdFlagVertex*)m_VertexBuffer[bufferIndex].Lock();
	
	if (pVertices)
	{
		for (U32 iFlag = 0, flagCount = m_NumBaseFlags; iFlag < flagCount; ++iFlag)
		{
			for (U32 iSide = 0, sideCount = m_NumSides; iSide < sideCount; ++iSide)
			{
				for (U32 j = 0, rowCount = m_NumBaseFlagRows; j < rowCount; ++j)
				{
					for (U32 i = 0, colCount = m_NumBaseFlagColumns; i < colCount; ++i)
					{
						const U32 vertexIndex = ((j * colCount) + i) + (m_NumBaseFlagPoints * iSide) + (iFlag * m_NumBaseFlagPoints * m_NumSides);

						const DynamicEffect::Cloth::C_Particle& particle = m_ClothSheets[iFlag].Particle(i, j);
						
						pVertices[vertexIndex].x = particle.position.X();
						pVertices[vertexIndex].y = particle.position.Y();
						pVertices[vertexIndex].z = particle.position.Z();

						if (iSide == 0)
						{
							pVertices[vertexIndex].u = (i * m_DeltaU);
							pVertices[vertexIndex].v = (j * m_DeltaV);

							pVertices[vertexIndex].nx = particle.normal.X();
							pVertices[vertexIndex].ny = particle.normal.Y();
							pVertices[vertexIndex].nz = particle.normal.Z();
						}
						else
						{
							pVertices[vertexIndex].u = (i * m_DeltaU);
							pVertices[vertexIndex].v = (j * m_DeltaV);

							pVertices[vertexIndex].nx = -particle.normal.X();
							pVertices[vertexIndex].ny = -particle.normal.Y();
							pVertices[vertexIndex].nz = -particle.normal.Z();
						}
					}
				}
			}
		}
		m_VertexBuffer[bufferIndex].Unlock();
	}
}

void C_CrowdFlags::OnDestroy()
{
	for (U32 i = 0, count = static_cast<U32>(m_ClothSheets.Size()); i < count; ++i)
		m_ClothSheets[i].Shutdown();
	m_ClothSheets.Destroy();

	Display2::Device()->Destroy(&m_FlagVertexShader);
	Display2::Device()->Destroy(&m_FlagPrepassVertexShader);
	Display2::Device()->Destroy(&m_FlagPixelShader);
	Display2::Device()->Destroy(&m_FlagPrepassPixelShader);

	if (m_Ready)
	{
		for (U32 i = 0; i < 2; ++i)
			Display2::Device()->DestroyVertexBuffer(&m_VertexBuffer[i]);

		Display2::Device()->DestroyIndexBuffer(&m_IndexBuffer);
	}

	Display2::Device()->DestroyVertexDeclaration(&m_VertexDeclaration);

	if (m_ClothSimulationGroupHandle >= 0)
	{
		Scheduler::DestroyGroup(m_ClothSimulationGroupHandle);
		m_ClothSimulationGroupHandle = -1;
	}

	m_Created = false;
	m_Ready = false;
}

// scenegraph I_GatherNode implementation
void C_CrowdFlags::OnGather(const SceneGraph::C_View& view) const
{
	if (!m_Ready || !g_DrawCrowdFlags || !m_NumFlags)
		return;

	if (view.RenderNodeEnabled(m_NodeFlag))
	{
		if (m_ViewFlags & view.GetTypeFlag())
		{
			const SceneGraph::C_RenderPass* pRenderPass = SceneGraph::I_Renderer::m_pSceneGraph->GetRenderPass(m_RenderPass);

			//for (FlagVector::ConstIterator iter = m_Flags.Begin(); iter != m_Flags.End(); ++iter)
			for (U32 i = 0, count = m_NumFlags; i < count; ++i)
			{
				//const C_CrowdFlagInstance& flag = *iter;
				const C_CrowdFlagInstance& flag = m_pFlags[i];
				
				if (flag.BaseFlagIndex < 0)
					continue;

				// see if we need to calculate depth for sorting
				SceneGraph::VolatileNodeInfo volatileNodeInfo;

				// calculate depth if we need it
				if (pRenderPass->GetVolatileInfoFlags() & SceneGraph::E_VolatileNodeInfo_Distance)
				{
					F32 depthKey = 0;
					Maths::HVec4 centre(flag.Transform.AW());
					centre.W(Maths::Constants<Maths::HF32>::One());
					Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
					Maths::HF32 depthScale(view.GetViewData().depthScale);
					Maths::HF32 depth = (centre | forward) * depthScale;
					Maths::Store(depth, &depthKey);
					volatileNodeInfo.distance = (U32)depth;
				}

				view.AddVisibleNode(&flag, m_RenderPass, 0, volatileNodeInfo.data);
			}
		}
	}
}

void C_CrowdFlags::UpdateClothPins(const U32 flagIndex, const Maths::HMat43& transform)
{
	BIZ_ASSERT(flagIndex < m_NumBaseFlags);

	for (U32 i = 0, count = m_NumBaseFlagRows; i < count; ++i)
	{
		Maths::HVec3 position = Maths::HVec3(g_FlagOffset.Value() - (i * m_ParticleSeperation), 0.f, 0.f) * transform;

		m_ClothSheets[flagIndex].SetPinnedParticlePosition(0, i, position);
	}
	
	m_EnabledFlags.SetBit(flagIndex);
}

void C_CrowdFlags::ScheduleClothSimulationTask(const Scheduler::PolicyHandle jobQueuePolicyHandle, const F32 deltaTime)
{
	if (!m_Created || !m_Ready)
		return;

	for (U32 i = 0, count = static_cast<U32>(m_ClothSheets.Size()); i < count; ++i)
	{
		if (m_EnabledFlags[i])
			m_ClothSheets[i].Update(deltaTime);
	}

	UpdateVertexBuffer((m_BufferIndex + 1) % 2);
	
	m_EnabledFlags.ClearBits();
}

void C_CrowdFlags::FlushClothSimulationTask() const
{
	if (m_ClothSimulationGroupHandle >= 0)
		Scheduler::Flush(m_ClothSimulationGroupHandle);

	m_BufferIndex = ((m_BufferIndex + 1) % 2);
}

void C_CrowdFlags::SetDynamicLightingTexture(const void* pLightBufferAddress)
{
	m_pLightBufferAddress = pLightBufferAddress;
}

void C_CrowdFlags::SetHalfPixelOffset(const U32 widthInPixels, const U32 heightInPixels)
{
	if ((widthInPixels > 0) && (heightInPixels > 0))
	{
		m_HalfPixelOffset.X(0.5f / widthInPixels);
		m_HalfPixelOffset.Y(0.5f / heightInPixels);
	}
}

void C_CrowdFlags::SetLuminance(const Display::C_Float4& luminance)
{
	m_Luminance = luminance;
}

void C_CrowdFlags::SetFogColour(const Maths::C_RGBA_F32& fogColour)
{
	m_FogColour = fogColour;
}

void C_CrowdFlags::SetDepthFogSettings(const Maths::HVec4& depthFogSettings)
{
	m_DepthFogSettings = depthFogSettings;
}

FORCE_INLINE void StoreMat43(MathsStorage::MatrixRow44* dest, const Maths::Mat43& src)
{
	Maths::Store(Maths::MakeVector4(Maths::RowX(src)), &dest->v[0]);
	Maths::Store(Maths::MakeVector4(Maths::RowY(src)), &dest->v[4]);
	Maths::Store(Maths::MakeVector4(Maths::RowZ(src)), &dest->v[8]);
	Maths::Store(Maths::MakeVector4(Maths::MakePoint3(Maths::RowW(src))), &dest->v[12]);
}

FORCE_INLINE void StoreMat44(MathsStorage::MatrixRow44* dest, const Maths::Mat44& src)
{
	Maths::Store(Maths::RowX(src), &dest->v[0]);
	Maths::Store(Maths::RowY(src), &dest->v[4]);
	Maths::Store(Maths::RowZ(src), &dest->v[8]);
	Maths::Store(Maths::RowW(src), &dest->v[12]);
}

void C_CrowdFlags::Render(const SceneGraph::C_View& view, SceneGraph::C_Context& context, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const
{
	PERF_EVENT("CrowdFlags.Render");

	if (m_Ready && batch.m_NodeCount)
	{
		Display::C_Context* pDisplay = Display::GetContext(&context);

		if(context.GetContextFlags() != pDisplay->GetFlags())
		{
#if PLATFORM_PS3
			pDisplay->SetVertexShaderConstantF(4, (F32*)&view.GetViewSettings3D().GetViewProjMatrix(), 4);
			pDisplay->SetVertexShaderConstantF(8, (F32*)&view.GetViewSettings3D().GetViewMatrix(), 4);
#endif
		}

		context.SetContextFlags(pDisplay->GetFlags());

		//Set vertex and pixel shaders
		pDisplay->SetVertexDeclaration(&m_VertexDeclaration);
#ifdef PLATFORM_PS3
		pDisplay->SetVtxAttrUsageFlags(UINT_MAX);
#endif

#ifdef PLATFORM_PS3
		uint32_t new_mask = 0;
		cellCgbGetVertexAttributeOutputMask( m_FlagVertexShader.GetCgbProgram(), &new_mask );
		// Enable User Clip 0 attribute
		new_mask |= CELL_GCM_ATTRIB_OUTPUT_MASK_UC0; 
		pDisplay->SetVertexAttribOutputMask( new_mask );
#endif

		pDisplay->SetIndexBuffer(&m_IndexBuffer);
		pDisplay->SetVertexBuffer(0, &(m_VertexBuffer[m_BufferIndex]), 0, sizeof(CrowdFlagVertex));

		if (view.GetRenderGroupSettings())
		{
			if (view.GetRenderGroupSettings()->GetSubtechnique() == m_PrepassSubtechniqueHandle)
				RenderPrepass(pDisplay, view, visibleNodes, batch);
			else
				RenderMain(pDisplay, view, visibleNodes, batch);
		}
	}
}

void C_CrowdFlags::RenderMain(Display::C_Context* pDisplay, const SceneGraph::C_View& view, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const
{
	if (!pDisplay)
		return;

	pDisplay->SetVertexShader(&m_FlagVertexShader);
	pDisplay->SetPixelShader(&m_FlagPixelShader);

	pDisplay->SetAlphaTestState(Display::E_Bool_True, 0x0);
	pDisplay->SetAlphaToMaskEnable(Display::E_Bool_True);

	//Set Params
	Display::C_Float4 flagColourParam(1.f, 1.f, 1.f, 1.f);
	pDisplay->SetVertexShaderConstantF(m_FlagColourParameter.m_RegisterIndex, (F32*)&flagColourParam, m_FlagColourParameter.m_RegisterCount);

	Display::C_Float4 flagLodFadeParam(g_LodFadeX, g_LodFadeY, g_LodFadeZ, g_LodFadeW);
	pDisplay->SetVertexShaderConstantF(m_FlagLodFadeParameter.m_RegisterIndex, (F32*)&flagLodFadeParam, m_FlagLodFadeParameter.m_RegisterCount);

	Display::C_Float4 flagLodDistanceScaleParam(view.GetViewData().instanceDrawDistanceScaleRcp * view.GetViewData().forwardMultiplier, 0.f, 0.f, 0.f);
	pDisplay->SetVertexShaderConstantF(m_FlagLodDistanceScaleParameter.m_RegisterIndex, (F32*)&flagLodDistanceScaleParam, m_FlagLodDistanceScaleParameter.m_RegisterCount);

	pDisplay->SetTexture(1, m_pLightBufferAddress);
	pDisplay->SetSamplerState(1, &m_LightingSamplerState);

	Display::C_Float4 halfPixelOffsetParam(m_HalfPixelOffset.X(), m_HalfPixelOffset.Y(), 0.f, 0.f);
	pDisplay->SetPixelShaderParameterF(m_FlagHalfPixelOffsetParameter, (F32*)&halfPixelOffsetParam);
	pDisplay->SetPixelShaderParameterF(m_FlagLuminanceParameter, (F32*)&m_Luminance);
	
	Display::C_Float4 fogColourParam(m_FogColour.R(), m_FogColour.G(), m_FogColour.B(), m_FogColour.A());
	pDisplay->SetPixelShaderParameterF(m_FlagFogColourParameter, (F32*)&fogColourParam);
	Display::C_Float4 depthFogSettingsParam(m_DepthFogSettings.X(), m_DepthFogSettings.Y(), m_DepthFogSettings.Z(), m_DepthFogSettings.W());
	pDisplay->SetPixelShaderParameterF(m_FlagDepthFogParameter, (F32*)&depthFogSettingsParam);

	const U32 numTotalVertices = m_NumBaseFlagPoints * m_NumSides * m_NumBaseFlags;
	const U32 numIndicesPerFlag = m_NumBaseFlagIndices * m_NumSides;

	for (U32 i = 0, count = batch.m_NodeCount; i < count; ++i)
	{
		const C_CrowdFlagInstance* pInstance = (const C_CrowdFlagInstance*)visibleNodes[i + batch.m_StartIndex].m_pNode;
		if (pInstance)
		{
			const Display::C_Texture2D* pFlagTexture = NULL;
			if (m_pTextureCache)
			{
				if (pInstance->TextureIndex >= 0)
					pFlagTexture = m_pTextureCache->GetTexture(pInstance->TextureIndex);
			}

			pDisplay->SetTexture(0, pFlagTexture ? pFlagTexture->GetGPUTexture() : NULL);
			pDisplay->SetSamplerState(0, &m_SamplerState);

			const Maths::HMat43& flagTransform(pInstance->Transform);
			const S32& baseFlagIndex(pInstance->BaseFlagIndex);

			BIZ_ASSERT((static_cast<U32>(baseFlagIndex) >= 0) && (static_cast<U32>(baseFlagIndex) < m_NumBaseFlags));
			
			MathsStorage::MatrixRow44 destTransform;				
			StoreMat43(&destTransform, flagTransform);
			pDisplay->SetVertexShaderConstantF(m_FlagTransformParameter.m_RegisterIndex, (F32*)&destTransform, m_FlagTransformParameter.m_RegisterCount);

			pDisplay->SetVertexShaderConstantF(m_FlagSHParameter.m_RegisterIndex, pInstance->SH, m_FlagSHParameter.m_RegisterCount);

			pDisplay->DrawIndexedPrimitive(Display2::E_PrimitiveType_TriangleList, 0, 0, numTotalVertices, (numIndicesPerFlag * static_cast<U32>(baseFlagIndex)), numIndicesPerFlag);
		}
	}
}

void C_CrowdFlags::RenderPrepass(Display::C_Context* pDisplay, const SceneGraph::C_View& view, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const
{
	if (!pDisplay)
		return;

	Display::C_Float4 flagNearFarParam(view.GetViewSettings3D().GetNearClip(), view.GetViewSettings3D().GetFarClip(), view.GetViewSettings3D().GetNearClip(), view.GetViewData().farClip);
	pDisplay->SetVertexShaderConstantF(m_FlagPrepassNearFarParameter.m_RegisterIndex, (F32*)&flagNearFarParam, m_FlagPrepassNearFarParameter.m_RegisterCount);

	pDisplay->SetVertexShader(&m_FlagPrepassVertexShader);
	pDisplay->SetPixelShader(&m_FlagPrepassPixelShader);

	const U32 numTotalVertices = m_NumBaseFlagPoints * m_NumSides * m_NumBaseFlags;
	const U32 numIndicesPerFlag = m_NumBaseFlagIndices * m_NumSides;

	for (U32 i = 0, count = batch.m_NodeCount; i < count; ++i)
	{
		const C_CrowdFlagInstance* pInstance = (const C_CrowdFlagInstance*)visibleNodes[i + batch.m_StartIndex].m_pNode;
		if (pInstance)
		{
			const Maths::HMat43& flagTransform(pInstance->Transform);
			const S32& baseFlagIndex(pInstance->BaseFlagIndex);

			BIZ_ASSERT((static_cast<U32>(baseFlagIndex) >= 0) && (static_cast<U32>(baseFlagIndex) < m_NumBaseFlags));
			
			MathsStorage::MatrixRow44 destTransform;				
			StoreMat43(&destTransform, flagTransform);
			pDisplay->SetVertexShaderConstantF(m_FlagPrepassTransformParameter.m_RegisterIndex, (F32*)&destTransform, m_FlagPrepassTransformParameter.m_RegisterCount);

			pDisplay->DrawIndexedPrimitive(Display2::E_PrimitiveType_TriangleList, 0, 0, numTotalVertices, (numIndicesPerFlag * static_cast<U32>(baseFlagIndex)), numIndicesPerFlag);
		}
	}
}

void C_CrowdFlags::SetFlags(const C_CrowdFlagInstance* pFlags, const U32 numFlags)
{
	m_pFlags = pFlags;
	m_NumFlags = pFlags ? numFlags : 0;
}

void C_CrowdFlags::SetNodeType(C_CrowdFlagInstance* pFlagInstance) const
{
	if (pFlagInstance)
		pFlagInstance->m_Type = m_NodeType;
}

#ifndef CFG_GOLD
const F32 C_CrowdFlags::GetFlagWidth() const
{
	return m_CreationParams.Width;
}

const F32 C_CrowdFlags::GetFlagHeight() const
{
	return m_CreationParams.Height;
}

const F32 C_CrowdFlags::GetFlagStretchFactor() const
{
	return m_CreationParams.StretchFactor;
}

const F32 C_CrowdFlags::GetFlagWindCoeff() const
{
	return m_CreationParams.WindCoeff;
}

const F32 C_CrowdFlags::GetFlagParticleMass() const
{
	return m_CreationParams.ParticleMass;
}

void C_CrowdFlags::ResizeFlagMesh(const F32 width, const F32 height, const F32 stretchFactor, const F32 particleMass, const F32 windCoeff)
{
	if (!m_Created)
		return;

	for (U32 i = 0, count = static_cast<U32>(m_ClothSheets.Size()); i < count; ++i)
		m_ClothSheets[i].Shutdown();

	m_Ready = false;

	if (!m_NumFlags)
		return;

	m_CreationParams.Width = width;
	m_CreationParams.Height = height;

	m_CreationParams.StretchFactor = stretchFactor;
	m_CreationParams.ParticleMass = particleMass;
	m_CreationParams.WindCoeff = windCoeff;
	
	DynamicEffect::Cloth::SheetParameters sheetParams;

	sheetParams.size.x = m_CreationParams.NumColumns;
	sheetParams.size.y = m_CreationParams.NumRows;

	sheetParams.stretchFactor = stretchFactor;
	sheetParams.stretchNaturalLength = Maths::Min<F32>(width / (float)(m_CreationParams.NumColumns - 1), height / (float)(m_CreationParams.NumRows - 1));

	sheetParams.windResistanceCoeff = windCoeff;

	sheetParams.worldMatrix.SetIdentity();

	sheetParams.particleMass = particleMass;
	sheetParams.gravity = 9.8f;

	for (U32 i = 0, count = static_cast<U32>(sheetParams.size.y); i < count; ++i)
		sheetParams.pinnedPositions.PushBack(MathsStorage::Int2(0, i));

	for (U32 i = 0, count = m_NumBaseFlags; i < count; ++i)
	{
		m_ClothSheets[i].Initialise(sheetParams);
		m_ClothSheets[i].Update(1.f / 30.f);
	}

	BIZ_ASSERT(m_NumBaseFlagColumns == sheetParams.size.x);
	BIZ_ASSERT(m_NumBaseFlagRows == sheetParams.size.y);

	m_Ready = true;
}
#endif

}; // namespace AnimatedCrowd
