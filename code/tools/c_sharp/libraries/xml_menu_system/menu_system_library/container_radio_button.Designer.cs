namespace MenuSystemLibrary
{
	partial class Container_RadioButton
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
			this.radioButton = new System.Windows.Forms.RadioButton();
			this.SuspendLayout();
			// 
			// radioButton
			// 
			this.radioButton.AutoSize = true;
			this.radioButton.Location = new System.Drawing.Point(3, 3);
			this.radioButton.Name = "radioButton";
			this.radioButton.Size = new System.Drawing.Size(85, 17);
			this.radioButton.TabIndex = 0;
			this.radioButton.TabStop = true;
			this.radioButton.Text = "radioButton1";
			this.radioButton.UseVisualStyleBackColor = true;
			this.radioButton.CheckedChanged += new System.EventHandler(this.radioButton_CheckedChanged);
			// 
			// Container_RadioButton
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoSize = true;
			this.Controls.Add(this.radioButton);
			this.Name = "Container_RadioButton";
			this.Size = new System.Drawing.Size(179, 24);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.RadioButton radioButton;
	}
}
