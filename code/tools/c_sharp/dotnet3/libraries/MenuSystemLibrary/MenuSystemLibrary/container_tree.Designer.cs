namespace MenuSystemLibrary
{
	partial class Container_Tree
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if(disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.groupBox = new System.Windows.Forms.GroupBox();
			this.panelTreeElements = new System.Windows.Forms.Panel();
			this.treeView = new System.Windows.Forms.TreeView();
			this.groupBox.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox
			// 
			this.groupBox.Controls.Add(this.panelTreeElements);
			this.groupBox.Controls.Add(this.treeView);
			this.groupBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.groupBox.Location = new System.Drawing.Point(0, 0);
			this.groupBox.Name = "groupBox";
			this.groupBox.Size = new System.Drawing.Size(477, 251);
			this.groupBox.TabIndex = 0;
			this.groupBox.TabStop = false;
			this.groupBox.Text = "Tree groupbox";
			// 
			// panelTreeElements
			// 
			this.panelTreeElements.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.panelTreeElements.AutoScroll = true;
			this.panelTreeElements.Location = new System.Drawing.Point(204, 19);
			this.panelTreeElements.Name = "panelTreeElements";
			this.panelTreeElements.Size = new System.Drawing.Size(267, 226);
			this.panelTreeElements.TabIndex = 1;
			// 
			// treeView
			// 
			this.treeView.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)));
			this.treeView.HideSelection = false;
			this.treeView.Location = new System.Drawing.Point(6, 19);
			this.treeView.Name = "treeView";
			this.treeView.Size = new System.Drawing.Size(192, 226);
			this.treeView.TabIndex = 0;
			this.treeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView_AfterSelect);
			this.treeView.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.treeView_NodeMouseClick);
			// 
			// Container_Tree
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.Controls.Add(this.groupBox);
			this.MinimumSize = new System.Drawing.Size(350, 150);
			this.Name = "Container_Tree";
			this.Size = new System.Drawing.Size(477, 251);
			this.groupBox.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.GroupBox groupBox;
		private System.Windows.Forms.Panel panelTreeElements;
		private System.Windows.Forms.TreeView treeView;
	}
}
