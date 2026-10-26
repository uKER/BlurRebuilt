#ifndef GAME_RESOURCE_ELEMENT_LIST_H
#define GAME_RESOURCE_ELEMENT_LIST_H

#ifdef _MSC_VER
#pragma once
#endif

/*

#include <btl/containers/vector.h>

#include <resource/Resource.h>
#include <misc_shared/DataStructure/StringTable.h>

#include <system/debug/I_DebugRender.h>

namespace GameResource
{

class C_ElementList;

//! C_Element
class C_Element
{
friend class C_ElementList;
public:
	C_Element();

	~C_Element();

	const Maths::HMat43 GetMatrix() const;

	const C_ElementList* GetElementListPtr() const;

	Util::C_String GetName() const;

	const S32 GetIndex() const;

	const C_Element* GetParent() const;

	const C_Element* GetFirstChild() const;

	const C_Element* GetNextSibling() const;

	template<typename T_Stream>
	void Deserialise( T_Stream& ar );

	U32 GetPhysicsShapeId() const;

private:
	Maths::HMat43			m_Matrix;
	Util::C_StringId		m_NameId;
	S32						m_Index;
	C_ElementList*			m_pElementList;
	C_Element*				m_pParent;
	C_Element*				m_pFirstChild;
	C_Element*				m_pNextSibling;

	S32						m_RenderNodeId;
	U32						m_PhysicsShapeId;
};

template<typename T_Stream>
void C_Element::Deserialise( T_Stream& ar )
{
	ar >> m_Matrix;
	ar >> m_NameId;
	ar >> m_Index;
	S32 parentIndex;
	ar >> parentIndex;
	m_pParent = const_cast<C_Element*>( m_pElementList->GetElement(parentIndex) );
	S32 firstChildIndex;
	ar >> firstChildIndex;
	m_pFirstChild = const_cast<C_Element*>( m_pElementList->GetElement(firstChildIndex) );
	S32 nextSiblingIndex;
	ar >> nextSiblingIndex;
	m_pNextSibling = const_cast<C_Element*>( m_pElementList->GetElement(nextSiblingIndex) );
	ar >> m_RenderNodeId;
	ar >> m_PhysicsShapeId;

	LOGINFO( g_Log, "Physics shape Id 0x%08x", m_PhysicsShapeId);
}



class C_ElementList
{
friend class C_Element;

public:
	C_ElementList();

	~C_ElementList();

	S32 GetElementCount() const;

	// returns NULL if index is out of range
	const C_Element* GetElement( const S32 index ) const;

	// returns NULL if element not found
	const C_Element* GetElement( const Util::C_String &elementName ) const;

	const C_Element* GetRootElement() const;

	const Util::C_StringTable& GetStringTable() const;

	bool Read( Sys::I_File* pFile );

	void DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m );

private:
	Util::C_StringTable			m_StringTable;
	Btl::Vector<C_Element>		m_ElementList;
	C_Element*					m_pRootElement;
};


} // namespace GameResource
*/

#endif // GAME_RESOURCE_ELEMENT_LIST_H
