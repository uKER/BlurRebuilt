#ifndef PROPERTYTYPES_H
#define PROPERTYTYPES_H

#include "wx/propgrid/propgrid.h"
#include "wx/propgrid/propdev.h"
#include "wx/propgrid/advprops.h"
#include "wx/propgrid/manager.h"

typedef Maths::HVec3 PGHVec3;
typedef Maths::HVec4 PGHVec4;


WX_PG_DECLARE_VARIANT_DATA(PGHVec3VariantData, PGHVec3, wxPG_NO_DECL)

class C_HVec3Property: public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(C_HVec3Property)
public:

    C_HVec3Property( const wxString& label = wxPG_LABEL,
                      const wxString& name = wxPG_LABEL,
                      const PGHVec3& value = PGHVec3() );
    virtual ~C_HVec3Property();

    WX_PG_DECLARE_PARENTAL_METHODS()

protected:
};


WX_PG_DECLARE_VARIANT_DATA(PGHVec4VariantData, PGHVec4, wxPG_NO_DECL)

class C_HVec4Property: public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(C_HVec4Property)
public:

    C_HVec4Property( const wxString& label = wxPG_LABEL,
                      const wxString& name = wxPG_LABEL,
                      const PGHVec4& value = PGHVec4() );
    virtual ~C_HVec4Property();

    WX_PG_DECLARE_PARENTAL_METHODS()

protected:
};



#endif

