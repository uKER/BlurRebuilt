namespace server_app
{
	partial class TabPane
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.label_id = new System.Windows.Forms.Label();
			this.textBox_id = new System.Windows.Forms.TextBox();
			this.richTextBox = new System.Windows.Forms.RichTextBox();
			this.SuspendLayout();
			// 
			// label_id
			// 
			this.label_id.AutoSize = true;
			this.label_id.Location = new System.Drawing.Point(13, 12);
			this.label_id.Name = "label_id";
			this.label_id.Size = new System.Drawing.Size(18, 13);
			this.label_id.TabIndex = 0;
			this.label_id.Text = "ID";
			// 
			// textBox_id
			// 
			this.textBox_id.Location = new System.Drawing.Point(37, 9);
			this.textBox_id.Name = "textBox_id";
			this.textBox_id.Size = new System.Drawing.Size(100, 20);
			this.textBox_id.TabIndex = 1;
			// 
			// richTextBox
			// 
			this.richTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.richTextBox.Location = new System.Drawing.Point(16, 35);
			this.richTextBox.Name = "richTextBox";
			this.richTextBox.Size = new System.Drawing.Size(537, 352);
			this.richTextBox.TabIndex = 2;
			this.richTextBox.Text = "";
			this.richTextBox.WordWrap = false;
			// 
			// TabPane
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.richTextBox);
			this.Controls.Add(this.textBox_id);
			this.Controls.Add(this.label_id);
			this.Name = "TabPane";
			this.Size = new System.Drawing.Size(556, 390);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label_id;
		private System.Windows.Forms.TextBox textBox_id;
		private System.Windows.Forms.RichTextBox richTextBox;
	}
}
