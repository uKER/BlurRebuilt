using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using Global;
using System.IO;
using System.Diagnostics;

namespace MenuSystemLibrary
{
	public partial class Container_Filename : MenuSystemLibrary.Container_Base
	{
		string m_strPrefix;
		string m_strFullPrefix;
		string m_strHostName;
		bool m_bForceLocalNetworkPath;

		Color m_backColour;

		bool m_bForceFileExists;
		bool m_bForcePathExists;

		string m_strOpenWith;

		private Container_Filename()
		{
			InitializeComponent();
		}

		public Container_Filename(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			string strValue = XmlHelper.GetKeyValue(m_xObject, "value");

			this.toolTip.SetToolTip(this.textBox, "String:\n\"" + strValue + "\"");

			m_strPrefix = XmlHelper.GetKeyValue(m_xObject, "filepicker_share_name");

			m_bForceLocalNetworkPath = XmlHelper.GetKeyBool(m_xObject, "force_local_network_path");

			m_bForceFileExists = XmlHelper.GetKeyBool(m_xObject, "force_file_exists");
			m_bForcePathExists = XmlHelper.GetKeyBool(m_xObject, "force_path_exists");

			m_strOpenWith = XmlHelper.GetKeyValue(m_xObject, "open_with");

			string strStyle = XmlHelper.GetKeyValue(m_xObject, "style");

			if(strStyle == "sparse")
			{
				this.label.Visible = false;

				this.Height = 28;
			}

			m_backColour = this.textBox.BackColor;

			m_strHostName = System.Net.Dns.GetHostName();

			if(m_bForceLocalNetworkPath == true)
			{
				m_strFullPrefix = "\\\\" + m_strHostName + "\\";
				if(m_strPrefix != null)
				{
					m_strFullPrefix = m_strFullPrefix + m_strPrefix + "\\";
				}
				this.labelPrefix.Text = m_strFullPrefix;
				this.Width += labelPrefix.Width;
				this.textBox.Left = this.labelPrefix.Right - 3;
				this.textBox.Width = this.buttonBrowse.Left - 5 - this.textBox.Left;
				this.ObjectValue = TransbobulateFilename(strValue);
				this.textBox.Select(this.textBox.Text.Length, 0);
			}
			else
			{
				m_strFullPrefix = m_strPrefix = "";
				this.labelPrefix.Visible = false;
				this.textBox.Left = this.labelPrefix.Left;
				this.textBox.Width = this.buttonBrowse.Left - 5 - this.textBox.Left;
			}
		}

		override internal string ObjectName
		{
			set { this.label.Text = value; }
		}

