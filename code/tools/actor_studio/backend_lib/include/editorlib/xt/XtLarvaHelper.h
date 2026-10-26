#ifndef EDITOR_LIB_XT_LARVA_HELPER_H
#define EDITOR_LIB_XT_LARVA_HELPER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Xt/XtDefinitions.h>

namespace EditorLib
{
namespace XtLarvaHelpers
{

LtStruct* FindLarvaType(LarvaSchema* larvaSchema, const Btl::String& typeName, E_XtSchemaType filter);

}
}

#endif
