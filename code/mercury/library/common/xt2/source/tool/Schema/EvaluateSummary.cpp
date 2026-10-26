#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "Xt/Impl/Grimoire.h"

bool FindSummaryText(LtStruct* type, XtString& format)
{
	if (LarvaAttr* attr = type->FindAttr("Summary"))
	{
		format = attr->GetString("0");

		return true;
	}

	return false;
}

XtString CacheSummary(Xt::DomRecord* record, XtId id, const XtString& summary)
{
	if (record && id)
	{
		size_t first	= XtUt::FirstOf(&record->summaryCache, id);
		size_t last		= XtUt::LastOf(&record->summaryCache, id);

		if (first == last)
			XtUt::Insert(&record->summaryCache, id, new XtString(summary));
		else
			XtUt::Replace(&record->summaryCache, first, new XtString(summary));
	}

	return summary;
}

class GstFormatter : public EstFormatter
{
public:
	virtual bool GetFormatByType(LtStruct* type, XtString& format)
	{
		return FindSummaryText(type, format);
	}

	virtual bool GetFormatByOption(LarvaOption* option, XtString& format)
	{
		format = option->summary;
		return true;
	}

public:
	void CacheComponentText(Xt::DomRecord* record, XtId id, const char* text)
	{
		if (record && id)
			CacheSummary(record, id, text);
	}

	bool FindComponentText(Xt::DomRecord* record, XtId id, XtString& text)
	{
		if (record && id)
		{
			if (XtString* cache = record->summaryCache[id])
			{
				text = *cache;
				return true;
			}
		}

		return false;
	}
};

XtString EvaluateSummaryText(Xt::DomDb* db, LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, const XtString& defaultFormat)
{
	if (!schema || !type || !block)
		return kXtNullString;

	XtString format;
	
	if (!FindSummaryText(type, format))
		format = defaultFormat;

	return EvaluateStructuredText(db, schema, type, block, format, &GstFormatter());
}

XtString EvaluateSummaryText(Xt::DomDb* db, LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, const XtString& defaultFormat)
{
	if (!schema || !type || !component)
		return kXtNullString;

	XtString format;
	
	if (!FindSummaryText(type, format))
		format = defaultFormat;

	const XtString& summary	= EvaluateStructuredText(db, schema, type, component, format, &GstFormatter());

	return CacheSummary(RecordOf(component), component->id, summary);
}

XtString EvaluateSummaryText(Xt::DomDb* db, LarvaSchema* schema, LarvaField* field, Xt::DomElement* element)
{
	if (!schema || !field || !element)
		return kXtNullString;

	return EvaluateStructuredText(db, schema, field, element, &GstFormatter());
}