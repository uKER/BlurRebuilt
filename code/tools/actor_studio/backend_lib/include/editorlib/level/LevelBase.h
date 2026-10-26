#ifndef EDITOR_LIB_LEVELBASE_H
#define EDITOR_LIB_LEVELBASE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Level/ILevel.h>

namespace EditorLib
{

class C_LevelBase : public I_Level
{
public:
	C_LevelBase();
	virtual ~C_LevelBase();

public:
	virtual E_LevelType GetType() const;
	virtual C_Status Load();
	virtual void Unload();

protected:
	virtual C_Status Initialise();
	virtual C_Status LoadData();
	virtual C_Status UnloadData();

public:
	virtual const C_AftkLevelRef* AddAftkRef(const Btl::String& filepath);
	virtual C_Status RemoveAftkRef(const Btl::String& filepath);
	virtual const C_AftkLevelRef* GetAftkRef(const Btl::String& filepath) const;
	virtual const AftkLevelRefList& GetAftkRefs() const;

	virtual C_SceneNode* GetRootSceneNode();
	virtual const C_SceneNode* GetRootSceneNode() const;

	virtual void SetName(const Btl::String& name);
	virtual const Btl::String& GetName() const;

	virtual void BuildActorNodeArray(Btl::Vector<const I_DgNode*>& nodes) const;

	virtual bool ContainsNode(const C_NodeId& id) const;

	virtual const C_Flags& GetFlags() const;

	virtual void OnAttachDgNode(I_DgNode* pNode);
	virtual void OnDetachDgNode(I_DgNode* pNode);

	virtual bool IsHidden() const;
	virtual void SetHidden(bool hidden);

	virtual bool IsReadOnly() const;
	virtual void SetReadOnly(bool readOnly);

protected:
	void ClearAftkRefs();

protected:
	C_Flags				m_flags;
	
	Btl::String			m_Name;

	//Scene graph nodes for this level. Note that these nodes cannot be created or destroyed by
	//users of the application.
	C_SceneNode*		m_pRootSceneNode;

	//We store a list of the refs that we're using. These are managed at a scene-wide level by C_AftkRefManager as we may need
	//to share reference scenes across multiple levels nested within one another.
	AftkLevelRefList	m_aftkRefs;

	DgIdPtrMap			m_dgNodes;
};

}

#endif