#include "stdafx.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/propdev.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>

#include <EditorApp/properties/SceneNodeProperties.h>
#include <EditorApp/properties/PropertyTypes.h>

#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

using namespace Editor;


C_SceneNodeProperties::C_SceneNodeProperties(wxPropertyGridManager *pgMgr)
: m_pPropertyGridMgr(pgMgr)
{
	m_pPropertyGridMgr->Freeze();
	m_pPropertyGridMgr->AddPage(_T("SceneNode"));
	m_pPropertyGridMgr->SelectPage(_T("SceneNode"));

	m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Transform"), wxPG_LABEL));
	m_pTranslateProp = m_pPropertyGridMgr->Append(new PGVec3Property(_T("Translate"), wxPG_LABEL));
	m_pRotateProp = m_pPropertyGridMgr->Append(new PGVec3Property(_T("Rotate"), wxPG_LABEL));
	m_pScaleProp = m_pPropertyGridMgr->Append(new PGVec3Property(_T("Scale"), wxPG_LABEL));
	m_pShearProp = m_pPropertyGridMgr->Append(new PGVec3Property(_T("Shear"), wxPG_LABEL));

	m_pPropertyGridMgr->Thaw();

	m_pDgNode = 0;
	Refresh();
}

C_SceneNodeProperties::~C_SceneNodeProperties()
{
	m_pPropertyGridMgr->RemovePage(m_pPropertyGridMgr->GetPageByName(_T("SceneNode")));
}

void C_SceneNodeProperties::SetDgNode(EditorLib::I_DgNode *pNode)
{
	m_pDgNode = pNode;
	wxPropertyGridPage *pPage = m_pPropertyGridMgr->GetPage(_T("SceneNode"));
	if (pPage)
		pPage->ClearSelection();

	Refresh();
}

const EditorLib::I_DgNode* C_SceneNodeProperties::GetDgNode() const
{
	return m_pDgNode;
}

void C_SceneNodeProperties::OnPropertyChange(wxPropertyGridEvent& e)
{
	wxVariant value = e.GetPropertyValue();
	if (value.IsNull())
		return;

	if (!m_pDgNode)
		return;

	EditorLib::C_SceneNode *pSceneNode = (EditorLib::C_SceneNode*) m_pDgNode;

	m_pPropertyGridMgr->Freeze();

	EditorLib::C_ViewDisabler viewDisabler;
	const wxPGProperty *pProperty = e.GetProperty();
	if (pProperty == m_pTranslateProp)
		EditorLib::PlugHelpers::SetCompoundValue(pSceneNode->GetTranslatePlug(), PGVec3FromVariant(value));
	else if (pProperty == m_pRotateProp)
	{
		PGVec3 v = PGVec3FromVariant(value);
		v[0] = Maths::DegToRad(v[0]);
		v[1] = Maths::DegToRad(v[1]);
		v[2] = Maths::DegToRad(v[2]);
		EditorLib::PlugHelpers::SetCompoundValue(pSceneNode->GetRotatePlug(), v);
	}
	else if (pProperty == m_pScaleProp)
		EditorLib::PlugHelpers::SetCompoundValue(pSceneNode->GetScalePlug(), PGVec3FromVariant(value));
	else if (pProperty == m_pShearProp)
		EditorLib::PlugHelpers::SetCompoundValue(pSceneNode->GetShearPlug(), PGVec3FromVariant(value));

	m_pPropertyGridMgr->Thaw();
}

void C_SceneNodeProperties::Refresh()
{
	m_pPropertyGridMgr->Freeze();
	if (m_pDgNode && m_pDgNode->IsA(EditorLib::E_ObjType_SceneNode))
	{
		m_pPropertyGridMgr->SelectPage(_T("SceneNode"));

		PGVec3 tmp;
		EditorLib::C_SceneNode *pSceneNode = (EditorLib::C_SceneNode*) m_pDgNode;

		EditorLib::PlugHelpers::GetCompoundValue(pSceneNode->GetTranslatePlug(), tmp);
		m_pTranslateProp->SetValue(PGVec3ToVariant(tmp));

		EditorLib::PlugHelpers::GetCompoundValue(pSceneNode->GetRotatePlug(), tmp);
		tmp[0] = Maths::RadToDeg(tmp[0]);
		tmp[1] = Maths::RadToDeg(tmp[1]);
		tmp[2] = Maths::RadToDeg(tmp[2]);
		m_pRotateProp->SetValue(PGVec3ToVariant(tmp));

		EditorLib::PlugHelpers::GetCompoundValue(pSceneNode->GetScalePlug(), tmp);
		m_pScaleProp->SetValue(PGVec3ToVariant(tmp));

		EditorLib::PlugHelpers::GetCompoundValue(pSceneNode->GetShearPlug(), tmp);
		m_pShearProp->SetValue(PGVec3ToVariant(tmp));

		m_pTranslateProp->Hide(false);
		m_pRotateProp->Hide(false);
		m_pScaleProp->Hide(false);
		m_pShearProp->Hide(false);

		m_pPropertyGridMgr->EnableProperty(m_pTranslateProp, !pSceneNode->IsReadOnly());
		m_pPropertyGridMgr->EnableProperty(m_pRotateProp, !pSceneNode->IsReadOnly());
		m_pPropertyGridMgr->EnableProperty(m_pScaleProp, false);
		m_pPropertyGridMgr->EnableProperty(m_pShearProp, false);

		
	}
	else
	{
		m_pPropertyGridMgr->SelectPage(_T("SceneNode"));

		PGVec3 tmp(0.f, 0.f, 0.f);
		m_pTranslateProp->SetValue(PGVec3ToVariant(tmp));
		m_pTranslateProp->Hide(true);
		m_pRotateProp->SetValue(PGVec3ToVariant(tmp));
		m_pRotateProp->Hide(true);
		m_pScaleProp->SetValue(PGVec3ToVariant(tmp));
		m_pScaleProp->Hide(true);
		m_pShearProp->SetValue(PGVec3ToVariant(tmp));
		m_pShearProp->Hide(true);

		m_pPropertyGridMgr->SelectPage(-1);
	}

	m_pPropertyGridMgr->Thaw();

}