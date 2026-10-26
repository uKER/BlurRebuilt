#pragma once

class AftkImport;
class AftkExport;

class AftkMayaPlugin : public MPxFileTranslator
{
public:
	AftkMayaPlugin();
	virtual ~AftkMayaPlugin();

	bool		haveReadMethod() const;
	bool		haveWriteMethod() const;
	MString		defaultExtension() const;
	MString		filter() const;

	MFileKind	identifyFile(
					const MFileObject& file,
					const char* buffer,
					short size
				) const;

	MStatus		reader(
					const MFileObject& file,
					const MString& options,
					FileAccessMode mode
				);

	MStatus		writer(
					const MFileObject& file,
					const MString& options,
					FileAccessMode mode
				);

	static void*	creator();
	static void		setPluginName(const MString& name);
	static MString	translatorName();
	const MString*	GetPluginName();

	void			SetExportOptionsMaxNGonNumVertices(U32 numVertices);

protected:
	void	GetAddAttrCmds(const MObject& node, MStringArray& cmds);
	void	GetSetAttrCmds(const MObject& node, MStringArray& cmds);

	void	WriteParent(
			std::fstream& f,
				const MDagPath& parent,
				const MDagPath& child,
				bool addIt
			);

	void	Clear();

	static MString	m_extension;
	static MString	m_fileVersion;
	static MString	m_pluginName;
	static MString	m_translatorName;
	static MString	m_filter;

private:
	MStatus AllocateFlags();
	MStatus ClearFlags();
	void ReleaseFlags();
	// These are used to keep track of connections which were made within
	// a referenced file but then broken by main scene file.
	MPlugArray		m_brokenConnSrcs;
	MPlugArray		m_brokenConnDests;

	// This is used to keep track of default nodes.
	MObjectArray	m_defaultNodes;

	// These are used to keep track of those DAG nodes which have multiple
	// instances.  'fInstanceParents' holds the first parent, which is
	// usually set up when the child is created.
	MDagPathArray	m_instanceChildren;
	MDagPathArray	m_instanceParents;

	// This is used to keep track of non-reference nodes with referenced
	// parents and referenced nodes with non-referenced parents, as their
	// parenting requires special handling.
	MDagPathArray	m_parentingRequired;

	U32				m_attrFlag;
	U32				m_createFlag;
	U32				m_connectionFlag;

	// Indicates whether the above flags have been registered with Maya
	bool			m_mayaFlagsAllocated;
};


