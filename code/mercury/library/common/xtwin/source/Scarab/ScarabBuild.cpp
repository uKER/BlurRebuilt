#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

struct LvBuildField : public LvField
{
	ScarabNode* _parentNode;

public:
	LvBuildField(ScarabNode* parentNode) :
		_parentNode(parentNode)
	{
	}

public:
	virtual bool Visit(LfStruct* field)		{ return SetupFieldNode(new SnStruct(field), field);		}
	virtual bool Visit(LfInt* field)		{ return SetupFieldNode(new SnInt(field), field);			}
	virtual bool Visit(LfFloat* field)		{ return SetupFieldNode(new SnFloat(field), field);			}
	virtual bool Visit(LfString* field)		{ return SetupFieldNode(new SnString(field), field);		}
	virtual bool Visit(LfWideString* field)	{ return SetupFieldNode(new SnWideString(field), field);	}
	virtual bool Visit(LfBool* field)		{ return SetupFieldNode(new SnBool(field), field);			}
	virtual bool Visit(LfEnum* field)		{ return SetupFieldNode(new SnEnum(field), field);			}
	virtual bool Visit(LfFlags* field)		{ return SetupFieldNode(new SnFlags(field), field);			}
	virtual bool Visit(LfStrictPtr* field)	{ return SetupFieldNode(new SnStrictPtr(field), field);		}
	virtual bool Visit(LfPtr* field)		{ return SetupFieldNode(new SnPtr(field), field);			}
	virtual bool Visit(LfWeakPtr* field)	{ return SetupFieldNode(new SnWeakPtr(field), field);		}
	virtual bool Visit(LfHandle* field)		{ return SetupFieldNode(new SnHandle(field), field);		}
	virtual bool Visit(LfLocString* field)	{ return SetupFieldNode(new SnLocString(field), field);		}

	virtual bool Visit(LfArray* field)
	{
		ScarabNode* node;

		if (field->attrs.FindAttr("FlatArray"))			node = new SnFlatArray(field);
		else											node = new SnArray(field);

		return SetupFieldNode(node, field);
	}

	virtual bool Visit(LfControl* field)
	{
		SnControl* node = 0;

		if (field->type == "ColourSwatch")			node = new SnColourSwatch(field);
		else if (field->type == "Chart")			node = new SnChart(field);
		else if (field->type == "Graph")			node = new SnChart(field);
		else if (field->type == "GothamWeather")	node = new SnGothamWeather(field);
		else if (field->type == "NodeGraph")		node = new SnNodeGraph(field);
		else if (field->type == "Info")				node = new SnInfo(field);
		else if (field->type == "Sequencer")		node = new SnSequencer(field);
		else if (field->type == "UserRef")			node = new SnUserRef(field);
		else if (field->type == "DopeSheet")		node = new SnDopeSheet(field);
		else if (field->type == "Gradient")			node = new SnGradient(field);
		else if (field->type == "PropertySheet")	node = new SnPropertySheet(field);
		else if (field->type == "GridView")			node = new SnGridView(field);
		else										node = new SnControl(field);

		return SetupFieldNode(node, field);
	}

public:
	XtString MakeLabel(LarvaField* field)
	{
		if (field->noLabelColon || !*field->label)
			return field->label;

		return field->label + ":";
	}

	bool SetupFieldNode(ScarabNode* node, LarvaField* field)
	{
		if (!_parentNode->Add(node))
			return false;

		node->_labelText = MakeLabel(field);

		return true;
	}
};

bool ScarabBuild(ScarabNode* parentNode, LarvaField* field)
{
	return parentNode && field && field->Accept(&LvBuildField(parentNode));
}

namespace
{
	bool PopulateGroups(XtUt::Set<XtString>* groups, LtStruct* type, bool doFilter, const XtString& filter)
	{
		if (type->base)
		{
			if (!type->base->hidden)
			{
				if (!PopulateGroups(groups, type->base->type, doFilter, filter))
					return false;
			}
		}

		for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
		{
			if (it->hidden)
				continue;

			if (doFilter && (it->tab != filter))
				continue;

			if (!XtUt::Exists(groups, it->group))
			{
				if (!XtUt::Insert(groups, it->group))
					return false;
			}
		}

		return true;
	}
}

bool ScarabBuild(ScarabNode* parentNode, LtStruct* type, bool doFilter, const XtString& filter, const XtString& group)
{
	if (!parentNode || !type)
		return false;

	if (type->base)
	{
		if (!type->base->hidden)
		{
			if (!ScarabBuild(parentNode, type->base->type, doFilter, filter, group))
				return false;
		}
	}

	for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
	{
		if (it->hidden)
			continue;

		if (doFilter && (it->tab != filter))
			continue;

		if (it->group != group)
			continue;

		if (!ScarabBuild(parentNode, it))
			return false;
	}

	return true;
}

