using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Alienbrain;
using System.Xml;
using System.Diagnostics;
using System.IO;

namespace DataLib.DataModel
{
	public class AbData
	{
		public static AlienBrainInterface m_alienbrainInterface;

		/// <summary>
		/// A simple data transfer object (DTO) that contains raw data about an Alienbrain object.
		/// </summary>
		public AbData(AlienbrainInfo abInfo)
		{
			AbInfo = abInfo;
		}

		public XmlDocument xMenuSystem { get; set; }			// Auto-implemented property.

		/// <summary>
		/// Used to track whether changes to the xMenuSystem have been saved to disk.
		/// </summary>
		public bool bXmlDocumentDirty { get; set; }			// Auto-implemented property.

		// TODO: Make this private? (Or remove it altogether?)
		public AlienbrainInfo AbInfo { get; private set; }		// Auto-implemented property. (Readonly)

		#region Not used

		// TODO: Why is this not used?
		readonly List<AbData> m_children = new List<AbData>();
		public List<AbData> Children
		{
			get { return m_children; }
		}

		#endregion

		public string Name { get { return AbInfo.StrName; } }
		public string NamespacePath { get { return AbInfo.strNamespacePath; } }
		public string LocalPath { get { return AbInfo.strLocalPath; } }
		public bool IsLocked { get { return AbInfo.bIsLocked; } }
		public bool IsCheckedOutByMe { get { return AbInfo.bIsCheckedOutByMe; } }
		public bool IsRemoteFile { get { return AbInfo.bIsRemoteFile; } }
		public bool IsLocalFile { get { return AbInfo.bIsLocalfile; } }
		public AlienbrainInfo.eNxNBasicNamespaceType Type { get { return AbInfo.Type; } }

		public bool TestProperty { get { /*System.Threading.Thread.Sleep(50) ;*/ return AbInfo.bIsRemoteFile; } }

		public List<AbData> m_abListFiles = new List<AbData>();
		public List<AbData> m_abListFolders = new List<AbData>();

		public void GetChildren()
		{
			m_abListFolders.Clear();
			m_abListFiles.Clear();

			List<AlienbrainInfo> abListAllEntries = new List<AlienbrainInfo>();

			abListAllEntries = PrepareGetChildren();

			foreach(AlienbrainInfo abInfo in abListAllEntries)
			{
				switch(abInfo.Type)
				{
					case AlienbrainInfo.eNxNBasicNamespaceType.WORKSPACE:
						m_abListFolders.Add(new AbData(abInfo));
						break;

					case AlienbrainInfo.eNxNBasicNamespaceType.PROJECT:
						m_abListFolders.Add(new AbData(abInfo));
						break;

					case AlienbrainInfo.eNxNBasicNamespaceType.FILE:
						m_abListFiles.Add(new AbData(abInfo));
						break;
					case AlienbrainInfo.eNxNBasicNamespaceType.FOLDER:
						m_abListFolders.Add(new AbData(abInfo));
						break;

					case AlienbrainInfo.eNxNBasicNamespaceType.DISKFILE:
						m_abListFiles.Add(new AbData(abInfo));
						break;
					case AlienbrainInfo.eNxNBasicNamespaceType.DISKFOLDER:
						m_abListFolders.Add(new AbData(abInfo));
						break;

					case AlienbrainInfo.eNxNBasicNamespaceType.UNKNOWN:
						break;
					default:
						break;
				}
			}
		}

		public List<AlienbrainInfo> PrepareGetChildren()
		{
			int numChildren = AbData.m_alienbrainInterface.PrepareToEnumerateChildren(AlienbrainRequest.Priority.Normal, this.NamespacePath);
			if(numChildren == -1)
			{
				Debug.WriteLine("numChildren is -1");
				numChildren = 0;
			}
			List<AlienbrainInfo> abListAllEntries = new List<AlienbrainInfo>(numChildren);

			AlienbrainInfo abInfo = AbData.m_alienbrainInterface.GetNextEnumeratedChild(AlienbrainRequest.Priority.Normal);
			while(abInfo != null)
			{
				abListAllEntries.Add(abInfo);
				abInfo = AbData.m_alienbrainInterface.GetNextEnumeratedChild(AlienbrainRequest.Priority.Normal);
			}

			return abListAllEntries;
		}

		public XmlDocument m_xFileData = null;

		static public string TAG_EXTENSION = ".tagdata3";
		static public string LOCK_EXTENSION = "lock.tagdata3lock";


