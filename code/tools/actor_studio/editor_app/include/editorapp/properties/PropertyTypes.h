#ifndef EDITOR_PROPERTY_TYPES
#define EDITOR_PROPERTY_TYPES

#ifndef _MSC_VER
#pragma once
#endif

//evil wxPropertyGrid string concatenation macros means 
//we need to declare these in the global namespace
typedef EditorLib::Vector3	PGVec3;
typedef Maths::HVec3		PGHVec3;

WX_PG_DECLARE_VARIANT_DATA(PGVec3VariantData, PGVec3, wxPG_NO_DECL)

class PGVec3Property : public wxPGProperty
{
public:
	PGVec3Property(const wxString& label = wxPG_LABEL,
					const wxString& name = wxPG_LABEL,
					const PGVec3& value = PGVec3(0.f, 0.f, 0.f));

	~PGVec3Property();
	WX_PG_DECLARE_PARENTAL_METHODS()

private:
	WX_PG_DECLARE_PROPERTY_CLASS(PGVec3Property);
};



WX_PG_DECLARE_VARIANT_DATA(PGHVec3VariantData, PGHVec3, wxPG_NO_DECL)

class PGHVec3Property : public wxPGProperty
{
public:
	PGHVec3Property(const wxString& label = wxPG_LABEL,
					const wxString& name = wxPG_LABEL,
					const PGHVec3& value = PGHVec3(0.f, 0.f, 0.f));

	~PGHVec3Property();
	WX_PG_DECLARE_PARENTAL_METHODS()

private:
	WX_PG_DECLARE_PROPERTY_CLASS(PGHVec3Property);
};

#endif