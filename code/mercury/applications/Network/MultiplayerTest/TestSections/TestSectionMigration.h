#ifndef TESTSECTIONMIGRATION_H
#define TESTSECTIONMIGRATION_H

#ifdef _MSC_VER
#pragma once
#endif


#include <TestSections/TestSection.h>
 
namespace TestMigration { class C_MigrationSession; }

class C_TestMigrationSection : public Framework::C_TestSection
{
public:
	C_TestMigrationSection(const Util::C_String& name);
	~C_TestMigrationSection();

	virtual void OnUpdate();
	virtual void OnRender2DView();

private:
	TestMigration::C_MigrationSession* m_session;
};


#endif //TESTSECTIONMIGRATION_H