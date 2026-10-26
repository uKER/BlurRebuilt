namespace application
{
	partial class BaseGadget
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(BaseGadget));
			this.SuspendLayout();
			// 
			// BaseGadget
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.BoundsF = ((System.Drawing.RectangleF)(resources.GetObject("$this.BoundsF")));
			this.ClientSize = new System.Drawing.Size(400, 400);
			this.Name = "BaseGadget";
			this.ShowInTaskbar = false;
			this.TopMost = true;
			this.ResumeLayout(false);

		}

		#endregion
	}
}
