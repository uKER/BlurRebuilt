#include "StdAfx.h"

#include "gravity/broadphase/DistributionTree.h"
#include "gravity/narrowphase/optimisedaabbtree.h"
#include "gravity/narrowphase/shapes.h"
#include "gravity/sdk/BodyEvents.h"
#include "gravity/cloths/HighFieldFluid.h"
#include "gravity/cloths/cloth.h"
#include "gravity/cloths/SoftBody.h"
#include "gravity/sdk/Scene.h"
#include <gravity/sharedphysics/MemoryAllocator.h>
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/triggers/triggerEvent.h"
#include "gravity/solver2/ConstraintGraphManager.h"

#include "gravity/narrowphase/AxisBox.inl"

#include "system/perf.h"

#ifndef PLATFORM_SPU
#include "Btl/Utility/String.h"
#endif

#ifndef PLATFORM_SPU
Debug::Bool g_dbDrawActiveObjects("Gravity.Debug.Culling.RenderActiveObjects", true);
Debug::Bool g_dbDrawGroundPlane("Gravity.Debug.Culling.RenderGroundPlane", false);
Debug::Bool g_dbDrawSleepingObjects("Gravity.Debug.Culling.RenderSleepingObjects", true);
Debug::Bool g_dbDrawStaticObjects("Gravity.Debug.Culling.RenderStaticObjects", true);
Debug::Bool g_dbDrawKinematicObjects("Gravity.Debug.Culling.RenderKinematicObjects", true);
Debug::Bool g_dbDrawRaytraceOnlyObjects("Gravity.Debug.Culling.RenderRaytraceOnlyObjects", true);
Debug::Bool g_dbDrawContinousBoxes("Gravity.Debug.Culling.RenderContinuousObjectsBox", true);
Debug::Bool g_dbDrawBoundingBoxes("Gravity.Debug.RenderBoundingBoxes", false);
Debug::Bool g_dbCullObjects("Gravity.Debug.Culling.CullObjects", true);
Debug::Bool g_dbCullMesh("Gravity.Debug.Culling.CullMesh", true);
Debug::Bool g_dbDrawEdges("Gravity.Debug.Geom.DrawMeshEdges", false);
Debug::Bool g_dbDrawSolidTriangles("Gravity.Debug.Geom.DrawSolidTriangles", true);
Debug::Bool g_dbDrawInstancedMesh("Gravity.Debug.Culling.DrawInstancedMesh", false);
Debug::Bool g_dbDrawFluids("Gravity.Debug.Culling.DrawFluid", true);
Debug::Bool g_dbDrawCloth("Gravity.Debug.Culling.DrawCloth", true);
Debug::Bool g_dbDrawDeformable("Gravity.Debug.Culling.DrawDeformable", false);
Debug::Bool g_dbDrawRope("Gravity.Debug.Culling.DrawRope", true);
Debug::Bool g_dbDrawSpring("Gravity.Debug.Culling.DrawSpring", true);
Debug::Bool g_dbDrawCentres("Gravity.Debug.Geom.DrawCofM", false);
Debug::Bool g_dbWriteBodyInfo("Gravity.Debug.Info.WriteBodyInfo", false);
Debug::Bool g_dbWriteBodyDetailedInfo("Gravity.Debug.Info.DetailedBodyInfo", false);
Debug::Bool g_dbWriteBodyDeactivatorInfo("Gravity.Debug.Info.WriteBodyDeactivatorInfo", false);
Debug::Bool g_dbShowExternalConstants("Gravity.Debug.Info.ShowExternalConstants", false);
Debug::Bool g_dbWriteBodyInertiaTensor("Gravity.Debug.Info.WriteBodyInertiaTensor", false);
Debug::Bool g_dbWriteBodyBuoyancy("Gravity.Debug.Info.WriteBodyBuoyancyInfo", false);
Debug::Bool g_BodyShowFilter("Gravity.Debug.Info.WriteBodyFilterInfo", false);
Debug::Bool g_dbWriteElementInfo("Gravity.Debug.Info.WriteElementInfo", false);
Debug::Bool g_dbDrawWorldBounds("Gravity.Debug.DrawWorldBounds", false);
Debug::Bool g_dbShowStaticObjectCount("Gravity.Debug.ShowStaticObjectCount", false);
Debug::Bool g_dbShowDynamicObjectCount("Gravity.Debug.ShowDynamicObjectCount", false);
Debug::Bool g_dbShowActiveObjectCount("Gravity.Debug.ShowActiveObjectCount", false);
Debug::Bool g_dbCollisionFlag("Gravity.Debug.Culling.CollisionFlag", false);
Debug::Bool g_dbShowDefaultMaterial("Gravity.HighlightDefaultMaterial", false);

Debug::Bool g_dbDrawActiveList("Gravity.Debug.Culling.DrawActiveList", false);
Debug::Bool g_dbDrawKineticEnergy("Gravity.Debug.Info.DrawKineticEnergy", false);

Debug::Bool g_dbDrawActiveNodes("Gravity.Debug.DrawBroadPhaseActiveNodes", false);
Debug::Bool g_dbShowInactivePartsOfActiveNodes("Gravity.Debug.DrawInactivePartsOfActiveNodes", false);
Debug::Bool g_dbDrawSleepingNodes("Gravity.Debug.DrawBroadPhaseSleepingNodes", false);

Debug::Bool g_dbDrawConstraints("Gravity.Debug.Culling.DrawConstraints", false);
Debug::Bool g_dbDrawStaticFluidMesh("Gravity.Debug.Culling.DrawStaticFluidMesh", false);

Debug::Bool g_dbDrawTriggers("Gravity.Debug.Culling.DrawTriggers", false);
Debug::Bool g_dbDrawTriggerActors("Gravity.Debug.Culling.DrawTriggerActors", false);
Debug::Bool g_dbShowTriggerCount("Gravity.Debug.ShowTriggerCount", false);
Debug::Bool g_dbShowTriggerActorCount("Gravity.Debug.ShowTriggerActorCount", false);
Debug::Bool g_dbShowActiveTriggerCount("Gravity.Debug.ShowActiveTriggerNodeCount", false);



Debug::Bool g_dbDrawMesh("Gravity.Debug.Culling.RenderStaticMesh", true, Debug::Persist());
Debug::Bool g_dbDrawIndices("Gravity.Debug.Geom.DrawVertexIndices", false);
Debug::Bool g_dbDrawEdgeNormals("Gravity.Debug.Geom.DrawEdgeNormals", false);
Debug::Bool g_dbDrawActiveEdges("Gravity.Debug.Geom.DrawActiveEdges", false);
Debug::Bool g_dbConvexHullWireframe("Gravity.Debug.Geom.WireframeConvexHull", false);
Debug::Bool g_dbDrawShapes("Gravity.Debug.Geom.DrawShapes", true);

Debug::Bool g_dbDrawImmediate("Gravity.Debug.RenderImmediate", false);
#endif

namespace Gravity
{

#if !defined(PLATFORM_SPU)

	static const U32 kMaxDebugLineDrawBufferSize		= 16384;
	static const U32 kDebugLineDrawBatchSize			= 1024;

	static const U32 kMaxDebugTriangleDrawBufferSize	= 16384;
	static const U32 kDebugTriangleDrawBatchSize		= 512;

	static const U32 kMaxDebugLitTriangleDrawBufferSize	= 16384;
	static const U32 kDebugLitTriangleDrawBatchSize		= 512;

	static const U32 kMaxDebugArrowDrawBufferSize		= 8192;

	static const U32 kMaxDebugStarDrawBufferSize		= 8192;

	struct DebugLineDrawBuffer
	{
		Maths::RGBA					Colour;
		Btl::Vector<Maths::HVec3>	LineStarts;
		Btl::Vector<Maths::HVec3>	LineEnds;

		void AddLine(const Maths::HVec3& start, const Maths::HVec3& end)
		{
			if (static_cast<U32>(LineStarts.Size()) < kMaxDebugLineDrawBufferSize)
			{
				LineStarts.PushBack(start);
				LineEnds.PushBack(end);
			}
		}
	};

	struct DebugTriangleDrawBuffer
	{
		Btl::Vector<Maths::HVec3>	TriPointsA;
		Btl::Vector<Maths::HVec3>	TriPointsB;
		Btl::Vector<Maths::HVec3>	TriPointsC;
		Btl::Vector<Maths::RGBA>	Colours;

		void AddTriangle(const Maths::HVec3& vA, const Maths::HVec3& vB, const Maths::HVec3& vC, const Maths::RGBA& vColour)
		{
			TriPointsA.PushBack(vA);
			TriPointsB.PushBack(vB);
			TriPointsC.PushBack(vC);
			Colours.PushBack(vColour);
		}

		void Clear()
		{
			TriPointsA.Clear();
			TriPointsB.Clear();
			TriPointsC.Clear();
			Colours.Clear();
		}
	};

	struct DebugArrow
	{
		Maths::HVec3	Position;
		Maths::HVec3	Direction;
		Maths::RGBA		Colour;
		F32				LengthScale;
		F32				ArrowHeadSize;
	};

	struct DebugStar
	{
		Maths::HVec3	Position;
		Maths::RGBA		Colour;
		F32				Size;
	};

	struct DebugMeshDrawBuffer
	{
		DbUtil::I_DebugRender*					pDebugRender;

		Btl::Vector<DebugLineDrawBuffer>		LineDrawBuffers;
		DebugTriangleDrawBuffer					TriangleDrawBuffer;
		DebugTriangleDrawBuffer					LitTriangleDrawBuffer;
		Btl::Vector<DebugArrow>					ArrowDrawBuffer;
		Btl::Vector<DebugStar>					StarDrawBuffer;

		DebugMeshDrawBuffer() : pDebugRender(NULL) { }