bool IsGroupCollapsedByDefaultSub(LtStruct* type, const char* groupName, bool* result)
{
	for(XtUt::List<LarvaAttr>::Iterator it(&type->attrs.attrs); it; ++it)
	{
		if (it->name != "Group")
			continue;

		if (it->GetString("name") != groupName)
			continue;

		*result = it->GetBool("collapsed", false);

		return true;
	}

	if (type->base)
	{
		if (IsGroupCollapsedByDefaultSub(type->base->type, groupName, result))
			return true;
	}

	return false;
}

bool IsGroupCollapsedByDefault(LtStruct* type, const char* groupName)
{
	bool result = false;

	if (!IsGroupCollapsedByDefaultSub(type, groupName, &result))
		return false;

	return result;
}

bool GroupOrderContains(XtUt::Array<XtString>* order, const char* name)
{
	for(XtUt::Array<XtString>::Iterator it(order); it; ++it)
	{
		if (*it == name)
			return true;
	}

	return false;
}

void PopulateGroupOrder(XtUt::Array<XtString>* order, XtUt::Set<XtString>* groups, LtStruct* type)
{
	for(XtUt::List<LarvaAttr>::Iterator it(&type->attrs.attrs); it; ++it)
	{
		if (it->name != "Group")
			continue;

		XtString name = it->GetString("name");

		if (!Exists(groups, name))
			continue;

		if (GroupOrderContains(order, name))
			continue;

		XtUt::AppendEx(order, name);
	}

	if (type->base)
		PopulateGroupOrder(order, groups, type->base->type);
}

XtU32 GetGroupColourSub(LtStruct* type, const char* name)
{
	for(XtUt::List<LarvaAttr>::Iterator it(&type->attrs.attrs); it; ++it)
	{
		if (it->name != "Group")
			continue;

		if (it->GetString("name") != name)
			continue;

		if (XtU32 colour = it->GetColour("colour", 0))
			return colour;
	}

	if (type->base)
	{
		if (XtU32 colour = GetGroupColourSub(type->base->type, name))
			return colour;
	}

	return 0;
}

XtU32 GetGroupColour(LtStruct* type, const char* name)
{
	if (XtU32 colour = GetGroupColourSub(type, name))
		return ((colour & 0xFF0000) >> 16) | (colour & 0x00FF00) | ((colour & 0x0000FF) << 16);

	if (!*name)
		return RGB(192, 192, 192);

	XtU32 rgb = 0;

	for(const char* p = name; *p != '\0'; p++)
		rgb = (rgb << 13) ^ (rgb >> 13) + (rgb << 7) + 7919 + *p;

	int r = Clamp<int>(80 + ((rgb & 0xFF) / 2), 0, 255);
	int g = Clamp<int>(80 + (((rgb & 0xFF00) >> 8) / 2), 0, 255);
	int b = Clamp<int>(80 + (((rgb & 0xFF0000) >> 16) / 2), 0, 255);

	NgColour c(r / 255.0f, g / 255.0f, b / 255.0f);

	c = RgbToHsv(c);

	c.g = 0.5f;
	c.b = 0.9f;

	c = HsvToRgb(c);

	r = Clamp((int)(c.r * 255), 0, 255);
	g = Clamp((int)(c.g * 255), 0, 255);
	b = Clamp((int)(c.b * 255), 0, 255);

	return RGB(r, g, b);
}

bool ScarabBuild(ScarabNode* parentNode, LtStruct* type, bool doFilter, const XtString& filter)
{
	if (!parentNode || !type)
		return false;

	// Look for groups

	XtUt::Set<XtString> groups;

	if (!PopulateGroups(&groups, type, doFilter, filter))
		return false;

	// Build fields

	if (Count(&groups) > 1)
	{
		XtUt::Array<XtString> order;

		PopulateGroupOrder(&order, &groups, type);

		for(XtUt::Set<XtString>::Iterator itGroup(&groups); itGroup; ++itGroup)
		{
			if (GroupOrderContains(&order, *itGroup))
				continue;

			XtUt::AppendEx(&order, *itGroup);
		}

		for(XtUt::Array<XtString>::Iterator itGroup(&order); itGroup; ++itGroup)
		{
			SnGroup* groupNode = new SnGroup(parentNode->Panel()->Form(), itGroup, IsGroupCollapsedByDefault(type, *itGroup), GetGroupColour(type, *itGroup));

			if (!parentNode->Add(groupNode))
				return false;

			ScarabNode* groupChild = groupNode->Body();

			if (!groupChild)
				return false;

			if (!ScarabBuild(groupChild, type, doFilter, filter, itGroup))
				return false;
		}
	}
	else
	{
		for(XtUt::Set<XtString>::Iterator itGroup(&groups); itGroup; ++itGroup)
		{
			if (!ScarabBuild(parentNode, type, doFilter, filter, itGroup))
				return false;
		}
	}

	return true;
}