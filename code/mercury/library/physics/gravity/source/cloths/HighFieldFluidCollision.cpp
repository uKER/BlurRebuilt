#include "Stdafx.h"
#include "gravity/cloths/HighFieldFluidCollision.h"
//#include "gravity/narrowphase/Shape.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/shape.h"
#include "gravity/sdk/Body.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/sharedphysics/RigidBody.inl"
#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "system/Debug/I_DebugRender.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/Primitive.inl"
#include "gravity/narrowphase/AxisBox.inl"


namespace Collision
{

	//bool C_HFFCollisionManager::Collided(C_Fluid* fluid, const C_CollisionObject* pObject, Maths::HF32& averageHeight)
	//{
	//	using namespace Maths;

	//	fluid->ClearFlag();

	//	bool intersect = false;
	//	
	//	if(fluid->m_Box.Intersects(pObject->m_TransformedBox))
	//	{
	//		SharedPhysics::C_RigidBody* pRigidBody= pObject->m_pNodePtr->m_pRigidBody;
	//		const HF32 recipFour(0.25f);

	//		const HF32 boxHeight=pObject->m_TransformedBox.extents.Y()*2.f;
	//		const HF32 half=Constants<HF32>::Half();
	//		const HF32 zero=Constants<HF32>::Zero();
	//		const HVec3 zeroV=Constants<HVec3>::Zero();
	//		const HF32 one(Constants<HF32>::One());
	//		const HF32 b(2.f);
	//		
	//	
	//		const HF32 recipWidth=fluid->m_Width.Reciprocal();
	//		const HF32 ext=fluid->m_Width*half;
	//		const HF32 x0=fluid->m_Grid[1][1].m_WorldPosition.X() - ext;
	//		const HF32 z0=fluid->m_Grid[1][1].m_WorldPosition.Z() - ext;
	//		const HF32 y0=fluid->m_Grid[1][1].m_WorldPosition.Y();
	//		const Maths::HVec3 _min=pObject->m_TransformedBox.centre - pObject->m_TransformedBox.extents;
	//		const Maths::HVec3 _max=pObject->m_TransformedBox.centre + pObject->m_TransformedBox.extents;
	//		/*Maths::HMat43 identity;
	//		identity.SetIdentity();
	//		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, identity, Maths::RGBA(255, 0, 0 ));*/

	//		U32 xMinIndex=((_min.X() - x0)* recipWidth) + 1;
	//		U32 xMaxIndex=((_max.X() - x0)* recipWidth) + 1;

	//		U32 zMinIndex=((_min.Z() - z0)*recipWidth) + 1;
	//		U32 zMaxIndex=((_max.Z() - z0)*recipWidth) + 1;

	//		if(xMaxIndex > fluid->m_Dimension) 
	//			xMaxIndex=fluid->m_Dimension;
	//		if(xMinIndex < 0) 
	//			xMinIndex = 1;

	//		if(zMaxIndex > fluid->m_Dimension) 
	//			zMaxIndex=fluid->m_Dimension;
	//		if(zMinIndex < 0) 
	//			zMinIndex = 1;

	//		Maths::HVec3 force=zeroV;
	//		Maths::HVec3 torque=zeroV;

	//		HVec3 contact;
	//		HF32 difHeight;
	//		HF32 ratio;
	//		//Maths::HVec3 applyPoint;
	//		//Maths::HF32 scale;
	//		const HVec3 extents(ext, (fluid->m_Height) * half, ext);

	//		
	//		U32 count=0;
 //			for(U32 i = zMinIndex; i<=zMaxIndex; ++i)
	//		{
	//			for(U32 j=xMinIndex; j<=xMaxIndex; ++j)
	//			{

	//				C_GridCell& cell=fluid->m_Grid[i][j];
	//				
	//			/*	const HF32 height = cell.m_CurHeight*half;
	//				const Maths::HVec3 centre(cell.m_WorldPosition.X(), cell.m_WorldPosition.Y() + height, cell.m_WorldPosition.Z());
	//				const HVec3 extents(ext, height, ext);*/

	//				const HVec3 centre(cell.m_WorldPosition.X(), cell.m_WorldPosition.Y() + extents.Y(), cell.m_WorldPosition.Z());
	//				
	//				const HVec3 _minCell(centre - extents);
	//				const HVec3 _maxCell(centre + extents);

	//				C_Shape* pShape=pObject->m_pNodePtr->m_pBody->GetShape();

	//				bool immersed = false; 

	//				Collision::C_CRAxisBox aabb;
	//				pShape->TransformBox(aabb);

	//				const HVec3 _minAABB=aabb.centre - aabb.extents;
	//				const HVec3 _maxAABB=aabb.centre + aabb.extents;

	//				

	//				HMat43 mat;
	//				mat.SetIdentity();

	//				//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_minAABB, _maxAABB, mat, RGBA(255, 0, 0, 255));

	//				//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_minCell, _maxCell, mat, RGBA(0, 255,0 ,255));