		void Draw3dLine(const Maths::HVec3& vA, const Maths::HVec3& vB, const Maths::RGBA& vColour)
		{
			if (g_dbDrawImmediate)
			{
				if (pDebugRender)
					pDebugRender->Draw3dLine(vA, vB, vColour);
			}
			else
			{
				S32 iColourIndex = -1;
				
				for (S32 i = 0, count = LineDrawBuffers.Size(); i < count; ++i)
				{
					if (LineDrawBuffers[i].Colour == vColour)
					{
						iColourIndex = i;
						break;
					}
				}

				if (iColourIndex >= 0)
				{
					BIZ_ASSERT(iColourIndex < LineDrawBuffers.Size());
					LineDrawBuffers[iColourIndex].AddLine(vA, vB);
				}
				else
				{
					DebugLineDrawBuffer lineDrawBuffer;
					lineDrawBuffer.Colour = vColour;
					lineDrawBuffer.AddLine(vA, vB);
					LineDrawBuffers.PushBack(lineDrawBuffer);
				}
			}
		}

		void Draw3dTriangle(const Maths::HVec3& vA, const Maths::HVec3& vB, const Maths::HVec3& vC, const Maths::RGBA& vColour)
		{
			if (g_dbDrawImmediate)
			{
				if (pDebugRender)
					pDebugRender->Draw3dTriangle(vA, vB, vC, vColour);
			}
			else
			{
				if (static_cast<U32>(TriangleDrawBuffer.Colours.Size()) < kMaxDebugTriangleDrawBufferSize)
					AddTriangle(TriangleDrawBuffer, vA, vB, vC, vColour);
			}
		}

		void Draw3dTriangleLit(const Maths::HVec3& vA, const Maths::HVec3& vB, const Maths::HVec3& vC, const Maths::RGBA& vColour)
		{
			if (g_dbDrawImmediate)
			{
				if (pDebugRender)
					pDebugRender->Draw3dTriangleLit(vA, vB, vC, vColour);
			}
			else
			{
				if (static_cast<U32>(LitTriangleDrawBuffer.Colours.Size()) < kMaxDebugLitTriangleDrawBufferSize)
					AddTriangle(LitTriangleDrawBuffer, vA, vB, vC, vColour);
			}
		}

		void AddTriangle(DebugTriangleDrawBuffer& triangleDrawBuffer, const Maths::HVec3& vA, const Maths::HVec3& vB, const Maths::HVec3& vC, const Maths::RGBA& vColour)
		{
			triangleDrawBuffer.AddTriangle(vA, vB, vC, vColour);
		}

		void Draw3dArrow(const Maths::HVec3& vPos, const Maths::HVec3& vDir, const F32 fLenScale, const F32 fHeadSize, const Maths::RGBA& vColour)
		{
			if (g_dbDrawImmediate)
			{
				if (pDebugRender)
					pDebugRender->Draw3dArrow(vPos, vDir, fLenScale, fHeadSize, vColour);
			}
			else
			{
				if (static_cast<U32>(ArrowDrawBuffer.Size()) < kMaxDebugArrowDrawBufferSize)
				{
					DebugArrow arrow;
					arrow.Position = vPos;
					arrow.Direction = vDir;
					arrow.LengthScale = fLenScale;
					arrow.ArrowHeadSize = fHeadSize;
					arrow.Colour = vColour;

					ArrowDrawBuffer.PushBack(arrow);
				}
			}
		}

		void Draw3dStar(const Maths::HVec3& vPos, const F32 fSize, const Maths::RGBA& vColour)
		{
			if (g_dbDrawImmediate)
			{
				if (pDebugRender)
					pDebugRender->Draw3dStar(vPos, fSize, vColour);
			}
			else
			{
				if (static_cast<U32>(StarDrawBuffer.Size()) < kMaxDebugStarDrawBufferSize)
				{
					DebugStar star;
					star.Position = vPos;
					star.Size = fSize;
					star.Colour = vColour;
					
					StarDrawBuffer.PushBack(star);
				}
			}
		}

		void Draw(DbUtil::I_DebugRender* pRender)
		{
			if (pRender)
			{
				if (TriangleDrawBuffer.Colours.Size() > 0)
				{
					BIZ_ASSERT(TriangleDrawBuffer.Colours.Size() == TriangleDrawBuffer.TriPointsA.Size());
					BIZ_ASSERT(TriangleDrawBuffer.Colours.Size() == TriangleDrawBuffer.TriPointsB.Size());
					BIZ_ASSERT(TriangleDrawBuffer.Colours.Size() == TriangleDrawBuffer.TriPointsC.Size());

					U32 numTriangles = static_cast<U32>(LitTriangleDrawBuffer.Colours.Size());
					BIZ_ASSERT(numTriangles <= kMaxDebugTriangleDrawBufferSize);

					U32 numBatches = (numTriangles / kDebugTriangleDrawBatchSize) + (numTriangles % kDebugTriangleDrawBatchSize != 0);

					for (U32 iBatch = 0; iBatch < numBatches; ++iBatch)
					{
						U32 batchStart = iBatch * kDebugTriangleDrawBatchSize;
						U32 batchSize = Maths::Min<U32>(numTriangles - batchStart, kDebugTriangleDrawBatchSize);

						pRender->Draw3dTriangles(&TriangleDrawBuffer.TriPointsA[batchStart], &TriangleDrawBuffer.TriPointsB[batchStart], &TriangleDrawBuffer.TriPointsC[batchStart],
							&TriangleDrawBuffer.Colours[batchStart], batchSize);
					}
				}

				if (LitTriangleDrawBuffer.Colours.Size() > 0)
				{
					BIZ_ASSERT(LitTriangleDrawBuffer.Colours.Size() == LitTriangleDrawBuffer.TriPointsA.Size());
					BIZ_ASSERT(LitTriangleDrawBuffer.Colours.Size() == LitTriangleDrawBuffer.TriPointsB.Size());
					BIZ_ASSERT(LitTriangleDrawBuffer.Colours.Size() == LitTriangleDrawBuffer.TriPointsC.Size());

					U32 numTriangles = static_cast<U32>(LitTriangleDrawBuffer.Colours.Size());
					BIZ_ASSERT(numTriangles <= kMaxDebugLitTriangleDrawBufferSize);

					U32 numBatches = (numTriangles / kDebugLitTriangleDrawBatchSize) + (numTriangles % kDebugLitTriangleDrawBatchSize != 0);

					for (U32 iBatch = 0; iBatch < numBatches; ++iBatch)
					{
						U32 batchStart = iBatch * kDebugLitTriangleDrawBatchSize;
						U32 batchSize = Maths::Min<U32>(numTriangles - batchStart, kDebugLitTriangleDrawBatchSize);

						pRender->Draw3dTrianglesLit(&LitTriangleDrawBuffer.TriPointsA[batchStart], &LitTriangleDrawBuffer.TriPointsB[batchStart], &LitTriangleDrawBuffer.TriPointsC[batchStart],
							&LitTriangleDrawBuffer.Colours[batchStart], batchSize);
					}
				}

				for (U32 i = 0, count = static_cast<U32>(LineDrawBuffers.Size()); i < count; ++i)
				{
					BIZ_ASSERT(LineDrawBuffers[i].LineStarts.Size() >= 0 && LineDrawBuffers[i].LineStarts.Size() <= static_cast<S32>(kMaxDebugLineDrawBufferSize));
					if (LineDrawBuffers[i].LineStarts.Size() > 0)
					{
						BIZ_ASSERT(LineDrawBuffers[i].LineStarts.Size() == LineDrawBuffers[i].LineEnds.Size());
						
						U32 numLines = static_cast<U32>(LineDrawBuffers[i].LineStarts.Size());
						BIZ_ASSERT(numLines <= kMaxDebugLineDrawBufferSize);

						U32 numBatches = (numLines / kDebugLineDrawBatchSize) + (numLines % kDebugLineDrawBatchSize != 0);

						for (U32 iBatch = 0; iBatch < numBatches; ++iBatch)
						{
							U32 batchStart = iBatch * kDebugLineDrawBatchSize;
							U32 batchSize = Maths::Min<U32>(numLines - batchStart, kDebugLineDrawBatchSize);

							pRender->Draw3dLines(&LineDrawBuffers[i].LineStarts[batchStart], &LineDrawBuffers[i].LineEnds[batchStart], batchSize, LineDrawBuffers[i].Colour);
						}
					}
				}

				if (ArrowDrawBuffer.Size() > 0)
				{
					for (U32 i = 0, count = static_cast<U32>(ArrowDrawBuffer.Size()); i < count; ++i)
						pRender->Draw3dArrow(ArrowDrawBuffer[i].Position, ArrowDrawBuffer[i].Direction, ArrowDrawBuffer[i].LengthScale, ArrowDrawBuffer[i].ArrowHeadSize, ArrowDrawBuffer[i].Colour);
				}

				if (StarDrawBuffer.Size() > 0)
				{
					for (U32 i = 0, count = static_cast<U32>(StarDrawBuffer.Size()); i < count; ++i)
						pRender->Draw3dStar(StarDrawBuffer[i].Position, StarDrawBuffer[i].Size, StarDrawBuffer[i].Colour);
				}
			}
			
			// Clean up

			for (U32 i = 0, count = static_cast<U32>(LineDrawBuffers.Size()); i < count; ++i)
			{
				LineDrawBuffers[i].LineStarts.Clear();
				LineDrawBuffers[i].LineEnds.Clear();
			}
			LineDrawBuffers.Clear();
			
			TriangleDrawBuffer.Clear();
			LitTriangleDrawBuffer.Clear();
			
			ArrowDrawBuffer.Clear();
			StarDrawBuffer.Clear();

			pDebugRender = NULL;
		}
	};

#ifndef CFG_GOLD
	static DebugMeshDrawBuffer s_MeshDrawBuffer;
#endif // !CFG_GOLD

