#include "stdafx.h"

#include "SectionPhysics.h"
#include "SegmentSetting.h"
#include "gravity/narrowphase/ConvexHull.h"
#include "gravity/narrowphase/primitivetest.inl"
#include "gravity/narrowphase/Shapes.h"
//#include "gravity/narrowphase/CylinderRay.h"
#include "gravity/narrowphase/ConvexHullRay.inl"
#include "gravity/narrowphase/Ray.h"
#include "gravity/narrowphase/primitive.inl"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>
#include "gravity/exporter/convexhullgeneration.h"
#include "btl/utility/string.h"
#include "gravity/sdk/ShapeDescriptors.h"


void C_SectionPhysics::ConvexHullGenerationCodeTest()
{
	using namespace Maths;

	static U32 num =0;
	static bool bInitialised = false;

	Input::C_GameInput* gameInput = Input::GetGameInput(0);

	if(!bInitialised)
	{

		delete pConvexShape;
		pConvexShape = NULL;
		pConvexHullDesc.m_Triangles.m_numElements = 0;
		pConvexHullDesc.m_Points.m_numElements = 0;
		//Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open("z:\\ConvexHullPoints.dat");
		

		//if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonY))
		//{
			//num++;
		//}

		Btl::String filename;
		filename = Btl::String::Format("game:\\ConvexHullPoints%i.dat", num++);
		Sys::I_BinaryStream *pFile = Sys::C_FileSystem::Instance().Open( filename.Chars() );
		 
		SharedPhysics::SortedArray<HVec3> points;
		S32 vertexCount;
		if(pFile)
		{

			pFile->SetEndianSwapFlag( false );
			{
				Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);

				ar  >> vertexCount;

				F32 x,y,z;

				for(S32 i=0 ;i<vertexCount; ++i)
				{
					ar >> x >> y >> z;
					HVec3 p(x, y, z);
					points.AddToEnd(&p);
				}
			}
		}

		
		mesh.Initialise(points);
		delete pFile;
		bInitialised = true;
	}

	
	Gravity::GenerationState remap = Gravity::E_Generation_Fail;
	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA))
	{
		remap = mesh.StepBuild();
	}

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB))
	{
		mesh.m_Points.m_numElements =0;
		mesh.m_SupportPoints.m_numElements=0;
		mesh.m_Facets.m_numElements=0;
		mesh.m_Iterator.Reset();
		mesh.heap.makeEmpty();
		mesh.edgeBuffer.m_Size = 0;
		bInitialised = false;
	}

	for(S32 i = 0; i<mesh.m_SupportPoints.m_numElements; ++i)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mesh.m_SupportPoints[i].m_Point, 0.05f, RGBA(255, 255, 255));
	}

	int count =0;
	for(S32 i=0; i<mesh.m_Facets.m_numElements; ++i)
	{
		Gravity::C_Facet* facet = mesh.m_Facets[i];
		if(!facet->m_Discarded)
		{
			count++;
			const HVec3 a(mesh.m_SupportPoints[mesh.m_Points[facet->m_indice[0]]].m_Point);
			const HVec3 b(mesh.m_SupportPoints[mesh.m_Points[facet->m_indice[1]]].m_Point);
			const HVec3 c(mesh.m_SupportPoints[mesh.m_Points[facet->m_indice[2]]].m_Point);
			/*Btl::String str;
			str = str.Format("%i", facet->m_indice[0]);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(a, str.Chars());
			str = str.Format("%i", facet->m_indice[1]);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(b, str.Chars());
			str = str.Format("%i", facet->m_indice[2]);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(c, str.Chars());
			const Maths::HVec3 position((a+b+c)/3.f);
			
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(position, 0.5f, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow( (a + b +c)/3.f, facet->m_Normal, 1.f, 0.25f, RGBA(255, 255, 0)); */
		//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, c, b, RGBA(0, 255, 255));
		//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(a, b, c, RGBA(255, 0 ,0));
		}
	}

	Btl::String fileVersion;
	fileVersion = Btl::String::Format("File Version number = %i", num-1);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(2.f, 236.f), fileVersion.Chars());

	Btl::String strRes;
	strRes = Btl::String::Format("Number triangles = %i", count);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(2.f, 256.f), strRes.Chars());

	/*for(S32 i=0; i<mesh.m_Points.m_numElements; ++i)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mesh.m_SupportPoints[mesh.m_Points[i]].m_Point, 0.25f, RGBA(0, 255, 0));
	}*/


	if(remap  == Gravity::E_Generation_Succeed && pConvexShape == NULL)
	{
		S32 index = 0;
		Maths::HVec3* pVertice = new Maths::HVec3[mesh.m_Points.m_numElements];
		S32 * remapAddress = new S32[mesh.m_SupportPoints.m_numElements];

		for(S32 a = 0; a < mesh.m_SupportPoints.m_numElements; ++a)
		{
			remapAddress[a] = -1;
		}


		for(S32 a = 0; a < mesh.m_Facets.m_numElements; ++a)
		{
			Gravity::C_Facet* pFacet = mesh.m_Facets[a];
			if(!pFacet->m_Discarded)
			{
				if(remapAddress[mesh.m_Points[pFacet->m_indice[0]]] == -1)
				{
					pVertice[index] = mesh.m_SupportPoints[mesh.m_Points[pFacet->m_indice[0]]].m_Point;
					remapAddress[mesh.m_Points[pFacet->m_indice[0]]] = index++;
				}
				if(remapAddress[mesh.m_Points[pFacet->m_indice[1]]] == -1)
				{
					pVertice[index] = mesh.m_SupportPoints[mesh.m_Points[pFacet->m_indice[1]]].m_Point;
					remapAddress[mesh.m_Points[pFacet->m_indice[1]]] = index++;
				}
				if(remapAddress[mesh.m_Points[pFacet->m_indice[2]]] == -1)
				{
					pVertice[index] = mesh.m_SupportPoints[mesh.m_Points[pFacet->m_indice[2]]].m_Point;
					remapAddress[mesh.m_Points[pFacet->m_indice[2]]] = index++;
				}

				//Remap facet indices...
				/*U32 oldIndexA = pFacet->m_indice[0];
				U32 oldIndexB = pFacet->m_indice[1];
				U32 oldIndexC = pFacet->m_indice[2];
				pFacet->m_indice[0] = (U32)remapAddress[mesh.m_Points[pFacet->m_indice[0]]];
				pFacet->m_indice[1] = (U32)remapAddress[mesh.m_Points[pFacet->m_indice[1]]];
				pFacet->m_indice[2] = (U32)remapAddress[mesh.m_Points[pFacet->m_indice[2]]];*/

				/*BIZ_ASSERT(pFacet->m_indice[0] < index);
				BIZ_ASSERT(pFacet->m_indice[1] < index);
				BIZ_ASSERT(pFacet->m_indice[2] < index);*/

			}
		}

		/*mesh.m_ResultPoints.Resize(index);
		mesh.m_ResultPoints.m_numElements = index;
		for(S32 a = 0; a < index; ++a)
		{
			mesh.m_ResultPoints[a] = pVertice[a];
		}*/

		/*for(S32 i = 0; i < mesh.m_Points.m_numElements; ++i)
		{
			pDesc->AddPoint(mesh.m_SupportPoints[mesh.m_Points[i]].m_Point);
		}*/

		for(S32 i = 0; i < index; ++i)
		{
			pConvexHullDesc.AddPoint(pVertice[i]);
		}


		if(pConvexHullDesc.m_Points.m_numElements > 4)
		{
			//delete pDesc;
			//return NULL;

			//pConvexHullDesc.m_Triangles.m_numElements = 0;
			//pConvexHullDesc.m_Points.m_numElements = 0;

			for(S32 i = 0; i < mesh.m_Facets.m_numElements; ++i)
			{
				Gravity::C_Facet* facet = mesh.m_Facets[i];
				if(!facet->IsDistarded())
				{
					Gravity::C_TriangleIndices inds;
					inds.m_IndexA = remapAddress[mesh.m_Points[facet->m_indice[0]]];
					inds.m_IndexB = remapAddress[mesh.m_Points[facet->m_indice[2]]];
					inds.m_IndexC = remapAddress[mesh.m_Points[facet->m_indice[1]]];

					/*inds.m_IndexA = facet->m_indice[0];
					inds.m_IndexB = facet->m_indice[2];
					inds.m_IndexC = facet->m_indice[1];*/
					pConvexHullDesc.AddTriangle(inds);
				}
			}
		}
		delete [] pVertice;
		delete [] remapAddress;

		Collision::C_CollisionFilter allSolidFilter;

		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open("Game:\\Test.dat", Sys::E_FileAccess_Write);
		Sys::BinarySerialiser<Ut::NoEndianSwap> archive(*pFile);
		//Sys::BinarySerialiserAutoEndian archive(*pFile);
		pConvexHullDesc.Export(archive);
		delete pFile;
		


		pConvexHullDesc.m_Points.m_numElements = 0;
		pConvexHullDesc.m_Triangles.m_numElements = 0;

		
		Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( "Game:\\Test.dat" );
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*filePtr);
		U32 m_Type;
		ar >> m_Type;
		ar.m_stream.SetVersionNumber(10);

		pConvexHullDesc.Import(ar);
		delete filePtr;

		
		pConvexShape = pConvexHullDesc.CreateShape(&allSolidFilter);


	}

	if(pConvexShape)
	{
		Collision::C_ConvexHullShape* pConvex = (Collision::C_ConvexHullShape*)pConvexShape;
		Collision::C_ConvexHull& hull = pConvex->m_Hull;
		for(U32 i=0; i< hull.m_NumTriangles; ++i)
		{
			/*const HVec3 a(pConvexHullDesc.m_Points[pConvexHullDesc.m_Triangles[i].m_IndexA]);
			const HVec3 b(pConvexHullDesc.m_Points[pConvexHullDesc.m_Triangles[i].m_IndexB]);
			const HVec3 c(pConvexHullDesc.m_Points[pConvexHullDesc.m_Triangles[i].m_IndexC]);*/
			const HVec3 a(hull.GetVertex(pConvexHullDesc.m_Triangles[i].m_IndexA));
			const HVec3 b(hull.GetVertex(pConvexHullDesc.m_Triangles[i].m_IndexB));
			const HVec3 c(hull.GetVertex(pConvexHullDesc.m_Triangles[i].m_IndexC));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, c, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(a, b, c, RGBA(0, 0, 255));
		}

		for(U32 i=0; i<hull.m_NumVertices; ++i)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(hull.GetVertex(i), 0.15f, RGBA(0, 0, 255));
		}
	}
}

