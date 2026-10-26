#include "stdafx.h"
#include "SectionPhysics.h"

#include "OBBSetting.h"
#include "gravity/broadphase/SweepAndPrune.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/sdk/Body.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "System/misc/Random.h"
#include <System/Serialise/TextSerialise.h>
#include "gravity/sharedphysics/AllocationManager.inl"
#include "gravity/broadphase/SweepAndPrune.inl"
#include "RangeSetting.h"

using namespace Collision;
using namespace Gravity;
using namespace SharedPhysics;


namespace Samples
{
	/*const U32 numOfObjects = 10;
	Gravity::C_Body body[numOfObjects];
	Maths::HMat33 u;

	void DrawList(SharedPhysics::C_DoubleList<C_SweepAndPruneNode>& list, char* name)
	{
		C_SweepAndPruneNode* pNode=list.GetHead();

		Sys::I_BinaryStream* pStream = Sys::C_FileSystem::Instance().Open(name, Sys::E_FileAccess_Write);
		Sys::TextSerialiser archive(*pStream);

		F32 value;
		while(pNode->m_pNext)
		{
			pNode=pNode->m_pNext;
			Maths::Store<Maths::HF32>(pNode->m_Value, &value);
			if(pNode->m_Type == SP_MIN)
				archive << "Object Index: " << pNode->m_pObj->m_pObj->m_Index << ", Value: "<< value<<", SP: MIN\n";
			else
				archive << "Object Index: " << pNode->m_pObj->m_pObj->m_Index << ", Value: "<< value<<", SP: MAX\n";

			
		}
		delete pStream;
		//fclose(listFile);

	}

	//void DrawList(SharedPhysics::SortedArray<SharedPhysics::C_RadixPointerWrapper<C_SweepAndPrunePair>>& list, char* name)
	//{

	//	Sys::I_BinaryStream* pStream = Sys::C_FileSystem::Instance().Open(name, Sys::E_FileAccess_Write);
	//	Sys::TextSerialiser archive(*pStream);
	//	//archive << "NumOfElements "<<list.m_numElements <<"\n";
	//	for(S32 i=0; i<list.m_numElements; ++i)
	//	{
	//		SharedPhysics::C_RadixPointerWrapper<C_SweepAndPrunePair> wrapper=list.m_pElems[i];

	//		C_SweepAndPrunePair* pNode=wrapper.m_pElement;
	//		
	//		archive << "Pair( " << pNode->IndexA()<< " " << pNode->IndexB() <<" )\n";
	//	}
	//	delete pStream;
	//}



	void C_SectionPhysics::SweepAndPruneTest()
	{
		u.SetIdentity();

		static HF32 t(Maths::Constants<HF32>::Zero());
		static F32 tra=0.5f;
		static F32 rotate(0.f);
		static U32 num=0;
		const static HF32 half(Maths::Constants<HF32>::Half());

		const F32 min(-100.f);
		const F32 max(100.f);


		if(tra < 10.f)
		{
			tra +=0.5f;
		}
		else
		{
			tra =-10.f;
		}

		Util::C_Random generator;
		generator.Seed(10);
		C_SweepAndPrune sap;
		for(U32 i=0; i<numOfObjects; ++i)
		{
			C_Object* obj=body[i].GetObject();
			obj->m_Nodes[0].m_pObject=obj;
	
			F32 cX=generator.SignedFRand(range_x);
			F32 cY=generator.SignedFRand(range_y);
			F32 cZ=generator.SignedFRand(range_z);
			F32 eX=generator.FRand(10.f);
			F32 eY=generator.FRand(10.f);
			F32 eZ=generator.FRand(10.f);
			Collision::C_CRAxisBox& box=obj->m_TransformedBox;

			box.centre=HVec3(HF32(cX), HF32(cY), HF32(cZ));

			if(num % 2 ==0)
			{
				if(obj->m_Index % 3 ==0)
				{
					obj->m_pGraphNode->m_NodeFlags=SharedPhysics::C_GraphNode::E_Type_Active;
					box.centre.X(HF32(cX + tra));
				}
				else if(obj->m_Index % 2 ==0)
				{
					obj->m_pGraphNode->m_NodeFlags=SharedPhysics::C_GraphNode::E_Type_Active;
					box.centre.Y(HF32(cY + tra));
				}
			}
			else
			{
				if(obj->m_Index % 3 ==0)
				{
					obj->m_pGraphNode->m_NodeFlags=SharedPhysics::C_GraphNode::E_Type_Active;
					box.centre.Z(HF32(cZ + tra));
				}
				else if(obj->m_Index % 2 ==0)
				{
					obj->m_pGraphNode->m_NodeFlags=SharedPhysics::C_GraphNode::E_Type_Active;
					box.centre.X(HF32(cX + tra));
				}

			}
			num++;

			box.extents=HVec3(HF32(eX), HF32(eY), HF32(eZ));

			sap.AddObject(&obj->m_Nodes[0]);

			const Maths::HVec3 minV(-box.extents);
			const Maths::HVec3 maxV(box.extents);
			const Maths::HMat43 m(u, box.centre);
			

			//if(obj->m_Index == body[3].GetObject()->m_Index)
			//{
			//	
			//	const Maths::HVec3 scale(t, t, t);
			//	box.centre=box.centre + scale;
			//	const Maths::RotX rotX(rotate);
			//	const Maths::HMat33 mat(rotX);
			//	const Maths::HMat43 mA(mat, box.centre);
			//	const Maths::HVec3 zero;
			//	Collision::C_CRAxisBox untransformedBox;
			//	untransformedBox.centre = zero;
			//	untransformedBox.extents = box.extents;
			//	
			//	const Maths::HMat33 mat(rotX);
			//	const Maths::HMat43 mA(mat, box.centre);
			//	untransformedBox.Transform(mA, box);
			//	Util::C_String message;
			//	message.Printf("%d", obj->m_Index);
			//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(box.centre, message);
			//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minV, maxV, mA, RGBA(0, 255, 0, 255));
			//	continue;
			//}

			Util::C_String message;
			message.Printf("%d", obj->m_Index);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(box.centre, message);
			
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minV, maxV, m, RGBA(255, 0, 0, 255));
	
		}

		//DrawList(sap.m_XList, "xList.txt");
		
		
		for(U32 i=0; i<50; ++i)
		{
			sap.UpdateObjects();
		}
		SortedArray<C_RadixPointerWrapper<C_SweepAndPrunePair>>& list=sap.m_ActivePairList;

		C_RadixPointerWrapper<C_SweepAndPrunePair>* elem=list.m_pElems;

		const F32 minColor(0.f);
		const F32 maxColor(255.f);
	
		//DrawList(list, "pairList.txt");

		for(S32 i=0; i<list.m_numElements; ++i)
		{
			C_SweepAndPrunePair* pair=elem[i].m_pElement;
			C_Object* objA=pair->m_pObjA->m_pObj;
			C_Object* objB=pair->m_pObjB->m_pObj;
			Collision::C_CRAxisBox boxA=objA->m_TransformedBox;
			Collision::C_CRAxisBox boxB=objB->m_TransformedBox;

			const Maths::HVec3 minA(-boxA.extents);
			const Maths::HVec3 maxA(boxA.extents);
			const Maths::HVec3 minB(-boxB.extents);
			const Maths::HVec3 maxB(boxB.extents);

			
			F32 valX = minColor + (maxColor - minColor * (F32)(rand() - RAND_MAX));
			F32 valY = minColor + (maxColor - minColor * (F32)(rand() - RAND_MAX));
			F32 valZ = minColor + (maxColor - minColor * (F32)(rand() - RAND_MAX));

			Maths::HMat43 mA(Maths::UNDEFINED);
			if(objA->m_Index == body[3].GetObject()->m_Index)
			{
				const Maths::RotX rotX(rotate);
				const Maths::HMat33 mat(rotX);
				mA=Maths::HMat43(mat, boxA.centre);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, mA, RGBA(valX, valY, valZ, 255.f));
			}
			else
			{	
				mA=Maths::HMat43(u, boxA.centre);
				
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, mA, RGBA(valX, valY, valZ, 255.f));
			}

			Maths::HMat43 mB(Maths::UNDEFINED);
			if(objB->m_Index == body[3].GetObject()->m_Index)
			{
				const Maths::RotX rotX(rotate);
				const Maths::HMat33 mat(rotX);
				mB=Maths::HMat43(mat, boxB.centre);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, mB, RGBA(valX, valY, valZ, 255.f));
			}
			else
			{	
				mB=Maths::HMat43(u, boxB.centre);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, mB, RGBA(valX, valY, valZ, 255.f));
			}
			
		}
		
		for(U32 i=0; i<numOfObjects; ++i)
		{
			sap.RemoveObjects();
		}
	}*/
};
