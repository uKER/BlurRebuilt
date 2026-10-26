#ifndef __GLOBALS_H
#define __GLOBALS_H

#ifdef _MSC_VER
#pragma once
#endif


namespace airacingtech
{
	class CObjectManager;

	namespace GlobalsComponent
	{
		CObjectManager			&GetObjectManager();
	}
}

#endif //__GLOBALS_H
