#ifndef TEST_NET_STATE_SECTION_H
#define TEST_NET_STATE_SECTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <list>
#include <TestSections/TestSection.h>

class C_TestStateSection : public Framework::C_TestSection
{
public:
	C_TestStateSection(const Util::C_String& name);
	~C_TestStateSection();

	virtual void OnUpdate();
	virtual void OnRender2DView();
private:
	void DebugDeltaCompression();
	void DebugStateManagerText();
	void DebugStateManager2d();

private:
	bool DropState(int uShots, int uProbability);
	bool Migrating();
	bool AllMigrating();
	int ServerId();
};

#endif // TEST_NET_STATE_SECTION_H
