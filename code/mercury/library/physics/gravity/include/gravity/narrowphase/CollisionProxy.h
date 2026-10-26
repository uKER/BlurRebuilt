#ifndef COLLISIONPROXY_H
#define COLLISIONPROXY_H

#include "gravity/narrowphase/primitive.h"
#include "gravity/narrowphase/CharacterControllerPrimitive.h"

namespace Collision
{

	class C_CollisionProxy
	{
	public:
		Maths::HMat43 m_PosRot;

		C_CollisionProxy(const Maths::HMat43 PCREF posRot) : m_PosRot(posRot)
		{
		}
	};

	class C_BoxProxy : public C_CollisionProxy
	{
	public:
		C_OBB m_Box;


		C_BoxProxy(const C_OBB& origBox, Maths::HMat43 PCREF transformation) : C_CollisionProxy(transformation)
		{
			const Maths::HMat33 orientation(transformation.Rotation());
			const Maths::HVec3 pos(transformation.Translation());
			m_Box.m_Extent = origBox.m_Extent;
			m_Box.m_Centre = orientation * (origBox.m_Centre - pos);
			m_Box.m_Orientation = origBox.m_Orientation * orientation.InverseRotation();
		}
	};

	class C_SphereProxy : public C_CollisionProxy
	{
	public:
		C_Sphere m_Sphere;
		C_SphereProxy(C_Sphere& origSphere, Maths::HMat43 PCREF transformation) : C_CollisionProxy(transformation)
		{
			const Maths::HMat33 orientation(transformation.Rotation());
			const Maths::HVec3 pos(transformation.Translation());

			m_Sphere.Radius(origSphere.Radius());
			const Maths::HVec3 centre(orientation * (origSphere.Centre() - pos));
			m_Sphere.Centre(centre);
			m_Sphere.WarmStart(centre);
		}
	};

	class C_CapsuleProxy : public C_CollisionProxy
	{
	public:
		C_Capsule m_Capsule;
		C_CapsuleProxy(C_Capsule& origCapsule, Maths::HMat43 PCREF transformation) : C_CollisionProxy(transformation)
		{
			const Maths::HMat33 orientation(transformation.Rotation());
			const Maths::HVec3 pos(transformation.Translation());

			m_Capsule.m_LocalStart =  origCapsule.m_LocalStart;
			m_Capsule.m_LocalEnd =  origCapsule.m_LocalEnd;
			m_Capsule.m_Radius =  origCapsule.m_Radius;
			m_Capsule.m_Start = orientation * (origCapsule.m_Start - pos);
			m_Capsule.m_End = orientation * (origCapsule.m_End - pos);
			m_Capsule.Margin(origCapsule.Margin());
			m_Capsule.Epsilon(origCapsule.Epsilon());
			m_Capsule.m_wStart = origCapsule.m_wStart;
		}
	};

	class C_CylinderProxy : public C_CollisionProxy
	{
	public:
		C_Cylinder m_Cylinder;

		C_CylinderProxy(C_Cylinder& origCylinder, Maths::HMat43 PCREF transformation) : C_CollisionProxy(transformation)
		{
			const Maths::HMat33 orientation(transformation.Rotation());
			const Maths::HVec3 pos(transformation.Translation());
			m_Cylinder.m_Centre = orientation * (origCylinder.m_Centre - pos);
			m_Cylinder.m_CentreAxis = orientation * origCylinder.m_CentreAxis;
			m_Cylinder.m_halfHeight = origCylinder.m_halfHeight;
			m_Cylinder.m_Radius = origCylinder.m_Radius;
			m_Cylinder.Epsilon(origCylinder.Epsilon());
			m_Cylinder.Margin(origCylinder.Margin());
			m_Cylinder.m_wStart = origCylinder.m_wStart;
		}
	};

	class C_ConeProxy : public C_CollisionProxy
	{
	public:
		C_Cone m_Cone;

		C_ConeProxy(C_Cone& origCone, Maths::HMat43 PCREF transformation) : C_CollisionProxy(transformation)
		{
			const Maths::HMat33 orientation(transformation.Rotation());
			const Maths::HVec3 pos(transformation.Translation());
			m_Cone.m_CenterOfMass = orientation * (origCone.m_CenterOfMass - pos);
			m_Cone.m_OffSet = origCone.m_OffSet;
			m_Cone.m_Centre = orientation * (origCone.m_Centre - pos);
			m_Cone.m_MidPointAxis = orientation * origCone.m_MidPointAxis;
			m_Cone.m_Height = origCone.m_Height;
			m_Cone.m_BaseRadius = origCone.m_BaseRadius;
			m_Cone.m_SinTheta = origCone.m_SinTheta;
			m_Cone.Epsilon(origCone.Epsilon());
			m_Cone.Margin(origCone.Margin());
			m_Cone.m_wStart = origCone.m_wStart;
			
		}
	};