	//				bool collided= !(_maxCell.Any_LT(_minAABB) | _minCell.Any_GT(_maxAABB));
	//				if(collided)
	//				{
	//					
	//					intersect = true;
	//					//cell.m_Hit = true;

	//					difHeight =_maxCell.Y() - _minAABB.Y();
	//					const HF32 dif = difHeight*0.25f;

	//					const HF32 totalHeight=_maxAABB.Y() - _minAABB.Y();
	//					if(totalHeight.GE(difHeight))
	//					{
	//						
	//						const HF32 energy=pRigidBody->m_KineticEnergy|pRigidBody->m_KineticEnergy;
	//						if(energy.GT(0.05f))
	//						{
	//							fluid->m_Grid[i-1][j].m_CurHeight +=dif;
	//							fluid->m_Grid[i+1][j].m_CurHeight +=dif;
	//							fluid->m_Grid[i][j-1].m_CurHeight +=dif;
	//							fluid->m_Grid[i][j+1].m_CurHeight +=dif;
	//							
	//						}
	//					}

	//					averageHeight +=cell.m_CurHeight;
	//					count++;

	//					/*if(i == zMaxIndex)
	//					{
	//						fluid->m_Grid[i+1][j].m_CurHeight +=dif;
	//					}

	//					if(i == zMinIndex)
	//					{
	//						fluid->m_Grid[i-1][j].m_CurHeight +=dif;
	//					}

	//					if(j == xMaxIndex)
	//					{
	//						fluid->m_Grid[i][j+1].m_CurHeight +=dif;
	//					}

	//					if(j == xMinIndex)
	//					{
	//						fluid->m_Grid[i][j-1].m_CurHeight +=dif;
	//					}*/
	//					
	//					

	//					

	//				}


	//				

	//				//if(Collided(_minCell, _maxCell, pShape, difHeight, immersed))
	//				//{
	//				////	cell.m_Hit=true;

	//				//	intersect = true;
	//				//	const HF32 dif = difHeight*0.25f;
	//				//	
	//				//	if(immersed)
	//				//		cell.m_CurHeight=cell.GetNeighbourAverageHeight()*recipFour;
	//				//	/*else
	//				//		cell.m_CurHeight -=difHeight;*/
	//				//	
	//				//	const HF32 energy=pRigidBody->m_KineticEnergy|pRigidBody->m_KineticEnergy;
	//				//	if(energy.GT(0.05f))
	//				//	{
	//				//		fluid->m_Grid[i-1][j].m_CurHeight +=dif;
	//				//		fluid->m_Grid[i+1][j].m_CurHeight +=dif;
	//				//		fluid->m_Grid[i][j-1].m_CurHeight +=dif;
	//				//		fluid->m_Grid[i][j+1].m_CurHeight +=dif;
	//				//	}
	//				//

	//				//	/*if(i == zMaxIndex)
	//				//	{
	//				//		fluid->m_Grid[i+1][j].m_CurHeight +=dif;
	//				//	}

	//				//	if(i == zMinIndex)
	//				//	{
	//				//		fluid->m_Grid[i-1][j].m_CurHeight +=dif;
	//				//	}

	//				//	if(j == xMaxIndex)
	//				//	{
	//				//		fluid->m_Grid[i][j+1].m_CurHeight +=dif;
	//				//	}

	//				//	if(j == xMinIndex)
	//				//	{
	//				//		fluid->m_Grid[i][j-1].m_CurHeight +=dif;
	//				//	}*/
	//				//}
	//			}
	//		}
	//		averageHeight =averageHeight/count;

	//	}
	//	return intersect;

