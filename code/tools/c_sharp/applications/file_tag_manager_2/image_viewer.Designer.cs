namespace file_tag_manager_2
{
	partial class ImageViewer
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
			this.panelMain = new System.Windows.Forms.Panel();
			this.panel1 = new System.Windows.Forms.Panel();
			this.panelPicture = new System.Windows.Forms.Panel();
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.panelMain.SuspendLayout();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// panelMain
			// 
			this.panelMain.Controls.Add(this.panel1);
			this.panelMain.Controls.Add(this.toolStrip1);
			this.panelMain.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panelMain.Location = new System.Drawing.Point(0, 0);
			this.panelMain.Name = "panelMain";
			this.panelMain.Size = new System.Drawing.Size(292, 266);
			this.panelMain.TabIndex = 0;
			// 
			// panel1
			// 
			this.panel1.AutoScroll = true;
			this.panel1.Controls.Add(this.panelPicture);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(0, 25);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(292, 241);
			this.panel1.TabIndex = 1;
			// 
			// panelPicture
			// 
			this.panelPicture.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panelPicture.Location = new System.Drawing.Point(0, 0);
			this.panelPicture.Name = "panelPicture";
			this.panelPicture.Size = new System.Drawing.Size(292, 241);
			this.panelPicture.TabIndex = 0;
			this.panelPicture.Paint += new System.Windows.Forms.PaintEventHandler(this.panelPicture_Paint);
			// 
			// toolStrip1
			// 
			this.toolStrip1.Location = new System.Drawing.Point(0, 0);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(292, 25);
			this.toolStrip1.TabIndex = 0;
			this.toolStrip1.Text = "toolStrip1";
			// 
			// image_viewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(292, 266);
			this.Controls.Add(this.panelMain);
			this.DoubleBuffered = true;
			this.Name = "image_viewer";
			this.Text = "image_viewer";
			this.Resize += new System.EventHandler(this.image_viewer_Resize);
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.image_viewer_FormClosing);
			this.Load += new System.EventHandler(this.image_viewer_Load);
			this.panelMain.ResumeLayout(false);
			this.panelMain.PerformLayout();
			this.panel1.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel panelMain;
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.Panel panelPicture;
	}
}