#ifndef GAME_RESOURCE_MODEL_DATA_H
#define GAME_RESOURCE_MODEL_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <system/debug/I_DebugRender.h>
#include <misc_shared/DataStructure/StringTable.h>

namespace Gravity
{
	class C_ImportShape;
	class C_ImportScene;
	class C_ConstraintDescriptors;
}

namespace Sys
{
	class I_File;
}

namespace MetaDebug
{
	class C_MetaNode;
}

namespace GameResource
{

class C_Model;
class C_ModelElement;
class C_ModelData;



//! class C_ModelElement
class C_ModelElement
{
	friend class C_Model;
	friend class C_ModelData;
public:
	C_ModelElement();

	~C_ModelElement();

	const Maths::HMat43 GetMatrix() const;

	const Maths::C_AxisBox GetLocalBounds() const;

	const Maths::C_Sphere GetBoundingSphere() const;

	Util::C_String GetName() const;

	C_Model* GetModel();

	const C_Model* GetModel() const;

	const S32 GetIndex() const
	{
		return m_Index;
	}

	const C_ModelElement* GetParent() const;

	const C_ModelElement* GetFirstChild() const;

	const C_ModelElement* GetNextSibling() const;

	const Gravity::C_ImportShape* GetPhysicsImportShape() const;

	void Dump( const Util::C_String& prefix = "" ) const;

private:
	const Util::C_StringTable& GetStringTable() const;

private:
	Maths::HMat43			m_Matrix;
	Maths::C_AxisBox		m_LocalBounds; 
	Maths::C_Sphere			m_Sphere;

	Util::C_StringId		m_NameId;
	S32						m_Index;

	C_Model*				m_pModel;
	C_ModelElement*			m_pParent;
	C_ModelElement*			m_pFirstChild;
	C_ModelElement*			m_pNextSibling;

	S32						m_RenderMeshId;
	U32						m_PhysicsShapeId;
};

//! class C_Model
class C_Model
{
	friend class C_ModelElement;
	friend class C_ModelData;
public:
	C_Model();

	~C_Model();

	const Maths::HMat43 GetMatrix() const;

	const Maths::C_AxisBox GetLocalBounds() const;

	const Maths::C_Sphere GetBoundingSphere() const;

	const char *GetName() const;

	const S32 GetIndex() const;

	const C_Model* GetParent() const;

	const C_Model* GetFirstChild() const;

	const C_Model* GetNextSibling() const;

	S32 GetElementCount() const;

	C_ModelData* GetModelData();

	const C_ModelData* GetModelData() const;

	// returns NULL if index is out of range
	C_ModelElement* GetElement( const S32 index );

	// returns NULL if index is out of range
	const C_ModelElement* GetElement( const S32 index ) const;

	// returns NULL if model not found
	const C_ModelElement* GetElement( const Util::C_String &elementName ) const;

	// return -1 if element not found
	const S32 GetElementIndex( const Util::C_String &elementName ) const;

	const C_ModelElement* GetRootElement() const;

	void Dump( const Util::C_String& prefix = "" ) const;

private:
	const Util::C_StringTable& GetStringTable() const;

private:
	Maths::HMat43			m_Matrix;
	Maths::C_AxisBox		m_LocalBounds;
	Maths::C_Sphere			m_Sphere;

	Util::C_StringId		m_NameId;
	S32						m_Index;

	S32						m_ElementCount;
	C_ModelElement*			m_ElementList;

	C_ModelData*			m_pModelData;
	C_Model*				m_pParent;
	C_Model*				m_pFirstChild;
	C_Model*				m_pNextSibling;
};

//! class C_Constraint
class C_Constraint
{
	friend class C_ModelData;
public:
	C_Constraint();

	~C_Constraint();

	const Gravity::C_ConstraintDescriptors* GetPhysicsConstraint() const;

	Util::C_String GetName() const;

private:
	C_ModelData*			m_pModelData;
	S32						m_Index;
	Util::C_StringId		m_NameId;
	U32						m_PhysicsConstraintId;
};

//! class C_ModelData
class C_ModelData
{
	friend class C_Constraint;
	friend class C_ModelElement;
	friend class C_Model;
	friend class C_LevelResource;
	friend class C_ModelResource;
public:
	C_ModelData();

	~C_ModelData();

	S32 GetModelCount() const;

	// returns NULL if index is out of range
	C_Model* GetModel( const S32 index );

	// returns NULL if index is out of range
	const C_Model* GetModel( const S32 index ) const;

	// returns NULL if model not found
	const C_Model* GetModel( const Util::C_String &modelName ) const;

	const C_Model* GetRootModel() const;

	const Maths::C_AxisBox GetBounds() const;

	const S32 GetConstraintCount() const;

	const C_Constraint* GetConstraint( const S32 index ) const;

	bool Read( Sys::I_File* pFile );

	void DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m );

	void Dump() const;

	// returns NULL in gold build
	MetaDebug::C_MetaNode*		GetRootMetaNode() const;


private:
	void SetRootMetaNode(MetaDebug::C_MetaNode*);

	const Util::C_StringTable& GetStringTable() const;

private:
	Maths::C_AxisBox			m_Bounds;

	Util::C_StringTable			m_StringTable;
	Btl::Vector<C_Model>		m_ModelList;
	Btl::Vector<C_ModelElement>	m_ElementList;
	Btl::Vector<C_Constraint>	m_ConstraintList;

	Gravity::C_ImportScene*		m_pPhysicsImport;

	#ifndef CFG_GOLD
	MetaDebug::C_MetaNode*		m_pRootMetaNode;
	#endif
};

} // namespace GameResource

#endif // GAME_RESOURCE_MODEL_DATA_H