	//}


bool C_HFFCollisionManager::Collided(C_Fluid* fluid, const C_CollisionObject* pObject,  Maths::HF32& averageHeight)
	{
		using namespace Maths;

		fluid->ClearFlag();

		bool intersect = false;
		
		if(fluid->m_Box.Intersects(pObject->m_TransformedBox))
		{
			SharedPhysics::C_RigidBody* pRigidBody= pObject->m_pNodePtr->m_pRigidBody;
			const HF32 recipFour(0.25f);
			const HF32 half=Constants<HF32>::Half();
		
			const HF32 recipWidth=fluid->m_Width.Reciprocal();
			const HF32 ext=fluid->m_Width*half;
			const HF32 x0=fluid->m_Grid[1][1].m_WorldPosition.X() - ext;
			const HF32 z0=fluid->m_Grid[1][1].m_WorldPosition.Z() - ext;
			const HF32 y0=fluid->m_Grid[1][1].m_WorldPosition.Y();
			const Maths::HVec3 _min=pObject->m_TransformedBox.centre - pObject->m_TransformedBox.extents;
			const Maths::HVec3 _max=pObject->m_TransformedBox.centre + pObject->m_TransformedBox.extents;
			/*Maths::HMat43 identity;
			identity.SetIdentity();
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, identity, Maths::RGBA(255, 0, 0 ));*/

			U32 xMinIndex=(U32)((_min.X() - x0)* recipWidth) + 1;
			U32 xMaxIndex=(U32)((_max.X() - x0)* recipWidth) + 1;

			U32 zMinIndex=(U32)((_min.Z() - z0)*recipWidth) + 1;
			U32 zMaxIndex=(U32)((_max.Z() - z0)*recipWidth) + 1;

			if(xMaxIndex > fluid->m_Dimension) 
				xMaxIndex=fluid->m_Dimension;
			if(xMinIndex < 0) 
				xMinIndex = 1;

			if(zMaxIndex > fluid->m_Dimension) 
				zMaxIndex=fluid->m_Dimension;
			if(zMinIndex < 0) 
				zMinIndex = 1;

			HF32 difHeight(Maths::Constants<Maths::HF32>::Zero());

			U32 count=0;
 			for(U32 i = zMinIndex; i<=zMaxIndex; ++i)
			{
				for(U32 j=xMinIndex; j<=xMaxIndex; ++j)
				{

					C_GridCell& cell=fluid->m_Grid[i][j];

					//calculate the center of the grid in 3d
					const HF32 h = fluid->m_Position.Y() - cell.m_WorldPosition.Y();
					const HF32 halfHeight = (fluid->m_Height - h)*half;
					const HVec3 center(cell.m_WorldPosition.X(), cell.m_WorldPosition.Y() + halfHeight, cell.m_WorldPosition.Z());
					const HVec3 extents(ext, halfHeight, ext);
					const HVec3 _minCell(center - extents);
					const HVec3 _maxCell(center + extents);
				

					C_Shape* pShape=pObject->m_pNodePtr->m_pBody->GetShape();

				//	bool immersed = false; 

					Collision::C_CRAxisBox aabb;
					pShape->TransformBox(aabb);

					const HVec3 _minAABB=aabb.centre - aabb.extents;
					const HVec3 _maxAABB=aabb.centre + aabb.extents;

					HMat43 mat;
					mat.SetIdentity();

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_minAABB, _maxAABB, mat, RGBA(255, 0, 0, 255));

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_minCell, _maxCell, mat, RGBA(0, 255,0 ,255));

					bool collided= !(_maxCell.Any_LT(_minAABB) | _minCell.Any_GT(_maxAABB));
					if(collided)
					{
						
						intersect = true;
						cell.m_Hit = true;

						difHeight =_maxCell.Y() - _minAABB.Y();
						const HF32 dif = difHeight*recipFour;

						const HF32 totalHeight=_maxAABB.Y() - _minAABB.Y();
						if(totalHeight.GE(difHeight))
						{
							
							const HF32 energy=pRigidBody->m_KineticEnergy|pRigidBody->m_KineticEnergy;
							if(energy.GT(0.05f))
							{
								fluid->m_Grid[i-1][j].m_CurHeight +=dif;
								fluid->m_Grid[i+1][j].m_CurHeight +=dif;
								fluid->m_Grid[i][j-1].m_CurHeight +=dif;
								fluid->m_Grid[i][j+1].m_CurHeight +=dif;
								
							}
						}

						averageHeight +=cell.m_CurHeight;
						count++;

						/*if(i == zMaxIndex)
						{
							fluid->m_Grid[i+1][j].m_CurHeight +=dif;
						}

						if(i == zMinIndex)
						{
							fluid->m_Grid[i-1][j].m_CurHeight +=dif;
						}

						if(j == xMaxIndex)
						{
							fluid->m_Grid[i][j+1].m_CurHeight +=dif;
						}

						if(j == xMinIndex)
						{
							fluid->m_Grid[i][j-1].m_CurHeight +=dif;
						}*/
					}
				}
			}
			averageHeight =averageHeight/count;
		

		}

		/*if(intersect)
		{
			const HF32 zero=Constants<HF32>::Zero();
			C_Normal n(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			C_Normal m(Maths::Constants<Maths::HVec3>::UnitAxisNegativeY(), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			C_Shape* pShape= pObject->m_pNodePtr->m_pBody->GetShape();
			const HVec3 supportPoint = pShape->Support(m);

			const HF32 d = n|(supportPoint - averageCenterPoint);
			if(d.GT(zero))
				return intersect;

			Collision::C_CollisionResult<1, 1> result;
			result.m_pObjectA = const_cast<C_CollisionObject*>(pObject);
			result.m_pObjectB = const_cast<C_CollisionObject*>(pFluidObject);
			result.m_NumContactPatches = 1;
			Collision::C_ContactPatch<1>& patch = result.m_ContactPatches[0];
			patch.m_Normal = Maths::C_Normal(Maths::Constants<Maths::HVec3>::UnitAxisNegativeY(), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			patch.m_NumContactPoints = 1;
			Collision::C_ContactPoint& point = patch.m_ContactPoints[0];
			point.m_ContactPoint = supportPoint;
			point.m_PenetrationDepth = Abs(d);
			point.m_HashValue = 0;
			point.m_MaterialIndices = 0;

			Collision::ReduceContactsAndPackToStream<1, 1, 1, 1>(result, *stream);
		}*/
		return intersect;

	}



}