	void DrawMesh(DbUtil::I_DebugRender* pRender, const Collision::OptimisedAABBTree& tree, const Maths::HMat43 PCREF matrix, const Collision::C_CRAxisBox& untransformedBox,
		SharedPhysics::C_SafeStack<true>& stack, const Maths::RGBA colour, Collision::C_CollisionFilter* pFilter, DebugMeshDrawBuffer* pDebugMeshDrawBuffer)
	{
		using namespace Maths;
		{
			if(tree.m_NumNodes == 0)
				return;
			Collision::C_CRAxisBox box;
			const Maths::HMat33 rot(matrix.Rotation());
			untransformedBox.Transform(matrix.InverseRotTrans(), box);
			const Maths::HF32 three(Maths::Constants<Maths::HF32>::Three());
			const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());

			Collision::C_AxisBoxMinMax axisBox;

			Collision::OptimisedAABBNode* pStackTop(tree.m_pNodes);
			stack.Push(&tree.m_pNodes[0], 0);
			S32 size(1);
			bool bForceDraw = !g_dbCullMesh;
			bool bDrawEdges = g_dbDrawEdges;
			bool bDrawSolid = g_dbDrawSolidTriangles;
			bool bDrawIndices = g_dbDrawIndices;
			bool bDrawEdgeNormals = g_dbDrawEdgeNormals;
			while(size--)
			{
				Collision::OptimisedAABBNode* pNode(pStackTop);
				pStackTop = stack.Pop<Collision::OptimisedAABBNode>(Maths::Max(size-1, 0));
				pNode->m_BoundingBox.LoadBox(axisBox, tree.m_Low, tree.m_High);
				if(bForceDraw || axisBox.Intersects(box))
				{
					if(pNode->m_TriangleSizeIndices > 0)
					{
						//Draw mesh!
						U32 processed = 0;
						
						const Collision::C_TriangleHeader* header(tree.m_Triangles.GetHeader(pNode->m_TriangleStartIndex));
						U32 offset = pNode->m_TriangleStartIndex + sizeof(Collision::C_TriangleHeader);
						//while(processed < pNode->m_NumIndices)

						Collision::C_AxisBoxMinMax b;
						pNode->m_BoundingBox.LoadBox(b, tree.m_Low, tree.m_High);

						const HVec3 range(b.m_max - b.m_min);
						
						U32 headerIndex = 0;
						for(U32 p = 0; p < pNode->m_TriangleSizeIndices; ++p, ++headerIndex)
						{
							if(header->m_NumTriangles <= headerIndex)
							{
								header = tree.m_Triangles.GetHeader(offset);
								offset += sizeof(Collision::C_TriangleHeader);
								headerIndex = 0;
								

							}
							const Collision::AABBTreeTriangle* tri(tree.m_Triangles.GetTriangles(offset));
							offset += sizeof(Collision::AABBTreeTriangle);

							MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)(tree.m_pVertices + pNode->m_VerticeStartIndex);

							const bool highlightDefaultMaterial = g_dbShowDefaultMaterial && header->m_EffectMaterialIndex==0;
							if (((pFilter == NULL || pFilter->CanCollide(header->m_CollisionFlags)|| g_dbCollisionFlag) && ((!g_dbShowDefaultMaterial) || highlightDefaultMaterial)))
							{
								Maths::RGBA clr = highlightDefaultMaterial ? Maths::RGBA(255, 0, 255) : colour;

								/*const Maths::HVec3 vA(Maths::Load<Maths::HVec3>(&tree.m_pVertices[tri->m_IndexA]) * matrix);
								const Maths::HVec3 vB(Maths::Load<Maths::HVec3>(&tree.m_pVertices[tri->m_IndexB]) * matrix);
								const Maths::HVec3 vC(Maths::Load<Maths::HVec3>(&tree.m_pVertices[tri->m_IndexC]) * matrix);*/
								const HVec3 localA(&pVertices[tri->m_IndexA]);
								const HVec3 localB(&pVertices[tri->m_IndexB]);
								const HVec3 localC(&pVertices[tri->m_IndexC]);

								const HVec3 worldA(localA*range + b.m_min);
								const HVec3 worldB(localB*range + b.m_min);
								const HVec3 worldC(localC*range + b.m_min);

								const HVec3 vA(worldA*matrix);
								const HVec3 vB(worldB*matrix);
								const HVec3 vC(worldC*matrix);

								if(bDrawSolid)
								{
									if (pDebugMeshDrawBuffer)
										pDebugMeshDrawBuffer->Draw3dTriangleLit(vA,vB, vC, clr);
									else
										pRender->Draw3dTriangleLit(vA,vB, vC, clr);
								}
								if(bDrawEdges)
								{
									//Draw the 3 edges...
									if(tri->ActiveEdge(0))
									{
										if (pDebugMeshDrawBuffer)
											pDebugMeshDrawBuffer->Draw3dLine(vA, vB, Maths::RGBA(255, 0, 0));
										else
											pRender->Draw3dLine(vA, vB, Maths::RGBA(255, 0, 0));
									}
									else
									{
										if (pDebugMeshDrawBuffer)
											pDebugMeshDrawBuffer->Draw3dLine(vA, vB, Maths::RGBA(0, 255, 0));
										else
											pRender->Draw3dLine(vA, vB, Maths::RGBA(0, 255, 0));
									}

									if(tri->ActiveEdge(1))
									{
										if (pDebugMeshDrawBuffer)
											pDebugMeshDrawBuffer->Draw3dLine(vB, vC, Maths::RGBA(255, 0, 0));
										else
											pRender->Draw3dLine(vB, vC, Maths::RGBA(255, 0, 0));
									}
									else
									{
										if (pDebugMeshDrawBuffer)
											pDebugMeshDrawBuffer->Draw3dLine(vB, vC, Maths::RGBA(0, 255, 0));
										else
											pRender->Draw3dLine(vB, vC, Maths::RGBA(0, 255, 0));
									}

									if(tri->ActiveEdge(2))
									{
										if (pDebugMeshDrawBuffer)
											pDebugMeshDrawBuffer->Draw3dLine(vA, vC, Maths::RGBA(255, 0, 0));
										else
											pRender->Draw3dLine(vA, vC, Maths::RGBA(255, 0, 0));
									}
									else
									{
										if (pDebugMeshDrawBuffer)
											pDebugMeshDrawBuffer->Draw3dLine(vA, vC, Maths::RGBA(0, 255, 0));
										else
											pRender->Draw3dLine(vA, vC, Maths::RGBA(0, 255, 0));
									}
								}
								if(bDrawEdgeNormals)
								{
									if(tri->ActiveEdge(0))
									{
										Collision::C_ActiveEdgeNormals& norm(tree.m_pActiveEdgeNormals[tri->ActiveEdgeDataIndex()]);
										const Maths::HVec3 centre((vA + vB)*half);
										const Maths::HVec3 n(Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&norm.m_EdgeNormA)*rot);
										if (pDebugMeshDrawBuffer)
										{
											pDebugMeshDrawBuffer->Draw3dArrow(centre, n, 0.5f, 0.1f, Maths::RGBA(255, 0, 0));
											pDebugMeshDrawBuffer->Draw3dLine(vA, vB, Maths::RGBA(255, 0, 0));
										}
										else
										{
											pRender->Draw3dArrow(centre, n, 0.5f, 0.1f, Maths::RGBA(255, 0, 0));
											pRender->Draw3dLine(vA, vB, Maths::RGBA(255, 0, 0));
										}
									}
									if(tri->ActiveEdge(1))
									{
										Collision::C_ActiveEdgeNormals& norm(tree.m_pActiveEdgeNormals[tri->ActiveEdgeDataIndex()]);
										const Maths::HVec3 centre((vB + vC)*half);
										const Maths::HVec3 n(Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&norm.m_EdgeNormB)*rot);
										if (pDebugMeshDrawBuffer)
										{
											pDebugMeshDrawBuffer->Draw3dArrow(centre, n, 0.5f, 0.1f, Maths::RGBA(255, 0, 0));
											pDebugMeshDrawBuffer->Draw3dLine(vB, vC, Maths::RGBA(255, 0, 0));
										}
										else
										{
											pRender->Draw3dArrow(centre, n, 0.5f, 0.1f, Maths::RGBA(255, 0, 0));
											pRender->Draw3dLine(vB, vC, Maths::RGBA(255, 0, 0));
										}
									}
									if(tri->ActiveEdge(2))
									{
										Collision::C_ActiveEdgeNormals& norm(tree.m_pActiveEdgeNormals[tri->ActiveEdgeDataIndex()]);
										const Maths::HVec3 centre((vA + vC)*half);
										const Maths::HVec3 n(Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&norm.m_EdgeNormC)*rot);
										if (pDebugMeshDrawBuffer)
										{
											pDebugMeshDrawBuffer->Draw3dArrow(centre, n, 0.5f, 0.1f, Maths::RGBA(255, 0, 0));
											pDebugMeshDrawBuffer->Draw3dLine(vA, vC, Maths::RGBA(255, 0, 0));
										}
										else
										{
											pRender->Draw3dArrow(centre, n, 0.5f, 0.1f, Maths::RGBA(255, 0, 0));
											pRender->Draw3dLine(vA, vC, Maths::RGBA(255, 0, 0));
										}
									}
								}
								if(bDrawIndices)
								{
									Btl::String str = Btl::String::Format("%i", tri->m_IndexA);
									pRender->Draw3dText(vA, 0.1f, str.Chars());
									str = Btl::String::Format("%i", tri->m_IndexB);
									pRender->Draw3dText(vB, 0.1f, str.Chars());
									str = Btl::String::Format("%i", tri->m_IndexC);
									pRender->Draw3dText(vC, 0.1f, str.Chars());
								}
							}
							//const Maths::HVec3 norm = Maths::Load<Maths::HVec3>(&tree.m_pNormals[tri->m_NormalIndex]);
							//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow((vA + vB + vC)/Maths::Constants<Maths::HF32>::Three(),
							//	norm, 0.3f, 0.05f, Maths::RGBA(255, 0, 0));
						}
					}
					else
					{
						pStackTop = pNode + 1;
						stack.Push(pNode + pNode->m_RightChildOffset, size++);
						stack.Push(pNode + 1, size++);
						
					}
					//if(triangleDrawn > maxTriangles)
					//	goto Simulate;
				}
			}
		}
	}

	void DrawCompositeShape(DbUtil::I_DebugRender* pRender, const Collision::C_CompositeShape* pCompShape,
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter)
	{
		using namespace Maths;

		if(!g_dbDrawShapes)
			return;


		bool bDrawFluid = g_dbDrawStaticFluidMesh;
		Collision::C_CRAxisBox axisBox;
		axisBox.extents = Maths::Constants<Maths::HVec3>::Max();
		static bool subComp=false;
		Collision::C_CompositeShapeIterator iter(pCompShape);
		//for(U32 i = 0; i < pCompShape->m_NumShapes; ++i)
		while(iter.HasNextShape())
		{
			iter.NextShape();
			Collision::C_Shape* pSubShape = iter.GetCurrentShape<Collision::C_Shape>();
			if(!g_dbCollisionFlag && (pFilter != NULL && !pSubShape->m_Filter.CanCollide(pFilter)))
				continue;
			switch(pSubShape->GetShape())
			{
			case Collision::E_InstancedTriangleMesh_Shape:
				{
					Collision::C_InstancedTriangleMeshShape* pMeshShape((Collision::C_InstancedTriangleMeshShape*)pSubShape);
					const HMat43 transform(pMeshShape->Orientation(), pMeshShape->Position());
					const RGBA colour(55, 55, 55);
					if(g_dbDrawInstancedMesh || g_dbShowDefaultMaterial)
						DrawMesh(pRender, *pMeshShape->m_AABBTree.m_pTree, transform, axisBox, stack, colour, pFilter, NULL);
					break;
				}
			case Collision::E_Box_Shape:
				{
			
				const Collision::C_BoxShape* const __restrict pBoxShape((Collision::C_BoxShape*)pSubShape);
				pBoxShape->DebugRender(pRender);
 
			/*	const HMat33& u = pBoxShape->m_Box.Orientation();
				const HMat43 absPosRot = Maths::HMat43(u, pBoxShape->m_Box.Centre());
				const HVec3 maxV(pBoxShape->m_Box.Extent());
				const HVec3 minV(-maxV);
				if(!subComp)
					pRender->Draw3dBoxLit(minV, maxV, absPosRot, RGBA(0, 255, 0, 255));
				else
					pRender->Draw3dBoxLit(minV, maxV, absPosRot, RGBA(255, 255, 0, 255));*/

				break;
				}
			case Collision::E_Sphere_Shape:
				{
					//Must be a sphere
					const Collision::C_SphereShape* const __restrict pSphereShape((Collision::C_SphereShape*)pSubShape);
					pSphereShape->DebugRender(pRender);
					/*const HVec3 center=pSphereShape->m_Sphere.Centre();
					
					if(!subComp)
						pRender->Draw3dSphere(center, pSphereShape->m_Sphere.Radius(), Maths::RGBA(0, 255, 0));
					else
						pRender->Draw3dSphere(center, pSphereShape->m_Sphere.Radius(), Maths::RGBA(255, 255, 0));*/

					break;
				}
			case Collision::E_ConvexHull_Shape:
				{
					Collision::C_ConvexHullShape* pShape((Collision::C_ConvexHullShape*)pSubShape);
					pShape->DebugRender(pRender);
					break;
				}
			case Collision::E_Cylinder_Shape:
				{
					Collision::C_CylinderShape* pShape((Collision::C_CylinderShape*)pSubShape);
					pShape->DebugRender(pRender);
					break;
				}
			case Collision::E_Cone_Shape:
				{
					const Collision::C_ConeShape* pConeShape((Collision::C_ConeShape*)pSubShape);
					pConeShape->DebugRender(pRender);
					break;

				}
			case Collision::E_Capsule_Shape:
				{
					Collision::C_CapsuleShape* pShape((Collision::C_CapsuleShape*)pSubShape);
					pShape->DebugRender(pRender);
					break;
				}
			case Collision::E_Composite_Shape:
				{
					//const Collision::C_CompositeShape* const __restrict pCompShape((Collision::C_CompositeShape*)pCompShape->m_pShapes[i].m_pShape);
					const Collision::C_CompositeShape* pCompShape2((Collision::C_CompositeShape*)pSubShape);
					subComp = true;
					DrawCompositeShape(pRender, pCompShape2, stack, pFilter);
					subComp = false;
					break;
				}
			case Collision::E_FluidMesh_Shape:
				{
					//Maths::HMat43 identity;
						//identity.SetIdentity();
						//Draw the fluid mesh
					const Collision::C_FluidMeshShape* pShape = (const Collision::C_FluidMeshShape*)pSubShape;
					const Collision::OptimisedAABBTree & tree(*pShape->m_AABBTree.m_pTree);
						
					DrawMesh(pRender, tree, pShape->m_AABBTree.m_PosRot, axisBox, stack, Maths::RGBA(0, 0, 255), pFilter, NULL);
					
					break;
				}
			default:
				{
					break;
				}
			}
		}
	}

	void C_Scene::RenderObjectList(DbUtil::I_DebugRender* pRender, Collision::C_CRAxisBox& box,
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
		SharedPhysics::SortedArray<C_Scene::BodyPtr>& bodies)
	{

		bool drawActive = g_dbDrawActiveObjects, drawSleeping = g_dbDrawSleepingObjects, drawStatic = g_dbDrawStaticObjects,
			drawKinematic = g_dbDrawKinematicObjects;
		bool drawRaytraceOnly = g_dbDrawRaytraceOnlyObjects;
		bool bDrawFluid = g_dbDrawStaticFluidMesh;
		bool bDrawPlane = g_dbDrawGroundPlane;
		bool bDrawBoundingBox = g_dbDrawBoundingBoxes;
		bool bDrawContinuousBox = g_dbDrawContinousBoxes;
		bool bCullObjects = g_dbCullObjects;
		bool bDrawCofM = g_dbDrawCentres;
		bool bWriteBodyInfo = g_dbWriteBodyInfo || g_dbWriteBodyDetailedInfo;
		bool bWriteElementInfo = g_dbWriteElementInfo;
		const U32 numBodies = bodies.m_numElements;
		Maths::HMat43 identity;
		identity.SetIdentity();

#ifndef CFG_GOLD
		s_MeshDrawBuffer.pDebugRender = pRender;
#endif // !CFG_GOLD

		if(drawActive || drawSleeping || drawStatic || drawKinematic)
		{
			for(U32 a = 0; a < numBodies; ++a)
			{
				Gravity::C_Body* pBody(bodies[a].m_pElement);
				SharedPhysics::C_RigidBody& rigid = pBody->m_RigidBody;
				 C_Object*  pCollisionObject(pBody->GetObject());
				if(bCullObjects && !pCollisionObject->m_TransformedBox.Intersects(box))
					continue;

				
				
				if(
						!g_dbCollisionFlag && 
						(pFilter != NULL && !pBody->GetShape()->m_Filter.CanCollide(pFilter)) ||
						(!drawActive && pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active)) ||
						(!drawStatic && pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static)) ||
						(!drawKinematic && pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject)) ||
						(!drawSleeping && pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping)) ||
						(!drawRaytraceOnly && pBody->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_RaytraceOnly))
					)
				{
					continue;
				}

				if(bDrawCofM)
				{
					pRender->Draw3dStar(rigid.m_CenterOfMass, 0.3f, Maths::RGBA(0, 255, 0));
				}

				Btl::String str;

				if(bWriteBodyInfo || bWriteElementInfo)
				{
					const SharedPhysics::C_GraphNode* pNode = pBody->GetGraphNode();
					str += ( pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active) ? "Active" : 
					pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject) ? "Kinematic" : 
					pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static) ? "Static" : 
					pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping) ? "Sleeping" : "INVALID!");
					str += Btl::String::Format(" %.03fKg", F32(pBody->Mass()));
					if (g_dbWriteBodyDetailedInfo)
					{
						const Maths::HVec3 position = bWriteBodyInfo ? pBody->Position() : pBody->ElementMatrix().AW();
						str += Btl::String::Format("\nPos (%.2f, %.2f, %.2f)", (F32)position.X(), (F32)position.Y(), (F32)position.Z());
						const Maths::HVec3 renderOffset(pBody->m_ElementOffset);
						str += Btl::String::Format("\nR-Offset (%.2f, %.2f, %.2f)", (F32)renderOffset.X(), (F32)renderOffset.Y(), (F32)renderOffset.Z());
						const Maths::HVec3 linearVelocity(pBody->GetBody().m_LinearVelocity);
						str += Btl::String::Format("\nLinVel (%.2f, %.2f, %.2f)", (F32)linearVelocity.X(), (F32)linearVelocity.Y(), (F32)linearVelocity.Z());
						const Maths::HVec3 angularVelocity(pBody->GetBody().m_AngularVelocity);
						str += Btl::String::Format("\nAngVel (%.2f, %.2f, %.2f)", (F32)angularVelocity.X(), (F32)angularVelocity.Y(), (F32)angularVelocity.Z());
						if (pBody->IsContinuousShape())
							str += Btl::String::Format("\nContinuous");
					}
				}

				if (g_dbShowExternalConstants)
				{
					if (pBody->m_pExternalConstants)
					{
						const Gravity::C_ExternalConstants* extConst = pBody->m_pExternalConstants;
						str += Btl::String::Format("\nGrav: (%.2f, %.2f, %.2f)", (F32)extConst->m_Gravity.X(), (F32)extConst->m_Gravity.Y(), (F32)extConst->m_Gravity.Z());
					}
					else
					{
						str += "\nNo external constants set";
					}
				}

				if (g_dbWriteBodyDeactivatorInfo)
				{
					const SharedPhysics::C_GraphNode* pNode = pBody->GetGraphNode();
					str += Btl::String::Format("\nActivateThreshSq: %.2f, SleepThreshSq: %.4f, StaticActivate: %.2f", (F32)pNode->Deactivator().m_ActivateThresholdSq, (F32)pNode->Deactivator().m_SleepThresholdSq, (F32)pNode->Deactivator().m_StaticActivateThresholdSq);
				}

				if (g_dbWriteBodyBuoyancy)
				{
					const SharedPhysics::C_RigidBody& body = pBody->GetBody();
					str += Btl::String::Format("\nBuoyancy: %.2f, LinearDrag: %.4f, AngularDrag: %.2f", (F32)body.BuoyancyCoefficient(), (F32)body.FluidLinearDragCoefficient(), (F32)body.FluidAngularDragCoefficient());
				}
				
				if(g_dbWriteBodyInertiaTensor)
				{
					const SharedPhysics::C_RigidBody& body = pBody->GetBody();
					Btl::String it;
					it += Btl::String::Format("\n%.04f %.04f %.04f", (F32)body.m_IBody.AX().X(), (F32)body.m_IBody.AX().Y(), (F32)body.m_IBody.AX().Z());
					it += Btl::String::Format("\n%.04f %.04f %.04f", (F32)body.m_IBody.AY().X(), (F32)body.m_IBody.AY().Y(), (F32)body.m_IBody.AY().Z());
					it += Btl::String::Format("\n%.04f %.04f %.04f", (F32)body.m_IBody.AZ().X(), (F32)body.m_IBody.AZ().Y(), (F32)body.m_IBody.AZ().Z());
					str += it;
				}

				if (g_BodyShowFilter)
				{
					Btl::String filter;
					filter += Btl::String::Format("\nGroup id: %d", pBody->GetShape()->GetFilter()->GetGroupId());
					filter += Btl::String::Format("\nFlags: 0x%08X", pBody->GetShape()->GetFilter()->GetCollisionFlags());
					filter += Btl::String::Format("\nMask: 0x%08X", pBody->GetShape()->GetFilter()->GetCollisionMask());
					filter += Btl::String::Format("\nGrpFlags: 0x%08X", pBody->GetShape()->GetFilter()->GetGroupFlags());
					filter += Btl::String::Format("\nGrpMask: 0x%08X", pBody->GetShape()->GetFilter()->GetGroupMask());
					filter += Btl::String::Format("\nBody flags: 0x%08X", pBody->GetShape()->GetFilter()->GetBodyFlags());
					filter += Btl::String::Format("\nBody mask: 0x%08X", pBody->GetShape()->GetFilter()->GetBodyMask());
					str += filter;
				}

				if(g_dbDrawKineticEnergy)
				{
					Btl::String filter;
					filter += Btl::String::Format("\nKinetic Energy id: (%f, %f, %f)", (F32)pBody->m_RigidBody.m_KineticEnergy.X(),
					(F32)pBody->m_RigidBody.m_KineticEnergy.Y(), (F32)pBody->m_RigidBody.m_KineticEnergy.Z() );
					str += filter;
					
				}

				if (str.Length() > 0)
					pRender->Draw3dText(pBody->Position()-Maths::HVec3(0,0.5,0), 0.15f, str.Chars());
					
				Collision::C_Shape* pBodyShape = pBody->GetShape();

				
				if(bDrawContinuousBox && pBodyShape->m_ShapeType == Collision::E_Continuous_Shape)
				{
					pRender->Draw3dBox(pCollisionObject->m_TransformedBox.centre - pCollisionObject->m_TransformedBox.extents,
						pCollisionObject->m_TransformedBox.centre + pCollisionObject->m_TransformedBox.extents, identity, Maths::RGBA(255, 255, 0));
					pBodyShape = ((Collision::C_ContinuousShape*)pBodyShape)->m_pDiscreteShape;
					
				}

				//Draw bounding box...
				if(bDrawBoundingBox)
				{
					pRender->Draw3dBox(pCollisionObject->m_TransformedBox.centre - pCollisionObject->m_TransformedBox.extents,
						pCollisionObject->m_TransformedBox.centre + pCollisionObject->m_TransformedBox.extents, identity, Maths::RGBA(255, 255, 0));
				}

				if(!g_dbDrawShapes)
					continue;

				if(pBodyShape->GetShape() == Collision::E_Box_Shape)
				{
					const Collision::C_BoxShape* const __restrict pBoxShape((Collision::C_BoxShape*)pBodyShape);
					//Draw the box around the body!
					pBoxShape->DebugRender(pRender);
				}
				else if(pBodyShape->GetShape() == Collision::E_Cone_Shape)
				{
					const Collision::C_ConeShape* const __restrict pConeShape((Collision::C_ConeShape*)pBodyShape);
					pConeShape->DebugRender(pRender);
				}
				else if(pBodyShape->GetShape() == Collision::E_Composite_Shape)
				{
					//Draw composite here...
					const Collision::C_CompositeShape* const __restrict pCompShape((Collision::C_CompositeShape*)pBodyShape);
					DrawCompositeShape(pRender, pCompShape, stack, pFilter);
				}
				else if (pBodyShape->GetShape() == Collision::E_ConvexHull_Shape)
				{
					////We are going to hard-code in our 10-point convex hull here...
					Collision::C_ConvexHullShape* pShape((Collision::C_ConvexHullShape*)pBodyShape);
					pShape->DebugRender(pRender);
				}
				else if (pBodyShape->GetShape() == Collision::E_Capsule_Shape)
				{
					Collision::C_CapsuleShape* pShape((Collision::C_CapsuleShape*)pBodyShape);
					pShape->DebugRender(pRender);
				}
				else if (pBodyShape->GetShape() == Collision::E_Cylinder_Shape)
				{
					Collision::C_CylinderShape* pShape((Collision::C_CylinderShape*)pBodyShape);
					pShape->DebugRender(pRender);
				}
				else if (pBodyShape->GetShape() == Collision::E_Sphere_Shape)
				{
					Collision::C_SphereShape* pShape((Collision::C_SphereShape*)pBodyShape);
					pShape->DebugRender(pRender);
				}
				else if (pBodyShape->GetShape() == Collision::E_InstancedTriangleMesh_Shape)
				{
					if(g_dbDrawInstancedMesh)
					{
						Collision::C_InstancedTriangleMeshShape* pTriShape = (Collision::C_InstancedTriangleMeshShape*)pBodyShape;
						Collision::C_InstancedAABBTree& tree = pTriShape->m_AABBTree;

						S32 id=pBody->GetRemapExportBodyId();
						Maths::RGBA colour(55, 55, 55);
						
#ifndef CFG_GOLD
						DrawMesh(pRender, *tree.m_pTree, tree.m_PosRot, box, stack, colour, pFilter, &s_MeshDrawBuffer);
#else
						DrawMesh(pRender, *tree.m_pTree, tree.m_PosRot, box, stack, colour, pFilter, NULL);
#endif
					}
				}
				else if (pBodyShape->GetShape() == Collision::E_CharacterController_Shape)
				{
					Collision::C_CharacterControllerShape* pShape = (Collision::C_CharacterControllerShape*)pBodyShape;
					pShape->DebugRender(pRender);
				}
				else if (pBodyShape->GetShape() == Collision::E_FluidMesh_Shape)
				{
					//if(bDrawFluid)
					{
						//Draw the fluid mesh
						const Collision::C_FluidMeshShape* pShape = (const Collision::C_FluidMeshShape*)pBodyShape;
						const Collision::OptimisedAABBTree & tree(*pShape->m_AABBTree.m_pTree);
						
#ifndef CFG_GOLD
						DrawMesh(pRender, tree, pShape->m_AABBTree.m_PosRot, box, stack, Maths::RGBA(0, 0, 255), pFilter, &s_MeshDrawBuffer);
#else
						DrawMesh(pRender, tree, pShape->m_AABBTree.m_PosRot, box, stack, Maths::RGBA(0, 0, 255), pFilter, NULL);
#endif
					}
				}
			}
		}

