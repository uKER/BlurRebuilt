namespace ProfilingApplication
{
    partial class HierarchyView
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
			this.components = new System.ComponentModel.Container();
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(HierarchyView));
			this.panel1 = new System.Windows.Forms.Panel();
			this.panel2 = new SynapticEffect.Forms.TreeListView();
			this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.blackToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.blueToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.orangeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.greenToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.redToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.silverToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.notSetToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.panel1.SuspendLayout();
			this.contextMenuStrip1.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.panel2);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(401, 432);
			this.panel1.TabIndex = 0;
			// 
			// panel2
			// 
			this.panel2.BackColor = System.Drawing.SystemColors.Window;
			this.panel2.ColumnSortColor = System.Drawing.Color.Gainsboro;
			this.panel2.ColumnTrackColor = System.Drawing.Color.WhiteSmoke;
			this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel2.GridLineColor = System.Drawing.Color.WhiteSmoke;
			this.panel2.HeaderMenu = null;
			this.panel2.HideSelection = false;
			this.panel2.ItemHeight = 20;
			this.panel2.ItemMenu = this.contextMenuStrip1;
			this.panel2.LabelEdit = false;
			this.panel2.Location = new System.Drawing.Point(0, 0);
			this.panel2.Name = "panel2";
			this.panel2.RowSelectColor = System.Drawing.SystemColors.Highlight;
			this.panel2.RowTrackColor = System.Drawing.Color.WhiteSmoke;
			this.panel2.Size = new System.Drawing.Size(401, 432);
			this.panel2.SmallImageList = null;
			this.panel2.StateImageList = null;
			this.panel2.TabIndex = 0;
			// 
			// contextMenuStrip1
			// 
			this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.blackToolStripMenuItem,
            this.blueToolStripMenuItem,
            this.orangeToolStripMenuItem,
            this.greenToolStripMenuItem,
            this.redToolStripMenuItem,
            this.silverToolStripMenuItem,
            this.notSetToolStripMenuItem});
			this.contextMenuStrip1.Name = "contextMenuStrip1";
			this.contextMenuStrip1.Size = new System.Drawing.Size(122, 158);
			// 
			// blackToolStripMenuItem
			// 
			this.blackToolStripMenuItem.Image = global::ProfilingApplication.Properties.Resources.black_tick;
			this.blackToolStripMenuItem.Name = "blackToolStripMenuItem";
			this.blackToolStripMenuItem.Size = new System.Drawing.Size(121, 22);
			this.blackToolStripMenuItem.Text = "&Black";
			this.blackToolStripMenuItem.Click += new System.EventHandler(this.blackToolStripMenuItem_Click);
			// 
			// blueToolStripMenuItem
			// 
			this.blueToolStripMenuItem.Image = global::ProfilingApplication.Properties.Resources.blue_tick;
			this.blueToolStripMenuItem.Name = "blueToolStripMenuItem";
			this.blueToolStripMenuItem.Size = new System.Drawing.Size(121, 22);
			this.blueToolStripMenuItem.Text = "B&lue";
			this.blueToolStripMenuItem.Click += new System.EventHandler(this.blueToolStripMenuItem_Click);
			// 
			// orangeToolStripMenuItem
			// 
			this.orangeToolStripMenuItem.Image = global::ProfilingApplication.Properties.Resources.orange_tick;
			this.orangeToolStripMenuItem.Name = "orangeToolStripMenuItem";
			this.orangeToolStripMenuItem.Size = new System.Drawing.Size(121, 22);
			this.orangeToolStripMenuItem.Text = "&Orange";
			this.orangeToolStripMenuItem.Click += new System.EventHandler(this.orangeToolStripMenuItem_Click);
			// 
			// greenToolStripMenuItem
			// 
			this.greenToolStripMenuItem.Image = global::ProfilingApplication.Properties.Resources.green_tick;
			this.greenToolStripMenuItem.Name = "greenToolStripMenuItem";
			this.greenToolStripMenuItem.Size = new System.Drawing.Size(121, 22);
			this.greenToolStripMenuItem.Text = "&Green";
			this.greenToolStripMenuItem.Click += new System.EventHandler(this.greenToolStripMenuItem_Click);
			// 
			// redToolStripMenuItem
			// 
			this.redToolStripMenuItem.Image = global::ProfilingApplication.Properties.Resources.red_tick;
			this.redToolStripMenuItem.Name = "redToolStripMenuItem";
			this.redToolStripMenuItem.Size = new System.Drawing.Size(121, 22);
			this.redToolStripMenuItem.Text = "&Red";
			this.redToolStripMenuItem.Click += new System.EventHandler(this.redToolStripMenuItem_Click);
			// 
			// silverToolStripMenuItem
			// 
			this.silverToolStripMenuItem.Image = global::ProfilingApplication.Properties.Resources.silver_tick;
			this.silverToolStripMenuItem.Name = "silverToolStripMenuItem";
			this.silverToolStripMenuItem.Size = new System.Drawing.Size(121, 22);
			this.silverToolStripMenuItem.Text = "&Silver";
			this.silverToolStripMenuItem.Click += new System.EventHandler(this.silverToolStripMenuItem_Click);
			// 
			// notSetToolStripMenuItem
			// 
			this.notSetToolStripMenuItem.Name = "notSetToolStripMenuItem";
			this.notSetToolStripMenuItem.Size = new System.Drawing.Size(121, 22);
			this.notSetToolStripMenuItem.Text = "&Not set";
			this.notSetToolStripMenuItem.Click += new System.EventHandler(this.notSetToolStripMenuItem_Click);
			// 
			// HierarchyView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(401, 432);
			this.Controls.Add(this.panel1);
			this.HideOnClose = true;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "HierarchyView";
			this.TabText = "HierarchyView";
			this.Text = "HierarchyView";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.HierarchyView_FormClosing);
			this.panel1.ResumeLayout(false);
			this.contextMenuStrip1.ResumeLayout(false);
			this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private SynapticEffect.Forms.TreeListView panel2;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        private System.Windows.Forms.ToolStripMenuItem blackToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem blueToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem orangeToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem greenToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem redToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem silverToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem notSetToolStripMenuItem;
    }
}