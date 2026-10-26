using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using SuperControls;

namespace Alienbrain
{
    public partial class AlienBrainInterface
    {
		static AlienBrainInterface m_me = null;
		AlienbrainQueue m_queue;
		const int STRING_SIZE = 2048;

        private AlienBrainInterface()
        {
			m_queue = AlienbrainQueue.CreateQueue();
        }

		public void KillThread()
		{
			this.m_queue.KillThread();
		}


		/// <summary>
		/// Creates an Alienbrain interface as a singleton, or returns a previously created instance.
		/// </summary>
		/// <returns></returns>
		static public AlienBrainInterface CreateInterface()
		{
			if(m_me == null)
			{
				m_me = new AlienBrainInterface();
			}
			return m_me;
		}

		private enum StatusFlags
		{
			NXNCONST_NYNDB_FOLDER = 0,						//  only as a shortcut to "not NYNDB_FILE"
			NXNCONST_NYNDB_FILE = 1,						//  bit 0
			NXNCONST_NYNDB_LINK = 2,						//  bit 1
			NXNCONST_NYNDB_SYSTEM = 4,						//  bit 2
			NXNCONST_NYNDB_READONLY = 8,					//  bit 3
			NXNCONST_NYNDB_LOCKED = 16,						//  bit 4,  locked = locked by someone else!
			NXNCONST_NYNDB_CHECKED_OUT = 32,				//  bit 5,  checked out = checked out by requesting user
			NXNCONST_NYNDB_MOUNTED = 64,					//  bit 6,  mount point
			NXNCONST_NYNDB_LOCKED_MULTIPLE = 128,			//  bit 7,  locked by several people
			NXNCONST_NYNDB_LOCKED_EXCLUSIVE = STRING_SIZE,	//  bit 8,  locked, avoid subsequent multiple check-outs
		};

		#region Imports.
#if (DEBUG)
		private const string dllName = "alienbrain_wrapper.dll";
#else
		private const string dllName = "alienbrain_wrapper.dll";
#endif
		[DllImport(dllName)]
		unsafe private static extern bool InitialiseAB();

		[DllImport(dllName)]
		unsafe private static extern bool DeInitialiseAB();

		[DllImport(dllName)]
		unsafe private static extern int LoadProjectAB(string strProjectName, StringBuilder buf, int buflen);

		[DllImport(dllName)]
		unsafe private static extern bool GetLatestFileAB(string strFile);

		[DllImport(dllName)]
		unsafe private static extern bool DoTestAB(string strFileOrFolder);

		[DllImport(dllName)]
		unsafe private static extern bool GetNodeDetailsAB(string strNamespacePath, out long flags, out int type, StringBuilder local_path, StringBuilder name, StringBuilder namespace_path, StringBuilder smot, StringBuilder cmot, bool bPathIsNamespace);

		[DllImport(dllName)]
		unsafe private static extern bool GetFirstChildAB(string strNamespacePath, out long flags, out int type, StringBuilder local_path, StringBuilder name, StringBuilder namespace_path, StringBuilder smot, StringBuilder cmot);

		[DllImport(dllName)]
		unsafe private static extern bool GetFirstFolderAB(string strNamespacePath, out long flags, out int type, StringBuilder local_path, StringBuilder name, StringBuilder namespace_path, StringBuilder smot, StringBuilder cmot);

		[DllImport(dllName)]
		unsafe private static extern bool GetNextChildAB(out long flags, out int type, StringBuilder local_path, StringBuilder name, StringBuilder namespace_path, StringBuilder smot, StringBuilder cmot);

		[DllImport(dllName)]
		unsafe private static extern bool ImportAB(string local_path);

		[DllImport(dllName)]
		unsafe private static extern bool CheckoutAB(string local_path);

		[DllImport(dllName)]
		unsafe private static extern bool UndoCheckoutAB(string local_path);

		[DllImport(dllName)]
		unsafe private static extern bool CheckinAB(string local_path);

		[DllImport(dllName)]
		unsafe private static extern int PrepareToEnumerateChildrenAB(string strPath);

		[DllImport(dllName)]
		unsafe private static extern bool GetNextEnumeratedChildAB(out long flags, out int type, StringBuilder local_path, StringBuilder name, StringBuilder namespace_path);

		#endregion Imports.

