namespace application
{
	partial class FrameForm
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FrameForm));
			this.panelClientArea = new System.Windows.Forms.Panel();
			this.SuspendLayout();
			// 
			// panelClientArea
			// 
			this.panelClientArea.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.panelClientArea.BackColor = System.Drawing.Color.White;
			this.panelClientArea.Location = new System.Drawing.Point(59, 37);
			this.panelClientArea.Margin = new System.Windows.Forms.Padding(50);
			this.panelClientArea.Name = "panelClientArea";
			this.panelClientArea.Size = new System.Drawing.Size(224, 163);
			this.panelClientArea.TabIndex = 0;
			// 
			// FrameForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.BoundsF = ((System.Drawing.RectangleF)(resources.GetObject("$this.BoundsF")));
			this.ClientSize = new System.Drawing.Size(342, 259);
			this.Controls.Add(this.panelClientArea);
			this.Name = "FrameForm";
			this.ShowInTaskbar = false;
			this.TopMost = true;
			this.TransparencyKey = System.Drawing.Color.White;
			this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.FrameForm_MouseUp);
			this.VirtualBoundsChange += new application.BaseForm.BoundsChangeHandler(this.FrameForm_VirtualBoundsChange);
			this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.FrameForm_MouseDown);
			this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.FrameForm_MouseMove);
			this.ResumeLayout(false);

		}

		#endregion

		public System.Windows.Forms.Panel panelClientArea;

	}
}