void C_SectionPhysics::TestConvexHullRay()
{
	using namespace Maths;

	Collision::C_CollisionFilter filter;
	Collision::C_ConvexHullShape cShape(&filter);
	
	Collision::C_ConvexHull& hull=cShape.m_Hull;
	
	hull.m_Position=Maths::HVec3(-5, 4, -1);
	hull.SetNumberVertices(10);
	hull.m_pVertices[0]=HVec3(-2.f, -2.f, -2.f);
	hull.m_pVertices[1]=HVec3(2.f, -2.f, -2.f);
	hull.m_pVertices[2]=HVec3(-2.f, 2.f, -2.f);
	hull.m_pVertices[3]=HVec3(2.f, 2.f, -2.f);

	hull.m_pVertices[4]=HVec3(-2.f, -2.f, 2.f);
	hull.m_pVertices[5]=HVec3(2.f, -2.f, 2.f);
	hull.m_pVertices[6]=HVec3(-2.f, 2.f, 2.f);
	hull.m_pVertices[7]=HVec3(2.f, 2.f, 2.f);

	hull.m_pVertices[8]=HVec3(0.f, -4.f, 0.f);
	hull.m_pVertices[9]=HVec3(0.f, 4.f, 0.f);

	hull.SetNumberTriangles(16);
	hull.SetTriangles(0, 3, 1);
	hull.SetTriangles(0, 2, 3);
	hull.SetTriangles(4, 5, 7);
	hull.SetTriangles(4, 7, 6);

	hull.SetTriangles(4, 2, 0);
	hull.SetTriangles(4, 6, 2);
	hull.SetTriangles(5, 3, 7);
	hull.SetTriangles(5, 1, 3);

	hull.SetTriangles(6, 7, 9);
	hull.SetTriangles(6, 9, 2);
	hull.SetTriangles(7, 3, 9);
	hull.SetTriangles(3, 2, 9);

	hull.SetTriangles(4, 8, 5);
	hull.SetTriangles(8, 1, 5);
	hull.SetTriangles(1, 8, 0);
	hull.SetTriangles(4, 0, 8);

	Collision::C_Ray ray;
	ray.m_start=Maths::HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	ray.m_end=Maths::HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));

	HMat33 matra;
	matra.SetIdentity();

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	Input::C_GameInput* gameInput = Input::GetGameInput(0);

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA))
	{
		rot-=0.02f;
	}

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB))
	{
		rot+=0.02f;
	}
	

	const Maths::RotX rotxa(rot);
	matra = rotxa;

	hull.m_Orientation=matra;
	U32 addy = (U32)alloca(sizeof(Maths::HVec3)*hull.m_NumVertices + 16);
	addy = (addy + 15)& 0xfffffff0;
	Maths::HVec3* verts((Maths::HVec3*)addy);

	for(U32 b = 0; b < hull.m_NumVertices; ++b)
	{
		const Maths::HVec3 localV(&hull.m_pVertices[b]);
		verts[b] = hull.m_Position + (localV * (hull.m_High - hull.m_Low) + hull.m_Low) * matra;
	}

	//Loop through all the convex triangles and draw them!
	const Maths::RGBA colour(0, 0, 255);
	for(U32 b = 0; b < hull.m_NumTriangles; ++b)
	{
		Collision::C_ConvexTriangle& tri(hull.m_pTriangles[b]);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleLit(verts[tri.m_VertexA],
			verts[tri.m_VertexB], verts[tri.m_VertexC], colour);	
	}

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_start, 0.05f, RGBA(0, 0, 255, 255));

	bool collide=Collision::CollideConvexHullRay(&cShape, ray);

	if(collide)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_end, 0.5f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(ray.m_end, ray.m_result.m_normal, 2.f, 0.5f, RGBA(0, 255, 0, 0));
	}

}
