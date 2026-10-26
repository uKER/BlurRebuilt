namespace CustomMessageBox
{
	partial class CustomMessageBoxForm
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
			this.richTextBox = new System.Windows.Forms.RichTextBox();
			this.SuspendLayout();
			// 
			// richTextBox
			// 
			this.richTextBox.BackColor = System.Drawing.SystemColors.Control;
			this.richTextBox.HideSelection = false;
			this.richTextBox.Location = new System.Drawing.Point(12, 12);
			this.richTextBox.Name = "richTextBox";
			this.richTextBox.ReadOnly = true;
			this.richTextBox.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.None;
			this.richTextBox.Size = new System.Drawing.Size(100, 96);
			this.richTextBox.TabIndex = 0;
			this.richTextBox.Text = "";
			// 
			// CustomMessageBoxForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(422, 211);
			this.ControlBox = false;
			this.Controls.Add(this.richTextBox);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "CustomMessageBoxForm";
			this.ShowInTaskbar = false;
			this.Text = "CustomMessageBoxForm";
			this.TopMost = true;
			this.Load += new System.EventHandler(this.CustomMessageBoxForm_Load);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.RichTextBox richTextBox;
	}
}