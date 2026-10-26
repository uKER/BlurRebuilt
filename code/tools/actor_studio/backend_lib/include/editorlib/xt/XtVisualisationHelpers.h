#ifndef EDITOR_LIB_XT_VISUALISATION_HELPERS_H
#define EDITOR_LIB_XT_VISUALISATION_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace EditorLib
{
	namespace XtVisualisationHelpers
	{
		XtString EvaluateSummaryText(const XtString& attr, const XtString& param, LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component);
	}
}

#endif