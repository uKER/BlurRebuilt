#include "GameResourcePch.h"

#include <system/serialise.h>
#include <system/serialise/C_FileBlock.h>
#include <game_resource/GameResource.h>
#include <game_resource/ModelData.h>
#include <system/debug/metadebug/MetaDebug.h>

#include <gravity/importer/Importer.h>

namespace GameResource
{

enum E_ModelDataVersion
{
	E_ModelDataFileVersion_Initial = 0,
	E_ModelDataFileVersion_BoundsData
};


// C_ModelElement

C_ModelElement::C_ModelElement()
	: m_NameId( -1 )
	, m_Index( -1 )
	, m_pModel( NULL )
	, m_pParent( NULL )
	, m_pFirstChild( NULL )
	, m_pNextSibling( NULL )
	, m_RenderMeshId( -1 )
	, m_PhysicsShapeId( -1 )
{
}

C_ModelElement::~C_ModelElement()
{
}

const Maths::HMat43 C_ModelElement::GetMatrix() const
{
	return m_Matrix;
}

Util::C_String C_ModelElement::GetName() const
{
	return Util::C_String( GetStringTable().GetString( m_NameId ) );
}

C_Model* C_ModelElement::GetModel()
{
	return m_pModel;
}

const C_Model* C_ModelElement::GetModel() const
{
	return m_pModel;
}

const C_ModelElement* C_ModelElement::GetParent() const
{
	return m_pParent;
}

const C_ModelElement* C_ModelElement::GetFirstChild() const
{
	return m_pFirstChild;
}

const C_ModelElement* C_ModelElement::GetNextSibling() const
{
	return m_pNextSibling;
}

const Util::C_StringTable& C_ModelElement::GetStringTable() const
{
	return GetModel()->GetStringTable();
}
	
const Gravity::C_ImportShape* C_ModelElement::GetPhysicsImportShape() const
{
	return GetModel()->GetModelData()->m_pPhysicsImport->GetImportShape( m_PhysicsShapeId );
}


const C_ModelElement* GetFirstSibling( const C_ModelElement* pModelElement )
{
	const C_ModelElement* pParent = pModelElement->GetParent();
	if( pParent )
	{
		return pParent->GetFirstChild();
	}
	else
	{
		return pModelElement->GetModel()->GetRootElement();
	}

	return NULL;
}

void C_ModelElement::Dump( const Util::C_String& prefix ) const
{
	const C_ModelElement* pElement = this;
	while(pElement)
	{
		Util::C_String printPrefix;
		Util::C_String subnodePrefix;

		const bool isFirstNode = ( GetFirstSibling(pElement) == pElement );
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


		LOGDEBUG( g_Log, "%s[Element] '%s' (index:%d)", printPrefix.Chars(), pElement->GetName().Chars(), pElement->GetIndex());
		{
			LOGDEBUG( g_Log, "%s:   RenderMeshId:%d", subnodePrefix.Chars(), m_RenderMeshId);
			LOGDEBUG( g_Log, "%s:   PhysicsShapeId:0x%08x", subnodePrefix.Chars(), m_PhysicsShapeId);
		}

		if(pElement->GetFirstChild())
		{
			pElement->GetFirstChild()->Dump( subnodePrefix );
			LOGDEBUG( g_Log, "%s", subnodePrefix.Chars());
		}

		pElement = pElement->GetNextSibling();
	}
}

const Maths::C_AxisBox C_ModelElement::GetLocalBounds() const
{
	return m_LocalBounds;
}

const Maths::C_Sphere C_ModelElement::GetBoundingSphere() const
{
	return m_Sphere;
}

// C_Model

C_Model::C_Model()
: m_NameId( -1 )
, m_Index( -1 )
, m_ElementCount( 0 )
, m_ElementList( NULL )
, m_pModelData( NULL )
, m_pParent( NULL )
, m_pFirstChild( NULL )
, m_pNextSibling( NULL )
{
}

C_Model::~C_Model()
{
}

const Maths::HMat43 C_Model::GetMatrix() const
{
	return m_Matrix;
}

const char *C_Model::GetName() const
{
	return GetStringTable().GetString( m_NameId );
}

const S32 C_Model::GetIndex() const
{
	return m_Index;
}

const C_Model* C_Model::GetParent() const
{
	return m_pParent;
}

const C_Model* C_Model::GetFirstChild() const
{
	return m_pFirstChild;
}

const C_Model* C_Model::GetNextSibling() const
{
	return m_pNextSibling;
}

C_ModelData* C_Model::GetModelData()
{
	return m_pModelData;
}

const C_ModelData* C_Model::GetModelData() const
{
	return m_pModelData;
}

S32 C_Model::GetElementCount() const
{
	return m_ElementCount;
}

const C_ModelElement* C_Model::GetElement( const S32 index ) const
{
	if( ( index < 0 ) || ( index >= GetElementCount() ) )
		return NULL;

	return &m_ElementList[index];
}

C_ModelElement* C_Model::GetElement( const S32 index )
{
	return const_cast<C_ModelElement*>( const_cast<const C_Model*>(this)->GetElement( index ) );	
}

const C_ModelElement* C_Model::GetElement( const Util::C_String &elementName ) const
{
	const S32 elementCount = GetElementCount();
	for(S32 i=0; i<elementCount; i++)
	{
		const C_ModelElement* pElement = GetElement(i);
		if(_stricmp( pElement->GetName().Chars(), elementName.Chars() ) == 0 )
		{
			return pElement;
		}
	}

	return NULL;
}

const S32 C_Model::GetElementIndex( const Util::C_String &elementName ) const
{
	const S32 elementCount = GetElementCount();
	for(S32 i=0; i<elementCount; i++)
	{
		const C_ModelElement* pElement = GetElement(i);
		if(_stricmp( pElement->GetName().Chars(), elementName.Chars() ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

const C_ModelElement* C_Model::GetRootElement() const
{
	return m_ElementList;
}

const Util::C_StringTable& C_Model::GetStringTable() const
{
	return GetModelData()->GetStringTable();
}

const C_Model* GetFirstSibling( const C_Model* pModel )
{
	const C_Model* pParent = pModel->GetParent();
	if( pParent )
	{
		return pParent->GetFirstChild();
	}
	else
	{
		return pModel->GetModelData()->GetRootModel();
	}

	return NULL;
}

void C_Model::Dump( const Util::C_String& prefix ) const
{
	const C_Model* pModel = this;
	while(pModel)
	{
		Util::C_String printPrefix;
		Util::C_String subnodePrefix;

		const bool isFirstNode = (GetFirstSibling(pModel) == pModel);
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


		LOGDEBUG( g_Log, "%s<Model> '%s' (index:%d)", printPrefix.Chars(), pModel->GetName(), pModel->GetIndex());

		if( pModel->GetRootElement() )
		{
			pModel->GetRootElement()->Dump( subnodePrefix + "{  "  );
		}

		if(pModel->GetFirstChild())
		{
			pModel->GetFirstChild()->Dump( subnodePrefix );
			LOGDEBUG( g_Log, "%s", subnodePrefix.Chars());
		}

		pModel = pModel->GetNextSibling();
	}
}

const Maths::C_AxisBox C_Model::GetLocalBounds() const
{
	return m_LocalBounds;
}

const Maths::C_Sphere C_Model::GetBoundingSphere() const
{
	return m_Sphere;
}

// C_Constraint

C_Constraint::C_Constraint()
{
}

C_Constraint::~C_Constraint()
{
}

const Gravity::C_ConstraintDescriptors* C_Constraint::GetPhysicsConstraint() const
{
	return m_pModelData->m_pPhysicsImport->GetConstraint( m_PhysicsConstraintId );
}

Util::C_String C_Constraint::GetName() const
{
	return Util::C_String( m_pModelData->GetStringTable().GetString( m_NameId ) );
}


// C_ModelData

C_ModelData::C_ModelData()
	: m_pPhysicsImport(NULL)
#ifndef CFG_GOLD
	, m_pRootMetaNode( NULL )
#endif
{
}

C_ModelData::~C_ModelData()
{
#ifndef CFG_GOLD
	SAFE_DELETE_PTR(m_pRootMetaNode);
#endif
}

S32 C_ModelData::GetModelCount() const
{
	return m_ModelList.Size();
}

const C_Model* C_ModelData::GetModel( const S32 index ) const
{
	if( ( index < 0 ) || ( index >= GetModelCount() ) )
		return NULL;

	return &m_ModelList[index];
}

C_Model* C_ModelData::GetModel( const S32 index )
{
	if( ( index < 0 ) || ( index >= GetModelCount() ) )
		return NULL;

	return &m_ModelList[index];
}

const C_Model* C_ModelData::GetModel( const Util::C_String &modelName ) const
{
	const S32 modelCount = GetModelCount();
	for(S32 i=0; i<modelCount; i++)
	{
		const C_Model* pModel = GetModel(i);
		if(_stricmp( pModel->GetName(), modelName.Chars() ) == 0 )
		{
			return pModel;
		}
	}

	return NULL;
}

const C_Model* C_ModelData::GetRootModel() const
{
	if(GetModelCount() <= 0)
		return NULL;

	return &m_ModelList[0];
}

const Util::C_StringTable& C_ModelData::GetStringTable() const
{
	return m_StringTable;
}

const Maths::C_AxisBox C_ModelData::GetBounds() const
{
	return m_Bounds;
}

const S32 C_ModelData::GetConstraintCount() const
{
	return m_ConstraintList.Size();
}

const C_Constraint* C_ModelData::GetConstraint( const S32 index ) const
{
	if( ( index < 0 ) || ( index >= GetConstraintCount() ) )
		return NULL;

	return &m_ConstraintList[index];
}

bool C_ModelData::Read( Sys::I_File* pFile )
{	
	const U32 prevFileVersion = pFile->GetVersionNumber();
	U32 fileVersion = E_ModelDataFileVersion_Initial;

	// keep reading child blocks
	while ( pFile->GetPosition() < pFile->Length() )
	{
		Sys::C_FileBlock block( pFile );

		if( block.HeaderId() == "Header" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			S32 modelCount;
			S32 elementCount;
			S32 constraintCount;

			ar >> fileVersion;
			pFile->SetVersionNumber( fileVersion );

			ar >> modelCount;
			ar >> elementCount;
			ar >> constraintCount;

			if( fileVersion >= E_ModelDataFileVersion_BoundsData )
			{
				ar >> m_Bounds;
			}

			m_ModelList.Resize( modelCount );
			m_ElementList.Resize( elementCount );
			m_ConstraintList.Resize( constraintCount );
		}
		else
		if( block.HeaderId() == "StrTab" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			ar >> m_StringTable;
		}
		else
		if( block.HeaderId() == "Models" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			const S32 modelCount = m_ModelList.Size();
			for(S32 modelNo=0; modelNo<modelCount; modelNo++)
			{
				C_Model& model = m_ModelList[modelNo];

				model.m_pModelData = this;

				ar >> model.m_Matrix;
				if( fileVersion >= E_ModelDataFileVersion_BoundsData )
				{
					ar >> model.m_LocalBounds; 
					model.m_Sphere = model.m_LocalBounds.BoundingSphere();
				}
				ar >> model.m_NameId;
				ar >> model.m_Index;
				ar >> model.m_ElementCount;
				S32 firstElementIndex;
				ar >> firstElementIndex;
				if( firstElementIndex == -1 )
				{
					model.m_ElementList = NULL;
				}
				else
				{
					model.m_ElementList = &m_ElementList[firstElementIndex];
					
				}
				S32 parentIndex;
				S32 firstChildIndex;
				S32 nextSiblingIndex;
				ar >> parentIndex;
				ar >> firstChildIndex;
				ar >> nextSiblingIndex;
				model.m_pParent = model.GetModelData()->GetModel( parentIndex );
				model.m_pFirstChild = model.GetModelData()->GetModel( firstChildIndex );
				model.m_pNextSibling = model.GetModelData()->GetModel( nextSiblingIndex );

				//LOGDEBUG( g_Log, "   Model '%s'", model.GetName() );
			}			
		}
		else
		if( block.HeaderId() == "Element" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );

			const S32 elementCount = m_ElementList.Size();
			for(S32 elementNo=0; elementNo<elementCount; elementNo++)
			{
				C_ModelElement& modelElement = m_ElementList[elementNo];

				S32 modelIndex;
				ar >> modelIndex;
				modelElement.m_pModel = &m_ModelList[modelIndex];

				ar >> modelElement.m_Matrix;
				if( fileVersion >= E_ModelDataFileVersion_BoundsData )
				{
					ar >> modelElement.m_LocalBounds;
					modelElement.m_Sphere = modelElement.m_LocalBounds.BoundingSphere();
				}
				ar >> modelElement.m_NameId;
				ar >> modelElement.m_Index;

				S32 parentIndex;
				S32 firstChildIndex;
				S32 nextSiblingIndex;
				ar >> parentIndex;
				ar >> firstChildIndex;
				ar >> nextSiblingIndex;

				modelElement.m_pParent = modelElement.GetModel()->GetElement(parentIndex);
				modelElement.m_pFirstChild = modelElement.GetModel()->GetElement(firstChildIndex);
				modelElement.m_pNextSibling = modelElement.GetModel()->GetElement(nextSiblingIndex);

				ar >> modelElement.m_RenderMeshId;
				ar >> modelElement.m_PhysicsShapeId;
			}
		}
		else
		if( block.HeaderId() == "Constr" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );

			const S32 constraintCount = m_ConstraintList.Size();
			for(S32 constraintNo=0; constraintNo<constraintCount; constraintNo++)
			{
				C_Constraint& constraint = m_ConstraintList[constraintNo];

				constraint.m_Index = constraintNo;
				constraint.m_pModelData = this;
				ar >> constraint.m_NameId;
				ar >> constraint.m_PhysicsConstraintId;
			}
		}
		else
		{
			LOGWARN( g_Log, "Unknown Block Header '%s'", block.HeaderId().Chars() );
		}
	}

	pFile->SetVersionNumber( prevFileVersion );

	return true;
}

MetaDebug::C_MetaNode* C_ModelData::GetRootMetaNode() const
{
#ifdef CFG_GOLD
	return NULL;
#else
	return m_pRootMetaNode;
#endif
}

void C_ModelData::SetRootMetaNode(MetaDebug::C_MetaNode* pMetaNode)
{
#ifndef CFG_GOLD
	m_pRootMetaNode = pMetaNode;
#endif
}

void C_ModelData::DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m )
{
	const S32 modelCount = GetModelCount();
	for(S32 modelNo=0; modelNo<modelCount; modelNo++)
	{
		C_Model* pModel = GetModel( modelNo );
		if( pModel )
		{
			Maths::HMat43 worldMat = pModel->GetMatrix() * m;
			pDebugRender->Draw3dMatrix( worldMat );

			pDebugRender->Draw3dText( worldMat.AW(), 0.25f, pModel->GetName() );

			const C_Model* pParentModel = pModel->GetParent();
			if(pParentModel)
			{
				Maths::HMat43 parentMat = pParentModel->GetMatrix() * m;
				pDebugRender->Draw3dArrow( worldMat.AW(), parentMat.AW() - worldMat.AW(), 1.0f, 0.2f, Maths::RGBA(0x00, 0xff, 0xff, 0xff) );

			}
		}
	}
}

void C_ModelData::Dump( ) const
{
#ifndef CFG_SPEED
	LOGDEBUG( g_Log, "" );
	LOGDEBUG( g_Log, "C_ModelData" );
	LOGDEBUG( g_Log, "" );

	const C_Model* pRootModel = GetRootModel();
	if(pRootModel)
	{
		LOGDEBUG( g_Log, "|" );
		pRootModel->Dump();
	}
#endif
}


} // namespace GameResource

