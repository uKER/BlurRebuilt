
#include "stdafx.h"

#include <aftk/Aftk.h>

#include <system/serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>
#include <converter_shared/ModelData.h>
#include <converter_shared/helpers/AftkHelpers.h>

namespace ModelData
{

enum E_ModelDataVersion
{
	E_ModelDataFileVersion_Initial = 0,
	E_ModelDataFileVersion_BoundsData,

	E_ModelDataFileVersion_Current = E_ModelDataFileVersion_BoundsData
};

S32 GetModelExportIndex( const C_Model* pModel )
{
	if( pModel == NULL )
		return -1;

	return pModel->GetIndex();
}

S32 GetModelElementExportIndex( const C_ModelElement* pModelElement )
{
	if( pModelElement == NULL )
		return -1;
		
	return pModelElement->GetIndex();
}
// C_ModelElement

C_ModelElement::C_ModelElement()
	: m_Name( "Unknown" )
	, m_NameId(-1)
	, m_pRootAftkNode(NULL)
	, m_RenderMeshId(-1)
	, m_PhysicsShapeId(-1)
	, m_Index(-1)
{
}

C_ModelElement::C_ModelElement(	const Util::C_String& name, 
				Maths::HMat43 PCREF matrix, 
				const Aftk::C_SceneNode* pRootAftkNode)
	: m_Name(name)
	, m_Matrix(matrix)
	, m_pRootAftkNode(pRootAftkNode)
	, m_NameId(-1)
	, m_RenderMeshId(-1)
	, m_PhysicsShapeId(-1)
	, m_Index(-1)
{
}

C_ModelElement::~C_ModelElement()
{
}

const Maths::HMat43 C_ModelElement::GetMatrix() const
{
	return m_Matrix;
}

void C_ModelElement::SetMatrix( Maths::HMat43& matrix )
{
	m_Matrix = matrix;
}

const Util::C_String& C_ModelElement::GetName() const
{
	return m_Name;
}

void C_ModelElement::SetName( const Util::C_String& name )
{
	m_Name = name;
}

void C_ModelElement::SetPhysicsShapeId( const U32 id )
{
	m_PhysicsShapeId = id;
}

const C_Model* C_ModelElement::GetModel() const
{
	return (C_Model*)GetHierarchy();
}

const S32 C_ModelElement::GetIndex() const
{
	return m_Index;
}

void C_ModelElement::SetRootAftkNode( Aftk::C_SceneNode* pRootAftkNode )
{
	m_pRootAftkNode = pRootAftkNode;
}

const Aftk::C_SceneNode* C_ModelElement::GetRootAftkNode() const
{
	return m_pRootAftkNode;
}

void C_ModelElement::AddAftkNode( const Aftk::C_SceneNode* pRootAftkNode )
{
	m_AftkNodeList.PushBack( pRootAftkNode );
}

C_AftkNodeList& C_ModelElement::GetAftkNodeList()
{
	return m_AftkNodeList;
}

const C_AftkNodeList& C_ModelElement::GetAftkNodeList() const
{
	return m_AftkNodeList;
}

void C_ModelElement::Dump( const bool dumpAftkNodeList, const Util::C_String& prefix )
{
	C_ModelElement* pElement = this;
	while(pElement)
	{
		Util::C_String printPrefix;
		Util::C_String subnodePrefix;

		const bool isFirstNode = (!pElement->GetPrevSibling());
		const bool isLastNode = (!pElement->GetNextSibling());

		if( isLastNode )
		{
			// Last
			printPrefix = prefix +		"`---";
			subnodePrefix = prefix +	"    ";
		}
		else
		if( isFirstNode )
		{
			// First
			printPrefix = prefix +		"+---";
			subnodePrefix = prefix +	"|   ";
		}
		else
		{
			// Middle
			printPrefix = prefix +		"|---";
			subnodePrefix = prefix +	"|   ";
		}


		LOGDEBUG( Tools::g_log, "%s[Element] '%s'", printPrefix.Chars(), pElement->GetName().Chars());
		if( dumpAftkNodeList )
		{
			for(Btl::Vector<const Aftk::C_SceneNode*>::Iterator i = pElement->m_AftkNodeList.Begin(); i < pElement->m_AftkNodeList.End(); ++i)
			{
				const Aftk::C_SceneNode* pSceneNode = (*i);
				LOGDEBUG( Tools::g_log, "%s:   AftkNode '%s'", subnodePrefix.Chars(), pSceneNode->GetName().c_str());
			}
		}

		if(pElement->GetFirstChild())
		{
			pElement->GetFirstChild()->Dump( dumpAftkNodeList, subnodePrefix );
			LOGDEBUG( Tools::g_log, "%s", subnodePrefix.Chars());
		}

		pElement = pElement->GetNextSibling();
	}
}

// C_Model

C_Model::C_Model()
	: m_Name( "Unknown" )
	, m_NameId(-1)
	, m_pRootAftkNode(NULL)
	, m_Index(-1)
{
}

C_Model::C_Model(	const Util::C_String& name, 
			Maths::HMat43 PCREF matrix, 
			const Aftk::C_SceneNode* pRootAftkNode)
	: m_Name( name )
	, m_Matrix( matrix )
	, m_NameId(-1)
	, m_pRootAftkNode(pRootAftkNode)
	, m_Index(-1)
	, m_FirstElementIndex(-1)
{
}

C_Model::~C_Model()
{
}

const Maths::HMat43 C_Model::GetMatrix() const
{
	return m_Matrix;
}

void C_Model::SetMatrix( Maths::HMat43& matrix )
{
	m_Matrix = matrix;
}

const Util::C_String& C_Model::GetName() const
{
	return m_Name;
}

void C_Model::SetName( const Util::C_String& name )
{
	m_Name = name;
}

void C_Model::SetRootAftkNode( Aftk::C_SceneNode* pRootAftkNode )
{
	m_pRootAftkNode = pRootAftkNode;
}

const S32 C_Model::GetIndex() const
{
	return m_Index;
}

const C_ModelData* C_Model::GetModelData() const
{
	return (C_ModelData*)GetHierarchy();
}

void C_Model::Dump( const bool dumpAftkNodeList, const Util::C_String& prefix )
{
	C_Model* pModel = this;
	while(pModel)
	{
		Util::C_String printPrefix;
		Util::C_String subnodePrefix;

		const bool isFirstNode = (!pModel->GetPrevSibling());
		const bool isLastNode = (!pModel->GetNextSibling());

		if( isLastNode )
		{
			// Last
			printPrefix = prefix +		"`---";
			subnodePrefix = prefix +	"    ";
		}
		else
		if( isFirstNode )
		{
			// First
			printPrefix = prefix +		"+---";
			subnodePrefix = prefix +	"|   ";
		}
		else
		{
			// Middle
			printPrefix = prefix +		"|---";
			subnodePrefix = prefix +	"|   ";
		}


		LOGDEBUG( Tools::g_log, "%s<Model> '%s'", printPrefix.Chars(), pModel->GetName().Chars());

		C_ModelElement* pRootElement = pModel->GetRootNode();
		if( pRootElement )
		{
			pRootElement->Dump( dumpAftkNodeList, subnodePrefix + "{  " );
		}
		//if( dumpAftkNodeList )
		//{
		//	for(Btl::Vector<const Aftk::C_SceneNode*>::Iterator i = pNode->m_AftkNodeList.Begin(); i < pNode->m_AftkNodeList.End(); ++i)
		//	{
		//		const Aftk::C_SceneNode* pSceneNode = (*i);
		//		LOGDEBUG( Tools::g_log, "%s:   AftkNode '%s'", subnodePrefix.Chars(), pSceneNode->GetName().c_str());
		//	}
		//}

		if(pModel->GetFirstChild())
		{
			pModel->GetFirstChild()->Dump( dumpAftkNodeList, subnodePrefix );
			LOGDEBUG( Tools::g_log, "%s", subnodePrefix.Chars());
		}

		pModel = pModel->GetNextSibling();
	}
}

// C_Constraint

C_Constraint::C_Constraint()
	: m_Name("Unnamed")
	, m_NameId( -1 )
	, m_pAftkSceneNode( NULL )
	, m_PhysicsConstraintId( -1 )
{
}

C_Constraint::C_Constraint(const Util::C_String& name, const Aftk::C_SceneNode* pAftkSceneNode)
	: m_Name( name )
	, m_NameId( -1 )
	, m_pAftkSceneNode( pAftkSceneNode )
	, m_PhysicsConstraintId( -1 )
{
}

C_Constraint::~C_Constraint()
{
}

const Util::C_String& C_Constraint::GetName() const
{
	return m_Name;
}

// C_ModelData

C_ModelData::C_ModelData()
{
}

C_ModelData::~C_ModelData()
{
}

bool C_ModelData::Export( Sys::I_File* pFile ) const
{

	// Build String Table & enumerate elements and models
	Util::C_StringTableBuilder stringTableBuilder;

	S32 modelCount = 0;
	S32 elementCount = 0;

	for( C_ModelConstIterator iModel( this ); iModel; ++iModel)
	{
		const C_Model* pModel = iModel;

		pModel->m_Index = modelCount;
		if(pModel->GetNodeCount() > 0)
		{
			pModel->m_FirstElementIndex = elementCount;
		}
		else
		{
			pModel->m_FirstElementIndex = -1;
		}
		pModel->m_NameId = stringTableBuilder.AddString( pModel->GetName() );

		S32 modelElementCount = 0;
		for( C_ModelElementConstIterator iModelElement( pModel ); iModelElement; ++iModelElement)
		{
			const C_ModelElement* pModelElement = iModelElement;

			pModelElement->m_Index = modelElementCount;
			pModelElement->m_NameId = stringTableBuilder.AddString( pModelElement->GetName() );

			modelElementCount++;
			elementCount++;
		}

		assert( modelElementCount == pModel->GetNodeCount() );

		modelCount++;
	}

	assert( modelCount == GetNodeCount() );

	const S32 constraintCount = GetConstraintCount();
	for(S32 constraintNo=0; constraintNo<constraintCount; constraintNo++)
	{
		const C_Constraint& constraint = GetConstraint( constraintNo );
		constraint.m_NameId = stringTableBuilder.AddString( constraint.GetName() );
	}

	// Header Block
	{
		Sys::C_FileBlock headerBlock( pFile, "Header" );
		Sys::BinarySerialiserAutoEndian ar( headerBlock );

		ar << (S32)E_ModelDataFileVersion_Current;

		ar << modelCount;
		ar << elementCount;
		ar << constraintCount;
		
		ar << m_Bounds; // E_ModelDataFileVersion_BoundsData
	}
	
	// String table block
	{
		Sys::C_FileBlock stringTableBlock( pFile, "StrTab" );
		Sys::BinarySerialiserAutoEndian ar( stringTableBlock );

		Util::C_StringTable stringTable( stringTableBuilder );

		ar << stringTable;
	}

	{
		Sys::C_FileBlock modelsBlock( pFile, "Models" );
		Sys::BinarySerialiserAutoEndian ar( modelsBlock );

		for( C_ModelConstIterator iModel( this ); iModel; ++iModel)
		{
			const C_Model* pModel = iModel;

			ar << pModel->GetMatrix();
			ar << pModel->m_LocalBounds; // E_ModelDataFileVersion_BoundsData
			ar << pModel->m_NameId;
			ar << pModel->GetIndex();
			ar << (S32)pModel->GetNodeCount();
			ar << pModel->m_FirstElementIndex;
			ar << GetModelExportIndex( pModel->GetParent() );
			ar << GetModelExportIndex( pModel->GetFirstChild() );
			ar << GetModelExportIndex( pModel->GetNextSibling() );
		}
	}

	// Elements Block
	{
		Sys::C_FileBlock elementsBlock( pFile, "Element" );
		Sys::BinarySerialiserAutoEndian ar( elementsBlock );

		for( C_ModelConstIterator iModel( this ); iModel; ++iModel)
		{
			const C_Model* pModel = iModel;

			for( C_ModelElementConstIterator iModelElement( pModel ); iModelElement; ++iModelElement)
			{
				const C_ModelElement* pModelElement = iModelElement;

				ar << ((C_Model*)pModelElement->GetHierarchy())->GetIndex();
				ar << pModelElement->GetMatrix();
				ar << pModelElement->m_LocalBounds; // E_ModelDataFileVersion_BoundsData
				ar << pModelElement->m_NameId;
				ar << pModelElement->GetIndex();
				ar << GetModelElementExportIndex( pModelElement->GetParent() );
				ar << GetModelElementExportIndex( pModelElement->GetFirstChild() );
				ar << GetModelElementExportIndex( pModelElement->GetNextSibling() );
				ar << pModelElement->m_RenderMeshId;
				ar << pModelElement->m_PhysicsShapeId;
			}
		}
	}

	// Constraints Block
	{
		Sys::C_FileBlock constraintsBlock( pFile, "Constr" );
		Sys::BinarySerialiserAutoEndian ar( constraintsBlock );

		for(S32 constraintNo=0; constraintNo<constraintCount; constraintNo++)
		{
			const C_Constraint& constraint = GetConstraint( constraintNo );
			ar << constraint.m_NameId;
			ar << constraint.m_PhysicsConstraintId;
		}
	}

	return true;
}

void C_ModelData::AddConstraint( const Aftk::C_SceneNode* pAftkSceneNode )
{
	m_ConstraintList.PushBack( C_Constraint( pAftkSceneNode->GetName().c_str(), pAftkSceneNode ) );
}

const S32 C_ModelData::GetConstraintCount() const
{
	return m_ConstraintList.Size();
}

const C_Constraint& C_ModelData::GetConstraint( const S32 index ) const
{
	return m_ConstraintList[index];
}

C_Constraint& C_ModelData::GetConstraint( const S32 index )
{
	return m_ConstraintList[index];
}

void UpdateBounds(const Aftk::C_SceneNode* pSceneNode, const Maths::HMat43 PCREF mInvBounds, Maths::C_AxisBox& bounds)
{
	const Aftk::C_SceneNode* pSourceNode = AftkHelpers::InstanceGetSourceNode(pSceneNode);

	// only interested in mesh nodes
	if (!pSourceNode->HasType(Aftk::E_ObjType_SceneMesh))
		return;

	Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pSourceNode;

	Aftk::Matrix44 mWorldMatrix=pSceneNode->GetWorldMatrix();

	const U32 numLodLevels = pMesh->GetNumLodLevels();
	for( U32 lodLevel=0; lodLevel<numLodLevels; lodLevel++ )
	{
		const U32 numTriangles(pMesh->GetNumTriangles(lodLevel));

		for(U32 a = 0; a < numTriangles; ++a)
		{
			// TODO: check polygon is visible or has collision
			const Aftk::C_Polygon& poly(pMesh->GetPolygon(lodLevel, a));
			const Aftk::C_PolygonVertex& pPA = poly.GetPolygonVertex(0);
			const Aftk::C_PolygonVertex& pPB = poly.GetPolygonVertex(1);
			const Aftk::C_PolygonVertex& pPC = poly.GetPolygonVertex(2);

			Aftk::Position3 vPosA = pMesh->GetVertexPosition(pPA.GetPositionIndex());
			Aftk::Position3 vPosB = pMesh->GetVertexPosition(pPB.GetPositionIndex());
			Aftk::Position3 vPosC = pMesh->GetVertexPosition(pPC.GetPositionIndex());
			vPosA = Aftk::ApplyMatrix(mWorldMatrix, vPosA);
			vPosB = Aftk::ApplyMatrix(mWorldMatrix, vPosB);
			vPosC = Aftk::ApplyMatrix(mWorldMatrix, vPosC);

			Maths::HVec3 posA(vPosA.X(), vPosA.Y(), vPosA.Z());
			Maths::HVec3 posB(vPosB.X(), vPosB.Y(), vPosB.Z());
			Maths::HVec3 posC(vPosC.X(), vPosC.Y(), vPosC.Z());
			
			// transform from worl to bounds local
			bounds.UpdateExtents( posA * mInvBounds );
			bounds.UpdateExtents( posB * mInvBounds );
			bounds.UpdateExtents( posC * mInvBounds );
		}
	}
}

void C_ModelData::CalculateBounds()
{
	Maths::HMat43 mIdentity;
	mIdentity.SetIdentity();
	m_Bounds.SetInvalidExtents();

	for( C_ModelIterator iModel( this ); iModel; ++iModel)
	{
		C_Model* pModel = iModel;

		const Maths::HMat43 mModel = pModel->GetMatrix();
		const Maths::HMat43 mInvModel = mModel.InverseRotTrans();

		pModel->m_LocalBounds.SetInvalidExtents();

		for( C_ModelElementIterator iModelElement( pModel ); iModelElement; ++iModelElement)
		{
			C_ModelElement* pModelElement = iModelElement;

			Maths::HMat43 mElement = pModelElement->GetMatrix();
			const Maths::HMat43 mInvElement = mElement.InverseRotTrans();

			pModelElement->m_LocalBounds.SetInvalidExtents();

			for(Btl::Vector<const Aftk::C_SceneNode*>::Iterator i = pModelElement->m_AftkNodeList.Begin(); i < pModelElement->m_AftkNodeList.End(); ++i)
			{
				const Aftk::C_SceneNode* pSceneNode = (*i);

				UpdateBounds(pSceneNode, mInvModel, pModel->m_LocalBounds);
				UpdateBounds(pSceneNode, mInvElement, pModelElement->m_LocalBounds);
				UpdateBounds(pSceneNode, mIdentity, m_Bounds);				
			}
		}
	}
}

void C_ModelData::Dump( const bool dumpAftkNodeList )
{
	LOGDEBUG( Tools::g_log, "<pre><tt>" );

	LOGDEBUG( Tools::g_log, "" );
	LOGDEBUG( Tools::g_log, "C_ModelData" );
	LOGDEBUG( Tools::g_log, "" );


	if(GetRootNode())
	{
		LOGDEBUG( Tools::g_log, "|" );
		GetRootNode()->Dump( dumpAftkNodeList );
	}

	LOGDEBUG( Tools::g_log, "</tt></pre>" );
}

void DumpAfktNodeList( const C_AftkNodeList& aftkNodeList, const Util::C_String& prefix )
{
	for( C_AftkNodeList::ConstIterator i = aftkNodeList.Begin(); i != aftkNodeList.End(); i++ )
	{
		const Aftk::C_SceneNode* pAftkSceneNode = *i;
		Util::C_String nodeName;
		if( pAftkSceneNode )
		{
			nodeName = pAftkSceneNode->GetName().c_str();
		}
		else
		{
			nodeName = "<NULL>";
		}

		LOGDEBUG( Tools::g_log, "%s%s", prefix.Chars(), nodeName.Chars() );
	}
}

} // namespace ModelData