		override internal string ObjectValue
		{
			get
			{
				if(this.m_bForceLocalNetworkPath == true)
				{
					if(this.textBox.Text != "<invalid>")
					{
						return m_strFullPrefix + this.textBox.Text;
					}
					else
					{
						return m_strFullPrefix;
					}
				}
				else
					return this.textBox.Text;
			}

			set
			{
				if(this.m_bForceLocalNetworkPath == true)
				{
					if(value.Length >= m_strFullPrefix.Length)
						this.textBox.Text = value.Substring(m_strFullPrefix.Length);
					else
						this.textBox.Text = "<invalid>";
				}
				else
					this.textBox.Text = value;
			}
		}

		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.textBox;
			}
		}

		override protected void MakeReadonly(bool bReadonly)
		{
			this.textBox.ReadOnly = bReadonly;
			this.buttonBrowse.Enabled = !bReadonly;

			if(bReadonly == true)
				this.textBox.BackColor = SystemColors.Control;
			else
				this.textBox.BackColor = SystemColors.Window;
		}


		private void button_Click(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.CheckFileExists = false;

			this.ObjectValue = this.ObjectValue.Replace('/', '\\');
			ofd.FileName = this.ObjectValue;

			string strFilter = XmlHelper.GetKeyValue(m_xObject, "filepicker_filter");

			if(strFilter == null)
			{
				ofd.Filter = "All files (*.*)|*.*";
			}
			else
			{
				try
				{
					ofd.Filter = strFilter;
				}
				catch
				{
					MessageBox.Show(this, "This filepicker has a bad filter. The filter is\n'" + strFilter + "'\nPlease attend to your template to resolve this error.\nClick OK to use the default filter.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);

					ofd.Filter = "All files (*.*)|*.*";
				}
			}
			
			if(m_bForceFileExists == true)
			{
				ofd.CheckFileExists = true;
				ofd.CheckPathExists = true;
			}
			if(m_bForcePathExists == true)
			{
				ofd.CheckPathExists = true;
			}

//			if(File.Exists(this.textBox.Text) == false)
			if(File.Exists(this.ObjectValue) == false)
			{
				ofd.FileName = "";
				string strDirectory = null;

				try
				{
					strDirectory = Path.GetDirectoryName(this.ObjectValue);
					if(Directory.Exists(strDirectory) == false)
						strDirectory = m_strFullPrefix;
				}
				catch { }

				if (strDirectory == null)
					ofd.InitialDirectory = this.ObjectValue;
				else
					ofd.InitialDirectory = strDirectory;
			}

			DialogResult dr = ofd.ShowDialog(this);

			if(dr == DialogResult.OK)
			{
				if(m_bForceLocalNetworkPath == true)
				{
					this.ObjectValue = TransbobulateFilename(ofd.FileName);
				}
				else
				{
					this.ObjectValue = ofd.FileName;
				}
				this.textBox.Select(this.textBox.Text.Length, 0);
			}
		}

		private string TransbobulateFilename(string strFilepath)
		{
			string[]strings = strFilepath.Split('\\');

			if(strFilepath.ToUpper().StartsWith(m_strFullPrefix.ToUpper()))
			{
				return strFilepath;
			}
			else
			{
				if(strFilepath.StartsWith("\\\\"))
				{
					int index1 = strFilepath.IndexOf('\\', 2);					// \\Thing\stuff\
																				//        ^
					if(index1 != -1)
					{
						int index2 = strFilepath.IndexOf('\\', index1 + 1);		// \\Thing\stuff\
																				//              ^
						if(index2 != -1)
						{
							strFilepath = m_strFullPrefix + strFilepath.Substring(index2 + 1);
						}
						else
						{
							strFilepath = m_strFullPrefix + strFilepath.Substring(index1 + 1);
						}
					}
					else
					{
						strFilepath = m_strFullPrefix + strFilepath;
					}
				}
				else if((strFilepath.Length > 1) && (strFilepath[1] == ':'))
				{
					int index = strFilepath.IndexOf('\\', 2);					// c:\this\that		c:this\that
																				//   ^                    ^
					if(index == 2)
					{
						strFilepath = m_strFullPrefix + strFilepath.Substring(3);
					}
					else
					{
						strFilepath = m_strFullPrefix + strFilepath.Substring(2);
					}
				}
				else
				{
					strFilepath = m_strFullPrefix;
				}
			}
			return strFilepath;
		}

		private void textBox_TextChanged(object sender, EventArgs e)
		{
//			ValueChanged("value", this.textBox.Text, false);

			if(m_bForceLocalNetworkPath == true)
			{
				string strNewValue = TransbobulateFilename(this.ObjectValue);
				if(strNewValue != this.ObjectValue)
					this.ObjectValue = strNewValue;
//				ValueChanged("value", strNewValue, false);
//				this.textBox.Select(this.textBox.Text.Length, 0);
			}

			this.textBox.BackColor = m_backColour;

			if(m_bForcePathExists == true)
			{
				if(Directory.Exists(Path.GetDirectoryName(this.ObjectValue)) == false)
				{
					this.textBox.BackColor = Color.Red;
				}
			}
			if(m_bForceFileExists == true)
			{
				if(File.Exists(this.ObjectValue) == false)
				{
					this.textBox.BackColor = Color.Red;
				}
			}
			ValueChanged("value", this.ObjectValue, false);
//			this.textBox.Select(this.textBox.Text.Length, 0);
		}

		private void buttonCopyFilename_Click(object sender, EventArgs e)
		{
			string strFilename = System.IO.Path.GetFileNameWithoutExtension(this.ObjectValue);

			if(strFilename != null)
				Clipboard.SetDataObject(strFilename);
		}

		private void buttonMakePath_Click(object sender, EventArgs e)
		{
			string strDirectory = System.IO.Path.GetDirectoryName(this.ObjectValue);
			try
			{
				Directory.CreateDirectory(strDirectory);
			}
			catch(Exception ex)
			{
				MessageBox.Show(this, "Error creating path. System reports:\n'" + ex.Message + "'");
			}
		}

		private void buttonCopyFilePath_Click(object sender, EventArgs e)
		{
			string strFilename = this.ObjectValue;

			if(strFilename != null)
				Clipboard.SetDataObject(strFilename);
		}

		private void textBox_DragDrop(object sender, DragEventArgs e)
		{
			string strNewValue = null;

			if(e.Data.GetDataPresent(DataFormats.Text))
			{

				string fileName = (string)e.Data.GetData(DataFormats.Text);
				strNewValue = fileName;
//				this.ObjectValue = fileName;
			}
			else if(e.Data.GetDataPresent(DataFormats.FileDrop))
			{
				string[] fileNames;
				fileNames = (string[])e.Data.GetData(DataFormats.FileDrop);
				string fileName = fileNames[0];
				strNewValue = fileNames[0];
//				this.ObjectValue = fileName;
			}

			if(strNewValue != null)
			{
				if(this.m_bForceLocalNetworkPath == true)
				{
					if(strNewValue.ToLower().StartsWith(m_strFullPrefix.ToLower()))
						this.ObjectValue = strNewValue;
					else
					{
						MessageBox.Show(this, "The file you dropped must start with the following path:\n" + "'" + m_strFullPrefix + "'", "Executer", MessageBoxButtons.OK, MessageBoxIcon.Error);
//						this.ObjectValue = "";
					}
				}
				else
					this.ObjectValue = strNewValue;
			}

		}

		private void textBox_DragEnter(object sender, DragEventArgs e)
		{
			if(e.Data.GetDataPresent(DataFormats.FileDrop))
				e.Effect = DragDropEffects.Copy;
			else
				e.Effect = DragDropEffects.None;
		}



		private void buttonView_Click(object sender, EventArgs e)
		{
			if(m_strOpenWith ==  null || m_strOpenWith == "")
			{
				LaunchJob(this.textBox.Text, null, null);
			}
			else
			{
				LaunchJob(m_strOpenWith, this.textBox.Text, null);
			}
		}

		public bool LaunchJob(string strCommand, string strClArguments, string strWorkingDirectory)
		{
			try
			{
				Process p = new Process();
				
				p.StartInfo = new ProcessStartInfo(strCommand, strClArguments);

				if(strWorkingDirectory != null)
				{
					p.StartInfo.WorkingDirectory = strWorkingDirectory;
				}
				else
				{
					p.StartInfo.WorkingDirectory = Path.GetDirectoryName(strCommand);
				}

				p.Start();
				return true;
			}
			catch(Exception ex)
			{
				MessageBox.Show(this, "Cannot open the specified file. System reports:\n" + ex.Message, "Executer", MessageBoxButtons.OK);
				return false;
				//throw (ex);
			}
		}


		




		private void buttonOptions_Click(object sender, EventArgs e)
		{
			ContextMenu cm = new ContextMenu();

			cm.MenuItems.Add("View", buttonView_Click);
			cm.MenuItems.Add("Copy", buttonCopyFilePath_Click);
			cm.MenuItems.Add("Copy Filename", buttonCopyFilename_Click);

			cm.MenuItems.Add("Make Path", buttonMakePath_Click);

			try
			{
				string strDirectory = System.IO.Path.GetDirectoryName(this.ObjectValue);

				if(Directory.Exists(strDirectory))
					cm.MenuItems[3].Enabled = false;
			}
			catch
			{
				cm.MenuItems[3].Enabled = false;
			}

			cm.Show(this.buttonOptions, new Point(0, buttonOptions.Height));

			
		}

	}
}

