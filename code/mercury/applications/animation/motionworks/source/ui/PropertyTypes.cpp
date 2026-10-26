#include "stdafx.h"
#include "PropertyTypes.h"


//
// C_HVec3Property
//


WX_PG_IMPLEMENT_VARIANT_DATA(PGHVec3VariantData, PGHVec3)

WX_PG_IMPLEMENT_PROPERTY_CLASS(C_HVec3Property,wxPGProperty,
                               PGHVec3,const PGHVec3&,TextCtrl)


C_HVec3Property::C_HVec3Property( const wxString& label,
	const wxString& name,
    const PGHVec3& value )
    : wxPGProperty(label,name)
{
    SetValue( PGHVec3ToVariant(value) );
    AddChild( new wxFloatProperty(wxT("X"),wxPG_LABEL,(F32)value.X()) );
    AddChild( new wxFloatProperty(wxT("Y"),wxPG_LABEL,(F32)value.Y()) );
    AddChild( new wxFloatProperty(wxT("Z"),wxPG_LABEL,(F32)value.Z()) );
}

C_HVec3Property::~C_HVec3Property() { }

void C_HVec3Property::RefreshChildren()
{
    if ( !GetCount() ) return;
    PGHVec3& vector = PGHVec3FromVariant(m_value);
    Item(0)->SetValue( (F32)vector.X() );
    Item(1)->SetValue( (F32)vector.Y() );
    Item(2)->SetValue( (F32)vector.Z() );
}

void C_HVec3Property::ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const
{
    PGHVec3& vector = PGHVec3FromVariant(thisValue);
    switch ( childIndex )
    {
        case 0: vector.X(childValue.GetDouble()); break;
        case 1: vector.Y(childValue.GetDouble()); break;
        case 2: vector.Z(childValue.GetDouble()); break;
    }
}

//
// C_HVec4Property
//

WX_PG_IMPLEMENT_VARIANT_DATA(PGHVec4VariantData, PGHVec4)

WX_PG_IMPLEMENT_PROPERTY_CLASS(C_HVec4Property,wxPGProperty,
                               PGHVec4,const PGHVec4&,TextCtrl)


C_HVec4Property::C_HVec4Property( const wxString& label,
	const wxString& name,
    const PGHVec4& value )
    : wxPGProperty(label,name)
{
    SetValue( PGHVec4ToVariant(value) );
    AddChild( new wxFloatProperty(wxT("X"),wxPG_LABEL,(F32)value.X()) );
    AddChild( new wxFloatProperty(wxT("Y"),wxPG_LABEL,(F32)value.Y()) );
    AddChild( new wxFloatProperty(wxT("Z"),wxPG_LABEL,(F32)value.Z()) );
    AddChild( new wxFloatProperty(wxT("W"),wxPG_LABEL,(F32)value.W()) );
}

C_HVec4Property::~C_HVec4Property() { }

void C_HVec4Property::RefreshChildren()
{
    if ( !GetCount() ) return;
    PGHVec4& vector = PGHVec4FromVariant(m_value);
    Item(0)->SetValue( (F32)vector.X() );
    Item(1)->SetValue( (F32)vector.Y() );
    Item(2)->SetValue( (F32)vector.Z() );
    Item(3)->SetValue( (F32)vector.W() );
}

void C_HVec4Property::ChildChanged( wxVariant& thisValue, int childIndex, wxVariant& childValue ) const
{
    PGHVec4& vector = PGHVec4FromVariant(thisValue);
    switch ( childIndex )
    {
        case 0: vector.X(childValue.GetDouble()); break;
        case 1: vector.Y(childValue.GetDouble()); break;
        case 2: vector.Z(childValue.GetDouble()); break;
        case 3: vector.W(childValue.GetDouble()); break;
    }
}