		private bool SetExtendedFlags(AlienbrainInfo abInfo)
		{
			if((abInfo.Type == AlienbrainInfo.eNxNBasicNamespaceType.DISKFILE) || (abInfo.Type == AlienbrainInfo.eNxNBasicNamespaceType.DISKFOLDER))
			{
				abInfo.bIsLocalfile = true;
				abInfo.bIsModified = false;
				abInfo.bIsCheckedOutByMe = false;
				abInfo.bIsRemoteFile = false;
				abInfo.bIsLocked = false;
			}
			else
			{
				if(File.Exists(abInfo.strLocalPath))
				{
					abInfo.bIsLocalfile = true;
					//					if(lvo.m_abInfo.lastWriteTime != File.GetLastWriteTime(lvo.m_abInfo.strLocalPath))
					//						bIsModified = true;
				}

				if((abInfo.flags & (long)StatusFlags.NXNCONST_NYNDB_CHECKED_OUT) != 0)
					abInfo.bIsCheckedOutByMe = true;

				abInfo.bIsRemoteFile = true;

				if((abInfo.flags & (
					(long)StatusFlags.NXNCONST_NYNDB_LOCKED |
					(long)StatusFlags.NXNCONST_NYNDB_LOCKED_EXCLUSIVE |
					(long)StatusFlags.NXNCONST_NYNDB_LOCKED_MULTIPLE
				)) != 0)
					abInfo.bIsLocked = true;
			}
			return true;
		}

		private delegate string dLoadProject(string strProjectName);

		private string DoLoadProject(string strProjectName)
		{
			StringBuilder sb = new StringBuilder(STRING_SIZE);
			int result = LoadProjectAB(strProjectName, sb, sb.Capacity);
			return sb.ToString();
		}

		private delegate bool dGetLatestFile(string strLocalFile);

		private bool DoGetLatestFile(string strLocalFile)
		{
			bool bRetval = GetLatestFileAB(strLocalFile);
			return bRetval;
		}


		private delegate bool dInitialise();
		
		private bool DoInitialise()
		{
			InitialiseAB();
			return true;
		}

		private delegate bool dTest(string strFolder);		
		
		private bool DoTest(string strFolder)
		{
			DoTestAB(strFolder);
			return true;
		}


		public delegate AlienbrainInfo dGetNodeDetails(string strPath, bool bPathIsNamespace);
		
		private AlienbrainInfo DoGetNodeDetails(string strNamespacePath, bool bPathIsNamespace)
		{

			StringBuilder local_path = new StringBuilder(STRING_SIZE);
			StringBuilder name = new StringBuilder(STRING_SIZE);
			StringBuilder namespace_path = new StringBuilder(STRING_SIZE);

			StringBuilder smot = new StringBuilder(STRING_SIZE);
			StringBuilder cmot = new StringBuilder(STRING_SIZE);

			int type;
			long flags;

			//if(GetNodeDetailsAB(strNamespacePath + "\\", out flags, out type, local_path, name, namespace_path, smot, cmot, bPathIsNamespace) == true)
			if(GetNodeDetailsAB(strNamespacePath, out flags, out type, local_path, name, namespace_path, smot, cmot, bPathIsNamespace) == true)
			{
				AlienbrainInfo abInfo = new AlienbrainInfo();
				abInfo.Type = (AlienbrainInfo.eNxNBasicNamespaceType)type;
				abInfo.strLocalPath = local_path.ToString();
				abInfo.strName = name.ToString();
				abInfo.strNamespacePath = namespace_path.ToString();
				abInfo.flags = flags;

				abInfo.strServerModTime = smot.ToString();
				abInfo.strClientModTime = cmot.ToString();

				SetExtendedFlags(abInfo);
				
				return abInfo;
			}

			return null;
		}

		private delegate AlienbrainInfo dGetFirstChild(string strNamespacePath);

		private AlienbrainInfo DoGetFirstChild(string strNamespacePath)
		{
			StringBuilder local_path = new StringBuilder(STRING_SIZE);
			StringBuilder name = new StringBuilder(STRING_SIZE);
			StringBuilder namespace_path = new StringBuilder(STRING_SIZE);

			StringBuilder smot = new StringBuilder(STRING_SIZE);
			StringBuilder cmot = new StringBuilder(STRING_SIZE);

			int type;
			long flags;
			if(GetFirstChildAB(strNamespacePath + "\\", out flags, out type, local_path, name, namespace_path, smot, cmot) == true)
			{
				AlienbrainInfo abInfo = new AlienbrainInfo();
				abInfo.Type = (AlienbrainInfo.eNxNBasicNamespaceType)type;
				abInfo.strLocalPath = local_path.ToString();
				abInfo.strName = name.ToString();
				abInfo.strNamespacePath = namespace_path.ToString();
				abInfo.flags = flags;

				abInfo.strServerModTime = smot.ToString();
				abInfo.strClientModTime = cmot.ToString();

				SetExtendedFlags(abInfo);

				return abInfo;
			}

			return null;
		}

