#ifndef CONVERTER_SHARED_MODEL_DATA_H
#define CONVERTER_SHARED_MODEL_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/Vector.h>
#include <misc_shared/DataStructure/StringTable.h>
#include <converter_shared/Hierarchy.h>
#include <converter_shared/HierarchyIterator.h>

namespace Aftk
{
	class C_SceneNode;
}

namespace ModelData
{

typedef Btl::Vector<const Aftk::C_SceneNode*> C_AftkNodeList;

//! class C_ModelElement
class C_ModelElement : public Hrc::C_Node<C_ModelElement>
{
	friend class C_Model;
	friend class C_ModelData;
public:
	C_ModelElement();
	
	C_ModelElement(	const Util::C_String& name, 
					Maths::HMat43 PCREF matrix, 
					const Aftk::C_SceneNode* pRootAftkNode);

	~C_ModelElement();

	const Maths::HMat43 GetMatrix() const;

	void SetMatrix( Maths::HMat43& matrix );

	const Util::C_String& GetName() const;

	void SetName( const Util::C_String& name );

	void SetPhysicsShapeId( const U32 id );

	const C_Model* GetModel() const;

	const S32 GetIndex() const;

	void SetRootAftkNode( Aftk::C_SceneNode* pRootAftkNode );
	
	const Aftk::C_SceneNode* GetRootAftkNode() const;

	void AddAftkNode( const Aftk::C_SceneNode* pRootAftkNode );

	C_AftkNodeList& GetAftkNodeList();

	const C_AftkNodeList& GetAftkNodeList() const;

	void Dump( const bool dumpAftkNodeList, const Util::C_String& prefix );

private:
	Maths::HMat43				m_Matrix;
	Maths::C_AxisBox			m_LocalBounds;

	Util::C_String				m_Name;
	mutable Util::C_StringId	m_NameId;

	const Aftk::C_SceneNode*	m_pRootAftkNode;

	C_AftkNodeList				m_AftkNodeList;

	mutable S32					m_Index;

	S32							m_RenderMeshId;
	U32							m_PhysicsShapeId;
};

//! class C_Model
class C_Model : public Hrc::C_Node<C_Model>, public Hrc::C_Hierarchy<C_ModelElement>
{
	friend class C_ModelElement;
	friend class C_ModelData;
public:
	C_Model();

	C_Model(	const Util::C_String& name, 
				Maths::HMat43 PCREF matrix, 
				const Aftk::C_SceneNode* pRootAftkNode);

	~C_Model();

	const Maths::HMat43 GetMatrix() const;

	void SetMatrix( Maths::HMat43& matrix );

	const Util::C_String& GetName() const;

	void SetName( const Util::C_String& name );

	void SetRootAftkNode( Aftk::C_SceneNode* pRootAftkNode );

	const S32 GetIndex() const;

	const C_ModelData* GetModelData() const;

	void Dump( const bool dumpAftkNodeList, const Util::C_String& prefix = "" );

private:
	Maths::HMat43				m_Matrix;
	Maths::C_AxisBox			m_LocalBounds;

	Util::C_String				m_Name;
	mutable Util::C_StringId	m_NameId;

	const Aftk::C_SceneNode*	m_pRootAftkNode;

	mutable S32					m_Index;
	mutable S32					m_FirstElementIndex;
};

class C_Constraint
{
public:
	C_Constraint();
	C_Constraint(const Util::C_String& name, const Aftk::C_SceneNode* pAftkSceneNode);
	~C_Constraint();

	const Util::C_String& GetName() const;

public:
	Util::C_String				m_Name;
	mutable Util::C_StringId	m_NameId;

	const Aftk::C_SceneNode*	m_pAftkSceneNode;
	mutable U32					m_PhysicsConstraintId;
};

//! class C_ModelData
class C_ModelData : public Hrc::C_Hierarchy<C_Model>
{
	friend class C_ModelElement;
	friend class C_Model;
public:
	C_ModelData();

	~C_ModelData();

	bool Export( Sys::I_File* pFile ) const;

	void Dump( const bool dumpAftkNodeList );

	void AddConstraint( const Aftk::C_SceneNode* pAftkSceneNode );

	const S32 GetConstraintCount() const;

	const C_Constraint& GetConstraint( const S32 index ) const;

	C_Constraint& GetConstraint( const S32 index );

	void CalculateBounds();

private:
	Maths::C_AxisBox			m_Bounds;
	Btl::Vector<C_Constraint>	m_ConstraintList;
};

void DumpAfktNodeList( const C_AftkNodeList& aftkNodeList, const Util::C_String& prefix = "" );

typedef Hrc::C_HierarchyConstIterator<C_Model> C_ModelConstIterator;
typedef Hrc::C_HierarchyIterator<C_Model> C_ModelIterator;

typedef Hrc::C_HierarchyConstIterator<C_ModelElement> C_ModelElementConstIterator;
typedef Hrc::C_HierarchyIterator<C_ModelElement> C_ModelElementIterator;

} // namespace ModelData

#endif

