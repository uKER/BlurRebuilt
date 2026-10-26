#include "LightTrailsPch.h"
#include "LightTrails/LightTrails.h"
#include <SceneGraph/View.h>
#include "Display/Shaders/Constants.h"

#define SCENEGRAPH_ALLOW_INCLUDE
#include <SceneGraph/SceneGraphImpl.h>


EXTERN_SHADER(vsLightTrails);
EXTERN_SHADER(psLightTrails);
EXTERN_SHADER(vsParticleLightTrails);
EXTERN_SHADER(psParticleLightTrails);

namespace LightTrails
{
	Debug::Logger &g_LightTrailsLog( Debug::GetLogger("gamesys.lightTrails") );

	typedef Display::Vertex_XYZ_HDR_N2 Vertex;
	typedef Display::Vertex_XYZ_HDR_N2 ParticleVertex;

	//Instances
	void C_LightTrailInstance::Reset( const Maths::HVec3& position )
	{
		//Reset all to position and intensity 0
		m_RingBufferIndex = 0;
		for (U32 i = 0; i < LIGHTTRAILS_SIZE; i++)
		{
			m_Position[i] = position;
			m_Intensity[i] = 0.f;
			m_VectorX[i] = Maths::HVec3(0.f, 0.f, 0.f);
			m_VectorY[i] = Maths::HVec3(0.f, 0.f, 0.f);
		}

		m_LastActivateIndex = LIGHTTRAILS_SIZE;
	}

	void C_LightTrailInstance::Update( const Maths::HVec3& position, const Maths::HVec3& vectorX, const Maths::HVec3& vectorY, const F32& intensity )
	{
		m_Position[m_RingBufferIndex] = position;
		m_VectorX[m_RingBufferIndex] = vectorX;
		m_VectorY[m_RingBufferIndex] = vectorY;
		m_Intensity[m_RingBufferIndex] = intensity;
		if (intensity > 0.01f)
			m_LastActivateIndex = 0;
	}

	//Manager
	C_LightTrails::C_LightTrails(SceneGraph::C_SceneGraph* pSceneGraph) : Display::I_Renderer("LightTrails"), SceneGraph::I_Renderer(pSceneGraph), m_NearDistanceFade(80.f), m_FarDistanceFade(100.f)
	{
		m_bCreated = false;
		m_ViewFlags = 0xFFFFFFFF;
		for (U32 i = 0; i < LIGHTTRAILS_SIZE; i++)
			m_IntensityVector[i] = Maths::Clamp(1.f - F32(i) / F32(LIGHTTRAILS_SIZE - 2), 0.f, 1.f);
	}

	C_LightTrails::~C_LightTrails()
	{
		DestroyRenderer();
	}

	C_LightTrailInstance* C_LightTrails::AllocLightTrailInstance()
	{
		m_LightTrailList.PushBack();
		C_LightTrailInstance* pLightTrail = &m_LightTrailList.Back();
		pLightTrail->m_Type = m_NodeType;
		return pLightTrail;
	}

	bool C_LightTrails::OnCreate()
	{
		if (m_bCreated)
			return true;

		RESOURCE_MEMORY_ID("LightTrails");

		if (!Display::CreateVertexShaderFromFunc("Shader/vsLightTrails", 0, &m_LightTrailVertexShader, REF_SHADER(vsLightTrails)))
			return false;

		if (!Display::CreatePixelShaderFromFunc("Shader/psLightTrails", 0, &m_LightTrailPixelShader, REF_SHADER(psLightTrails)))
			return false;

		if (!Display::CreateVertexShaderFromFunc("Shader/vsParticleLightTrails", 0, &m_ParticleLightTrailVertexShader, REF_SHADER(vsParticleLightTrails)))
			return false;

		if (!Display::CreatePixelShaderFromFunc("Shader/psParticleLightTrails", 0, &m_ParticleLightTrailPixelShader, REF_SHADER(psParticleLightTrails)))
			return false;

		m_bCreated = true;

		return true;
	}

	void C_LightTrails::OnDestroy()
	{
		Display::Device()->Destroy(&m_LightTrailVertexShader);
		Display::Device()->Destroy(&m_LightTrailPixelShader);
		Display::Device()->Destroy(&m_ParticleLightTrailVertexShader);
		Display::Device()->Destroy(&m_ParticleLightTrailPixelShader);

		m_bCreated = false;
	}

