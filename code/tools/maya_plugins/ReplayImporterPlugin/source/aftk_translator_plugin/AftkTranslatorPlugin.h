#pragma once

class AftkImport;
class AftkExport;

#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>

class AftkMayaPlugin : public MPxCommand
{
public:
	AftkMayaPlugin();
	virtual ~AftkMayaPlugin();

	MStatus			doIt(const MArgList& args);
	MStatus			redoIt();
	MStatus			undoIt();
	bool			isUndoable() const;

	static void*	creator();
	static void		setPluginName(const MString& name);
//	static MString	translatorName();
//	const MString*	GetPluginName();

protected:
	static MString	m_extension;
	static MString	m_fileVersion;
	static MString	m_pluginName;
	static MString	m_translatorName;
	static MString	m_filter;

private:
};


