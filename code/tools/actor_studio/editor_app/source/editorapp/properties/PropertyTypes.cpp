#include "stdafx.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/propdev.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>

#include "EditorApp/Misc/MathsHelpers.h"
#include <EditorApp/properties/PropertyTypes.h>


WX_PG_IMPLEMENT_VARIANT_DATA(PGVec3VariantData, PGVec3)
WX_PG_IMPLEMENT_PROPERTY_CLASS(PGVec3Property, wxPGProperty, PGVec3, const PGVec3&, TextCtrl)

PGVec3Property::PGVec3Property(const wxString &label, const wxString &name, const PGVec3& value)
: wxPGProperty(label, name)
{
	SetValue(PGVec3ToVariant(value));
	AddChild(new wxFloatProperty(_T("x"), wxPG_LABEL, value.X()));
	AddChild(new wxFloatProperty(_T("y"), wxPG_LABEL, value.Y()));
	AddChild(new wxFloatProperty(_T("z"), wxPG_LABEL, value.Z()));
}

PGVec3Property::~PGVec3Property()
{
}

void PGVec3Property::RefreshChildren()
{
	if (!GetCount())
		return;

	PGVec3& v = PGVec3FromVariant(m_value);
	Item(0)->SetValue((F32)v.X());
	Item(1)->SetValue((F32)v.Y());
	Item(2)->SetValue((F32)v.Z());
}

void PGVec3Property::ChildChanged(wxVariant &thisValue, int childIndex, wxVariant &childValue) const
{
	PGVec3& v = PGVec3FromVariant(thisValue);
	switch(childIndex)
	{
	case 0: v.X(childValue.GetDouble()); break;
	case 1: v.Y(childValue.GetDouble()); break;
	case 2: v.Z(childValue.GetDouble()); break;
	}
}



WX_PG_IMPLEMENT_VARIANT_DATA(PGHVec3VariantData, PGHVec3)
WX_PG_IMPLEMENT_PROPERTY_CLASS(PGHVec3Property, wxPGProperty, PGHVec3, const PGHVec3&, TextCtrl)

PGHVec3Property::PGHVec3Property(const wxString &label, const wxString &name, const PGHVec3 &value)
: wxPGProperty(label, name)
{
	SetValue(PGHVec3ToVariant(value));
	AddChild(new wxFloatProperty(_T("x"), wxPG_LABEL, value.X()));
	AddChild(new wxFloatProperty(_T("y"), wxPG_LABEL, value.Y()));
	AddChild(new wxFloatProperty(_T("z"), wxPG_LABEL, value.Z()));
}

PGHVec3Property::~PGHVec3Property()
{
}

void PGHVec3Property::RefreshChildren()
{
	if (!GetCount())
		return;

	PGHVec3& v = PGHVec3FromVariant(m_value);
	Item(0)->SetValue((F32)v.X());
	Item(1)->SetValue((F32)v.Y());
	Item(2)->SetValue((F32)v.Z());
}

void PGHVec3Property::ChildChanged(wxVariant &thisValue, int childIndex, wxVariant &childValue) const
{
	PGHVec3& v = PGHVec3FromVariant(thisValue);
	switch(childIndex)
	{
	case 0: v.X(childValue.GetDouble()); break;
	case 1: v.Y(childValue.GetDouble()); break;
	case 2: v.Z(childValue.GetDouble()); break;
	}
}