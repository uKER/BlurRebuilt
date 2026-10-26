#include "stdafx.h"
#include <EditorLib/Xt/XtVisualisationHelpers.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>

namespace
{
	class EvaluateFormatter : public EstFormatter
	{
		XtString _attr;
		XtString _param;

	public:
		EvaluateFormatter(const XtString& attr, const XtString& param) :
			_attr(attr),
			_param(param)
		{
		}

	public:
		virtual bool GetFormatByType(LtStruct* type, XtString& format)
		{
			if (LarvaParam* param = type->FindParam(_attr, _param))
			{
				format = param->AsString();
				return true;
			}

			return false;
		}

		virtual bool GetFormatByOption(LarvaOption* option, XtString& format)
		{
			if (LarvaParam* param = option->attrs.FindParam(_attr, _param))
			{
				format = param->AsString();
				return true;
			}

			return false;
		}
	};
}

namespace EditorLib
{
namespace XtVisualisationHelpers
{

XtString EvaluateSummaryText(const XtString& attr, const XtString& param, LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component)
{
	EvaluateFormatter fmtr(attr, param);

	XtString format;

	if (!fmtr.GetFormatByType(type, format))
		format = "{__type}";

	return EvaluateStructuredText(C_Core::Instance().GetXtManager().GetDb(), schema, type, component, format, &fmtr);
}

}	//namespace XtVisualisation
}	//namespace EditorLib