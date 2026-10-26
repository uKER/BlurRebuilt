namespace application
{
	partial class BizarreToolbarManager
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
			this.components = new System.ComponentModel.Container();
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(BizarreToolbarManager));
			this.notifyIcon = new System.Windows.Forms.NotifyIcon(this.components);
			this.contextMenuStripNotifyIcon = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.contextMenuStripNotifyIcon.SuspendLayout();
			this.SuspendLayout();
			// 
			// notifyIcon
			// 
			this.notifyIcon.ContextMenuStrip = this.contextMenuStripNotifyIcon;
			this.notifyIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("notifyIcon.Icon")));
			this.notifyIcon.Text = "Bizarre Toolbar";
			this.notifyIcon.Visible = true;
			// 
			// contextMenuStripNotifyIcon
			// 
			this.contextMenuStripNotifyIcon.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exitToolStripMenuItem});
			this.contextMenuStripNotifyIcon.Name = "contextMenuStripNotifyIcon";
			this.contextMenuStripNotifyIcon.Size = new System.Drawing.Size(153, 48);
			this.contextMenuStripNotifyIcon.MouseMove += new System.Windows.Forms.MouseEventHandler(this.contextMenuStripNotifyIcon_MouseMove);
			// 
			// exitToolStripMenuItem
			// 
			this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			this.exitToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
			this.exitToolStripMenuItem.Text = "Exit";
			this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
			// 
			// BizarreToolbarManager
			// 
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Inherit;
			this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.BoundsF = ((System.Drawing.RectangleF)(resources.GetObject("$this.BoundsF")));
			this.ClientSize = new System.Drawing.Size(271, 245);
			this.Name = "BizarreToolbarManager";
			this.Text = "Application";
			this.contextMenuStripNotifyIcon.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.NotifyIcon notifyIcon;
		private System.Windows.Forms.ContextMenuStrip contextMenuStripNotifyIcon;
		private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;


	}
}

