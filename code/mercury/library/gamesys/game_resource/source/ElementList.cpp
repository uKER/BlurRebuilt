#include "GameResourcePch.h"

#include <game_resource/GameResource.h>
#include <game_resource/ElementList.h>
#include <system/Serialise.h>
#include <system/serialise/C_FileBlock.h>

namespace GameResource
{

	/*
// C_Element

C_Element::C_Element()
	:	m_NameId(-1)
	,	m_Index(-1)
	,	m_pElementList(NULL)
	,	m_pParent(NULL)
	,	m_pFirstChild(NULL)
	,	m_pNextSibling(NULL)
{
}

C_Element::~C_Element()
{
}

const Maths::HMat43 C_Element::GetMatrix() const
{
	return m_Matrix;
}

const C_ElementList* C_Element::GetElementListPtr() const
{
	return m_pElementList;
}

Util::C_String C_Element::GetName() const
{
	return Util::C_String( m_pElementList->m_StringTable.GetString( m_NameId ) );
}

const S32 C_Element::GetIndex() const
{
	return m_Index;
}

const C_Element* C_Element::GetParent() const
{
	return m_pParent;
}

const C_Element* C_Element::GetFirstChild() const
{
	return m_pFirstChild;
}

const C_Element* C_Element::GetNextSibling() const
{
	return m_pNextSibling;
}

U32 C_Element::GetPhysicsShapeId() const
{
	return m_PhysicsShapeId;
}


// C_ElementList 

C_ElementList::C_ElementList()
	: m_pRootElement(NULL)
{
}

C_ElementList::~C_ElementList()
{
}

S32 C_ElementList::GetElementCount() const
{
	return m_ElementList.Size();
}

const C_Element* C_ElementList::GetElement( const S32 index ) const
{
	if((index < 0) || (index >= m_ElementList.Size()) )
		return NULL;

	return &m_ElementList[index];
}

const C_Element* C_ElementList::GetElement( const Util::C_String &elementName ) const
{
	const S32 elementCount = m_ElementList.Size();
	for(S32 i=0; i<elementCount; i++)
	{
		if(elementName == Util::C_String(m_ElementList[i].GetName()))
		{
			return &m_ElementList[i];
		}
	}

	return NULL;
}

const C_Element* C_ElementList::GetRootElement() const
{
	return m_pRootElement;
}

const Util::C_StringTable& C_ElementList::GetStringTable() const
{
	return m_StringTable;
}

bool C_ElementList::Read( Sys::I_File* pFile )
{
	// keep reading child blocks
	while ( pFile->GetPosition() < pFile->Length() )
	{
		Sys::C_FileBlock block( pFile );

		if( block.HeaderId() == "Header" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			S32 elementCount;
			ar >> elementCount;
			m_ElementList.Resize( elementCount );
		}
		else
		if( block.HeaderId() == "StrTab" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			ar >> m_StringTable;
		}
		else
		if( block.HeaderId() == "Element" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );

			const S32 elementCount = m_ElementList.Size();
			for(S32 elementNo=0; elementNo<elementCount; elementNo++)
			{
				C_Element& element = m_ElementList[elementNo];
				element.m_pElementList = this;

				ar >> element;

				LOGINFO( g_Log, "Level Element '%s'", element.GetName().Chars() );
			}
		}
		else
		{
			LOGWARN( g_Log, "Unknown Block Header '%s'", block.HeaderId().Chars() );
		}
	}

	return true;
}

void C_ElementList::DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m )
{
	const S32 elementCount = m_ElementList.Size();
	for(S32 elementNo=0; elementNo<elementCount; elementNo++)
	{
		C_Element& element = m_ElementList[elementNo];

		Maths::HMat43 elementMatrix = element.GetMatrix() * m;
		pDebugRender->Draw3dMatrix( elementMatrix );
		pDebugRender->Draw3dText( elementMatrix.AW(), element.GetName() );

		const C_Element* pParentElement = element.GetParent();
		if( pParentElement )
		{
			Maths::HMat43 parentElementMatrix = pParentElement->GetMatrix() * m;

			Maths::HVec3 start = elementMatrix.AW();
			Maths::HVec3 end = parentElementMatrix.AW();

			pDebugRender->Draw3dArrow( start, end - start, 1.0f, 0.02f, Maths::RGBA(0x00, 0x80, 0x80,0xff) );
		}
	}
}
*/

} // namespace GameResource

