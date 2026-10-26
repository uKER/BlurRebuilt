namespace file_tag_manager_2
{
	partial class fileTagManager_MainForm
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(fileTagManager_MainForm));
			this.panelMain = new System.Windows.Forms.Panel();
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.panelBusy = new System.Windows.Forms.Panel();
			this.textBoxBusy = new System.Windows.Forms.TextBox();
			this.listViewManager = new file_tag_manager_2.ListViewManager();
			this.splitter1 = new System.Windows.Forms.Splitter();
			this.treeManager = new file_tag_manager_2.TreeManager();
			this.panelPropertiesView = new System.Windows.Forms.Panel();
			this.menuSystem1 = new MenuSystemLibrary.MenuSystem();
			this.folderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
			this.menuStrip1 = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.setDatabaseToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.externalViewerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.helpToolstripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.statusStrip1 = new System.Windows.Forms.StatusStrip();
			this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripContainer1 = new System.Windows.Forms.ToolStripContainer();
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.toolStripButtonEdit = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonSubmit = new System.Windows.Forms.ToolStripButton();
			this.toolStripButtonRevert = new System.Windows.Forms.ToolStripButton();
			this.toolStrip2 = new System.Windows.Forms.ToolStrip();
			this.toolStripLabelRegexp = new System.Windows.Forms.ToolStripLabel();
			this.toolStripComboBoxRegex = new System.Windows.Forms.ToolStripComboBox();
			this.toolStripButtonRegexp = new System.Windows.Forms.ToolStripButton();
			this.panelMain.SuspendLayout();
			this.splitContainer1.Panel1.SuspendLayout();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			this.panelBusy.SuspendLayout();
			this.panelPropertiesView.SuspendLayout();
			this.menuStrip1.SuspendLayout();
			this.statusStrip1.SuspendLayout();
			this.toolStripContainer1.BottomToolStripPanel.SuspendLayout();
			this.toolStripContainer1.ContentPanel.SuspendLayout();
			this.toolStripContainer1.TopToolStripPanel.SuspendLayout();
			this.toolStripContainer1.SuspendLayout();
			this.toolStrip1.SuspendLayout();
			this.toolStrip2.SuspendLayout();
			this.SuspendLayout();
			// 
			// panelMain
			// 
			this.panelMain.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.panelMain.Controls.Add(this.splitContainer1);
			this.panelMain.Location = new System.Drawing.Point(3, 3);
			this.panelMain.Name = "panelMain";
			this.panelMain.Size = new System.Drawing.Size(908, 568);
			this.panelMain.TabIndex = 0;
			// 
			// splitContainer1
			// 
			this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer1.Location = new System.Drawing.Point(0, 0);
			this.splitContainer1.Name = "splitContainer1";
			// 
			// splitContainer1.Panel1
			// 
			this.splitContainer1.Panel1.Controls.Add(this.panelBusy);
			this.splitContainer1.Panel1.Controls.Add(this.listViewManager);
			this.splitContainer1.Panel1.Controls.Add(this.splitter1);
			this.splitContainer1.Panel1.Controls.Add(this.treeManager);
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.panelPropertiesView);
			this.splitContainer1.Size = new System.Drawing.Size(908, 568);
			this.splitContainer1.SplitterDistance = 623;
			this.splitContainer1.TabIndex = 3;
			// 
			// panelBusy
			// 
			this.panelBusy.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.panelBusy.BackColor = System.Drawing.SystemColors.Control;
			this.panelBusy.Controls.Add(this.textBoxBusy);
			this.panelBusy.Location = new System.Drawing.Point(300, 155);
			this.panelBusy.Name = "panelBusy";
			this.panelBusy.Size = new System.Drawing.Size(243, 272);
			this.panelBusy.TabIndex = 5;
			// 
			// textBoxBusy
			// 
			this.textBoxBusy.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.textBoxBusy.BackColor = System.Drawing.SystemColors.Control;
			this.textBoxBusy.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.textBoxBusy.Location = new System.Drawing.Point(3, 3);
			this.textBoxBusy.Name = "textBoxBusy";
			this.textBoxBusy.Size = new System.Drawing.Size(236, 31);
			this.textBoxBusy.TabIndex = 0;
			this.textBoxBusy.Text = "Busy";
			this.textBoxBusy.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// listViewManager
			// 
			this.listViewManager.AllowColumnReorder = true;
			this.listViewManager.Dock = System.Windows.Forms.DockStyle.Fill;
			this.listViewManager.FullRowSelect = true;
			this.listViewManager.GridLines = true;
			this.listViewManager.HideSelection = false;
			this.listViewManager.Location = new System.Drawing.Point(188, 0);
			this.listViewManager.Name = "listViewManager";
			this.listViewManager.Size = new System.Drawing.Size(435, 568);
			this.listViewManager.TabIndex = 4;
			this.listViewManager.UseCompatibleStateImageBehavior = false;
			this.listViewManager.View = System.Windows.Forms.View.Details;
			// 
			// splitter1
			// 
			this.splitter1.Location = new System.Drawing.Point(185, 0);
			this.splitter1.Name = "splitter1";
			this.splitter1.Size = new System.Drawing.Size(3, 568);
			this.splitter1.TabIndex = 1;
			this.splitter1.TabStop = false;
			// 
			// treeManager
			// 
			this.treeManager.Dock = System.Windows.Forms.DockStyle.Left;
			this.treeManager.HideSelection = false;
			this.treeManager.Location = new System.Drawing.Point(0, 0);
			this.treeManager.Name = "treeManager";
			this.treeManager.Size = new System.Drawing.Size(185, 568);
			this.treeManager.Sorted = true;
			this.treeManager.TabIndex = 3;
			this.treeManager.BeforeSelect += new System.Windows.Forms.TreeViewCancelEventHandler(this.treeManager_BeforeSelect);
			// 
			// panelPropertiesView
			// 
			this.panelPropertiesView.Controls.Add(this.menuSystem1);
			this.panelPropertiesView.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panelPropertiesView.Location = new System.Drawing.Point(0, 0);
			this.panelPropertiesView.Name = "panelPropertiesView";
			this.panelPropertiesView.Size = new System.Drawing.Size(281, 568);
			this.panelPropertiesView.TabIndex = 0;
			// 
			// menuSystem1
			// 
			this.menuSystem1.AutoValidate = System.Windows.Forms.AutoValidate.Disable;
			this.menuSystem1.BackColor = System.Drawing.SystemColors.Control;
			this.menuSystem1.CausesValidation = false;
			this.menuSystem1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.menuSystem1.Location = new System.Drawing.Point(0, 0);
			this.menuSystem1.Name = "menuSystem1";
			this.menuSystem1.Size = new System.Drawing.Size(281, 568);
			this.menuSystem1.TabIndex = 0;
			// 
			// menuStrip1
			// 
			this.menuStrip1.Dock = System.Windows.Forms.DockStyle.None;
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.viewToolStripMenuItem,
            this.helpToolstripMenuItem});
			this.menuStrip1.Location = new System.Drawing.Point(0, 0);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(914, 24);
			this.menuStrip1.TabIndex = 1;
			this.menuStrip1.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.setDatabaseToolStripMenuItem});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
			this.fileToolStripMenuItem.Text = "File";
			// 
			// setDatabaseToolStripMenuItem
			// 
			this.setDatabaseToolStripMenuItem.Name = "setDatabaseToolStripMenuItem";
			this.setDatabaseToolStripMenuItem.Size = new System.Drawing.Size(150, 22);
			this.setDatabaseToolStripMenuItem.Text = "Set Database";
			this.setDatabaseToolStripMenuItem.Click += new System.EventHandler(this.setDatabaseToolStripMenuItem_Click);
			// 
			// viewToolStripMenuItem
			// 
			this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.externalViewerToolStripMenuItem});
			this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
			this.viewToolStripMenuItem.Size = new System.Drawing.Size(41, 20);
			this.viewToolStripMenuItem.Text = "View";
			// 
			// externalViewerToolStripMenuItem
			// 
			this.externalViewerToolStripMenuItem.Checked = true;
			this.externalViewerToolStripMenuItem.CheckOnClick = true;
			this.externalViewerToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
			this.externalViewerToolStripMenuItem.Name = "externalViewerToolStripMenuItem";
			this.externalViewerToolStripMenuItem.Size = new System.Drawing.Size(160, 22);
			this.externalViewerToolStripMenuItem.Text = "External viewer";
			// 
			// helpToolstripMenuItem
			// 
			this.helpToolstripMenuItem.Name = "helpToolstripMenuItem";
			this.helpToolstripMenuItem.Size = new System.Drawing.Size(40, 20);
			this.helpToolstripMenuItem.Text = "Help";
			// 
			// statusStrip1
			// 
			this.statusStrip1.Dock = System.Windows.Forms.DockStyle.None;
			this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel});
			this.statusStrip1.Location = new System.Drawing.Point(0, 0);
			this.statusStrip1.Name = "statusStrip1";
			this.statusStrip1.Size = new System.Drawing.Size(914, 22);
			this.statusStrip1.TabIndex = 2;
			this.statusStrip1.Text = "statusStrip1";
			// 
			// toolStripStatusLabel
			// 
			this.toolStripStatusLabel.Name = "toolStripStatusLabel";
			this.toolStripStatusLabel.Size = new System.Drawing.Size(0, 17);
			// 
			// toolStripContainer1
			// 
			// 
			// toolStripContainer1.BottomToolStripPanel
			// 
			this.toolStripContainer1.BottomToolStripPanel.Controls.Add(this.statusStrip1);
			// 
			// toolStripContainer1.ContentPanel
			// 
			this.toolStripContainer1.ContentPanel.AutoScroll = true;
			this.toolStripContainer1.ContentPanel.Controls.Add(this.panelMain);
			this.toolStripContainer1.ContentPanel.Size = new System.Drawing.Size(914, 574);
			this.toolStripContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.toolStripContainer1.Location = new System.Drawing.Point(0, 0);
			this.toolStripContainer1.Name = "toolStripContainer1";
			this.toolStripContainer1.Size = new System.Drawing.Size(914, 670);
			this.toolStripContainer1.TabIndex = 3;
			this.toolStripContainer1.Text = "toolStripContainer1";
			// 
			// toolStripContainer1.TopToolStripPanel
			// 
			this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.menuStrip1);
			this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.toolStrip1);
			this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.toolStrip2);
			// 
			// toolStrip1
			// 
			this.toolStrip1.Dock = System.Windows.Forms.DockStyle.None;
			this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonEdit,
            this.toolStripButtonSubmit,
            this.toolStripButtonRevert});
			this.toolStrip1.Location = new System.Drawing.Point(3, 24);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(176, 25);
			this.toolStrip1.TabIndex = 2;
			// 
			// toolStripButtonEdit
			// 
			this.toolStripButtonEdit.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonEdit.Image")));
			this.toolStripButtonEdit.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonEdit.Name = "toolStripButtonEdit";
			this.toolStripButtonEdit.Size = new System.Drawing.Size(45, 22);
			this.toolStripButtonEdit.Text = "Edit";
			this.toolStripButtonEdit.ToolTipText = "Start editing this folder";
			this.toolStripButtonEdit.Click += new System.EventHandler(this.toolStripButtonEdit_Click);
			// 
			// toolStripButtonSubmit
			// 
			this.toolStripButtonSubmit.Enabled = false;
			this.toolStripButtonSubmit.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonSubmit.Image")));
			this.toolStripButtonSubmit.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonSubmit.Name = "toolStripButtonSubmit";
			this.toolStripButtonSubmit.Size = new System.Drawing.Size(59, 22);
			this.toolStripButtonSubmit.Text = "Submit";
			this.toolStripButtonSubmit.ToolTipText = "Submit your changes to the alienbrain database";
			this.toolStripButtonSubmit.Click += new System.EventHandler(this.toolStripButtonSubmit_Click);
			// 
			// toolStripButtonRevert
			// 
			this.toolStripButtonRevert.Enabled = false;
			this.toolStripButtonRevert.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonRevert.Image")));
			this.toolStripButtonRevert.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonRevert.Name = "toolStripButtonRevert";
			this.toolStripButtonRevert.Size = new System.Drawing.Size(60, 22);
			this.toolStripButtonRevert.Text = "Revert";
			this.toolStripButtonRevert.ToolTipText = "Undo all changes you have made to this folder";
			this.toolStripButtonRevert.Click += new System.EventHandler(this.toolStripButtonRevert_Click);
			// 
			// toolStrip2
			// 
			this.toolStrip2.Dock = System.Windows.Forms.DockStyle.None;
			this.toolStrip2.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabelRegexp,
            this.toolStripComboBoxRegex,
            this.toolStripButtonRegexp});
			this.toolStrip2.Location = new System.Drawing.Point(3, 49);
			this.toolStrip2.Name = "toolStrip2";
			this.toolStrip2.Size = new System.Drawing.Size(210, 25);
			this.toolStrip2.TabIndex = 3;
			// 
			// toolStripLabelRegexp
			// 
			this.toolStripLabelRegexp.Name = "toolStripLabelRegexp";
			this.toolStripLabelRegexp.Size = new System.Drawing.Size(50, 22);
			this.toolStripLabelRegexp.Text = "File mask";
			// 
			// toolStripComboBoxRegex
			// 
			this.toolStripComboBoxRegex.AutoToolTip = true;
			this.toolStripComboBoxRegex.CausesValidation = false;
			this.toolStripComboBoxRegex.MaxDropDownItems = 16;
			this.toolStripComboBoxRegex.Name = "toolStripComboBoxRegex";
			this.toolStripComboBoxRegex.Overflow = System.Windows.Forms.ToolStripItemOverflow.Never;
			this.toolStripComboBoxRegex.Size = new System.Drawing.Size(121, 25);
			this.toolStripComboBoxRegex.ToolTipText = "Regular expression filemask";
			this.toolStripComboBoxRegex.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.toolStripComboBoxRegex_KeyPress);
			this.toolStripComboBoxRegex.TextUpdate += new System.EventHandler(this.toolStripComboBoxRegex_TextUpdate);
			this.toolStripComboBoxRegex.SelectedIndexChanged += new System.EventHandler(this.toolStripComboBoxRegex_SelectedIndexChanged);
			this.toolStripComboBoxRegex.TextChanged += new System.EventHandler(this.toolStripComboBoxRegex_TextChanged);
			// 
			// toolStripButtonRegexp
			// 
			this.toolStripButtonRegexp.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.toolStripButtonRegexp.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonRegexp.Image")));
			this.toolStripButtonRegexp.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.toolStripButtonRegexp.Name = "toolStripButtonRegexp";
			this.toolStripButtonRegexp.Size = new System.Drawing.Size(25, 22);
			this.toolStripButtonRegexp.Text = "OK";
			this.toolStripButtonRegexp.ToolTipText = "Apply the regular expression";
			this.toolStripButtonRegexp.Click += new System.EventHandler(this.toolStripButtonRegexp_Click);
			// 
			// fileTagManager_MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(914, 670);
			this.Controls.Add(this.toolStripContainer1);
			this.MainMenuStrip = this.menuStrip1;
			this.Name = "fileTagManager_MainForm";
			this.Text = "File Tag Manager";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.fileTagManager_MainForm_FormClosing);
			this.panelMain.ResumeLayout(false);
			this.splitContainer1.Panel1.ResumeLayout(false);
			this.splitContainer1.Panel2.ResumeLayout(false);
			this.splitContainer1.ResumeLayout(false);
			this.panelBusy.ResumeLayout(false);
			this.panelBusy.PerformLayout();
			this.panelPropertiesView.ResumeLayout(false);
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.statusStrip1.ResumeLayout(false);
			this.statusStrip1.PerformLayout();
			this.toolStripContainer1.BottomToolStripPanel.ResumeLayout(false);
			this.toolStripContainer1.BottomToolStripPanel.PerformLayout();
			this.toolStripContainer1.ContentPanel.ResumeLayout(false);
			this.toolStripContainer1.TopToolStripPanel.ResumeLayout(false);
			this.toolStripContainer1.TopToolStripPanel.PerformLayout();
			this.toolStripContainer1.ResumeLayout(false);
			this.toolStripContainer1.PerformLayout();
			this.toolStrip1.ResumeLayout(false);
			this.toolStrip1.PerformLayout();
			this.toolStrip2.ResumeLayout(false);
			this.toolStrip2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel panelMain;
		private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog;
		private System.Windows.Forms.SplitContainer splitContainer1;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.Panel panelPropertiesView;
		private MenuSystemLibrary.MenuSystem menuSystem1;
		private System.Windows.Forms.MenuStrip menuStrip1;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem setDatabaseToolStripMenuItem;
		private TreeManager treeManager;
		private ListViewManager listViewManager;
		private System.Windows.Forms.StatusStrip statusStrip1;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel;
		private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
		private System.Windows.Forms.ToolStripContainer toolStripContainer1;
		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.ToolStripButton toolStripButtonEdit;
		private System.Windows.Forms.ToolStripButton toolStripButtonSubmit;
		private System.Windows.Forms.ToolStripButton toolStripButtonRevert;
		private System.Windows.Forms.Panel panelBusy;
		private System.Windows.Forms.TextBox textBoxBusy;
		private System.Windows.Forms.ToolStripMenuItem helpToolstripMenuItem;
		private System.Windows.Forms.ToolStrip toolStrip2;
		private System.Windows.Forms.ToolStripLabel toolStripLabelRegexp;
		private System.Windows.Forms.ToolStripButton toolStripButtonRegexp;
		private System.Windows.Forms.ToolStripComboBox toolStripComboBoxRegex;
		private System.Windows.Forms.ToolStripMenuItem externalViewerToolStripMenuItem;
	}
}