	// scenegraph I_GatherNode implementation
	void C_LightTrails::OnGather(const SceneGraph::C_View& view) const
	{
		if (m_ViewFlags & view.GetTypeFlag())
		{
			if (view.RenderNodeEnabled(m_NodeFlag))
			{
				for (LightTrailList::ConstIterator it = m_LightTrailList.Begin(); it != m_LightTrailList.End(); ++it)
				{
					const C_LightTrailInstance& lightTrail = *it;

					// see if flare is visible in this view
					if(lightTrail.m_ViewFlags & view.GetTypeFlag())
					{
						// something to render?
						if (lightTrail.m_LastActivateIndex < LIGHTTRAILS_SIZE)
						{
							//Calculate approx sphere
							Maths::HVec3 lastPosition = lightTrail.GetLastPosition();
							Maths::HVec3 firtPosition = lightTrail.GetFirstPosition();
							Maths::HVec4 centre((lastPosition + firtPosition) / 2.f);
							centre.W(Maths::Constants<Maths::HF32>::One());
							F32 radius = (lastPosition - firtPosition).Length() / 2.f;
							Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
							Maths::HF32 depthScale(view.GetViewData().depthScale);
							Maths::HF32 depth = (centre | forward);

							if (depth < (m_FarDistanceFade + radius))
							{
								S32 visible = view.GetViewFrustum().CullSphere(centre, radius);
								if (visible)
								{
									// see if we need to calculate depth for sorting
									const SceneGraph::C_RenderPass* pRenderPass = SceneGraph::I_Renderer::m_pSceneGraph->GetRenderPass(m_RenderPass);
									SceneGraph::VolatileNodeInfo volatileNodeInfo;

									// calculate depth if we need it
									if(pRenderPass->GetVolatileInfoFlags() & SceneGraph::E_VolatileNodeInfo_Distance)
									{
										depth *= depthScale;
										volatileNodeInfo.distance = depth.F2I();
									}

									view.AddVisibleNode(&lightTrail, m_RenderPass, 0, volatileNodeInfo.data);
								}
							}
						}
					}
				}
			}
		}
	}

// 	void C_LightTrails::AddLightTrail(volatile Vertex* pVertices,  const C_LightTrailInstance& lightTrail ) const
// 	{
// 		//Get first positions
// 		Maths::HVec3 positions[4][2];
// 		Maths::HVec3 normals[4][2];
// 		F32 intensities[2];
// 		F32 fadeIntensity[2];
// 
// 		U32 index = (lightTrail.m_RingBufferIndex - 1 + LIGHTTRAILS_SIZE)%LIGHTTRAILS_SIZE;
// 
// 		normals[0][0] = (lightTrail.m_Intensity[index] > 0.01f) ? -lightTrail.m_VectorX[index] * lightTrail.m_SizeX : Maths::HVec3(0.f, 0.f, 0.f);
// 		normals[1][0] = (lightTrail.m_Intensity[index] > 0.01f) ? -lightTrail.m_VectorY[index] * lightTrail.m_SizeY : Maths::HVec3(0.f, 0.f, 0.f);
// 		normals[2][0] = (lightTrail.m_Intensity[index] > 0.01f) ? lightTrail.m_VectorX[index] * lightTrail.m_SizeX : Maths::HVec3(0.f, 0.f, 0.f);
// 		normals[3][0] = (lightTrail.m_Intensity[index] > 0.01f) ? lightTrail.m_VectorY[index] * lightTrail.m_SizeY : Maths::HVec3(0.f, 0.f, 0.f);
// 
// 		positions[0][0] = lightTrail.m_Position[index];
// 		positions[1][0] = lightTrail.m_Position[index];
// 		positions[2][0] = lightTrail.m_Position[index];
// 		positions[3][0] = lightTrail.m_Position[index];
// 
// 		intensities[0] = 0.f;
// 		fadeIntensity[0] = 1.f;
// 		
// 		for (U32 i = 1; i < (LIGHTTRAILS_SIZE - 1); i++)
// 		{
// 			U32 index2 = (index - 1 + LIGHTTRAILS_SIZE) % LIGHTTRAILS_SIZE;
// 			//Calculate 4 position
// 			U32 buffer = (i + 1) % 2;
// 			U32 buffer2 = i % 2;
// 
// 			normals[0][buffer2] = (lightTrail.m_Intensity[index2] > 0.01f) ? -lightTrail.m_VectorX[index2] * lightTrail.m_SizeX : Maths::HVec3(0.f, 0.f, 0.f);
// 			normals[1][buffer2] = (lightTrail.m_Intensity[index2] > 0.01f) ? -lightTrail.m_VectorY[index2] * lightTrail.m_SizeY : Maths::HVec3(0.f, 0.f, 0.f);
// 			normals[2][buffer2] = (lightTrail.m_Intensity[index2] > 0.01f) ? lightTrail.m_VectorX[index2] * lightTrail.m_SizeX : Maths::HVec3(0.f, 0.f, 0.f);
// 			normals[3][buffer2] = (lightTrail.m_Intensity[index2] > 0.01f) ? lightTrail.m_VectorY[index2] * lightTrail.m_SizeY : Maths::HVec3(0.f, 0.f, 0.f);
// 
// 			positions[0][buffer2] = lightTrail.m_Position[index2];
// 			positions[1][buffer2] = lightTrail.m_Position[index2];
// 			positions[2][buffer2] = lightTrail.m_Position[index2];
// 			positions[3][buffer2] = lightTrail.m_Position[index2];
// 
// 			intensities[buffer2] = lightTrail.m_Intensity[index2];
// 			fadeIntensity[buffer2] = fadeIntensity[buffer] * 0.80f;
// 
// 			Maths::HVec3 direction = lightTrail.m_Position[index2] - lightTrail.m_Position[index];
// 			//Fill vertex data
// 			for (U32 j = 0; j < 4; j++)
// 			{
// 				Maths::HVec4 colour(lightTrail.m_Colour.R(), lightTrail.m_Colour.G(), lightTrail.m_Colour.B(), lightTrail.m_Colour.A() * intensities[buffer] * fadeIntensity[buffer]);
// 				Maths::HVec4 colour2(lightTrail.m_Colour.R(), lightTrail.m_Colour.G(), lightTrail.m_Colour.B(), lightTrail.m_Colour.A() * intensities[buffer2] * fadeIntensity[buffer2]);
// 
// 				U32 vertex0 = j;
// 				U32 vertex1 = (j + 1) % 4;
// 				
// 				Maths::Store(positions[vertex0][buffer], (MathsStorage::Float3*)&pVertices[0].x);
// 				Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
// 				Maths::Store(normals[vertex0][buffer], (MathsStorage::Float3*)&pVertices[0].nx0);
// 				Maths::Store(direction, (MathsStorage::Float3*)&pVertices[0].nx1);
// 
// 				pVertices++;
// 
// 				Maths::Store(positions[vertex1][buffer], (MathsStorage::Float3*)&pVertices[0].x);
// 				Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
// 				Maths::Store(normals[vertex1][buffer], (MathsStorage::Float3*)&pVertices[0].nx0);
// 				Maths::Store(direction, (MathsStorage::Float3*)&pVertices[0].nx1);
// 
// 				pVertices++;
// 
// 				Maths::Store(positions[vertex1][buffer2], (MathsStorage::Float3*)&pVertices[0].x);
// 				Maths::Store(colour2, (MathsStorage::Float4*)&pVertices[0].r);
// 				Maths::Store(normals[vertex1][buffer2], (MathsStorage::Float3*)&pVertices[0].nx0);
// 				Maths::Store(direction, (MathsStorage::Float3*)&pVertices[0].nx1);
// 
// 				pVertices++;
// 
// 				Maths::Store(positions[vertex0][buffer2], (MathsStorage::Float3*)&pVertices[0].x);
// 				Maths::Store(colour2, (MathsStorage::Float4*)&pVertices[0].r);
// 				Maths::Store(normals[vertex0][buffer2], (MathsStorage::Float3*)&pVertices[0].nx0);
// 				Maths::Store(direction, (MathsStorage::Float3*)&pVertices[0].nx1);
// 
// 				pVertices++;
// 
// 			}
// 
// 			index = index2;
// 		}
// 
// 	}
	void C_LightTrails::AddLightTrail(volatile Vertex* pVertices,  const C_LightTrailInstance& lightTrail, const F32 fadeFactor ) const
	{
		Maths::HVec4 colour;
		Maths::HVec3 normalX;
		Maths::HVec3 normalY;

		//First degenerated vertex
		U32 index = (lightTrail.m_RingBufferIndex - 1 + LIGHTTRAILS_SIZE)%LIGHTTRAILS_SIZE;
		colour = Maths::HVec4(lightTrail.m_Colour.R(), lightTrail.m_Colour.G(), lightTrail.m_Colour.B(), 0.f);

		Maths::Store(lightTrail.m_Position[index], (MathsStorage::Float3*)&pVertices[0].x);
		Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
		normalX = lightTrail.m_VectorX[index] * lightTrail.m_SizeX;
		normalY = lightTrail.m_VectorY[index] * lightTrail.m_SizeY;
		Maths::Store(normalX , (MathsStorage::Float3*)&pVertices[0].nx0);
		Maths::Store(normalY , (MathsStorage::Float3*)&pVertices[0].nx1);
		pVertices++;

		for (U32 i = 0; i < (LIGHTTRAILS_SIZE - 1); i++)
		{
			index = (lightTrail.m_RingBufferIndex - 1 - i + LIGHTTRAILS_SIZE)%LIGHTTRAILS_SIZE;
			colour = Maths::HVec4(lightTrail.m_Colour.R(), lightTrail.m_Colour.G(), lightTrail.m_Colour.B(), (i == 0) ? F32(0.f) : F32(fadeFactor * lightTrail.m_Colour.A() * lightTrail.m_Intensity[index] * m_IntensityVector[i]));

			Maths::Store(lightTrail.m_Position[index], (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			normalX = lightTrail.m_VectorX[index] * lightTrail.m_SizeX;
			normalY = lightTrail.m_VectorY[index] * lightTrail.m_SizeY;
			Maths::Store(normalX , (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(normalY , (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			//Negative version
			colour.X(-colour.X());
			Maths::Store(lightTrail.m_Position[index], (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			normalX = lightTrail.m_VectorX[index] * lightTrail.m_SizeX;
			normalY = lightTrail.m_VectorY[index] * lightTrail.m_SizeY;
			Maths::Store(normalX , (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(normalY , (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;
		}

		//Last degenerated vertex
		Maths::Store(lightTrail.m_Position[index], (MathsStorage::Float3*)&pVertices[0].x);
		Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
		normalX = lightTrail.m_VectorX[index] * lightTrail.m_SizeX;
		normalY = lightTrail.m_VectorY[index] * lightTrail.m_SizeY;
		Maths::Store(normalX , (MathsStorage::Float3*)&pVertices[0].nx0);
		Maths::Store(normalY , (MathsStorage::Float3*)&pVertices[0].nx1);
		pVertices++;
	}

	void C_LightTrails::AddParticleLightTrail(volatile ParticleVertex* pVertices,  const C_LightTrailInstance& lightTrail, const F32 fadeFactor ) const
	{
		float sizeX = Maths::Max(0.2f, lightTrail.m_SizeX);
		float sizeY = Maths::Max(0.2f, lightTrail.m_SizeY);
		for (U32 i = 0; i < (LIGHTTRAILS_SIZE - 2); i++)
		{
			U32 index = (lightTrail.m_RingBufferIndex - 1 - i + LIGHTTRAILS_SIZE)%LIGHTTRAILS_SIZE;
			Maths::HVec4 colour = Maths::HVec4(lightTrail.m_Colour.R(), lightTrail.m_Colour.G(), lightTrail.m_Colour.B(), (i == 0) ? F32(0.f) : F32(fadeFactor * lightTrail.m_Colour.A() * lightTrail.m_Intensity[index] * m_IntensityVector[i] * lightTrail.m_CameraFadeFactor));

			Maths::HVec3 normalX = lightTrail.m_VectorX[index] * sizeX;
			Maths::HVec3 normalY = lightTrail.m_VectorY[index] * sizeY;

			Maths::HVec3 worldDirection = normalX ^ normalY;
			worldDirection.Normalise();
			Maths::HVec3 position;

			if (colour.W() < 0.01f)
			{
				normalX = normalY = Maths::HVec3(0.f, 0.f, 0.f);
			}


			position = lightTrail.m_Position[index];
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(-normalX, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			position = lightTrail.m_Position[index];
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(-normalY, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			position = lightTrail.m_Position[index];
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(normalX, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			position = lightTrail.m_Position[index];
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(normalY, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;


			//Middle1
			U32 index2 = (index - 1 + LIGHTTRAILS_SIZE)%LIGHTTRAILS_SIZE;
			colour = Maths::HVec4(lightTrail.m_Colour.R(), lightTrail.m_Colour.G(), lightTrail.m_Colour.B(), Maths::Lerp((i == 0) ? F32(0.f) : F32(fadeFactor * lightTrail.m_Colour.A() * lightTrail.m_Intensity[index] * m_IntensityVector[i]), F32(lightTrail.m_Colour.A() * lightTrail.m_Intensity[index2] * m_IntensityVector[i+1]), 0.33f) * lightTrail.m_CameraFadeFactor);
			
			normalX = Maths::Lerp(lightTrail.m_VectorX[index], lightTrail.m_VectorX[index2], 0.33f) * sizeX;
			normalY = Maths::Lerp(lightTrail.m_VectorY[index], lightTrail.m_VectorY[index2], 0.33f) * sizeY;

			worldDirection = normalX ^ normalY;
			worldDirection.Normalise();
			
			if (colour.W() < 0.01f)
			{
				normalX = normalY = Maths::HVec3(0.f, 0.f, 0.f);
			}

			Maths::HVec3 middlePosition = Maths::Lerp(lightTrail.m_Position[index], lightTrail.m_Position[index2], 0.33f); 
			position = middlePosition;
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(-normalX, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			position = middlePosition;
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(-normalY, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			position = middlePosition;
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(normalX, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			position = middlePosition;
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(normalY, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			//Middle2
			colour = Maths::HVec4(lightTrail.m_Colour.R(), lightTrail.m_Colour.G(), lightTrail.m_Colour.B(), Maths::Lerp((i == 0) ? F32(0.f) : F32(fadeFactor * lightTrail.m_Colour.A() * lightTrail.m_Intensity[index] * m_IntensityVector[i]), F32(lightTrail.m_Colour.A() * lightTrail.m_Intensity[index2] * m_IntensityVector[i+1]), 0.66f) * lightTrail.m_CameraFadeFactor);

			normalX = Maths::Lerp(lightTrail.m_VectorX[index], lightTrail.m_VectorX[index2], 0.66f) * sizeX;
			normalY = Maths::Lerp(lightTrail.m_VectorY[index], lightTrail.m_VectorY[index2], 0.66f) * sizeY;

			worldDirection = normalX ^ normalY;
			worldDirection.Normalise();

			if (colour.W() < 0.01f)
			{
				normalX = normalY = Maths::HVec3(0.f, 0.f, 0.f);
			}

			middlePosition = Maths::Lerp(lightTrail.m_Position[index], lightTrail.m_Position[index2], 0.66f); 
			position = middlePosition;
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(-normalX, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			position = middlePosition;
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(-normalY, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			position = middlePosition;
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(normalX, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;

			position = middlePosition;
			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			Maths::Store(normalY, (MathsStorage::Float3*)&pVertices[0].nx0);
			Maths::Store(worldDirection, (MathsStorage::Float3*)&pVertices[0].nx1);
			pVertices++;
		}
	}

	void C_LightTrails::Render(const SceneGraph::C_View& view, SceneGraph::C_Context& context, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const
	{
		PERF_EVENT("LightTrails.Render");

		
		Display::C_Context* pDisplay = Display::GetContext(&context);

		if(context.GetContextFlags() != pDisplay->GetFlags())
		{
	#if PLATFORM_PS3
			pDisplay->SetVertexShaderConstantF(4, (F32*)&view.GetViewSettings3D().GetViewProjMatrix(), 4);
	#endif
		}
		context.SetContextFlags(pDisplay->GetFlags());

		//Setup states
		pDisplay->SetBlendMode(Display::E_BlendMode_Additive);
		pDisplay->SetDepthEnable(Display::E_Bool_True);
		pDisplay->SetDepthWriteEnable(Display::E_Bool_False);
#ifdef PLATFORM_PS3
		pDisplay->SetCullMode(Display::E_CullMode_Ccw);
#else
		pDisplay->SetCullMode(Display::E_CullMode_Cw);
#endif
		
		//Draw Strip
	
		//Set vertex and pixel shaders
		pDisplay->SetVertexDeclaration(Vertex::m_pDeclaration);
#ifdef PLATFORM_PS3
		pDisplay->SetVtxAttrUsageFlags(UINT_MAX);
#endif
		pDisplay->SetVertexShader(&m_LightTrailVertexShader);
		pDisplay->SetPixelShader(&m_LightTrailPixelShader);

		S32 vertexCount = batch.m_NodeCount * (LIGHTTRAILS_SIZE) * 2;

		//Stack alloc
		F32* intensityFactors = (F32*)alloca(batch.m_NodeCount * sizeof(F32));

	#ifndef PLATFORM_PC
		// knock out until we have a begin vertices method for PC
	
		Vertex* pVertices = (Vertex*)pDisplay->BeginVertices(Display::E_PrimitiveType_TriangleStrip, vertexCount, sizeof(Vertex), Vertex::m_pDeclaration);

		if(pVertices)
		{
			
			for(S32 i = batch.m_StartIndex, intensitiesIndex = 0; i <= batch.m_EndIndex; ++i, intensitiesIndex++)
			{
				const C_LightTrailInstance* pInstance = visibleNodes[i].GetNode<C_LightTrailInstance>();
				
				//Calculate fade
 				Maths::HVec3 lastPosition = pInstance->GetLastPosition();
 				Maths::HVec3 firtPosition = pInstance->GetFirstPosition();
 				Maths::HVec4 centre((lastPosition + firtPosition) / 2.f);
 				centre.W(Maths::Constants<Maths::HF32>::One());
 				F32 radius = (lastPosition - firtPosition).Length() / 2.f;
 				Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
 				Maths::HF32 depthScale(view.GetViewData().depthScale);
 				Maths::HF32 depth = (centre | forward);
				F32 farPlusRadius = m_FarDistanceFade + radius;
				F32 fadeFactor = Maths::Clamp((F32(depth) - farPlusRadius) / (m_NearDistanceFade - farPlusRadius), 0.f, 1.f);
			
				intensityFactors[intensitiesIndex] = fadeFactor;

				AddLightTrail(pVertices, *pInstance, fadeFactor);
				pVertices += (LIGHTTRAILS_SIZE) * 2;
			}
			pDisplay->EndVertices();
		}
	#endif

		//Draw particle

		pDisplay->SetCullMode(Display::E_CullMode_None);
		pDisplay->SetVertexDeclaration(ParticleVertex::m_pDeclaration);
#ifdef PLATFORM_PS3
		pDisplay->SetVtxAttrUsageFlags(UINT_MAX);
#endif
		pDisplay->SetVertexShader(&m_ParticleLightTrailVertexShader);
		pDisplay->SetPixelShader(&m_ParticleLightTrailPixelShader);


		S32 particleVertexCount = batch.m_NodeCount * (LIGHTTRAILS_SIZE - 2) * 4 * 3;
#ifndef PLATFORM_PC
		// knock out until we have a begin vertices method for PC

		ParticleVertex* pParticleVertices = (ParticleVertex*)pDisplay->BeginVertices(Display::E_PrimitiveType_QuadList, particleVertexCount, sizeof(ParticleVertex), ParticleVertex::m_pDeclaration);

		if(pParticleVertices)
		{
			for(S32 i = batch.m_StartIndex, intensitiesIndex = 0; i <= batch.m_EndIndex; ++i, intensitiesIndex++)
			{
				const C_LightTrailInstance* pInstance = visibleNodes[i].GetNode<C_LightTrailInstance>();

				AddParticleLightTrail(pParticleVertices, *pInstance, intensityFactors[intensitiesIndex]);
				pParticleVertices += (LIGHTTRAILS_SIZE - 2) * 4 * 3;
			}
			pDisplay->EndVertices();
		}
#endif

	}

	void C_LightTrails::Update()
	{
		//For each light trail
		for (LightTrailList::Iterator it = m_LightTrailList.Begin(); it != m_LightTrailList.End(); ++it)
		{
			C_LightTrailInstance& lightTrail = *it;
			
			U32 lastRingBufferIndex = lightTrail.m_RingBufferIndex;
			lightTrail.m_RingBufferIndex = (lightTrail.m_RingBufferIndex + 1) % LIGHTTRAILS_SIZE;
			
			//Clean
			lightTrail.m_Position[lightTrail.m_RingBufferIndex] = lightTrail.m_Position[lastRingBufferIndex];
			lightTrail.m_VectorX[lightTrail.m_RingBufferIndex] = lightTrail.m_VectorX[lastRingBufferIndex];
			lightTrail.m_VectorY[lightTrail.m_RingBufferIndex] = lightTrail.m_VectorY[lastRingBufferIndex];
			lightTrail.m_Intensity[lightTrail.m_RingBufferIndex] = lightTrail.m_Intensity[lastRingBufferIndex];

			lightTrail.m_LastActivateIndex++;
		}
	}

}
