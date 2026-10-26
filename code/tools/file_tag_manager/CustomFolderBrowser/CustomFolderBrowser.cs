using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.IO;

namespace CustomFolderBrowser
{
	/// <summary>
	/// Summary description for UserControl1.
	/// </summary>
	public class CustomFolderBrowser : System.Windows.Forms.UserControl
	{
		private System.Windows.Forms.Panel treePanel;
		private System.Windows.Forms.TreeView treeView;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public CustomFolderBrowser()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitComponent call

			// Temp debug code.
//			this.SetRootFolder(@"c:\Test pictures");


		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if( components != null )
					components.Dispose();
			}
			base.Dispose( disposing );
		}

		#region Component Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.treePanel = new System.Windows.Forms.Panel();
			this.treeView = new System.Windows.Forms.TreeView();
			this.treePanel.SuspendLayout();
			this.SuspendLayout();
			// 
			// treePanel
			// 
			this.treePanel.Controls.Add(this.treeView);
			this.treePanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.treePanel.Location = new System.Drawing.Point(0, 0);
			this.treePanel.Name = "treePanel";
			this.treePanel.Size = new System.Drawing.Size(184, 280);
			this.treePanel.TabIndex = 0;
			// 
			// treeView
			// 
			this.treeView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.treeView.ImageIndex = -1;
			this.treeView.Location = new System.Drawing.Point(0, 0);
			this.treeView.Name = "treeView";
			this.treeView.SelectedImageIndex = -1;
			this.treeView.Size = new System.Drawing.Size(184, 280);
			this.treeView.TabIndex = 0;
			this.treeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView_AfterSelect);
			// 
			// CustomFolderBrowser
			// 
			this.Controls.Add(this.treePanel);
			this.Name = "CustomFolderBrowser";
			this.Size = new System.Drawing.Size(184, 280);
			this.treePanel.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion


		#region Member variables.
		string m_strPath;
		#endregion Member variables.

		public class TreeTagInfo
		{
			public string strPath;
		}


		#region Exposed events

		protected delegate void TreePaintDelegate(object sender, System.Windows.Forms.PaintEventArgs e);
		[RefreshProperties(RefreshProperties.Repaint), Description("Description here."), Category("Keefs Events"), Browsable(true)]
		protected event TreePaintDelegate TreePaint;
		private void tree_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
		{
			if(TreePaint != null)
				TreePaint(sender, e);
		}

		protected delegate void TreeMouseDownDelegate(object sender, System.Windows.Forms.MouseEventArgs e);
		[RefreshProperties(RefreshProperties.Repaint), Description("Description here."), Category("Keefs Events"), Browsable(true)]
		protected event TreeMouseDownDelegate TreeMouseDown;
		private void tree_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if(TreeMouseDown != null)
				TreeMouseDown(sender, e);
		}

		protected delegate void TreeMouseEnterDelegate(object sender, System.EventArgs e);
		[RefreshProperties(RefreshProperties.Repaint), Description("Description here."), Category("Keefs Events"), Browsable(true)]
		protected event TreeMouseEnterDelegate TreeMouseEnter;
		private void tree_MouseEnter(object sender, System.EventArgs e)
		{
			if(TreeMouseEnter != null)
				TreeMouseEnter(sender, e);
		}


		#endregion Exposed events

		public delegate void TreeAfterSelectDelegate(object sender, System.Windows.Forms.TreeViewEventArgs e);
		[RefreshProperties(RefreshProperties.Repaint), Description("After Select Description here."), Category("Keefs Events"), Browsable(true)]
		public event TreeAfterSelectDelegate TreeAfterSelect;

		private void treeView_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
		{
			if(TreeAfterSelect != null)
				TreeAfterSelect(sender, e);
		}


		public bool SetRootFolder(string strPath)
		{
			m_strPath = strPath;
			this.treeView.Nodes.Clear();

			bool retVal = PopulateTree(this.treeView.Nodes, strPath);

			this.treeView.SelectedNode = this.treeView.Nodes[0];
//			this.treeView.SelectedNode = null;
			return retVal;


		}

		public bool RefreshTree()
		{
			if(m_strPath != null)
				return PopulateTree(this.treeView.Nodes, m_strPath);
			else
				return false;
		}

		public TreeNode CurrentSelectedItem{get{return this.treeView.SelectedNode;}}

		private bool PopulateTree(TreeNodeCollection nodes, string strPath)
		{
			try 
			{
				DirectoryInfo di = new DirectoryInfo(strPath);

				TreeNode tn;
				tn = nodes.Add(di.Name);
				TreeTagInfo tti = new TreeTagInfo();
				tti.strPath = di.FullName;
				tn.Tag = tti;

				FileSystemInfo[] dirs = di.GetDirectories();
				foreach (DirectoryInfo diNext in dirs) 
				{
					if((diNext.Attributes & FileAttributes.Hidden) != FileAttributes.Hidden)
					{
						PopulateTree(tn.Nodes, diNext.FullName);
					}
				}
			} 
			catch (Exception e) 
			{
				Console.WriteLine("The process failed: {0}", e.ToString());
				MessageBox.Show(this.Parent, "Failed to populate the tree.\n\nThe system reports:'"+e.ToString(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Hand);
			}
			return true;
		}



	}
}
