using System;
using System.Collections.Generic;
using System.Text;

namespace Alienbrain
{
	public class AlienbrainInfo
	{
		public enum eNxNBasicNamespaceType
		{
			WORKSPACE = 0,
			PROJECT,
			FILE,
			FOLDER,
			DISKFILE,
			DISKFOLDER,
			UNKNOWN
		};


		public eNxNBasicNamespaceType Type = eNxNBasicNamespaceType.UNKNOWN;

		public string strLocalPath;
		private string strName;

		public string StrName
		{
			get { return strName; }
			set { strName = value; }
		}

		public string StrThing
		{
			get { /* System.Threading.Thread.Sleep(90); */ return "Hello " + strName; }
		}
  
		public string strNamespacePath;

		//public uint lastWriteTime;
		public uint fileSize;
		public long flags;

		public string strClientModTime;
		public string strServerModTime;

		public bool bIsLocalfile = false;
		public bool bIsModified = false;
		public bool bIsCheckedOutByMe = false;
		public bool bIsRemoteFile = false;
		public bool bIsLocked = false;
	}

}
