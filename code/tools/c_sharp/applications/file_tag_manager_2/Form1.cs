using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using Alienbrain;
using System.Diagnostics;
using System.Xml;
using Global;
using System.Text.RegularExpressions;

namespace file_tag_manager_2
{
	public partial class fileTagManager_MainForm : Form
	{
		//ImageViewer m_ImageViewer;
		public static AlienBrainInterface m_alienbrainInterface;

		public fileTagManager_MainForm()
		{
			InitializeComponent();
			this.panelBusy.Visible = false;
			this.panelBusy.Dock = DockStyle.Fill;
		}

		internal bool Initialise()
		{
			m_alienbrainInterface = AlienBrainInterface.CreateInterface();

			//m_ImageViewer = new ImageViewer();
			listViewManager.Initialise(this.statusStrip1, menuSystem1, this, externalViewerToolStripMenuItem);
			treeManager.SetListView(listViewManager);
			treeManager.SetMainForm(this);
			treeManager.Initialise(this.statusStrip1);

			//m_ImageViewer.Show(this);
			//m_ImageViewer.Visible = false;

			SetupUserMenuItems();
			SetupRegularExpressions();
			return true;
		}

		private void setDatabaseToolStripMenuItem_Click(object sender, EventArgs e)
		{
			string strRootFolder = m_alienbrainInterface.LoadProject(AlienbrainRequest.Priority.Normal, null);

			if(strRootFolder != "")
				treeManager.SetRootFolder(strRootFolder);
		}

		private void previewWindowToolStripMenuItem_Click(object sender, EventArgs e)
		{
			//this.m_ImageViewer.Visible = true;
		}

