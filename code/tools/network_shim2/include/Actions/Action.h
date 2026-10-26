#pragma once

class C_Options;

class C_Command
{
public:
	C_Command(const Sys::C_XmlDomBaseNode& dom);

	const Sys::C_XmlDomBaseNode& GetDom();
	
	void SetStatus(bool status);
	bool GetStatus() const;

	Util::C_String GetFullNodePath() const;
	static Util::C_String GetFullNodePath(const Sys::C_XmlDomBaseNode& dom);
	
private:
	static void GetFullNodePath(Util::C_String& nodePath, const Sys::C_XmlDomBaseNode& dom);
	static void GetFullNodePathParentData(Util::C_String& nodePath, const Sys::C_XmlDomBaseNode& dom);

	const Sys::C_XmlDomBaseNode& m_Dom;
	bool m_Status;
};

class C_Action
{
public:
	C_Action(const C_Options& options);

	bool Execute(const Sys::C_XmlDomBaseNode& dom);
	void PrintStatus();

private:
	bool ExecuteAction(const Sys::C_XmlDomBaseNode& dom);
	bool ExecuteAction(const Sys::C_XmlDomBaseNode& dom, Btl::String action);
	bool ExecuteBatch(const Sys::C_XmlDomBaseNode& dom);
	bool ExecuteCommand(C_Command& dom);

	bool ExecuteAssetProcess(const Sys::C_XmlDomBaseNode& dom, Ut::Vector<Util::C_String>& assets);
	bool ExecuteAftkConversion(const Sys::C_XmlDomBaseNode& dom, const Sys::C_XmlDomBaseNode& assetTag, Ut::Vector<Util::C_String>& assets);
	bool ExecuteCommandConversion(const Sys::C_XmlDomBaseNode& dom, const Sys::C_XmlDomBaseNode& assetTag, Ut::Vector<Util::C_String>& assets);

	bool ProcessCommandAsset(const Sys::C_XmlDomBaseNode& dom, const Util::C_String& command, const Util::C_String& arguments);

	bool GenerateParentData(const Sys::C_XmlDomBaseNode& dom, Sys::C_XmlDomBaseNode* node);
	bool GenerateTempXmlFilename(Util::C_String& filename);
	bool GenerateTempXml(const Sys::C_XmlDomBaseNode& dom, Util::C_String& tempXml, Util::C_String& newNodeName, const Sys::C_XmlDomElement* additionalData=0);

	bool CheckPathHash(const Util::C_String& path, const Util::C_String& hash);
	
	Btl::Map<Util::C_String, Util::C_String> m_Hashes;
	Btl::Vector<C_Command> m_Commands;
	const C_Options& m_Options;
};
