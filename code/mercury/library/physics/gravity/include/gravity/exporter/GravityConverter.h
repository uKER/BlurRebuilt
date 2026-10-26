#ifndef GRAVITYCONVERTER_H
#define GRAVITYCONVERTER_H

#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/exporter/ExportPrimitivesList.h"


namespace Gravity
{
	class C_ShapeNode;
	class C_CompoundNode;
	class C_InstanceNode;
	class C_ConstraintNode;
	class C_MeshNode;
	class C_ConverterTriangle;

	class C_GravityConverter
	{
	private:
		C_GravityConverter()
		{
		}

	public:
		
		
		void BuildConstraintNode(Gravity::C_ExportConstraint& buildConstraint, Gravity::C_ConstraintNode* pConstraint);

		
		void BuildShapeNode(Gravity::C_PhysicsExporter& buildPrim, Gravity::C_ShapeNode* pShapeNode);

	
		Gravity::C_CompositeDesc* BuildCompoundNode(Gravity::C_CompoundNode* pCompoundNode, Maths::HMat33& inertiaTensor, SharedPhysics::SortedArray<Gravity::C_MeshNode*>& meshList);

		
		void BuildCompoundObject( Maths::HMat43& parentWorldMatrix, Gravity::C_ShapeNode* pShapeNode, Maths::HMat33& ineriaTensor, Gravity::C_CompositeDesc& cObj);


		//void ConstructAABBTree(Baking::AABBTree& tree, Gravity::C_ConverterTriangle* pTri);
	
		static C_GravityConverter* Instance()
		{
			static C_GravityConverter m_sConverter;
			return &m_sConverter;
		}

		SharedPhysics::SortedArray<S32> m_Instances;
		
	};
};
#endif
