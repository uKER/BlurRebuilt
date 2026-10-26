#include "stdafx.h"
#include <converter_shared/config/ABFolderListConfig.h>
#include <system/serialise/FilePath.h>
									  
namespace ABFolderList
{
	Ut::UtDynamicVector<Util::C_String> C_Config::m_abFolderArray;

	Util::C_String GetLeft(const Util::C_String& string, int s)
	{
		return string(0, s);
	}

	Util::C_String GetRight(const Util::C_String& string, int s)
	{
		S32 lo = string.Length() - s;
		S32 hi = lo + s;
		return string(lo, hi);
	}

	void C_Config::ReadABFolders(const Sys::C_XmlDom& dom)
	{
		m_abFolderArray.Clear();

		Ut::Vector<const Sys::C_XmlDomBaseNode*> abNodes;
		Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "**/container[@shim_tag=\"shim_ab_folders\"]", abNodes);

		for (int i = 0; i < abNodes.Size(); i++)
		{
			Util::C_String text = ((Sys::C_XmlDomElement*)abNodes[i])->GetAttributes().GetAttributeByName("value")->value;

			//Here we go with some good old fashioned parsing code.
			while(true)
			{
				int offset;

				if ((offset = text.Search("&#xA")) != -1)
				{
					Util::C_String path = GetLeft(text.Chars(), offset);

					text = GetRight(text.Chars(), text.Length() - offset - 4);

					if (text.Chars()[0] == ';')
					{
						text = GetRight(text.Chars(), text.Length() - 1);
					}

					if (path.Length() > 0)
						m_abFolderArray.PushBack(path);
				}
				else
				{
					if (text.Length() > 0)
						AddToFolderList(text);

					//We've run out of delimiters and reached the end of our folder list.
					break;
				}
			}
		}
	}

	void C_Config::AddToFolderList(const Util::C_String& folder)
	{
		m_abFolderArray.PushBack(folder);
	}

	bool C_Config::IsFileInABFolderList(const Util::C_String& file)
	{
		//otao: HACK for tools not launched directly by executer (eg vis) to prevent message spam
		if (!m_abFolderArray.Size())
			return true;

		bool inABFolder = false;
		for (int i=0; i < m_abFolderArray.Size(); i++)
		{
			Sys::C_FilePath abFolder( Sys::ExtractDirectory(m_abFolderArray[i]) );
			Sys::C_FilePath fileFolder( Sys::ExtractDirectory(file) );
			if(fileFolder.Search(abFolder) >= 0)
			{
				inABFolder = true; 
				break;
			}
		}

		return inABFolder;
	}

} //namespace ABFolderList