		private void fileTagManager_MainForm_FormClosing(object sender, FormClosingEventArgs e)
		{
			if(m_CheckedOutTreeNode != null)
			{
				MessageBox.Show(this, "You must Submit or Revert your changes before you can proceed.", "Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Stop);
				e.Cancel = true;
			}
			else
			{
				//m_ImageViewer.KillThread();
				listViewManager.KillThread();
				treeManager.KillThread();
				m_alienbrainInterface.KillThread();
			}
		}


		TreeObject m_CheckedOutTreeNode = null;

		private void toolStripButtonEdit_Click(object sender, EventArgs e)
		{
			if(this.treeManager.SelectedNode != null)
			{
				TreeObject to = this.treeManager.SelectedNode as TreeObject;

				AlienbrainInfo parentAbInfo = to.m_abInfo;

				if(parentAbInfo != null && parentAbInfo.bIsRemoteFile == true)
				{
					string strDocumentFilename = parentAbInfo.strLocalPath + ListViewManager.TAG_FILENAME;

					AlienbrainInfo abInfo = m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, strDocumentFilename, false);

					if(abInfo == null)
					{
						// It is not in alienbrain.
						XmlDocument xDoc = listViewManager.CreateTagDataFile(strDocumentFilename);

						if(xDoc == null)
						{
							MessageBox.Show(this, "Cannot edit this folder since the Tag Data File cannot be created.",
							"Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Stop);
							return;
						}
						abInfo = m_alienbrainInterface.GetNodeDetails(AlienbrainRequest.Priority.Normal, strDocumentFilename, false);

						Debug.Assert(abInfo != null);
					}

					if(abInfo.bIsLocked == false)
					{
						if(abInfo.bIsRemoteFile == true)
						{
							bool bSuccess = fileTagManager_MainForm.m_alienbrainInterface.Checkout(AlienbrainRequest.Priority.Normal, strDocumentFilename);		// Returns true if the file is already checked out.

							if(bSuccess == true)
							{
								m_CheckedOutTreeNode = to;

								//if(listViewManager.GetLatest() == false)
								//	MessageBox.Show(this, "GetLatest failed. Files may not be up to date.", "Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Warning);
								
								treeManager.SelectNewTreeObject(to, false);		// No need to do another update. Yes there is, in case the tag data file is new and therefore there is no xml data associated with each list view item.

								// Sort out the buttons.
								this.toolStripButtonEdit.Enabled = false;
								this.toolStripButtonSubmit.Enabled = true;
								this.toolStripButtonRevert.Enabled = true;

								listViewManager.m_bMenuIsReadOnly = false;
								//listViewManager.RefreshListViewMenu();
							}
							else
							{
								MessageBox.Show(this, "Cannot edit this folder since\n'" + strDocumentFilename + "'\ncannot be checked out. The reason is unknown.\nYou could try manually checking it out using the Alienbrain client.",
									"Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Stop);
							}
						}
						else
						{
							MessageBox.Show(this, "Cannot edit this folder since it does not exist in Alienbrain.\nUse the Alienbrain client to import this folder before trying again.",
								"Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Stop);
						}

					}
					else
					{
						MessageBox.Show(this, "Cannot edit this folder since\n'" + strDocumentFilename + "'\nis locked by another user.",
							"Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Stop);
					}
				}
				else
				{
					MessageBox.Show(this, "Cannot edit this folder since it does not exist in Alienbrain.\nUse the Alienbrain client to import this folder before trying again.",
						"Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Stop);
				}
			}
		}

		private void toolStripButtonSubmit_Click(object sender, EventArgs e)
		{
			SubmitChanges();
		}

		private void SubmitChanges()
		{
			if(m_CheckedOutTreeNode != null)
			{
				TreeObject to = m_CheckedOutTreeNode;

				m_CheckedOutTreeNode = null;

				AlienbrainInfo abInfo = to.m_abInfo;

				string strDocumentFilename = abInfo.strLocalPath + ListViewManager.TAG_FILENAME;

				// Sort out the buttons.
				this.toolStripButtonEdit.Enabled = true;
				this.toolStripButtonSubmit.Enabled = false;
				this.toolStripButtonRevert.Enabled = false;
				listViewManager.m_bMenuIsReadOnly = true;
				listViewManager.RefreshListViewMenu();

				if(listViewManager.bDocumentDirty == true)
				{
					try
					{
						listViewManager.m_xDocument.Save(listViewManager.m_strDocumentFilename);
					}
					catch(System.IO.IOException ioex)
					{
						MessageBox.Show("IO Exception: System reports:" + ioex.Message);
					}
					catch(Exception ex)
					{
						MessageBox.Show("Cannot write the data document. System reports:" + ex.Message);
					}
					listViewManager.bDocumentDirty = false;
				}

				if(fileTagManager_MainForm.m_alienbrainInterface.Checkin( AlienbrainRequest.Priority.Normal, strDocumentFilename) == false)
				{
					MessageBox.Show(this, "Check In failed. Files may not be up to date.", "Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			}
			else
			{
				Debug.Assert(false, "toolStripButtonSubmit_Click(..) error.");
			}
		}

		private void treeManager_BeforeSelect(object sender, TreeViewCancelEventArgs e)
		{
			if(m_CheckedOutTreeNode != null)
			{
				MessageBox.Show(this, "You must Submit or Revert your changes before you can proceed.", "Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Stop);
				e.Cancel = true;
			}
		}

		private void toolStripButtonRevert_Click(object sender, EventArgs e)
		{
			if(m_CheckedOutTreeNode != null)
			{
				TreeObject to = m_CheckedOutTreeNode;

				m_CheckedOutTreeNode = null;

				AlienbrainInfo abInfo = to.m_abInfo;

				string strDocumentFilename = abInfo.strLocalPath + ListViewManager.TAG_FILENAME;

				// Sort out the buttons.
				this.toolStripButtonEdit.Enabled = true;
				this.toolStripButtonSubmit.Enabled = false;
				this.toolStripButtonRevert.Enabled = false;
				listViewManager.m_bMenuIsReadOnly = true;

				if(listViewManager.bDocumentDirty == true)
				{
					listViewManager.bDocumentDirty = false;
				}
				
				if(fileTagManager_MainForm.m_alienbrainInterface.UndoCheckout( AlienbrainRequest.Priority.Normal, strDocumentFilename) == false)
				{
					MessageBox.Show(this, "Undo checkout failed. Files may not be up to date.", "Texture Tag Manager", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
				listViewManager.m_xDocument = listViewManager.LoadTagDataFile(strDocumentFilename);

				//treeManager.SelectNewTreeObject(to);		// TODO: Code review. Check that RefreshListViewMenu is sufficient instead of SelectNewTreeObject.
				// Need to reload the tag data.

				// RefreshListView instead of just refreshing the menu because the menu is made from data that is stored with each
				// ListViewItem and that data needs to be reverted to it's old value.
				listViewManager.RefreshListView(to);					// This updates the menu when it has finished.
				//listViewManager.RefreshListViewMenu();
			}
			else
			{
				Debug.Assert(false, "toolStripButtonSubmit_Click(..) error.");
			}
		}

		internal void DisableToolstrip()
		{
			if(InvokeRequired)
			{
				this.Invoke((MethodInvoker)delegate() { this.toolStrip1.Enabled = false; this.panelBusy.Visible = true; });
			}
			else
			{
				this.toolStrip1.Enabled = false;
				this.panelBusy.Visible = true;
			}
		}

		internal void EnableToolstrip()
		{
			if(InvokeRequired)
			{
				this.Invoke((MethodInvoker)delegate() { this.toolStrip1.Enabled = true; this.panelBusy.Visible = false; });
			}
			else
			{
				this.toolStrip1.Enabled = true;
				this.panelBusy.Visible = false;
			}
		}

		int toolstripMenuIteminsertIndex = 2;

		private void SetupUserMenuItems()
		{
			XmlElement xMenuItems = this.listViewManager.GetMenuItems();

			if(xMenuItems != null)
			{
				XmlNodeList xListItems = xMenuItems.GetElementsByTagName("menu_item");

				foreach(XmlElement xMenuItem in xListItems)
				{
					string strOwner = XmlHelper.GetKeyValue(xMenuItem, "owner");
					string strName = XmlHelper.GetKeyValue(xMenuItem, "name");
					string strValue = XmlHelper.GetKeyValue(xMenuItem, "value");

					ToolStripMenuItem tsmiOwner = null;// this.menuStrip1.Items[strOwner] as ToolStripMenuItem;

					foreach(ToolStripMenuItem tsmi in menuStrip1.Items)
					{
						if(tsmi.Text == strOwner)
						{
							tsmiOwner = tsmi;
							break;
						}
					}

					if(tsmiOwner == null)
					{
						tsmiOwner = new ToolStripMenuItem(strOwner);
						menuStrip1.Items.Insert(toolstripMenuIteminsertIndex++, tsmiOwner);
					}

					ToolStripMenuItem tsmiMenuItem = new ToolStripMenuItem(strName);
					tsmiMenuItem.Tag = strValue;
					tsmiMenuItem.Click += new EventHandler(tsmiMenuItem_Click);

					tsmiOwner.DropDownItems.Add(tsmiMenuItem);
				}
			}
		}

		Font m_font = null;
		Font m_fontItalic = null;

		private void SetupRegularExpressions()
		{
			m_font = this.Font;
			m_fontItalic = new Font(m_font, FontStyle.Italic);
			XmlElement xRegularExpressions = this.listViewManager.GetRegularExpressions();

			if(xRegularExpressions != null)
			{
				XmlNodeList xListExpressions = xRegularExpressions.GetElementsByTagName("regular_expression");

				List<KeyValuePair<string, string>> listNameAndValue = new List<KeyValuePair<string, string>>(xListExpressions.Count);
				
				foreach(XmlElement xRegularExpression in xListExpressions)
				{
					string strName = XmlHelper.GetKeyValue(xRegularExpression, "name");
					string strValue = XmlHelper.GetKeyValue(xRegularExpression, "value");

					listNameAndValue.Add(new KeyValuePair<string,string>(strName, strValue));
					toolStripComboBoxRegex.Items.Add(strName);

				}
				toolStripComboBoxRegex.Tag = listNameAndValue;
				toolStripComboBoxRegex.SelectedIndex = 0;
			}
		}


		void tsmiMenuItem_Click(object sender, EventArgs e)
		{
			ToolStripMenuItem tsmi = sender as ToolStripMenuItem;

			string strCommand = tsmi.Tag as string;

			if(strCommand != null && strCommand != "")
				LaunchJob(strCommand, null, null);
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

		Regex m_regex = null;

		private void toolStripButtonRegexp_Click(object sender, EventArgs e)
		{
			//string strRegex = this.toolStripTextBoxRegexp.Text;
			listViewManager.SetRegexp(m_regex);
			if(m_regex != null)
				this.toolStripComboBoxRegex.BackColor = Color.LightGreen;

		}

		private void toolStripComboBoxRegex_TextChanged(object sender, EventArgs e)
		{

		}

		private void toolStripComboBoxRegex_TextUpdate(object sender, EventArgs e)
		{
			try
			{
				toolStripComboBoxRegex.Font = m_font;
				m_regex = new Regex(this.toolStripComboBoxRegex.Text, RegexOptions.IgnoreCase);
				this.toolStripComboBoxRegex.BackColor = SystemColors.Window;
			}
			catch
			{
				this.toolStripComboBoxRegex.BackColor = Color.Red;
				m_regex = null;
			}
		}

		private void toolStripComboBoxRegex_SelectedIndexChanged(object sender, EventArgs e)
		{
			try
			{
				int index = toolStripComboBoxRegex.SelectedIndex;
				if(index >= 0)
				{
					List<KeyValuePair<string, string>> listNameAndValue = toolStripComboBoxRegex.Tag as List<KeyValuePair<string, string>>;

					m_regex = new Regex(listNameAndValue[index].Value, RegexOptions.IgnoreCase);
					toolStripComboBoxRegex.Font = m_fontItalic;
					this.toolStripComboBoxRegex.BackColor = Color.LightCyan;
					this.toolStripComboBoxRegex.Text = listNameAndValue[index].Key;
					listViewManager.SetRegexp(m_regex);
				}
			}
			catch
			{
				this.toolStripComboBoxRegex.BackColor = Color.Red;
				m_regex = null;
			}
		}

		private void toolStripComboBoxRegex_KeyPress(object sender, KeyPressEventArgs e)
		{
			if(e.KeyChar == (char)Keys.Return)
			{
				listViewManager.SetRegexp(m_regex);
				if(m_regex != null)
					this.toolStripComboBoxRegex.BackColor = Color.LightGreen;
				e.Handled = true;
			}

		}


	}
}