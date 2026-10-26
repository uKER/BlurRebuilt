#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

//
// SnChart::Line
//

SnChart::Line::Line() :
	colour(RGB(0xFF, 0x88, 0x44)),
	xMin(0.0f),
	xMax(0.0f),
	yMin(0.0f),
	yMax(0.0f),
	generatedX(false),
	generatedY(false),
	hermite(false),
	isTemplate(false)
{
}

//
// SnChartData
//

SnChartData::SnChartData(SnChart* sn) :
	_sn(sn)
{
}

int SnChartData::GetLineCount()
{
	return (int) _sn->_line.count;
}

void SnChartData::GetLineInfo(int lineNo, ChartLine* line)
{
	SnChart::Line* snLine = _sn->_line[lineNo];

	if (Xt::DomBlock* block = AsBlock(_sn->_dom))
	{
		if (Xt::DomArray* array = AsArray(FindElementByPath(block, snLine->path)))
		{
			line->count = (int)Count(array);
		}
		else
			line->count = 0;
	}
	else
		line->count = 0;

	line->colour = snLine->colour;

	line->isHermite		= snLine->hermite;
	line->isTemplate	= snLine->isTemplate;

	line->xMin			= snLine->xMin;
	line->xMax			= snLine->xMax;
	line->yMin			= snLine->yMin;
	line->yMax			= snLine->yMax;
	line->generatedX	= snLine->generatedX;
	line->generatedY	= snLine->generatedY;
}

void SnChartData::GetLineValue(int lineNo, int pointNo, ChartPoint* point)
{
	SnChart::Line* line = _sn->_line[lineNo];

	if (line->generatedX)
	{
		point->x = (float)pointNo;
	}
	else
	{
		if (Xt::DomField* field = _sn->GetLineX(line, pointNo))
			point->x = atof(AsText(field));
	}

	if (line->generatedY)
	{
		point->y = (float)pointNo;
	}
	else
	{
		if (Xt::DomField* field = _sn->GetLineY(line, pointNo))
			point->y = atof(AsText(field));
	}
}

void SnChartData::SetLineValue(int lineNo, int pointNo, const ChartPoint* point)
{
	SnChart::Line* line = _sn->_line[lineNo];

	if (!line->generatedX)
	{
		if (Xt::DomField* field = _sn->GetLineX(line, pointNo))
		{
			XtString text;

			text = Format("%f", point->x);

			Set(field, text);
		}
	}

	if (!line->generatedY)
	{
		if (Xt::DomField* field = _sn->GetLineY(line, pointNo))
		{
			XtString text;

			text = Format("%f", point->y);

			Set(field, text);
		}
	}
}

void SnChartData::GetInTangentValue(int lineNo, int pointNo, double *in)
{
	SnChart::Line* line = _sn->_line[lineNo];

	if (Xt::DomField* field = _sn->GetInTangent(line, pointNo))
		*in = atof(AsText(field));
}

void SnChartData::GetOutTangentValue(int lineNo, int pointNo, double *out)
{
	SnChart::Line* line = _sn->_line[lineNo];

	if (Xt::DomField* field = _sn->GetOutTangent(line, pointNo))
		*out = atof(AsText(field));
}

void SnChartData::SetInTangentValue(int lineNo, int pointNo, double in)
{
	SnChart::Line* line = _sn->_line[lineNo];

	if (Xt::DomField* field = _sn->GetInTangent(line, pointNo))
	{
		XtString text = Format("%f", in);

		Set(field, text);
	}
}

void SnChartData::SetOutTangentValue(int lineNo, int pointNo, double out)
{
	SnChart::Line* line = _sn->_line[lineNo];

	if (Xt::DomField* field = _sn->GetOutTangent(line, pointNo))
	{
		XtString text = Format("%f", out);
		Set(field, text);
	}
}


//
// SnChart
//