		public XmlDocument GetFileData()
		{
			try
			{
				// Load up or create a file data document.
				if(m_xFileData == null)
				{
					string strDocumentFilename = this.AbInfo.strLocalPath + TAG_EXTENSION;
					AlienbrainInfo abInfo = m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, strDocumentFilename, false);

					if(abInfo != null)
					{

						bool bSuccess = m_alienbrainInterface.GetLatestFile(AlienbrainRequest.Priority.Normal, strDocumentFilename);
						if(bSuccess == true)
						{
							m_xFileData = new XmlDocument();
							m_xFileData.Load(strDocumentFilename);
						}
						else
						{
							throw (new ApplicationException("Cannot GetLatest on '" + strDocumentFilename + "' The reason is unknown. You could try manually checking it out using the Alienbrain client."));
						}
					}

					if(abInfo == null || m_xFileData.SelectSingleNode("/root/file_data/values") == null)
					{
						// It is not in alienbrain, so create one.
						m_xFileData = CreateTagDataFile(strDocumentFilename);
					}
				}

				//XmlDocument xDoc = menuSystem.BuildFullMenuDocument(xTemplate["menu_system"] as XmlElement, abData.xGetFileData["root"]["file_data"]["values"]);
				if(m_xFileData.SelectSingleNode("/root/file_data/values") == null)
					return null;
				else
					return m_xFileData;
			}
			catch
			{
				return null;
			}

		}

		public XmlDocument xFileData
		{
			get
			{
				return m_xFileData;
			}
			set
			{
				Debug.Assert(value == null);
				m_xFileData = value;
			}
		}



/*
	if(abInfo.bIsLocked == false)
	{
		if(abInfo.bIsRemoteFile == true)
		{
			bool bSuccess = m_alienbrainInterface.Checkout(AlienbrainRequest.Priority.Normal, strDocumentFilename);		// Returns true if the file is already checked out.

			if(bSuccess == false)
			{
				throw(new ApplicationException("Cannot edit this folder since\n'" + strDocumentFilename + "'\ncannot be checked out. The reason is unknown.\nYou could try manually checking it out using the Alienbrain client."));
			}
		}
		else
		{
			throw (new ApplicationException("Cannot edit this folder since it does not exist in Alienbrain.\nUse the Alienbrain client to import this folder before trying again."));
		}

	}
	else
	{
		throw (new ApplicationException("Cannot edit this folder since\n'" + strDocumentFilename + "'\nis locked by another user."));
	}

*/


		/// <summary>
		/// Creates a tag data file.
		/// If you have a local data file and no Ab file it will use the local one rather than create a new one.
		/// Fails for various reasons:
		/// 1) If there is already a data file in Ab.
		/// 
		/// </summary>
		/// <param name="strDocumentFilename">The name of the tag data file.</param>
		/// <returns></returns>
		internal XmlDocument CreateTagDataFile(string strDocumentFilename)
		{
			//m_tempValueNode = null;

			XmlDocument xConfig = new XmlDocument();

			xConfig.Load(new StringReader("<root><file_data><values/></file_data></root>"));

			return xConfig;

#if OldMethod
			try
			{
				bool bExistsInAb = m_alienbrainInterface.GetLatestFile(AlienbrainRequest.Priority.Normal, strDocumentFilename);	// This creates the local path if it needs to, asuming the file is in AB.
				bool bExistsLocally = File.Exists(strDocumentFilename);

				if(bExistsInAb == true)
				{
					Debug.WriteLine("Attempting to create the tag data file when there is already one in Alienbrain.\nPlease inform the tools team.");
					throw(new ApplicationException("Attempting to create the tag data file when there is already one in Alienbrain.\nPlease inform the tools team."));
					//return null;
				}

				if(bExistsLocally == true)
				{
					// use existing data.
					Debug.WriteLine("Using local data for the TagDataFile for '"+strDocumentFilename+"' instead of creating new.");
					xConfig.Load(strDocumentFilename);											// This will throw an exception if the file does not exist.
				}
				else
				{
					//string strPath = Path.GetDirectoryName(strDocumentFilename);
					//if(Directory.Exists(strPath) == false)
					//{
					//    DirectoryInfo di = Directory.CreateDirectory(strPath);
					//}
					xConfig.Load(new StringReader("<root><file_data><values/></file_data></root>"));
				}

				return xConfig;
			}
			catch(ApplicationException ex)
			{
				throw(ex);
			}
			catch(Exception ex)
			{
				Debug.WriteLine("Cannot load '" + strDocumentFilename + "'\nSystem report: " + ex.Message);
				throw(new ApplicationException("Cannot load '" + strDocumentFilename + "'\nSystem report: " + ex.Message));
				return null;
			}
#endif
		}


