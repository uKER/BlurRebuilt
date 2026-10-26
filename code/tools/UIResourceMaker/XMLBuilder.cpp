#include "stdafx.h"
#include "XMLBuilder.h"

#include "XMLParser.h"

#include "commandline.h"

#include "FontMaker.h"
#include "SystemCommands.h"
#include ".\uimaker2doc.h"

XMLBuilder gXMLBuilder;

bool XMLBuilder::BuildUI()
{
	gCommandLine.Clear();

	gXMLParser.ParseFile("BuildFile");
	
	gSystemCommands.AddCommand("pause");
	gSystemCommands.Execute();

	return true;
}