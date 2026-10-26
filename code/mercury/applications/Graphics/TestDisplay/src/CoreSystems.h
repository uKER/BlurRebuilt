#ifndef CORE_SYSTEMS_H
#define CORE_SYSTEMS_H

#ifdef _MSC_VER
#pragma once
#endif

struct C_CoreSystems
{
	static bool Initialise();
	static void Shutdown();
	static void Update();

	static bool StartupArtery();
	static void ShutdownArtery();
};

#endif // CORE_SYSTEMS_H
