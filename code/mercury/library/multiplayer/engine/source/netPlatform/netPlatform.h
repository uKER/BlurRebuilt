#ifndef NETPLATFORM_H
#define NETPLATFORM_H

#ifdef _MSC_VER
#pragma once
#endif

namespace netPlatform
{
	class Platform
	{
	public:
		static void Startup();
		static void Destroy();
		static void InitialiseVoice();

	};
}

#endif //NETPLATFORM_H