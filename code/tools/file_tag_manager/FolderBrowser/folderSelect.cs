/*
 * 
 * FolderSelect.cs: A folder browser example
 * Version 1.0
 * Copyright (C) 2001 Chris Warner
 * 
 * You are free to use this in any personal or commercial application
 * so long as none of these comments are changed or removed from this file.
 * 
 * E-mail: jabrwoky@pacbell.net
 * */
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;

namespace folderSelect
{
	/// <summary> class FolderSelect 
	/// <para>An example on how to build a folder browser dialog window using C# and the .Net framework.</para>
	/// </summary>
	public class FolderSelect : System.Windows.Forms.Form
	{
		private static string driveLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		private DirectoryInfo folder;

		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Panel panel3;
		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TreeView treeView1;
		private System.Windows.Forms.Button cancelBtn;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.Button selectBtn;
		private System.Windows.Forms.ImageList imageList1;
		private System.ComponentModel.IContainer components;

		public FolderSelect()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//

			// initialize the treeView
			fillTree();
		}

		/// <summary> method fillTree
		/// <para>This method is used to initially fill the treeView control with a list
		/// of available drives from which you can search for the desired folder.</para>
		/// </summary>
		private void fillTree()
		{
			DirectoryInfo directory;
			string sCurPath = "";

			// clear out the old values
			treeView1.Nodes.Clear();

			// loop through the drive letters and find the available drives.
			foreach( char c in driveLetters )
			{
				sCurPath = c + ":\\";
				try 
				{
					// get the directory informaiton for this path.
					directory = new DirectoryInfo(sCurPath);

					// if the retrieved directory information points to a valid
					// directory or drive in this case, add it to the root of the 
					// treeView.
					if ( directory.Exists == true )
					{
						TreeNode newNode = new TreeNode(directory.FullName);
						treeView1.Nodes.Add(newNode);	// add the new node to the root level.
						getSubDirs(newNode);			// scan for any sub folders on this drive.
					}
				}
				catch( Exception doh)
				{
					Console.WriteLine(doh.Message);
				}
			}
		}

		/// <summary> method getSubDirs
		/// <para>this function will scan the specified parent for any subfolders 
		/// if they exist.  To minimize the memory usage, we only scan a single 
		/// folder level down from the existing, and only if it is needed.</para>
		/// <param name="parent">the parent folder in which to search for sub-folders.</param>
		/// </summary>
		private void getSubDirs( TreeNode parent )
		{
			DirectoryInfo directory;
			try
			{
				// if we have not scanned this folder before
				if ( parent.Nodes.Count == 0 )
				{
					directory = new DirectoryInfo(parent.FullPath);
					foreach( DirectoryInfo dir in directory.GetDirectories())
					{
						TreeNode newNode = new TreeNode(dir.Name);
						parent.Nodes.Add(newNode);
					}
				}

				// now that we have the children of the parent, see if they
				// have any child members that need to be scanned.  Scanning 
				// the first level of sub folders insures that you properly 
				// see the '+' or '-' expanding controls on each node that represents
				// a sub folder with it's own children.
				foreach(TreeNode node in parent.Nodes)
				{
					// if we have not scanned this node before.
					if (node.Nodes.Count == 0)
					{
						// get the folder information for the specified path.
						directory = new DirectoryInfo(node.FullPath);

						// check this folder for any possible sub-folders
						foreach( DirectoryInfo dir in directory.GetDirectories())
						{
							// make a new TreeNode and add it to the treeView.
							TreeNode newNode = new TreeNode(dir.Name);
							node.Nodes.Add(newNode);
						}
					}
				}
			}
			catch( Exception doh )
			{
				Console.WriteLine(doh.Message);
			}
		}