		private delegate AlienbrainInfo dGetFirstFolder(string strNamespacePath);

		private AlienbrainInfo DoGetFirstFolder(string strNamespacePath)
		{
			StringBuilder local_path = new StringBuilder(STRING_SIZE);
			StringBuilder name = new StringBuilder(STRING_SIZE);
			StringBuilder namespace_path = new StringBuilder(STRING_SIZE);

			StringBuilder smot = new StringBuilder(STRING_SIZE);
			StringBuilder cmot = new StringBuilder(STRING_SIZE);

			int type;
			long flags;
			if(GetFirstFolderAB(strNamespacePath + "\\", out flags, out type, local_path, name, namespace_path, smot, cmot) == true)
			{
				AlienbrainInfo abInfo = new AlienbrainInfo();
				abInfo.Type = (AlienbrainInfo.eNxNBasicNamespaceType)type;
				abInfo.strLocalPath = local_path.ToString();
				abInfo.strName = name.ToString();
				abInfo.strNamespacePath = namespace_path.ToString();
				abInfo.flags = flags;

				abInfo.strServerModTime = smot.ToString();
				abInfo.strClientModTime = cmot.ToString();

				SetExtendedFlags(abInfo);

				return abInfo;
			}

			return null;
		}

		private delegate AlienbrainInfo dGetNextChild();		
		
		private AlienbrainInfo DoGetNextChild()
		{
			StringBuilder local_path = new StringBuilder(STRING_SIZE);
			StringBuilder name = new StringBuilder(STRING_SIZE);
			StringBuilder namespace_path = new StringBuilder(STRING_SIZE);
			StringBuilder smot = new StringBuilder(STRING_SIZE);
			StringBuilder cmot = new StringBuilder(STRING_SIZE);
			int type;
			long flags;

			if(GetNextChildAB(out flags, out type, local_path, name, namespace_path, smot, cmot) == true)
			{
				AlienbrainInfo abInfo = new AlienbrainInfo();
				abInfo.Type = (AlienbrainInfo.eNxNBasicNamespaceType)type;
				abInfo.strLocalPath = local_path.ToString();
				abInfo.strName = name.ToString();
				abInfo.strNamespacePath = namespace_path.ToString();
				abInfo.flags = flags;

				abInfo.strServerModTime = smot.ToString();
				abInfo.strClientModTime = cmot.ToString();
				SetExtendedFlags(abInfo);
				return abInfo;
			}
			return null;
		}

		private delegate bool dCheckout(string strLocalFile);		
		
		private bool DoCheckout(string strLocalFile)
		{
			return CheckoutAB(strLocalFile);
		}


		private delegate bool dUndoCheckout(string strLocalFile);		
		
		private bool DoUndoCheckout(string strLocalFile)
		{
			return UndoCheckoutAB(strLocalFile);
		}



		private delegate bool dCheckin(string strLocalFile);		
		
		private bool DoCheckin(string strLocalFile)
		{
			return CheckinAB(strLocalFile);
		}



		private delegate bool dImport(string strLocalFile);		
		
		private bool DoImport(string strLocalFile)
		{
			return ImportAB(strLocalFile);
		}



		private delegate int dPrepareToEnumerateChildren(string strPath);

		private int DoPrepareToEnumerateChildren(string strPath)
		{
			return PrepareToEnumerateChildrenAB(strPath);
		}



		private delegate AlienbrainInfo dGetNextEnumeratedChild();		
		private AlienbrainInfo DoGetNextEnumeratedChild()
		{
			StringBuilder local_path = new StringBuilder(STRING_SIZE);
			StringBuilder name = new StringBuilder(STRING_SIZE);
			StringBuilder namespace_path = new StringBuilder(STRING_SIZE);
			int type;
			long flags;

			if(GetNextEnumeratedChildAB(out flags, out type, local_path, name, namespace_path) == true)
			{
				AlienbrainInfo abInfo = new AlienbrainInfo();
				abInfo.Type = (AlienbrainInfo.eNxNBasicNamespaceType)type;
				abInfo.strLocalPath = local_path.ToString();
				abInfo.strName = name.ToString();
				abInfo.strNamespacePath = namespace_path.ToString();
				abInfo.flags = flags;

				SetExtendedFlags(abInfo);
				return abInfo;
			}
			return null;
		}
		
		private string MongleFilename(string strProject, string strInput)
		{
			string strRetval = strInput;

			string strSearch = "\\" + strProject + "\\";

			int index = strInput.IndexOf(strSearch);
			if(index != -1)
			{
				strRetval = strInput.Substring(index + strSearch.Length);
			}
			return strRetval;
		}








	}
}