	class C_ConvexHullProxy : public C_CollisionProxy
	{
	public:
		//TODO - turn this into a C_SpuConvexHull!
		C_LocalSpaceSpuConvexHull m_Hull;

		C_ConvexHullProxy(C_ConvexHull& hull, Maths::HMat43 PCREF transformation) : C_CollisionProxy(transformation), m_Hull(hull, true)
		{
			//const Maths::HMat43 transA(hull.m_Orientation, hull.m_Position);
			const Maths::HMat43 transA(hull.m_Orientation, hull.m_Centre);
			const Maths::HMat43 absTrans(transA * transformation.InverseRotTrans());

			m_Hull.Centre(absTrans.Translation());
			m_Hull.Orientation(absTrans.Rotation());
			m_Hull.WarmStart(absTrans.Translation());
			/*m_Hull.m_pVertices = hull.m_pVertices;
			m_Hull.m_pTriangles = hull.m_pTriangles;
			m_Hull.m_NumTriangles = hull.m_NumTriangles;
			m_Hull.m_NumVertices = hull.m_NumVertices;
			m_Hull.m_Orientation = absTrans.Rotation();
			m_Hull.m_Position = absTrans.Translation();
			m_Hull.Epsilon(hull.Epsilon());
			m_Hull.Margin(hull.Margin());
			m_Hull.wStart = hull.wStart;
			m_Hull.m_HillClimbingNodes = hull.m_HillClimbingNodes;
			m_Hull.m_pNeighbours = hull.m_pNeighbours;
			m_Hull.m_NumNeighbours = hull.m_NumNeighbours;*/
		}

		~C_ConvexHullProxy()
		{
			/*m_Hull.m_pVertices = NULL;
			m_Hull.m_pTriangles = NULL;
			m_Hull.m_HillClimbingNodes = NULL;
			m_Hull.m_pNeighbours = NULL;*/
		}
	};

	class C_CharacterControllerProxy : public C_CollisionProxy
	{
	public:
		Collision::C_Capsule m_Capsule;
		Collision::C_Ray m_Ray;
		Maths::HVec3 m_Position;
		Maths::HF32 m_RayLength;
		Maths::HF32 m_CosMaxAngle;
		Maths::HF32 m_MinPenetration;
		U16 m_FeetMaterial;

		C_CharacterControllerProxy(Collision::C_CharacterControllerPrimitive& cController, Maths::HMat43 PCREF transformation) : C_CollisionProxy(transformation),
			m_Position(Maths::UNDEFINED), m_RayLength(Maths::UNDEFINED), m_CosMaxAngle(Maths::UNDEFINED), m_MinPenetration(Maths::UNDEFINED)
		{
			using namespace Maths;
			const HMat33 rot(transformation.Rotation());
			m_Capsule = cController.m_Capsule;
			HMat43 invTrans(transformation.InverseRotTrans());
			/*m_Capsule.m_Start = cController.m_Capsule.m_Start * invTrans;
			m_Capsule.m_End = cController.m_Capsule.m_End * invTrans;*/

			const HVec3 start(cController.m_Capsule.Start() * invTrans);
			const HVec3 end(cController.m_Capsule.End() * invTrans);
			const HVec3 centre((start + end)*Constants<HF32>::Half());

			m_Capsule.Start(start);
			m_Capsule.End(end);
			m_Capsule.Centre(centre);
			m_Capsule.WarmStart(centre);

			m_Ray=cController.m_Ray;
			m_Ray.InitialiseRay(cController.m_Ray.m_start * invTrans, 
								Maths::C_Normal(rot * cController.m_Ray.m_Direction, Maths::C_Normal::UNIT_LENGTH_VECTOR),
								cController.m_Ray.m_maxDistance,
								&cController.m_Ray.m_Filter, cController.m_Ray.m_Radius);
			/*m_Ray.m_start = cController.m_Ray.m_start * invTrans;
			m_Ray.m_end = cController.m_Ray.m_end * invTrans;*/
			m_Ray.m_result.m_distance = cController.m_Ray.m_result.m_distance;
			m_Ray.m_result.m_bNewContact = false;
			m_Ray.m_result.m_contact = cController.m_Ray.m_result.m_contact;
			m_Ray.m_result.m_normal = cController.m_Ray.m_result.m_normal; 
			m_RayLength=cController.m_RayLength;
			m_Position=cController.m_Position * invTrans;
			m_CosMaxAngle=cController.m_CosMaxAngle;
			m_MinPenetration = cController.m_MinPenetration;
			m_FeetMaterial = cController.m_FeetMaterial;
		}
		
	};

};

#endif