		/// <summary> method fixPath
		/// <para>For some reason, the treeView will only work with paths constructed like the following example.
		/// "c:\\Program Files\Microsoft\...".  What this method does is strip the leading "\\" next to the drive
		/// letter.</para>
		/// <param name="node">the folder that needs it's path fixed for display.</param>
		/// <returns>The correctly formatted full path to the selected folder.</returns>
		/// </summary>
		private string fixPath( TreeNode node )
		{
			string sRet = "";
			try
			{
				sRet = node.FullPath;
				int index = sRet.IndexOf("\\\\");
				if (index > 1 )
				{
					sRet = node.FullPath.Remove(index, 1);
				}
			}
			catch( Exception doh )
			{
				Console.WriteLine(doh.Message);
			}
			return sRet;
		}
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(FolderSelect));
			this.panel1 = new System.Windows.Forms.Panel();
			this.label1 = new System.Windows.Forms.Label();
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.panel2 = new System.Windows.Forms.Panel();
			this.cancelBtn = new System.Windows.Forms.Button();
			this.selectBtn = new System.Windows.Forms.Button();
			this.panel3 = new System.Windows.Forms.Panel();
			this.treeView1 = new System.Windows.Forms.TreeView();
			this.imageList1 = new System.Windows.Forms.ImageList(this.components);
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.panel3.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.label1,
																				 this.textBox1});
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.DockPadding.All = 10;
			this.panel1.Location = new System.Drawing.Point(6, 6);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(280, 72);
			this.panel1.TabIndex = 0;
			// 
			// label1
			// 
			this.label1.Dock = System.Windows.Forms.DockStyle.Top;
			this.label1.Location = new System.Drawing.Point(10, 10);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(260, 23);
			this.label1.TabIndex = 1;
			this.label1.Text = "Full Path";
			// 
			// textBox1
			// 
			this.textBox1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.textBox1.Location = new System.Drawing.Point(10, 42);
			this.textBox1.Name = "textBox1";
			this.textBox1.Size = new System.Drawing.Size(260, 20);
			this.textBox1.TabIndex = 0;
			this.textBox1.Text = "";
			// 
			// panel2
			// 
			this.panel2.AutoScroll = true;
			this.panel2.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.cancelBtn,
																				 this.selectBtn});
			this.panel2.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel2.DockPadding.All = 10;
			this.panel2.Location = new System.Drawing.Point(6, 215);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(280, 48);
			this.panel2.TabIndex = 1;
			// 
			// cancelBtn
			// 
			this.cancelBtn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelBtn.Dock = System.Windows.Forms.DockStyle.Right;
			this.cancelBtn.Location = new System.Drawing.Point(195, 10);
			this.cancelBtn.Name = "cancelBtn";
			this.cancelBtn.Size = new System.Drawing.Size(75, 28);
			this.cancelBtn.TabIndex = 3;
			this.cancelBtn.Text = "Cancel";
			this.cancelBtn.Click += new System.EventHandler(this.cancelBtn_Click);
			// 
			// selectBtn
			// 
			this.selectBtn.Dock = System.Windows.Forms.DockStyle.Left;
			this.selectBtn.Location = new System.Drawing.Point(10, 10);
			this.selectBtn.Name = "selectBtn";
			this.selectBtn.Size = new System.Drawing.Size(75, 28);
			this.selectBtn.TabIndex = 2;
			this.selectBtn.Text = "Select";
			this.selectBtn.Click += new System.EventHandler(this.selectBtn_Click);
			// 
			// panel3
			// 
			this.panel3.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.treeView1});
			this.panel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel3.Location = new System.Drawing.Point(6, 78);
			this.panel3.Name = "panel3";
			this.panel3.Size = new System.Drawing.Size(280, 137);
			this.panel3.TabIndex = 2;
			// 
			// treeView1
			// 
			this.treeView1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.treeView1.ImageList = this.imageList1;
			this.treeView1.ImeMode = System.Windows.Forms.ImeMode.NoControl;
			this.treeView1.Name = "treeView1";
			this.treeView1.Nodes.AddRange(new System.Windows.Forms.TreeNode[] {
																				  new System.Windows.Forms.TreeNode("Node0", new System.Windows.Forms.TreeNode[] {
																																									 new System.Windows.Forms.TreeNode("Node1", new System.Windows.Forms.TreeNode[] {
																																																														new System.Windows.Forms.TreeNode("Node2")})})});
			this.treeView1.SelectedImageIndex = 1;
			this.treeView1.Size = new System.Drawing.Size(280, 137);
			this.treeView1.TabIndex = 0;
			this.treeView1.BeforeSelect += new System.Windows.Forms.TreeViewCancelEventHandler(this.treeView1_BeforeSelect);
			this.treeView1.BeforeExpand += new System.Windows.Forms.TreeViewCancelEventHandler(this.treeView1_BeforeExpand);
			// 
			// imageList1
			// 
			this.imageList1.ColorDepth = System.Windows.Forms.ColorDepth.Depth8Bit;
			this.imageList1.ImageSize = new System.Drawing.Size(16, 16);
			this.imageList1.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList1.ImageStream")));
			this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// FolderSelect
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(292, 269);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.panel3,
																		  this.panel2,
																		  this.panel1});
			this.DockPadding.All = 6;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.MinimumSize = new System.Drawing.Size(230, 300);
			this.Name = "FolderSelect";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Show;
			this.Text = "Folder Select Example";
			this.panel1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			this.panel3.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary> method treeView1_BeforeSelect
		/// <para>Before we select a tree node we want to make sure that we scan the soon to be selected
		/// tree node for any sub-folders.  this insures proper tree construction on the fly.</para>
		/// <param name="sender">The object that invoked this event</param>
		/// <param name="e">The TreeViewCancelEventArgs event arguments.</param>
		/// <see cref="System.Windows.Forms.TreeViewCancelEventArgs"/>
		/// <see cref="System.Windows.Forms.TreeView"/>
		/// </summary>
		private void treeView1_BeforeSelect(object sender, System.Windows.Forms.TreeViewCancelEventArgs e)
		{
			getSubDirs(e.Node);					// get the sub-folders for the selected node.
			textBox1.Text = fixPath(e.Node);	// update the user selection text box.
			folder = new DirectoryInfo(e.Node.FullPath);	// get it's Directory info.
		}

		/// <summary> method treeView1_BeforeExpand
		/// <para>Before we expand a tree node we want to make sure that we scan the soon to be expanded
		/// tree node for any sub-folders.  this insures proper tree construction on the fly.</para>
		/// <param name="sender">The object that invoked this event</param>
		/// <param name="e">The TreeViewCancelEventArgs event arguments.</param>
		/// <see cref="System.Windows.Forms.TreeViewCancelEventArgs"/>
		/// <see cref="System.Windows.Forms.TreeView"/>
		/// </summary>
		private void treeView1_BeforeExpand(object sender, System.Windows.Forms.TreeViewCancelEventArgs e)
		{
			getSubDirs(e.Node);					// get the sub-folders for the selected node.
			textBox1.Text = fixPath(e.Node);	// update the user selection text box.
			folder = new DirectoryInfo(e.Node.FullPath);	// get it's Directory info.
		}

		/// <summary> method cancelBtn_Click
		/// <para>This method cancels the folder browsing.</para>
		/// </summary>
		private void cancelBtn_Click(object sender, System.EventArgs e)
		{
			folder = null;
			this.Close();
		}

		/// <summary> method selectBtn_Click
		/// <para>This method accepts which ever folder is selected and closes this application 
		/// with a DialogResult.OK result if you invoke this form though Form.ShowDialog().  
		/// In this example this method simply looks up the selected folder, and presents the 
		/// user with a message box displaying the name and path of the selected folder.
		/// </para>
		/// </summary>
		private void selectBtn_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = DialogResult.OK;
			this.Close();
		}

		/// <summary> method name
		/// <para>A method to retrieve the short name for the selected folder.</para>
		/// <returns>The folder name for the selected folder.</returns>
		/// </summary>
		public string name
		{
			get { return ((folder != null && folder.Exists))? folder.Name : null; }
		}

		/// <summary> method fullPath
		/// <para>Retrieve the full path for the selected folder.</para>
		/// 
		/// <returns>The correctly formatted full path to the selected folder.</returns>
		/// <seealso cref="FolderSelect.fixPath"/>
		/// </summary>
		public string fullPath
		{
			get { return ((folder != null && folder.Exists && treeView1.SelectedNode != null))? fixPath(treeView1.SelectedNode) : null; }
		}

		/// <summary> method info
		/// <para>Retrieve the full DirectoryInfo object associated with the selected folder.  Note
		/// that this will not have the corrected full path string.</para>
		/// <returns>The full DirectoryInfo object associated with the selected folder.</returns>
		/// <see cref="System.IO.DirectoryInfo"/>
		/// </summary>
		public DirectoryInfo info
		{
			get { return ((folder != null && folder.Exists))? folder : null; }
		}
	}
}