		public bool SubmitDataFile(bool bSubmitCleanData)
		{
			if(this.bXmlDocumentDirty == true || bSubmitCleanData == true)
			{
				this.bXmlDocumentDirty = false;
				Debug.WriteLine("Cleaned up '" + this.Name + "'");
				/// TODO:
				/// Write it to disk.
				/// (Write/Import) or (Check out/Write/Check in)
				/// 
				string strTagFilename = this.LocalPath + AbData.TAG_EXTENSION;

				//abData.xGetFileData.Save(strTagFilename);
				//this.m_xFileData.Save(strTagFilename);


				AlienbrainInfo abInfo = m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, strTagFilename, false);

				if(abInfo == null /*Not in ab*/ || abInfo.bIsRemoteFile == false /*Not in ab*/)
				{
					this.m_xFileData.Save(strTagFilename);
					bool bSuccess = m_alienbrainInterface.Import(AlienbrainRequest.Priority.Normal, strTagFilename);
					return bSuccess;
				}
				else
				{
					// TODO: This is never true. The intent is to leave files checked out if they are checked out to start with.
					if(abInfo.bIsCheckedOutByMe == true)
					{
						this.m_xFileData.Save(strTagFilename);
						return false;
					}
					else
					{
						// TODO: Validate these steps.
						m_alienbrainInterface.Checkout(AlienbrainRequest.Priority.Normal, strTagFilename);
						this.m_xFileData.Save(strTagFilename);
						m_alienbrainInterface.Checkin(AlienbrainRequest.Priority.Normal, strTagFilename);
						return true;
					}
				}



			}
			else
			{
				return true;
			}
		}

		public bool LockAbFileFolder()
		{

			/*
			 * See if there's a remote lock file. If so, return false.
			 * 
			 * See if there's a local lock file. If not, create one.
			 * 
			 * Import the lock file. If fail, return false.
			 * 
			 */

			string strLockFilename = Path.GetDirectoryName(this.AbInfo.strLocalPath) + "\\" + AbData.LOCK_EXTENSION;

			AlienbrainInfo abLockInfo = m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, strLockFilename, false);

			//if(abLockInfo != null)
			{
				if(abLockInfo == null || abLockInfo.bIsLocked == false)
				{
					if(File.Exists(strLockFilename) == false)
					{
						using(StreamWriter sw = new StreamWriter(strLockFilename))
						{
							sw.WriteLine("This is a lock file for FileTagManager version 3. Please delete it if it looks old.");
							sw.Close();
						}
					}

					bool bImportSuccess;

					if(abLockInfo == null || abLockInfo.bIsRemoteFile == false)
					{
						bImportSuccess = m_alienbrainInterface.Import(AlienbrainRequest.Priority.Normal, strLockFilename);
					}
					else
					{
						if(abLockInfo.bIsCheckedOutByMe == true)
							bImportSuccess = true;
						else
							bImportSuccess = m_alienbrainInterface.Checkout(AlienbrainRequest.Priority.Normal, strLockFilename);
					}

					if(bImportSuccess == false)
					{
						File.SetAttributes(strLockFilename, File.GetAttributes(strLockFilename) &  (~FileAttributes.ReadOnly));
						File.Delete(strLockFilename);
						return false;
					}
					else
					{
						return true;
					}
				}
			}

			return false;

		}

		public bool UnLockAbFileFolder()
		{

			/*
			 * See if we have a lock file checked out.
			 * 
			 * If so, delete it from Alienbrain. If fail, return false.
			 * 
			 * Delete local lock file. (Just to be tidy.)
			 * 
			 */
			string strLockFilename = Path.GetDirectoryName(this.AbInfo.strLocalPath) +"\\"+ AbData.LOCK_EXTENSION;

			AlienbrainInfo abLockInfo = m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, strLockFilename, false);

			if(abLockInfo != null)
			{
				if(m_alienbrainInterface.UndoCheckout(AlienbrainRequest.Priority.Normal, strLockFilename) == true)
					return true;
			}
			return false;
		}

	}
}


/*
	bool temp = m_alienbrainInterface.Import(AlienbrainRequest.Priority.Normal, strDocumentFilename);
	if(temp == false)
	{
		Debug.WriteLine("Importing '" + strDocumentFilename + "' into Alienbrain failed.\nYou must use the Alienbrain client to get latest on this folder first.");
		throw(new ApplicationException("Importing '" + strDocumentFilename + "' into Alienbrain failed.\nYou must use the Alienbrain client to get latest on this folder first."));
		//return null;
	}

*/