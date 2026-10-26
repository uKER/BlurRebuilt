// filetypes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

Util::C_String g_IconDirectory;

void ProcessXMLAction(Registry::RegKey& shell_key, Sys::C_XmlDomBaseNode* pXMLAction)
{
	Util::C_String name;
	Util::C_String application;

	const Sys::C_XmlAttribute* pNameAttribute = ((Sys::C_XmlDomElement*)pXMLAction)->GetAttributes().GetAttributeByName("Name");
	if (pNameAttribute)
		name = pNameAttribute->value;

	const Sys::C_XmlAttribute* pApplicationAttribute =((Sys::C_XmlDomElement*)pXMLAction)->GetAttributes().GetAttributeByName("Application");
	if (pApplicationAttribute)
		application = pApplicationAttribute->value;

	//Replace ' -> "
	application = application.Replace('\'','\"');

	//Write to the register
	
	Registry::RegKey open_key(shell_key.GetKey(), name.NonConstChars());
	Registry::RegKey command_key(open_key.GetKey(), "command");

	if( strlen(application.Chars()) > 0 )
	{
		//printf("   Adding action '%s' with command '%s'\n", name.Chars(), application.Chars());
		command_key.SetStringValue(NULL, application.NonConstChars());
	}
}

void ProcessXMLItem(Sys::C_XmlDomBaseNode* pXMLItem)
{
	Util::C_String extension;
	Util::C_String fileTypeKey;
	Util::C_String description;
	Util::C_String icon;
	Util::C_String defaultAction;
	
	//Read information
	const Sys::C_XmlAttribute* pExtensionAttribute = ((Sys::C_XmlDomElement*)pXMLItem)->GetAttributes().GetAttributeByName("Extension");
	if (pExtensionAttribute)
		extension = pExtensionAttribute->value;

	const Sys::C_XmlAttribute* pFileTypeKeyAttribute = ((Sys::C_XmlDomElement*)pXMLItem)->GetAttributes().GetAttributeByName("FileTypeKey");
	if (pFileTypeKeyAttribute)
		fileTypeKey = pFileTypeKeyAttribute->value;
	
	const Sys::C_XmlAttribute* pDescriptionAttribute = ((Sys::C_XmlDomElement*)pXMLItem)->GetAttributes().GetAttributeByName("Description");
	if (pDescriptionAttribute)
		description = pDescriptionAttribute->value;

	const Sys::C_XmlAttribute* pIconAttribute = ((Sys::C_XmlDomElement*)pXMLItem)->GetAttributes().GetAttributeByName("Icon");
	if (pIconAttribute)
		icon = pIconAttribute->value;

	const Sys::C_XmlAttribute* pDefaultActionAttribute =  ((Sys::C_XmlDomElement*)pXMLItem)->GetAttributes().GetAttributeByName("DefaultAction");
	if (pDefaultActionAttribute)
		defaultAction = pDefaultActionAttribute->value;

	//Write to register
	{

		Util::C_String fullIconName;
		fullIconName << g_IconDirectory << icon;

		if(fileTypeKey.Length() == 0)
		{
			printf("Error %s file : has no [FileTypeKey]\n", extension );
			return;
		}

		// Add File Extension
		printf("Registering File Extension '%s'\n", extension.Chars() );

		Registry::RegKey file_extension_key(HKEY_CLASSES_ROOT, (CHAR*)extension.Chars() );
		file_extension_key.SetStringValue(NULL, (CHAR*)fileTypeKey.Chars());

		// Add File Type
		//printf("Registering File Type '%s'\n", fileTypeKey.Chars());

		Registry::RegKey file_type_key(HKEY_CLASSES_ROOT, (CHAR*)fileTypeKey.Chars());
		file_type_key.SetDwordValue("BrowserFlags", 0x0008);
		file_type_key.SetDwordValue("EditFlags", 0x0000);

		// File description
		if(strlen(description.Chars()))
		{
			//printf("Registering File description '%s'\n", description.Chars() );
			file_type_key.SetStringValue(NULL, (CHAR*)description.Chars());
		}
		else
		{
			file_type_key.DeleteValue(NULL);
		}

		file_type_key.DeleteSubkey("DefaultIcon", false);
		if(fullIconName.Length())
		{
			// Setup default Icon
			//printf("   Setting up Icon '%s'\n", fullIconName.Chars() );
			Registry::RegKey default_icon_key(file_type_key.GetKey(), "DefaultIcon");
			default_icon_key.SetStringValue( NULL, (CHAR*)fullIconName.Chars() );
		}
		
		// Clear out previous existance of the shell key
		file_type_key.DeleteSubkey("shell", true);

		// Add new shell key
		//printf("   Adding Shell Key\n");
		Registry::RegKey shell_key(file_type_key.GetKey(), "shell");

		//Process Actions
		for (S32 i = 0; i < pXMLItem->GetChildren().Size(); i++)
			ProcessXMLAction(shell_key, pXMLItem->GetChildren()[i]);

		//Write default
		shell_key.SetStringValue( NULL, (CHAR*)defaultAction.Chars() );

	}
}

void ProcessXMLFile(const char* fileName)
{
	//Read
	Sys::C_XmlDom dom;
	dom.Read(fileName);

	Sys::C_XmlDomBaseNode* pRootNode = dom.GetRootNodePtr();

	if (dom.GetRootNodePtr()->GetChildren().Size() > 0)
	{
		Sys::C_XmlDomBaseNode* pRootNode = dom.GetRootNodePtr()->GetChildren()[0];

		g_IconDirectory = ((Sys::C_XmlDomElement*)pRootNode)->GetAttributes().GetAttributeByName("IconDirectory")->value;

		//Process each item
		const Sys::C_XmlDomBaseNode::Array& itemArray = pRootNode->GetChildren();

		for (S32 i = 0; i < itemArray.Size(); i++)
		{
			ProcessXMLItem(itemArray[i]);
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("FileType <file");
	}
	else
	{
		ProcessXMLFile(argv[1]);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
	}

	return 0;
}
