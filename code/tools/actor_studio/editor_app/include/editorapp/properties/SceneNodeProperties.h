#ifndef EDITOR_SCENE_NODE_PROPERTIES_H
#define EDITOR_SCENE_NODE_PROPERTIES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_SceneNodeProperties
	{
	public:
		C_SceneNodeProperties(wxPropertyGridManager *pgMgr);
		virtual ~C_SceneNodeProperties();

		void SetDgNode(EditorLib::I_DgNode *pNode);
		const EditorLib::I_DgNode* GetDgNode() const;

		void Refresh();

		void C_SceneNodeProperties::OnPropertyChange(wxPropertyGridEvent& e);

	protected:

		EditorLib::I_DgNode *m_pDgNode;

		wxPropertyGridManager *m_pPropertyGridMgr;

		wxPGProperty *m_pTranslateProp;
		wxPGProperty *m_pRotateProp;
		wxPGProperty *m_pScaleProp;
		wxPGProperty *m_pShearProp;
	};
}

#endif