namespace MenuSystemLibrary
{
	partial class Container_Progressbar
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
			this.progressBar = new System.Windows.Forms.ProgressBar();
			this.labelProgress = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// progressBar
			// 
			this.progressBar.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.progressBar.Location = new System.Drawing.Point(3, 16);
			this.progressBar.Name = "progressBar";
			this.progressBar.Size = new System.Drawing.Size(306, 23);
			this.progressBar.TabIndex = 0;
			// 
			// labelProgress
			// 
			this.labelProgress.AutoSize = true;
			this.labelProgress.Location = new System.Drawing.Point(3, 0);
			this.labelProgress.Name = "labelProgress";
			this.labelProgress.Size = new System.Drawing.Size(35, 13);
			this.labelProgress.TabIndex = 1;
			this.labelProgress.Text = "label1";
			// 
			// Container_Progressbar
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.Controls.Add(this.labelProgress);
			this.Controls.Add(this.progressBar);
			this.Name = "Container_Progressbar";
			this.Size = new System.Drawing.Size(312, 42);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ProgressBar progressBar;
		private System.Windows.Forms.Label labelProgress;
	}
}
