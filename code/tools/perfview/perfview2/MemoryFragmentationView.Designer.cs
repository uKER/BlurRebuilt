namespace ProfilingApplication
{
	partial class MemoryFragmentationView
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MemoryFragmentationView));
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.panel2 = new System.Windows.Forms.Panel();
			this.panel1 = new ProfilingApplication.MemoryFragmentationChart();
			this.splitContainer2 = new System.Windows.Forms.SplitContainer();
			this.panel3 = new System.Windows.Forms.Panel();
			this.listView1 = new System.Windows.Forms.ListView();
			this.ID = new System.Windows.Forms.ColumnHeader();
			this.Address = new System.Windows.Forms.ColumnHeader();
			this.End = new System.Windows.Forms.ColumnHeader();
			this.AllocSize = new System.Windows.Forms.ColumnHeader();
			this.AllocTag = new System.Windows.Forms.ColumnHeader();
			this.blockLabel = new System.Windows.Forms.Label();
			this.panel4 = new System.Windows.Forms.Panel();
			this.button1 = new System.Windows.Forms.Button();
			this.listView2 = new System.Windows.Forms.ListView();
			this.Function = new System.Windows.Forms.ColumnHeader();
			this.FileHeader = new System.Windows.Forms.ColumnHeader();
			this.Line = new System.Windows.Forms.ColumnHeader();
			this.label1 = new System.Windows.Forms.Label();
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
			this.blockSizeListBox = new System.Windows.Forms.ToolStripComboBox();
			this.filterTagButton = new System.Windows.Forms.ToolStripButton();
			this.filterTagLabel = new System.Windows.Forms.ToolStripLabel();
			this.filterTagNo = new System.Windows.Forms.ToolStripTextBox();
			this.menuStrip1 = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.closeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.splitContainer1.Panel1.SuspendLayout();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.splitContainer2.Panel1.SuspendLayout();
			this.splitContainer2.Panel2.SuspendLayout();
			this.splitContainer2.SuspendLayout();
			this.panel3.SuspendLayout();
			this.panel4.SuspendLayout();
			this.toolStrip1.SuspendLayout();
			this.menuStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// splitContainer1
			// 
			this.splitContainer1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
			this.splitContainer1.Location = new System.Drawing.Point(0, 28);
			this.splitContainer1.Name = "splitContainer1";
			this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitContainer1.Panel1
			// 
			this.splitContainer1.Panel1.Controls.Add(this.panel2);
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.splitContainer2);
			this.splitContainer1.Size = new System.Drawing.Size(840, 621);
			this.splitContainer1.SplitterDistance = 372;
			this.splitContainer1.TabIndex = 1;
			// 
			// panel2
			// 
			this.panel2.AutoScroll = true;
			this.panel2.Controls.Add(this.panel1);
			this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel2.Location = new System.Drawing.Point(0, 0);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(840, 372);
			this.panel2.TabIndex = 1;
			// 
			// panel1
			// 
			this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(840, 391);
			this.panel1.TabIndex = 0;
			// 
			// splitContainer2
			// 
			this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer2.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
			this.splitContainer2.Location = new System.Drawing.Point(0, 0);
			this.splitContainer2.Name = "splitContainer2";
			// 
			// splitContainer2.Panel1
			// 
			this.splitContainer2.Panel1.Controls.Add(this.panel3);
			// 
			// splitContainer2.Panel2
			// 
			this.splitContainer2.Panel2.Controls.Add(this.panel4);
			this.splitContainer2.Size = new System.Drawing.Size(840, 245);
			this.splitContainer2.SplitterDistance = 420;
			this.splitContainer2.TabIndex = 1;
			// 
			// panel3
			// 
			this.panel3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel3.Controls.Add(this.listView1);
			this.panel3.Controls.Add(this.blockLabel);
			this.panel3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel3.Location = new System.Drawing.Point(0, 0);
			this.panel3.Name = "panel3";
			this.panel3.Size = new System.Drawing.Size(420, 245);
			this.panel3.TabIndex = 2;
			// 
			// listView1
			// 
			this.listView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.ID,
            this.Address,
            this.End,
            this.AllocSize,
            this.AllocTag});
			this.listView1.Location = new System.Drawing.Point(-2, 37);
			this.listView1.Name = "listView1";
			this.listView1.Size = new System.Drawing.Size(420, 206);
			this.listView1.TabIndex = 0;
			this.listView1.UseCompatibleStateImageBehavior = false;
			this.listView1.View = System.Windows.Forms.View.Details;
			this.listView1.SelectedIndexChanged += new System.EventHandler(this.listView1_SelectedIndexChanged);
			this.listView1.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView1_ColumnClick);
			// 
			// ID
			// 
			this.ID.Text = "Tracking ID";
			this.ID.Width = 139;
			// 
			// Address
			// 
			this.Address.Text = "Address";
			this.Address.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			this.Address.Width = 81;
			// 
			// End
			// 
			this.End.Text = "End";
			this.End.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			this.End.Width = 73;
			// 
			// AllocSize
			// 
			this.AllocSize.Text = "Size";
			this.AllocSize.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			this.AllocSize.Width = 66;
			// 
			// AllocTag
			// 
			this.AllocTag.Text = "Tag";
			this.AllocTag.Width = 33;
			// 
			// blockLabel
			// 
			this.blockLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.blockLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.blockLabel.Location = new System.Drawing.Point(3, 7);
			this.blockLabel.Name = "blockLabel";
			this.blockLabel.Size = new System.Drawing.Size(410, 23);
			this.blockLabel.TabIndex = 1;
			this.blockLabel.Text = "Allocations";
			this.blockLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// panel4
			// 
			this.panel4.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.panel4.Controls.Add(this.button1);
			this.panel4.Controls.Add(this.listView2);
			this.panel4.Controls.Add(this.label1);
			this.panel4.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel4.Location = new System.Drawing.Point(0, 0);
			this.panel4.Name = "panel4";
			this.panel4.Size = new System.Drawing.Size(416, 245);
			this.panel4.TabIndex = 3;
			// 
			// button1
			// 
			this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button1.Image = ((System.Drawing.Image)(resources.GetObject("button1.Image")));
			this.button1.Location = new System.Drawing.Point(378, 4);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(31, 28);
			this.button1.TabIndex = 3;
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// listView2
			// 
			this.listView2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.listView2.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.Function,
            this.FileHeader,
            this.Line});
			this.listView2.Location = new System.Drawing.Point(-2, 37);
			this.listView2.Name = "listView2";
			this.listView2.Size = new System.Drawing.Size(416, 206);
			this.listView2.TabIndex = 0;
			this.listView2.UseCompatibleStateImageBehavior = false;
			this.listView2.View = System.Windows.Forms.View.Details;
			this.listView2.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.listView2_MouseDoubleClick);
			// 
			// Function
			// 
			this.Function.Text = "Function";
			this.Function.Width = 202;
			// 
			// FileHeader
			// 
			this.FileHeader.Text = "File";
			this.FileHeader.Width = 137;
			// 
			// Line
			// 
			this.Line.Text = "Line";
			this.Line.Width = 45;
			// 
			// label1
			// 
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label1.Location = new System.Drawing.Point(3, 7);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(406, 20);
			this.label1.TabIndex = 2;
			this.label1.Text = "Call Stack";
			this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// toolStrip1
			// 
			this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabel1,
            this.blockSizeListBox,
            this.filterTagButton,
            this.filterTagLabel,
            this.filterTagNo});
			this.toolStrip1.Location = new System.Drawing.Point(0, 0);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(840, 25);
			this.toolStrip1.TabIndex = 2;
			this.toolStrip1.Text = "toolStrip1";
			// 
			// toolStripLabel1
			// 
			this.toolStripLabel1.Name = "toolStripLabel1";
			this.toolStripLabel1.Size = new System.Drawing.Size(53, 22);
			this.toolStripLabel1.Text = "Blocksize:";
			// 
			// blockSizeListBox
			// 
			this.blockSizeListBox.Items.AddRange(new object[] {
            "1MB",
            "512K",
            "256K",
            "128K",
            "64K",
            "32K",
            "16K",
            "8K",
            "4K",
            "2K",
            "1K"});
			this.blockSizeListBox.Name = "blockSizeListBox";
			this.blockSizeListBox.Size = new System.Drawing.Size(121, 25);
			this.blockSizeListBox.SelectedIndexChanged += new System.EventHandler(this.blockSizeListBox_SelectedIndexChanged);
			// 
			// filterTagButton
			// 
			this.filterTagButton.CheckOnClick = true;
			this.filterTagButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.filterTagButton.Image = ((System.Drawing.Image)(resources.GetObject("filterTagButton.Image")));
			this.filterTagButton.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.filterTagButton.Name = "filterTagButton";
			this.filterTagButton.Size = new System.Drawing.Size(23, 22);
			this.filterTagButton.Text = "Enable Filtering by Tag";
			this.filterTagButton.Click += new System.EventHandler(this.filterTagButton_Click);
			// 
			// filterTagLabel
			// 
			this.filterTagLabel.Enabled = false;
			this.filterTagLabel.Name = "filterTagLabel";
			this.filterTagLabel.Size = new System.Drawing.Size(56, 22);
			this.filterTagLabel.Text = "Filter Tag:";
			// 
			// filterTagNo
			// 
			this.filterTagNo.Enabled = false;
			this.filterTagNo.Name = "filterTagNo";
			this.filterTagNo.Size = new System.Drawing.Size(20, 25);
			this.filterTagNo.Text = "1";
			this.filterTagNo.ToolTipText = "Filter Tag Number";
			this.filterTagNo.Validated += new System.EventHandler(this.filterTagNo_Validated);
			// 
			// menuStrip1
			// 
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
			this.menuStrip1.Location = new System.Drawing.Point(0, 25);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(840, 24);
			this.menuStrip1.TabIndex = 3;
			this.menuStrip1.Text = "menuStrip1";
			this.menuStrip1.Visible = false;
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.saveToolStripMenuItem,
            this.closeToolStripMenuItem});
			this.fileToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.MatchOnly;
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
			this.fileToolStripMenuItem.Text = "&File";
			// 
			// saveToolStripMenuItem
			// 
			this.saveToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.saveToolStripMenuItem.MergeIndex = 2;
			this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
			this.saveToolStripMenuItem.Size = new System.Drawing.Size(121, 22);
			this.saveToolStripMenuItem.Text = "&Save...";
			this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
			// 
			// closeToolStripMenuItem
			// 
			this.closeToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.closeToolStripMenuItem.MergeIndex = 3;
			this.closeToolStripMenuItem.Name = "closeToolStripMenuItem";
			this.closeToolStripMenuItem.Size = new System.Drawing.Size(121, 22);
			this.closeToolStripMenuItem.Text = "&Close";
			this.closeToolStripMenuItem.Click += new System.EventHandler(this.closeToolStripMenuItem_Click);
			// 
			// MemoryFragmentationView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(840, 649);
			this.Controls.Add(this.menuStrip1);
			this.Controls.Add(this.toolStrip1);
			this.Controls.Add(this.splitContainer1);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "MemoryFragmentationView";
			this.Text = "Memory View";
			this.Activated += new System.EventHandler(this.MemoryFragmentationView_Activated);
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MemoryFragmentationView_FormClosing);
			this.splitContainer1.Panel1.ResumeLayout(false);
			this.splitContainer1.Panel2.ResumeLayout(false);
			this.splitContainer1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			this.splitContainer2.Panel1.ResumeLayout(false);
			this.splitContainer2.Panel2.ResumeLayout(false);
			this.splitContainer2.ResumeLayout(false);
			this.panel3.ResumeLayout(false);
			this.panel4.ResumeLayout(false);
			this.toolStrip1.ResumeLayout(false);
			this.toolStrip1.PerformLayout();
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private MemoryFragmentationChart panel1;
		private System.Windows.Forms.SplitContainer splitContainer1;
		private System.Windows.Forms.SplitContainer splitContainer2;
		private System.Windows.Forms.ListView listView1;
		private System.Windows.Forms.ColumnHeader ID;
		private System.Windows.Forms.ColumnHeader Address;
		private System.Windows.Forms.ColumnHeader End;
		private System.Windows.Forms.ColumnHeader AllocSize;
		private System.Windows.Forms.ListView listView2;
		private System.Windows.Forms.ColumnHeader Function;
		private System.Windows.Forms.ColumnHeader FileHeader;
		private System.Windows.Forms.ColumnHeader Line;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.Label blockLabel;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Panel panel3;
		private System.Windows.Forms.Panel panel4;
		private System.Windows.Forms.ColumnHeader AllocTag;
		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.ToolStripLabel toolStripLabel1;
		private System.Windows.Forms.ToolStripLabel filterTagLabel;
		private System.Windows.Forms.ToolStripTextBox filterTagNo;
		private System.Windows.Forms.ToolStripComboBox blockSizeListBox;
		private System.Windows.Forms.ToolStripButton filterTagButton;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.MenuStrip menuStrip1;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem closeToolStripMenuItem;
	}
}