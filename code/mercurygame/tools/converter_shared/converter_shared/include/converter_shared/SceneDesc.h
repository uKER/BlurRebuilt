#ifndef CONVERTER_SHARED_SCENE_DESC_H
#define CONVERTER_SHARED_SCENE_DESC_H

#ifdef _MSC_VER
#pragma once
#endif

/*
#include <btl/containers/Vector.h>

#include <converter_shared/Hierarchy.h>
#include "converter_shared/HierarchyIterator.h"

namespace Aftk
{
	class C_SceneNode;
};

namespace SceneDesc
{


class C_SceneDesc;
class C_Group;
class C_GroupNode;

typedef Btl::Vector<const Aftk::C_SceneNode*> C_AftkNodeList;

//! C_UserData

class C_UserData
{
public:
	C_UserData();
	virtual ~C_UserData();
};

//! C_GroupNode

class C_GroupNode : public Hrc::C_Node<C_GroupNode>
{
friend class C_SceneDesc;
friend class C_Group;
public:
	C_GroupNode();
	~C_GroupNode();

	void Dump( const bool dumpAftkNodeList, const Util::C_String& prefix = "" );

	const Maths::HMat43 GetMatrix() const;

	void SetMatrix( Maths::HMat43& matrix );

	const Util::C_String& GetName() const;

	void SetName( const Util::C_String& name );

	// This is invalid until enumerate is called.
	S32 GetIndex() const;

	void AddAftkNode( const Aftk::C_SceneNode* pNode );

	C_AftkNodeList& GetAftkNodeList();

	const C_AftkNodeList& GetAftkNodeList() const;

	void SetUserData( C_UserData* pUserData );

	C_UserData* GetUserData();

	const C_UserData* GetUserData() const;

private:
	Maths::HMat43 m_Matrix;
	Util::C_String m_Name;

	S32 m_Index;

	C_AftkNodeList m_AftkNodeList;

	C_UserData* m_pUserData;
};

//! C_Group

class C_Group : public Hrc::C_Hierarchy<C_GroupNode>
{
friend class C_GroupNode;
friend class C_SceneDesc;
public:
	C_Group();
	~C_Group();

	void Dump( const bool dumpAftkNodeList );

	S32 GetIndex() const;

	const Util::C_String& GetName() const;

	void SetName( const Util::C_String& name );

private:
	bool EnumerateNodes( C_GroupNode* pNode = NULL );

private:
	Util::C_String m_Name;

	S32 m_Index;

	C_SceneDesc* m_pSceneDesc;
};

//! C_SceneDesc
class C_SceneDesc
{
public:
	C_SceneDesc();
	~C_SceneDesc();

	void Dump( const bool dumpAftkNodeList );

	void SetGroupCount( const S32 groupCount );

	S32 GetGroupCount() const;

	C_Group& GetGroup( const S32 index );

	const C_Group& GetGroup( const S32 index ) const;

	bool Enumerate();

private:
	Btl::Vector<C_Group> m_GroupList;
};

typedef Hrc::C_HierarchyConstIterator<C_GroupNode> C_GroupNodeConstIterator;
typedef Hrc::C_HierarchyIterator<C_GroupNode> C_GroupNodeIterator;

} // namespace SceneDesc
*/
#endif

