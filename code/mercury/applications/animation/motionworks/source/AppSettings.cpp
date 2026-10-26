#include "stdafx.h"
#include "AppSettings.h"

AppSettings g_AppSettings;

AppSettings::AppSettings()
{
	
}

AppSettings::~AppSettings()
{
}

void AppSettings::SetDefaults()
{
	m_clipBrowserRootDir = "E:\\testdata\\";
}

