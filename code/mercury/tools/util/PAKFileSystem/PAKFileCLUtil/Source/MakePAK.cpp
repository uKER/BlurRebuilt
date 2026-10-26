// PAKFileCLUtil.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MakePAK.h"


struct DirContext{
	Sys::C_DirectoryInfo dirinfo;
	U32 position;
};

C_MakePAK::C_MakePAK( Debug::Logger& log )
: m_Log(log)
{
	fp = NULL;
	SetEndianSwap( true );
}

C_MakePAK::~C_MakePAK()
{
	if (fp) fclose(fp);
}

void C_MakePAK::SetEndianSwap( bool swap )
{
	mEndianSwap = swap;
}

void C_MakePAK::CreatePAKFileList()
{
	Util::C_String paklist = mPakRoot + "\\" + "paklist.txt";
	fp = fopen( paklist.Chars(), "w" );
}

void C_MakePAK::SetPakRoot( Util::C_String root )
{
	mPakRoot = root;
}

void C_MakePAK::AddPakToList( Util::C_String filename )
{
	if (fp)
	{
		char x = 10;
		Util::C_String item( filename( mPakRoot.Length(), filename.Length() ) + ".pak" );
		fwrite( item.Chars(), item.Length(), 1, fp );
		fwrite( &x, 1, 1, fp );
	}
}

bool C_MakePAK::CreatePAKFromFolder( 
	Util::C_String folder, 
	Util::C_String pakMountPoint,
	bool compress, 
	U32 sectorSize,
	Util::C_String excludeList )
{
	Util::C_String pakFileName = folder + ".pak";

	C_PAKFileSystem pakFileSystem;
	gPAK_EndianSwapHeaderInfo = mEndianSwap;

	//Delete old file
	if (Sys::C_FileSystem::Instance().FileExists(pakFileName))
		Sys::C_FileSystem::Instance().DeleteFile(pakFileName);



	if (mOrderedFiles.Size())
	{

		Util::C_String fixedFolderName = folder + "\\";
		Sys::C_DirectoryInfo dirinfo;
		if (!Sys::C_FileSystem::Instance().DirectoryInfo(fixedFolderName, &dirinfo))
		{
			LOGERROR(m_Log, "Folder does not exist %s\n", folder.Chars());
			return false;
		}

		Ut::OpenHashSet<std::string, Ut::HashKeyOpString> uniqueFiles;

		//we've got an ordered list of files to pak as well as the folder
		Ut::Vector<Util::C_String> newOrderedFiles;
		for (S32 i=0; i<mOrderedFiles.Size(); ++i)
		{
			if (mOrderedFiles[i].ISearch(folder) != -1)
			{
				Sys::C_Fileinfo fileinfo;
				if (Sys::C_FileSystem::Instance().FileInfo(mOrderedFiles[i], &fileinfo))
				{
					newOrderedFiles.PushBack(fileinfo.m_FileName);

					Ut::OpenHashSet<std::string, Ut::HashKeyOpString>::Iterator it = uniqueFiles.Find(mOrderedFiles[i].Lower().Chars());
					assert(it == uniqueFiles.End());
					uniqueFiles.Insert(fileinfo.m_FileName.Lower().Chars());
				}
				else
					LOGWARN(m_Log,"File does not exist %s\n", mOrderedFiles[i].Chars());
			}
			else
			{
				LOGWARN(m_Log,"Ignoring file: %s is not in the folder %s\n", mOrderedFiles[i].Chars());
			}
		}

		//now we need to recurse through the folders picking up any files that haven't been listed

		DirContext current;
		current.dirinfo = dirinfo;
		current.position = 0;

		Ut::Vector<DirContext> stack;
		stack.PushBack(current);

		while (stack.Size())
		{
			U32 i = stack.Size()-1;
			if (stack[i].position >= stack[i].dirinfo.NumFiles())
			{
				stack.PopBack();
				continue;
			}

			Sys::C_Fileinfo info = stack[i].dirinfo.FileInfo(stack[i].position);
			if (info.m_FileType == Sys::E_FileType_Directory)
			{
				Sys::C_DirectoryInfo newDirInfo;
				if (Sys::C_FileSystem::Instance().DirectoryInfo(info.m_FileName, &newDirInfo))
				{
					stack.PushBack();
					stack.Back().position = 0;
					stack.Back().dirinfo = newDirInfo;
				}
			}
			else
			{

				Ut::OpenHashSet<std::string, Ut::HashKeyOpString>::Iterator it = uniqueFiles.Find(info.m_FileName.Lower().Chars());
				if (it == uniqueFiles.End())
				{
					newOrderedFiles.PushBack(info.m_FileName);
					uniqueFiles.Insert(info.m_FileName.Lower().Chars());
				}
			}
			stack[i].position++;
		}

		Util::C_String pakFileName = folder + ".pak";

		if (!pakFileSystem.CreatePAKFromFileList(
			Sys::C_FileSystem::Instance(),
			pakFileName, 
			newOrderedFiles,
			compress, 
			sectorSize,
			false, 
			folder))
		{
			LOGERROR(m_Log,"FAILED TO MAKE PAK %s\n", pakFileName.Chars());
			return false;
		}

	}
	else if (!pakFileSystem.CreatePAKFromFolder(
		Sys::C_FileSystem::Instance(),
		folder,
		excludeList,
		compress, 
		sectorSize,
		false) )
		{
			LOGERROR(m_Log,"FAILED TO MAKE PAK %s\n", pakFileName.Chars());
			return false;
		}

	if (!pakFileSystem.VerifyPAK(
		Sys::C_FileSystem::Instance(),
		folder, 
		excludeList))
		return false;
	LOGINFO(m_Log,"Verified %s ok.\n", pakFileName.Chars());

	if (pakMountPoint.Length() != 0)
	{
		//Set mount point.
		if (!pakFileSystem.MountPAK(
			Sys::C_FileSystem::Instance(),
			pakFileName,
			false,
			true,
			pakMountPoint.Chars()))
			return false;

		if (!pakFileSystem.UnmountPAK())
			return false;
	}

	AddPakToList( folder );
	return true;
}

