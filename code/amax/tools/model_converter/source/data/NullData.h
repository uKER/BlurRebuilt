#ifndef APP_NULL_DATA_H
#define APP_NULL_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

class C_DataNull : public Tools::ProcessGraph::C_Data
{
public:
	C_DataNull( const Util::C_String& name );

	// inherited from C_Data
	const Util::C_String GetName() const;

private:
	Util::C_String m_Name;
};

#endif
