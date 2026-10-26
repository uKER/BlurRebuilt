using System;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using XmlHelperFunctions;
using System.Diagnostics;
using System.Windows.Forms;
namespace TextureTool
{
	/// <summary>
	/// Summary description for XmlManager.
	/// </summary>
	public class XmlManager
	{
		string m_strPathFilename;
		XmlDocument m_XmlDoc;
		public XmlDocument xmlDoc{get{return m_XmlDoc;}}
		bool m_dataLoaded;

		public bool dataIsWritable;

		public XmlManager(string pathFilename)
		{
			//
			// TODO: Add constructor logic here
			//
			m_strPathFilename = pathFilename;

			if(DataFileExists() == false)
			{
				m_dataLoaded = CreateDataFile(true);
				dataIsWritable = true;
			}
			else
			{
				FileAttributes fa = File.GetAttributes(m_strPathFilename);

				if((fa & FileAttributes.ReadOnly) == FileAttributes.ReadOnly)
				{
//					MessageBox.Show(null, "The datafile '"+m_strPathFilename+"' is locked.\nYou cannot work in this folder.\nMake sure you have the file checked out from Alienbrain.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Stop);

					dataIsWritable = false;
				}
				else
					dataIsWritable = true;

				m_dataLoaded = GetDataFile();
			}
		}

		public bool CreateDataFile(bool writeToDisk)
		{
			try
			{
				m_XmlDoc = new XmlDocument();

				XmlNode header = m_XmlDoc.CreateNode(XmlNodeType.XmlDeclaration, null, null);
				m_XmlDoc.AppendChild(header);

				XmlElement root = m_XmlDoc.CreateElement("root");
				m_XmlDoc.AppendChild(root);
				XmlElement textures = m_XmlDoc.CreateElement("textures");
				XmlHelper.AddAttribute(textures, "folder", m_strPathFilename);
				root.AppendChild(textures);


				if(writeToDisk == true)
					m_XmlDoc.Save(m_strPathFilename);
			}
			catch(Exception)
			{
				return false;
			}
			return true;
		}

		public bool	SaveDataFile()
		{
			if(dataIsWritable == true)
			{
				try
				{
					m_XmlDoc.Save(m_strPathFilename);
					return true;
				}
				catch(Exception ex)
				{
					MessageBox.Show("Error : " + ex.ToString());
				}
				return false;
			}
			else
				MessageBox.Show(null, "You cannot modify this folder because '"+m_strPathFilename+"' is not writable.\nYou cannot modify data in this folder.\nIs it checked out from Alienbrain?\nIf you make it writable you must rescan this folder.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Stop);

			return false;
		}

		public bool GetDataFile()
		{
			try
			{
				m_XmlDoc = new XmlDocument();
				m_XmlDoc.Load(m_strPathFilename);
			}
			catch (Exception) 
			{
				return false;
			}
			return true;
		}


		public bool DataFileExists()
		{
			return File.Exists(m_strPathFilename);
		}

		public XmlNode FindTextureEntry(string strTextureName)
		{
			Debug.Assert(m_XmlDoc != null);

			if(m_XmlDoc != null)
			{
				XmlNodeList textures = m_XmlDoc.SelectNodes("/root/textures");

				if(textures != null)
				{
					return XmlHelper.FindNodeByKeyValue(textures, "name", strTextureName);
				}
			}
			return null;
		}
	}
}
		