#ifndef CFG_GOLD
		s_MeshDrawBuffer.Draw(pRender);
#endif // !CFG_GOLD
	}

	void C_Scene::RenderObjectLinkedList(DbUtil::I_DebugRender* pRender, Collision::C_CRAxisBox& box,
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
		Gravity::C_BodyList& list)
	{
		Maths::HMat43 identity;
		identity.SetIdentity();


		U32 activeObjectCount = 0;
		Gravity::C_Body* pBody = list.m_pFirstBody;
		//for(U32 a = 0; a < numBodies; ++a)
		while(pBody)
		{
			++activeObjectCount;
			//Gravity::C_Body* pBody(bodies[a].m_pElement);
			SharedPhysics::C_RigidBody& rigid = pBody->m_RigidBody;
			 C_Object*  pCollisionObject(pBody->GetObject());

			//Draw bounding box...

			pRender->Draw3dBox(pCollisionObject->m_TransformedBox.centre - pCollisionObject->m_TransformedBox.extents,
					pCollisionObject->m_TransformedBox.centre + pCollisionObject->m_TransformedBox.extents, identity, Maths::RGBA(255, 255, 0));
			pBody = pBody->m_pNext;
		}
		Btl::String str;
		str += Btl::String::Format("Active Object Count = %i\n ", activeObjectCount);
		pRender->Draw2dText(Maths::HVec2(64.f, 128.f), str.Chars());
	}

	void Draw3dGrid(DbUtil::I_DebugRender*pRender, const Maths::HVec3 PCREF planeCentre, const Maths::HVec3 PCREF u, 
		const Maths::HVec3 PCREF v, const Maths::HVec3 PCREF n, const Maths::HF32 PCREF size, DebugMeshDrawBuffer* pMeshDrawBuffer)
	{
		const F32 segmentSize = 4.f;
		const U32 numSegments = (U32)(size / ((F32)segmentSize));

		const Maths::HVec3 bottomLeft(planeCentre - (u * size + v * size) * 0.5f);
		const Maths::HVec3 widthStep(u * segmentSize);
		const Maths::HVec3 heightStep(v * segmentSize);

		Maths::HVec3 leftToRight((Maths::Floor((bottomLeft | u)/segmentSize)*segmentSize) * u + (Maths::Floor((bottomLeft | v)/segmentSize)*segmentSize) * v + (bottomLeft | n) * n);


		
		for(U32 a = 0; a <= numSegments; ++a)
		{
			Maths::HVec3 topToBottom(leftToRight);
			if (pMeshDrawBuffer)
				pMeshDrawBuffer->Draw3dLine(topToBottom, topToBottom + heightStep, Maths::RGBA(255, 0, 0));
			else
				pRender->Draw3dLine(topToBottom, topToBottom + heightStep, Maths::RGBA(255, 0, 0));
			for(U32 a = 0; a < numSegments; ++a)
			{
				if (pMeshDrawBuffer)
				{
					pMeshDrawBuffer->Draw3dLine(topToBottom, topToBottom + heightStep, Maths::RGBA(255, 0, 0));
					pMeshDrawBuffer->Draw3dLine(topToBottom, topToBottom + widthStep, Maths::RGBA(255, 0, 0));
				}
				else
				{
					pRender->Draw3dLine(topToBottom, topToBottom + heightStep, Maths::RGBA(255, 0, 0));
					pRender->Draw3dLine(topToBottom, topToBottom + widthStep, Maths::RGBA(255, 0, 0));
				}
				topToBottom += heightStep;
			}
			if (pMeshDrawBuffer)
				pMeshDrawBuffer->Draw3dLine(topToBottom, topToBottom + widthStep, Maths::RGBA(255, 0, 0));
			else
				pRender->Draw3dLine(topToBottom, topToBottom + widthStep, Maths::RGBA(255, 0, 0));
			leftToRight += widthStep;
		}
	}

	void C_Scene::DrawTriggerLayer(DbUtil::I_DebugRender* pRender, C_TriggerLayer* pLayer, Collision::C_CRAxisBox& box, SharedPhysics::C_SafeStack<true>& stack, 
		Collision::C_CollisionFilter* pFilter)
	{
		using namespace Maths;
		bool bDrawTriggers = g_dbDrawTriggers;
		bool bDrawTriggerActors = g_dbDrawTriggerActors;


		if(bDrawTriggerActors)
		{
			for(S32 a = 0; a < pLayer->m_TriggerActorArray.m_numElements; ++a)
			{
				C_TriggerActor* pTrigger = pLayer->m_TriggerActorArray[a].m_pElement;
				if(box.Intersects(pTrigger->m_TransformedBox))
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-pTrigger->m_TransformedBox.extents, pTrigger->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), pTrigger->m_TransformedBox.centre), Maths::RGBA(0, 255, 0));
					if(pTrigger->m_pShape->m_TriggerType == C_TriggerShape::E_TriggerBox)
					{
						//Draw box
						C_TriggerBox* pBox = (C_TriggerBox*)pTrigger->m_pShape;
						pRender->Draw3dBox(-pBox->m_Box.Extent(), pBox->m_Box.Extent(), pBox->m_PosRot, Maths::RGBA(255, 0, 0));

					}
					else if (pTrigger->m_pShape->m_TriggerType == C_TriggerShape::E_TriggerSphere)
					{
						//Draw sphere
						C_TriggerSphere* pSphere = (C_TriggerSphere*)pTrigger->m_pShape;
						pRender->Draw3dSphere(pSphere->m_Sphere.Centre(), pSphere->m_Sphere.Radius(), Maths::RGBA(255, 0, 0));
					}
				}
			}
		}

		if(bDrawTriggers)
		{
			for(S32 a = 0; a < pLayer->m_TriggerArray.m_numElements; ++a)
			{
				C_Trigger* pTrigger = pLayer->m_TriggerArray[a].m_pElement;
				if(box.Intersects(pTrigger->m_TransformedBox))
				{
					//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-pTrigger->m_TransformedBox.extents, pTrigger->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), pTrigger->m_TransformedBox.centre), Maths::RGBA(0, 255, 0));
					switch(pTrigger->m_pTriggerShape->m_ShapeType)
					{
						case Collision::E_Box_Shape:
						{
							const Collision::C_BoxShape* const __restrict pBoxShape((Collision::C_BoxShape*)pTrigger->m_pTriggerShape);
							const HMat33& u(pBoxShape->m_Box.Orientation());
							const HMat43 absPosRot = Maths::HMat43(u, pBoxShape->m_Box.Centre());
							const HVec3 maxV(pBoxShape->m_Box.Extent());
							const HVec3 minV(-maxV);
							pRender->Draw3dBox(minV, maxV, absPosRot, RGBA(255, 0, 0, 255));
							break;
						}
					case Collision::E_Sphere_Shape:
						{
							//Must be a sphere
							const Collision::C_SphereShape* const __restrict pSphereShape((Collision::C_SphereShape*)pTrigger->m_pTriggerShape);
							pSphereShape->DebugRender(pRender);
							break;
						}
					case Collision::E_ConvexHull_Shape:
						{
							Collision::C_ConvexHullShape* pShape((Collision::C_ConvexHullShape*)pTrigger->m_pTriggerShape);
							pShape->DebugRender(pRender);
							break;
						}
					case Collision::E_Cylinder_Shape:
						{
							Collision::C_CylinderShape* pShape((Collision::C_CylinderShape*)pTrigger->m_pTriggerShape);
							pShape->DebugRender(pRender);
							break;
						}
					case Collision::E_Cone_Shape:
						{
							const Collision::C_ConeShape* pConeShape((Collision::C_ConeShape*)pTrigger->m_pTriggerShape);
							pConeShape->DebugRender(pRender);
							break;

						}
					case Collision::E_Capsule_Shape:
						{
							Collision::C_CapsuleShape* pShape((Collision::C_CapsuleShape*)pTrigger->m_pTriggerShape);
							pShape->DebugRender(pRender);
							break;
						}
					case Collision::E_Composite_Shape:
						{
							//const Collision::C_CompositeShape* const __restrict pCompShape((Collision::C_CompositeShape*)pCompShape->m_pShapes[i].m_pShape);
							const Collision::C_CompositeShape* pCompShape2((Collision::C_CompositeShape*)pTrigger->m_pTriggerShape);
							DrawCompositeShape(pRender, pCompShape2, stack, pFilter);
							break;
						}

					}
				}
			}
		}
	}



	void C_Scene::DebugRenderScene(DbUtil::I_DebugRender* pRender, Collision::C_CRAxisBox& box,
		SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter)
	{
		PERF_EVENT("Gravity.DebugDraw");

		bool drawActive = g_dbDrawActiveObjects, drawSleeping(g_dbDrawSleepingObjects), drawStatic = g_dbDrawStaticObjects,
			drawKinematic = g_dbDrawKinematicObjects;
		bool bDrawPlane = g_dbDrawGroundPlane;
		bool bDrawBoundingBox = g_dbDrawBoundingBoxes;
		bool bDrawContinuousBox = g_dbDrawContinousBoxes;
		bool bDrawConstraints = g_dbDrawConstraints;

		bool bShowStaticObjectCount = g_dbShowStaticObjectCount;
		bool bShowDynamicObjectCount = g_dbShowDynamicObjectCount;
		bool bShowActiveObjectCount = g_dbShowActiveObjectCount;

		bool bShowTriggerCount		= g_dbShowTriggerCount;
		bool bShowTriggerActorCount	= g_dbShowTriggerActorCount;
		bool bShowActiveNodeCount = g_dbShowActiveTriggerCount;
		


		bool bDrawTriggers = g_dbDrawTriggers;
		bool bDrawTriggerActors = g_dbDrawTriggerActors;

#ifndef CFG_GOLD
		s_MeshDrawBuffer.pDebugRender = pRender;
#endif // !CFG_GOLD

		if(g_dbDrawMesh || g_dbShowDefaultMaterial)
		{
			Maths::HMat43 matrix;
			matrix.SetIdentity();
			//const U32 maxTriangles(2048);
			//U32 triangleDrawn = 0;
			//Grab the current camera position!
			const Maths::HF32 three(Maths::Constants<Maths::HF32>::Three());
			const U32 numMeshes(NumMeshBodies());

			//const Maths::RGBA colours[7]=
			//{
			//	RGBA(55, 55, 55), RGBA(255, 255, 55), RGBA(255, 55, 255), RGBA(55, 255, 255),
			//		RGBA(128, 128, 0), RGBA(0, 128, 128), RGBA(128, 0, 128)
			//};

			if (numMeshes)
			{
				const Maths::RGBA colour(55, 55, 55);

				Collision::C_AxisBoxMinMax axisBox;
				for(U32 a = 0; a < numMeshes; ++a)
				{
					//Draw mesh!
					const Gravity::C_Body* pMeshBody(GetMeshBody(a));
					const Collision::C_TriangleMeshShape* pTriMeshShape((const Collision::C_TriangleMeshShape*)pMeshBody->GetShape());
					const Collision::OptimisedAABBTree & tree(pTriMeshShape->m_AABBTree);
					if(!g_dbCollisionFlag && (pFilter != NULL && !pTriMeshShape->m_Filter.CanCollide(pFilter)))
						continue;

					const Maths::RGBA colour(55, 55, 55);
#ifndef CFG_GOLD
					DrawMesh(pRender, tree, matrix, box, stack, colour, pFilter, &s_MeshDrawBuffer);
#else
					DrawMesh(pRender, tree, matrix, box, stack, colour, pFilter, NULL);
#endif
				}
			}
		}


		if(bShowStaticObjectCount || bShowDynamicObjectCount || bShowActiveObjectCount || 
			bShowTriggerCount || bShowTriggerActorCount || bShowActiveNodeCount)
		{
			Btl::String str;
			if(bShowStaticObjectCount)
			{
				str += Btl::String::Format("Static Object Count = %i\n ", this->m_StaticBodies.m_numElements);
			}
			if(bShowDynamicObjectCount)
			{
				str += Btl::String::Format("Dynamic Object Count = %i\n ", this->m_Bodies.m_numElements);
			}
			if(bShowActiveObjectCount)
			{
				str += Btl::String::Format("Active Object Count = %i\n", this->m_NumActiveBodies);
			}
			if(bShowTriggerCount)
			{
				str += Btl::String::Format("Trigger Count = %i\n", this->m_TriggerSystem.GetTriggerCount());
			}
			if(bShowTriggerActorCount)
			{
				str += Btl::String::Format("Trigger Actor Count = %i\n", this->m_TriggerSystem.GetTriggerActorCount());
			}
			if(bShowActiveNodeCount)
			{
				str += Btl::String::Format("Trigger Active Node Count = %i\n", this->m_TriggerSystem.GetActiveNodeCount());
			}

			pRender->Draw2dText(Maths::HVec2(64.f, 128.f), str.Chars());

		}


		Maths::HMat43 identity;
		identity.SetIdentity();

		if(g_dbDrawWorldBounds)
		{
			const Collision::C_AxisBoxMinMax& worldBox = GetWorldBounds();
			pRender->Draw3dBox(worldBox.m_min, worldBox.m_max, identity, Maths::RGBA(255, 0, 0));
		}


		if(bDrawPlane && m_pGroundPlane)
		{
			//Draw the ground plane!
			Collision::C_PlaneShape* pPlane = (Collision::C_PlaneShape*)m_pGroundPlane->GetShape();
			Collision::C_Plane& plane = pPlane->m_Plane;
			/*Maths::C_Plane plane(Maths::HVec4(_plane.m_n.X(), _plane.m_n.Y(), _plane.m_n.Z(), _plane.m_d));*/
			Maths::HVec3 u(Maths::UNDEFINED), v(Maths::UNDEFINED);
			plane.m_n.m_V.PerpendicularAxes(u, v);

			const Maths::HF32 boxDist((box.centre | plane.m_n) - plane.m_d);
			const Maths::HVec3 planeCentre(box.centre - plane.m_n * boxDist);

#ifndef CFG_GOLD
			Draw3dGrid(pRender, planeCentre, u, v, plane.m_n.m_V, 50.f, &s_MeshDrawBuffer);
#else
			Draw3dGrid(pRender, planeCentre, u, v, plane.m_n.m_V, 50.f, NULL);
#endif

			//const Maths::HMat43 gridTransform(Maths::HMat33(u, plane.m_n, v), Maths::HVec3(plane.m_n * plane.m_d));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dGrid(gridTransform);
		//	const Maths::HMat43 gridLocation(
		//	pRender->Draw3dGrid(plane, box.centre, 1.f, Maths::RGBA(255, 0, 0));
		}

		if(g_dbDrawActiveList)
		{
			RenderObjectLinkedList(pRender, box, stack, pFilter, m_ActiveBodyList);
		}

		if(drawActive || drawSleeping || drawStatic || drawKinematic)
		{
			{
				PERF_EVENT("Gravity.DebugDraw.Bodies");
				RenderObjectList(pRender, box, stack, pFilter, m_Bodies);
			}

			if(drawStatic)
			{
				PERF_EVENT("Gravity.DebugDraw.StaticBodies");
				RenderObjectList(pRender, box, stack, pFilter, m_StaticBodies);
			}

			if(g_dbDrawSpring)
			{
				for(S32 i=0; i<m_Springs.m_numElements; ++i)
				{
					SharedPhysics::C_Spring* pSpring = m_Springs[i].m_pElement;
					const Maths::HVec3 contactAW = pSpring->m_pRigidA->m_CenterOfMass + pSpring->m_LocalContactA * pSpring->m_pRigidA->m_OrientationMatrix;
					const Maths::HVec3 contactBW = pSpring->m_pRigidB->m_CenterOfMass + pSpring->m_LocalContactB * pSpring->m_pRigidB->m_OrientationMatrix;
#ifndef CFG_GOLD
					s_MeshDrawBuffer.Draw3dStar(contactAW, 0.2f, Maths::RGBA(0, 255, 0, 0));
					s_MeshDrawBuffer.Draw3dStar(contactBW, 0.2f, Maths::RGBA(0, 0, 255, 0));
					s_MeshDrawBuffer.Draw3dLine(contactAW, contactBW, Maths::RGBA(255, 0,  0, 0));
#else
					pRender->Draw3dStar(contactAW, 0.2f, Maths::RGBA(0, 255, 0, 0));
					pRender->Draw3dStar(contactBW, 0.2f, Maths::RGBA(0, 0, 255, 0));
					pRender->Draw3dLine(contactAW, contactBW, Maths::RGBA(255, 0,  0, 0));
#endif
				//	meshDrawBuffer.Draw3dArrow(contactAW, pSpring->m_Normal, 0.5f, 0.5f, Maths::RGBA(0, 0, 0, 0));
				}
			}

			if(g_dbDrawCloth)
			{
				for(S32 i=0; i<m_ClothBodies.m_numElements; ++i)
				{
					C_Body* pBody=m_ClothBodies[i].m_pElement;

					if((pBody->IsActive() && !g_dbDrawActiveObjects) || 
						(!pBody->IsActive() && !g_dbDrawSleepingObjects))
						continue;

					Collision::C_ClothShape* pShape=(Collision::C_ClothShape*)pBody->GetShape();
					Collision::C_Cloth& cloth=pShape->m_Cloth;
					Collision::C_ConstrainedParticleEdge* edges = cloth.m_pEdges;
					//Maths::HVec3* vertices=cloth.m_CurPosition;
					Collision::C_SoftBodyTriangle* tris=cloth.m_pTriangles;

					if(bDrawBoundingBox)
					{
						pRender->Draw3dBox(pShape->m_LocalBoundingBox.centre - pShape->m_LocalBoundingBox.extents,
							pShape->m_LocalBoundingBox.centre + pShape->m_LocalBoundingBox.extents, identity, Maths::RGBA(155, 155, 155));
					}


					/*for(U32 j=0; j< cloth.m_NumEdges; ++j)
					{
						const Collision::C_ConstrainedParticleEdge& edge=edges[j];
						if(edge.m_Active)
						{
							const Maths::HVec3& vA=cloth.m_Particles[edge.m_IndexA].m_CurPosition;
							const Maths::HVec3& vB=cloth.m_Particles[edge.m_IndexB].m_CurPosition;
							pRender->Draw3dLine(vA, vB, Maths::RGBA(0, 255, 0, 255));
						}
					}*/

					for(U32 j=0; j<cloth.m_numTriangles; ++j)
					{
						Collision::C_SoftBodyTriangle& tri=cloth.m_pTriangles[j];
						if(tri.m_Active)
						{
							const Maths::HVec3& vA = cloth.m_pParticles[tri.m_VertexIndexA].m_CurPosition;
							const Maths::HVec3& vB = cloth.m_pParticles[tri.m_VertexIndexB].m_CurPosition;
							const Maths::HVec3& vC = cloth.m_pParticles[tri.m_VertexIndexC].m_CurPosition;
#ifndef CFG_GOLD
							s_MeshDrawBuffer.Draw3dTriangle(vA, vB, vC, Maths::RGBA(0, 155, 155));
							s_MeshDrawBuffer.Draw3dTriangle(vA, vC, vB, Maths::RGBA(155, 155, 0));
#else
							pRender->Draw3dTriangle(vA, vB, vC, Maths::RGBA(0, 155, 155));
							pRender->Draw3dTriangle(vA, vC, vB, Maths::RGBA(155, 155, 0));
#endif
						}
					}

					for(U32 j=0; j<cloth.m_numEdges; ++j)
					{
						Collision::C_ConstrainedParticleEdge& edge =cloth.m_pEdges[j];
						if(edge.m_Active)
						{
							const Maths::HVec3& vA = cloth.m_pParticles[edge.m_IndexA].m_CurPosition;
							const Maths::HVec3& vB = cloth.m_pParticles[edge.m_IndexB].m_CurPosition;
#ifndef CFG_GOLD
							s_MeshDrawBuffer.Draw3dLine(vA, vB, Maths::RGBA(255, 0, 0));
#else
							pRender->Draw3dLine(vA, vB, Maths::RGBA(255, 0, 0));
#endif
						}
					}

					/*for(S32 j=0; j< cloth.m_References.m_numElements; ++j)
					{
						const Collision::C_ClothReference& reference= cloth.m_References[j];

						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(reference.m_Reference, 0.05f, Maths::RGBA(0, 255, 0, 255));
					}*/

					if(bDrawBoundingBox)
					{
						for(U32 j = 0; j < cloth.m_numNodes; ++j)
						{
							Collision::C_SoftBodyAABBNode& node = cloth.m_pNodes[j];
							if(node.m_NumTriangles > 0)
							{
								pRender->Draw3dBox(node.m_BoundingBox.m_min, node.m_BoundingBox.m_max, identity, Maths::RGBA(0, 255, 0));
							}
						}

					}
				}
			}

			if(g_dbDrawDeformable)
			{
				for(S32 i=0; i<m_DeformableBodies.m_numElements; ++i)
				{
					C_Body* pBody = m_DeformableBodies[i].m_pElement;
					Collision::C_DeformableShape* pDeformableShape = (Collision::C_DeformableShape*)pBody->GetShape();
					Collision::C_DeformableBody& deformable = pDeformableShape->m_Deformable;

					Collision::C_ConstrainedParticleEdge* edges = deformable.m_pEdges;
					Collision::C_SoftBodyTriangle* tris=deformable.m_pTriangles;

					if(bDrawBoundingBox)
					{
						pRender->Draw3dBox(pDeformableShape->m_LocalBoundingBox.centre - pDeformableShape->m_LocalBoundingBox.extents,
							pDeformableShape->m_LocalBoundingBox.centre + pDeformableShape->m_LocalBoundingBox.extents, identity, Maths::RGBA(155, 155, 155));
					}

					for(U32 j=0; j<deformable.m_numTriangles; ++j)
					{
						Collision::C_SoftBodyTriangle& tri=deformable.m_pTriangles[j];
						if(tri.m_Active)
						{
							const Maths::HVec3& vA = deformable.m_pParticles[tri.m_VertexIndexA].m_CurPosition;
							const Maths::HVec3& vB = deformable.m_pParticles[tri.m_VertexIndexB].m_CurPosition;
							const Maths::HVec3& vC = deformable.m_pParticles[tri.m_VertexIndexC].m_CurPosition;
#ifndef CFG_GOLD
							s_MeshDrawBuffer.Draw3dTriangle(vA, vB, vC, Maths::RGBA(0, 155, 155));
#else
							pRender->Draw3dTriangle(vA, vB, vC, Maths::RGBA(0, 155, 155));
#endif
						}
					}

					for(U32 j=0; j<deformable.m_numEdges; ++j)
					{
						Collision::C_ConstrainedParticleEdge& edge =deformable.m_pEdges[j];
						if(edge.m_Active)
						{
							const Maths::HVec3& vA = deformable.m_pParticles[edge.m_IndexA].m_CurPosition;
							const Maths::HVec3& vB = deformable.m_pParticles[edge.m_IndexB].m_CurPosition;
#ifndef CFG_GOLD
							s_MeshDrawBuffer.Draw3dLine(vA, vB, Maths::RGBA(255, 0, 0));
#else
							pRender->Draw3dLine(vA, vB, Maths::RGBA(255, 0, 0));
#endif
						}
					}

					if(bDrawBoundingBox)
					{
						for(U32 j = 0; j < deformable.m_numNodes; ++j)
						{
							Collision::C_SoftBodyAABBNode& node = deformable.m_pNodes[j];
							if(node.m_NumTriangles > 0)
							{
								pRender->Draw3dBox(node.m_BoundingBox.m_min, node.m_BoundingBox.m_max, identity, Maths::RGBA(0, 255, 0));
							}
						}

					}

				}
			}

			if(g_dbDrawRope)
			{
				for(S32 i=0; i<m_RopeBodies.m_numElements; ++i)
				{
					C_Body* pBody=m_RopeBodies[i].m_pElement;
					Collision::C_RopeShape* pRopeShape = (Collision::C_RopeShape*)pBody->GetShape();
					Collision::C_Rope& rope = pRopeShape->m_Rope;

					if(bDrawBoundingBox)
					{
						pRender->Draw3dBox(pRopeShape->m_LocalBoundingBox.centre - pRopeShape->m_LocalBoundingBox.extents,
							pRopeShape->m_LocalBoundingBox.centre + pRopeShape->m_LocalBoundingBox.extents, identity, Maths::RGBA(155, 155, 155));
					}


					for(U32 a = 1; a < rope.m_numParticles; ++a)
					{
						Collision::C_GravityParticle& parA = rope.m_pParticles[a-1];
						Collision::C_GravityParticle& parB = rope.m_pParticles[a];
#ifndef CFG_GOLD
						s_MeshDrawBuffer.Draw3dLine(parA.m_CurPosition, parB.m_CurPosition, Maths::RGBA(255, 0, 0));
#else
						pRender->Draw3dLine(parA.m_CurPosition, parB.m_CurPosition, Maths::RGBA(255, 0, 0));
#endif
					}
				}
			}

			if(g_dbDrawFluids)
			{
				const Maths::HF32 half=Maths::Constants<Maths::HF32>::Half();
				Maths::HMat33 rotation;
				rotation.SetIdentity();
				for(S32 i=0; i<m_FluidBodies.m_numElements; ++i)
				{
					//SharedPhysics::SortedArray<Maths::HVec3> surfaces;
					C_Body* pBody=m_FluidBodies[i].m_pElement;
					Collision::C_FluidShape* pShape=(Collision::C_FluidShape*)pBody->GetShape();
					Collision::C_Fluid& fluid=pShape->m_Fluid;
					
					const Maths::HF32 ext(fluid.m_Width*half);

					for(U32 a=1; a<=fluid.m_Dimension; ++a)
					{
						Collision::C_GridCell* gridRow=fluid.m_Grid[a];
						
						for(U32 b=1; b<=fluid.m_Dimension; ++b)
						{
							Collision::C_GridCell& cell=gridRow[b];
							const Maths::HF32 height=cell.m_CurHeight * half;
							const Maths::HVec3 center(cell.m_WorldPosition.X(), cell.m_WorldPosition.Y() + height, cell.m_WorldPosition.Z());
							Maths::HMat43 mat;//(rotation, center);
							mat.SetIdentity();
							const Maths::HVec3 extents(ext, height, ext);
							pRender->Draw3dBox(center-extents, center+extents, mat, Maths::RGBA(0, 0, 255, 255));
						}
					}

					for(U32 a=1; a<=fluid.m_Dimension; ++a)
					{
						Collision::C_GridCell* gridRow=fluid.m_Grid[a];
					
						for(U32 b=1; b<=fluid.m_Dimension; ++b)
						{
							Collision::C_GridCell& cell=gridRow[b];
							//const Maths::HVec3 temp=cell.m_WorldPosition;
							Maths::HVec3 center=cell.m_WorldPosition;
							center.Y(center.Y() + cell.m_CurHeight);
							const Maths::HF32 x(center.X());
							const Maths::HF32 y(center.Y());
							const Maths::HF32 z(center.Z());
							const Maths::HVec3 p1=Maths::HVec3(x-ext, y, z-ext);
							const Maths::HVec3 p2=Maths::HVec3(x-ext, y, z+ext);
							const Maths::HVec3 p3=Maths::HVec3(x+ext, y, z-ext);
							const Maths::HVec3 p4=Maths::HVec3(x+ext, y, z+ext);

#ifndef CFG_GOLD
							if(cell.m_Hit)
							{
								s_MeshDrawBuffer.Draw3dTriangleLit(p2, p3, p1, Maths::RGBA(0, 255, 200+b, 255));
								s_MeshDrawBuffer.Draw3dTriangleLit(p2, p4, p3, Maths::RGBA(0, 255, 200+b, 255));
							}
							else
							{
								s_MeshDrawBuffer.Draw3dTriangleLit(p2, p3, p1, Maths::RGBA(0, 0, 200+b, 255));
								s_MeshDrawBuffer.Draw3dTriangleLit(p2, p4, p3, Maths::RGBA(0, 0, 180+b, 255));
							}
#else
							if(cell.m_Hit)
							{
								pRender->Draw3dTriangleLit(p2, p3, p1, Maths::RGBA(0, 255, 200+b, 255));
								pRender->Draw3dTriangleLit(p2, p4, p3, Maths::RGBA(0, 255, 200+b, 255));
							}
							else
							{
								pRender->Draw3dTriangleLit(p2, p3, p1, Maths::RGBA(0, 0, 200+b, 255));
								pRender->Draw3dTriangleLit(p2, p4, p3, Maths::RGBA(0, 0, 180+b, 255));
							}
#endif
							
						}
						
					}
				}
			}
		}

		if(g_dbDrawActiveNodes)
		{
			m_pTree->DrawActiveNodes(pRender, (bool)g_dbShowInactivePartsOfActiveNodes);
		}
		if(g_dbDrawSleepingNodes)
		{
			m_pTree->DrawSleepingNodes(pRender);
		}

		if(bDrawConstraints)
		{

			for(S32 a = 0; a < m_pGraphManager->m_DebugConstraints.m_numElements; ++a)
			{
				SharedPhysics::C_Constraint* pConstraint =m_pGraphManager->m_DebugConstraints[a].m_pElement;
				pConstraint->DebugRender(pRender);
			}
			/*for(S32 a = 0; a < this->m_Constraints.m_numElements; ++a)
			{
				SharedPhysics::C_Constraint* pConstraint = m_Constraints[a].m_pElement;
				pConstraint->DebugRender(pRender);
			}*/
		}


		if(g_dbDrawStaticFluidMesh)
		{
			Maths::HMat43 matrix;
			matrix.SetIdentity();
			const Maths::RGBA colour(0, 0, 255, 255);

			for(S32 a=0; a< this->m_FluidMeshBodies.m_numElements; ++a)
			{
				const Gravity::C_Body* pFluidMeshBody = m_FluidMeshBodies[a].m_pElement;
				const Collision::C_FluidMeshShape* pShape = (const Collision::C_FluidMeshShape*)pFluidMeshBody->GetShape();
				const Collision::OptimisedAABBTree & tree(*pShape->m_AABBTree.m_pTree);
				const C_Object* pCollisionObject(pFluidMeshBody->GetObject());

				if(bDrawBoundingBox)
				{
					pRender->Draw3dBox(pCollisionObject->m_TransformedBox.centre - pCollisionObject->m_TransformedBox.extents,
						pCollisionObject->m_TransformedBox.centre + pCollisionObject->m_TransformedBox.extents, identity, Maths::RGBA(255, 255, 0));
				}
				
#ifndef CFG_GOLD
				DrawMesh(pRender, tree, pShape->m_AABBTree.m_PosRot, box, stack, colour, pFilter, &s_MeshDrawBuffer);
#else
				DrawMesh(pRender, tree, pShape->m_AABBTree.m_PosRot, box, stack, colour, pFilter, NULL);
#endif
			}
		}

		if(bDrawTriggers || bDrawTriggerActors)
		{
			//Go through the scene and trigger system and draw as much as required...

			C_TriggerSystem* pSystem = this->GetTriggerSystem();
			C_TriggerLayer* pLayer = pSystem->m_pLayers;
			while(pLayer)
			{
				DrawTriggerLayer(pRender, pLayer, box, stack, pFilter);
				pLayer = pLayer->m_pNext;
			}

		}

#ifndef CFG_GOLD
		s_MeshDrawBuffer.Draw(pRender);
#endif

	}

#endif

}
#ifndef CFG_SPEED
#pragma optimize("", on)
#endif