void C_MakePAK::XML_ReadEndian()
{
	const Sys::C_XmlDomBaseNode* pEndianNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "endian");
	if (pEndianNode)
	{
		const Sys::C_XmlDomElement* pEndianElement = (Sys::C_XmlDomElement*)pEndianNode;
		if (pEndianElement)
		{
			const Sys::C_XmlAttribute* pEndianAttr = pEndianElement->GetAttributes()[0];
			if (pEndianAttr)
			{
				if ((pEndianAttr->name == "swap") && (pEndianAttr->value == "yes"))
				{
					SetEndianSwap( true );
				}
				if ((pEndianAttr->name == "swap") && (pEndianAttr->value == "no"))
				{
					SetEndianSwap( false );
				}

			}
		}
	}
}

void C_MakePAK::XML_ReadRootFolder()
{
	const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "root_folder");
	if (pNode)
	{
		const Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pNode;
		if (pElement)
		{
			const Sys::C_XmlAttribute* pAttr = pElement->GetAttributes()[0];
			if (pAttr)
			{
				if (pAttr->name == "name")
				{
					SetPakRoot( pAttr->value.Chars() );
				}
			}
		}
	}
}

void C_MakePAK::XML_ReadFiles(U32 sectorSize)
{
	const Sys::C_XmlDomBaseNode* pFileListNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "instructions");

	Util::C_String pakName;
	Util::C_String pakMountPoint;
	Util::C_String excludeList;
	bool pakCompress = true;

	Ut::OpenHashSet<std::string, Ut::HashKeyOpString> uniqueFiles;

	if (pFileListNode)
	{
		for (S32 i= 0; i< pFileListNode->GetChildren().Size(); i++)
		{
			const Sys::C_XmlDomBaseNode* pChild = pFileListNode->GetChildren()[i];
			const Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pChild;
			if (pChild->GetName() == "file")
			{
				for (S32 i = 0; i < pElement->GetAttributes().Elements(); i++)
				{
					const Sys::C_XmlAttribute* pAttr = pElement->GetAttributes()[i];
					if (!pAttr)
						continue;
					if (pAttr->name == "name")
					{
						//LOGINFO(m_Log,"FileName : %s ",pAttr->value.Chars());
					}
				}
				
			}

			if (pChild->GetName() == "folder")
			{
				for (S32 j = 0; j < pElement->GetAttributes().Elements(); j++)
				{
					const Sys::C_XmlAttribute* pAttr = pElement->GetAttributes()[j];
					if (!pAttr)
						continue;

					if (pAttr->name == "name")
					{
						LOGINFO(m_Log,"folder : %s ",pAttr->value.Chars());
						pakName = pAttr->value;
					}
					else if (pAttr->name == "compress")
					{
						LOGINFO(m_Log,"compress : %s ",pAttr->value.Chars());
						if (pAttr->value == "y" || pAttr->value == "yes" || pAttr->value == "true")
						{
							pakCompress = true;
						}
						if (pAttr->value == "n" || pAttr->value == "no" || pAttr->value == "false")
						{
							pakCompress = false;
						}
					}
					else if (pAttr->name == "mount_point")
					{
						LOGINFO(m_Log,"mount point : %s ",pAttr->value.Chars());
						pakMountPoint = pAttr->value;
					}
					else if (pAttr->name == "exclude")
					{
						excludeList += pAttr->value;
						excludeList += " ";
					}
				}

				for (S32 j = 0; j < pElement->GetChildren().Size(); j++)
				{
					const Sys::C_XmlDomBaseNode* pChild2 = pElement->GetChildren()[j];
					const Sys::C_XmlDomElement* pElement2 = (Sys::C_XmlDomElement*)pChild2;

					if (pChild2->GetName() == "file")
					{
						for (S32 k = 0; k < pElement2->GetAttributes().Elements(); k++)
						{
							const Sys::C_XmlAttribute* pAttr = pElement2->GetAttributes()[k];
							if (!pAttr)
								continue;
							if (pAttr->name == "name")
							{
								Ut::OpenHashSet<std::string, Ut::HashKeyOpString>::Iterator it = uniqueFiles.Find(pAttr->value.Lower().Chars());
								if (it == uniqueFiles.End())
								{
									mOrderedFiles.PushBack(pAttr->value);
									uniqueFiles.Insert(pAttr->value.Lower().Chars());
								}
							}
						}
					}
				}

				CreatePAKFromFolder(pakName, pakMountPoint, pakCompress, sectorSize, excludeList );
			}
		}
	}
}

bool C_MakePAK::XML_Read( const char* xml_file, U32 sectorSize )
{
	if (!dom.Read( xml_file ))
		return false;

	XML_ReadEndian();
	XML_ReadRootFolder();
	CreatePAKFileList();
	XML_ReadFiles(sectorSize);

	return true;
}

