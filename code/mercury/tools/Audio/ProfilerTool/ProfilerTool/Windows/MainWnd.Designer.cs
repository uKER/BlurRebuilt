namespace ProfilerTool.Windows
{
    partial class MainWnd
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
            if (disposing && (components != null))
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWnd));
            this.mnuMain = new System.Windows.Forms.MenuStrip();
            this.mnuFileItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mnuEditItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mnuTargetItem = new System.Windows.Forms.ToolStripMenuItem();
            this.connectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.tbMain = new System.Windows.Forms.ToolStrip();
            this.tbbConnect = new System.Windows.Forms.ToolStripButton();
            this.tbspMain = new System.Windows.Forms.ToolStripSeparator();
            this.tbbRefresh = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.tbbLoadSnapshot = new System.Windows.Forms.ToolStripButton();
            this.tbbSaveSnapshot = new System.Windows.Forms.ToolStripButton();
            this.ssMain = new System.Windows.Forms.StatusStrip();
            this.sslblMain = new System.Windows.Forms.ToolStripStatusLabel();
            this.splMain = new System.Windows.Forms.SplitContainer();
            this.tbcInstances = new System.Windows.Forms.TabControl();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.tbbGroupComponents = new System.Windows.Forms.ToolStripButton();
            this.tvObjects = new ProfilerTool.Controls.MainTreeView();
            this.stats = new ProfilerTool.Windows.Stats();
            this.mnuMain.SuspendLayout();
            this.tbMain.SuspendLayout();
            this.ssMain.SuspendLayout();
            this.splMain.Panel1.SuspendLayout();
            this.splMain.Panel2.SuspendLayout();
            this.splMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // mnuMain
            // 
            this.mnuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFileItem,
            this.mnuEditItem,
            this.mnuTargetItem});
            this.mnuMain.Location = new System.Drawing.Point(0, 0);
            this.mnuMain.Name = "mnuMain";
            this.mnuMain.Size = new System.Drawing.Size(1016, 24);
            this.mnuMain.TabIndex = 0;
            this.mnuMain.Text = "Main";
            // 
            // mnuFileItem
            // 
            this.mnuFileItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exitToolStripMenuItem});
            this.mnuFileItem.Name = "mnuFileItem";
            this.mnuFileItem.Size = new System.Drawing.Size(35, 20);
            this.mnuFileItem.Text = "File";
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.X)));
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // mnuEditItem
            // 
            this.mnuEditItem.Name = "mnuEditItem";
            this.mnuEditItem.Size = new System.Drawing.Size(37, 20);
            this.mnuEditItem.Text = "Edit";
            // 
            // mnuTargetItem
            // 
            this.mnuTargetItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.connectToolStripMenuItem});
            this.mnuTargetItem.Name = "mnuTargetItem";
            this.mnuTargetItem.Size = new System.Drawing.Size(51, 20);
            this.mnuTargetItem.Text = "Target";
            // 
            // connectToolStripMenuItem
            // 
            this.connectToolStripMenuItem.Name = "connectToolStripMenuItem";
            this.connectToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Shift)
                        | System.Windows.Forms.Keys.C)));
            this.connectToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.connectToolStripMenuItem.Text = "Connect";
            this.connectToolStripMenuItem.Click += new System.EventHandler(this.connectToolStripMenuItem_Click);
            // 
            // tbMain
            // 
            this.tbMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tbbConnect,
            this.tbspMain,
            this.tbbRefresh,
            this.toolStripSeparator1,
            this.tbbLoadSnapshot,
            this.tbbSaveSnapshot,
            this.toolStripSeparator2,
            this.tbbGroupComponents});
            this.tbMain.Location = new System.Drawing.Point(0, 24);
            this.tbMain.Name = "tbMain";
            this.tbMain.Size = new System.Drawing.Size(1016, 25);
            this.tbMain.TabIndex = 1;
            this.tbMain.Text = "tbMain";
            // 
            // tbbConnect
            // 
            this.tbbConnect.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tbbConnect.Image = ((System.Drawing.Image)(resources.GetObject("tbbConnect.Image")));
            this.tbbConnect.ImageTransparentColor = System.Drawing.Color.White;
            this.tbbConnect.Name = "tbbConnect";
            this.tbbConnect.Size = new System.Drawing.Size(23, 22);
            this.tbbConnect.Text = "Connect";
            this.tbbConnect.ToolTipText = "Connect";
            this.tbbConnect.Click += new System.EventHandler(this.tbbConnect_Click);
            // 
            // tbspMain
            // 
            this.tbspMain.Name = "tbspMain";
            this.tbspMain.Size = new System.Drawing.Size(6, 25);
            // 
            // tbbRefresh
            // 
            this.tbbRefresh.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tbbRefresh.Image = global::ProfilerTool.Properties.Resources.refreshbutton;
            this.tbbRefresh.ImageTransparentColor = System.Drawing.Color.White;
            this.tbbRefresh.Name = "tbbRefresh";
            this.tbbRefresh.Size = new System.Drawing.Size(23, 22);
            this.tbbRefresh.Text = "Refresh";
            this.tbbRefresh.ToolTipText = "Refresh";
            this.tbbRefresh.Click += new System.EventHandler(this.tbbRefresh_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
            // 
            // tbbLoadSnapshot
            // 
            this.tbbLoadSnapshot.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tbbLoadSnapshot.Image = global::ProfilerTool.Properties.Resources.loadmixbutton;
            this.tbbLoadSnapshot.ImageTransparentColor = System.Drawing.Color.White;
            this.tbbLoadSnapshot.Name = "tbbLoadSnapshot";
            this.tbbLoadSnapshot.Size = new System.Drawing.Size(23, 22);
            this.tbbLoadSnapshot.Text = "Load mixer snapshot";
            this.tbbLoadSnapshot.Click += new System.EventHandler(this.tbbLoadSnapshot_Click);
            // 
            // tbbSaveSnapshot
            // 
            this.tbbSaveSnapshot.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tbbSaveSnapshot.Image = global::ProfilerTool.Properties.Resources.savemixbutton;
            this.tbbSaveSnapshot.ImageTransparentColor = System.Drawing.Color.White;
            this.tbbSaveSnapshot.Name = "tbbSaveSnapshot";
            this.tbbSaveSnapshot.Size = new System.Drawing.Size(23, 22);
            this.tbbSaveSnapshot.Text = "Save mixer snapshot";
            this.tbbSaveSnapshot.Click += new System.EventHandler(this.tbbSaveSnapshot_Click);
            // 
            // ssMain
            // 
            this.ssMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.sslblMain});
            this.ssMain.Location = new System.Drawing.Point(0, 712);
            this.ssMain.Name = "ssMain";
            this.ssMain.Size = new System.Drawing.Size(1016, 22);
            this.ssMain.TabIndex = 3;
            this.ssMain.Text = "ssMain";
            // 
            // sslblMain
            // 
            this.sslblMain.Name = "sslblMain";
            this.sslblMain.Size = new System.Drawing.Size(76, 17);
            this.sslblMain.Text = "not connected";
            // 
            // splMain
            // 
            this.splMain.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.splMain.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splMain.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splMain.Location = new System.Drawing.Point(0, 49);
            this.splMain.Name = "splMain";
            // 
            // splMain.Panel1
            // 
            this.splMain.Panel1.Controls.Add(this.tvObjects);
            this.splMain.Panel1.Padding = new System.Windows.Forms.Padding(5);
            // 
            // splMain.Panel2
            // 
            this.splMain.Panel2.AutoScroll = true;
            this.splMain.Panel2.Controls.Add(this.tbcInstances);
            this.splMain.Panel2.Padding = new System.Windows.Forms.Padding(5);
            this.splMain.Size = new System.Drawing.Size(1016, 663);
            this.splMain.SplitterDistance = 320;
            this.splMain.TabIndex = 12;
            // 
            // tbcInstances
            // 
            this.tbcInstances.Appearance = System.Windows.Forms.TabAppearance.FlatButtons;
            this.tbcInstances.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tbcInstances.Location = new System.Drawing.Point(5, 5);
            this.tbcInstances.Name = "tbcInstances";
            this.tbcInstances.SelectedIndex = 0;
            this.tbcInstances.Size = new System.Drawing.Size(680, 651);
            this.tbcInstances.TabIndex = 11;
            this.tbcInstances.SelectedIndexChanged += new System.EventHandler(this.tbcInstances_SelectedIndexChanged);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(6, 25);
            // 
            // tbbGroupComponents
            // 
            this.tbbGroupComponents.CheckOnClick = true;
            this.tbbGroupComponents.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tbbGroupComponents.Image = ((System.Drawing.Image)(resources.GetObject("tbbGroupComponents.Image")));
            this.tbbGroupComponents.ImageTransparentColor = System.Drawing.Color.White;
            this.tbbGroupComponents.Name = "tbbGroupComponents";
            this.tbbGroupComponents.Size = new System.Drawing.Size(23, 22);
            this.tbbGroupComponents.Text = "Group components";
            this.tbbGroupComponents.Click += new System.EventHandler(this.tbbGroupComponents_Click);
            // 
            // tvObjects
            // 
            this.tvObjects.BackColor = System.Drawing.SystemColors.Control;
            this.tvObjects.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tvObjects.CheckBoxes = true;
            this.tvObjects.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvObjects.Location = new System.Drawing.Point(5, 5);
            this.tvObjects.Margin = new System.Windows.Forms.Padding(0);
            this.tvObjects.Name = "tvObjects";
            this.tvObjects.Parent = null;
            this.tvObjects.Size = new System.Drawing.Size(308, 651);
            this.tvObjects.TabIndex = 0;
            // 
            // stats
            // 
            this.stats.Dock = System.Windows.Forms.DockStyle.Fill;
            this.stats.Location = new System.Drawing.Point(3, 3);
            this.stats.Name = "stats";
            this.stats.Size = new System.Drawing.Size(286, 631);
            this.stats.TabIndex = 0;
            this.stats.TabStop = false;
            // 
            // MainWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1016, 734);
            this.Controls.Add(this.splMain);
            this.Controls.Add(this.ssMain);
            this.Controls.Add(this.tbMain);
            this.Controls.Add(this.mnuMain);
            this.IsMdiContainer = true;
            this.KeyPreview = true;
            this.MainMenuStrip = this.mnuMain;
            this.Name = "MainWnd";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Columbo";
            this.Load += new System.EventHandler(this.MainWnd_Load);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainWnd_FormClosing);
            this.mnuMain.ResumeLayout(false);
            this.mnuMain.PerformLayout();
            this.tbMain.ResumeLayout(false);
            this.tbMain.PerformLayout();
            this.ssMain.ResumeLayout(false);
            this.ssMain.PerformLayout();
            this.splMain.Panel1.ResumeLayout(false);
            this.splMain.Panel2.ResumeLayout(false);
            this.splMain.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip mnuMain;
        private System.Windows.Forms.ToolStrip tbMain;
        private System.Windows.Forms.StatusStrip ssMain;
        private System.Windows.Forms.ToolStripMenuItem mnuFileItem;
        private System.Windows.Forms.ToolStripMenuItem mnuEditItem;
        private System.Windows.Forms.ToolStripMenuItem mnuTargetItem;
        private System.Windows.Forms.ToolStripButton tbbConnect;
        private System.Windows.Forms.ToolStripSeparator tbspMain;
        private System.Windows.Forms.ToolStripButton tbbRefresh;
        private System.Windows.Forms.ToolStripStatusLabel sslblMain;
        private System.Windows.Forms.SplitContainer splMain;
        //private System.Windows.Forms.TabControl tabMain;
        //private System.Windows.Forms.TabPage tabpObjects;
        private ProfilerTool.Controls.MainTreeView tvObjects;
        //private System.Windows.Forms.TabPage tabpProperties;
        public System.Windows.Forms.TabControl tbcInstances;
        private System.Windows.Forms.ToolStripMenuItem connectToolStripMenuItem;
        private Windows.Stats stats;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripButton tbbLoadSnapshot;
        private System.Windows.Forms.ToolStripButton tbbSaveSnapshot;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripButton tbbGroupComponents;
    }
}