SnChart::SnChart(LfControl* field) :
	SnControl(field),
	_idChart(this),
	_gd(this),
	_xSize(400),
	_ySize(300)
{
	_dom.SetCallback(DomCallback, this);

	for(XtUt::List<LarvaAttr>::Iterator it(&_field->attrs.attrs); it; ++it)
	{
		if (it->name == "Data")
		{
			Line* ln = new Line();

			LarvaAttr* attr = it;

			ln->path		= attr->GetString("path", ln->path);
			ln->colour		= attr->GetColour("colour", ln->colour);
			ln->xField		= attr->GetString("xField", ln->xField);
			ln->yField		= attr->GetString("yField", ln->yField);
			ln->xMin		= attr->GetFloat("xMin", ln->xMin);
			ln->xMax		= attr->GetFloat("xMax", ln->xMax);
			ln->yMin		= attr->GetFloat("yMin", ln->yMin);
			ln->yMax		= attr->GetFloat("yMax", ln->yMax);
			ln->isTemplate	= attr->GetBool("template", false);

			if (LarvaParam* param = FindParam(attr, "tanIn"))
			{
				ln->tanIn = param->AsString();
				ln->hermite = true;
			}

			if (LarvaParam* param = FindParam(attr, "tanOut"))
			{
				ln->tanOut = param->AsString();
				ln->hermite = true;
			}

			_xSize = (int)attr->GetInt("xSize", _xSize);
			_ySize = (int)attr->GetInt("ySize", _ySize);

			if (!*ln->xField)
				ln->generatedX = true;
			else if (ln->xField == "__value")
				ln->xField = "";

			if (!*ln->yField)
				ln->generatedY = true;
			else if (ln->yField == "__value")
				ln->yField = "";

			AppendEx(&_line, ln);
		}
	}
}

SnChart::~SnChart()
{
}

void SnChart::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnChart::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnChart::OnBuild()
{
	if (!_idChart.Allocate())
		return false;

	if ((HWND)_wndChart == 0)
	{
		if (!_wndChart.Create(*Panel(), (HMENU)(intptr_t)_idChart))
			return false;

		_wndChart.SetCallbacks(&_gd);
	}

	return true;
}

void SnChart::OnMeasure(HDC ic)
{
	_size.cx = _xSize;
	_size.cy = _ySize;
}

void SnChart::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndChart, _client.left, _client.top, _client.Width(), _client.Height(), SWP_SHOWWINDOW);
}

void SnChart::SetDom(Xt::DomBlock* block)
{
	SetEnabled(Monitor(&_dom, block) != 0);

	SyncControls();
}

void SnChart::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnChart* node)
{
	if ((event->from == monitor->element) && (event->code == Xt::kDomRemoved))
		node->SetDom(0);
	else if (*monitor == event->from)
		node->SyncControls();
	else if (node->_wndChart != NULL)
		InvalidateWindow(node->_wndChart);
}

void SnChart::SyncControls()
{
	for (XtUt::List<Line>::Iterator it(&_line); it; ++it)
	{
		Line& ln = *it;

		SetupLine(&ln);

		_wndChart.SetXExtents(ln.xMin, ln.xMax);
		_wndChart.SetYExtents(ln.yMin, ln.yMax);
	}

	if (_wndChart != NULL)
		InvalidateWindow(_wndChart);
}

void SnChart::SetupLine(Line* line)
{
	Xt::DomBlock* block = AsBlock(_dom);

	if (!block)
		return;

	Xt::DomArray* array = AsArray(FindElementByPath(block, line->path));

	if (!array)
		return;

	line->dom.SetCallback(DomCallback, this);
	Monitor(&line->dom, array);

	if (line->generatedX)
	{
		line->xMin = 0;
		line->xMax = (float)Count(array) - 1.0f;
	}

	if (line->generatedY)
	{
		line->yMin = 0;
		line->yMax = (float)Count(array) - 1.0f;
	}
}

Xt::DomField* SnChart::GetLineX(SnChart::Line* line, int pointNo)
{
	Xt::DomArray* array = AsArray(line->dom);

	if (!array)
		return 0;

	if (!*line->xField)
		return AsField(At(array, pointNo));

	Xt::DomComponent* block = AcquireComponent(array, pointNo);

	return AsField(FindElementByPath(block ? &block->data : 0, line->xField));
}

Xt::DomField* SnChart::GetLineY(SnChart::Line* line, int pointNo)
{
	Xt::DomArray* array = AsArray(line->dom);

	if (!array)
		return 0;

	if (!*line->yField)
		return AsField(At(array, pointNo));

	Xt::DomComponent* block = AcquireComponent(array, pointNo);

	return AsField(FindElementByPath(block ? &block->data : 0, line->yField));
}

Xt::DomField* SnChart::GetInTangent(SnChart::Line* line, int pointNo)
{
	Xt::DomArray* array = AsArray(line->dom);
	if (!array)
		return 0;

	if (!*line->tanIn)
		return AsField(At(array, pointNo));

	Xt::DomComponent* block = AcquireComponent(array, pointNo);

	return AcquireField(block ? &block->data : 0, line->tanIn);
}

Xt::DomField* SnChart::GetOutTangent(SnChart::Line* line, int pointNo)
{
	Xt::DomArray* array = AsArray(line->dom);
	if (!array)
		return 0;

	if (!*line->tanOut)
		return AsField(At(array, pointNo));

	Xt::DomComponent* block = AcquireComponent(array, pointNo);

	return AcquireField(block ? &block->data : 0, line->tanOut);
}
