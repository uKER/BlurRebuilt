namespace MenuSystemLibrary
{
	partial class Container_Text
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
			this.label = new System.Windows.Forms.Label();
			this.buttonEdit = new System.Windows.Forms.Button();
			this.richTextBox = new System.Windows.Forms.RichTextBox();
			this.SuspendLayout();
			// 
			// label
			// 
			this.label.AutoSize = true;
			this.label.Location = new System.Drawing.Point(3, 0);
			this.label.Name = "label";
			this.label.Size = new System.Drawing.Size(35, 13);
			this.label.TabIndex = 0;
			this.label.Text = "label1";
			// 
			// buttonEdit
			// 
			this.buttonEdit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonEdit.Location = new System.Drawing.Point(493, 208);
			this.buttonEdit.Name = "buttonEdit";
			this.buttonEdit.Size = new System.Drawing.Size(75, 23);
			this.buttonEdit.TabIndex = 2;
			this.buttonEdit.Text = "Edit ...";
			this.buttonEdit.UseVisualStyleBackColor = true;
			this.buttonEdit.Click += new System.EventHandler(this.button1_Click);
			// 
			// richTextBox
			// 
			this.richTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.richTextBox.Cursor = System.Windows.Forms.Cursors.IBeam;
			this.richTextBox.HideSelection = false;
			this.richTextBox.Location = new System.Drawing.Point(6, 16);
			this.richTextBox.Name = "richTextBox";
			this.richTextBox.Size = new System.Drawing.Size(562, 187);
			this.richTextBox.TabIndex = 1;
			this.richTextBox.Text = "";
			this.richTextBox.WordWrap = false;
			this.richTextBox.LinkClicked += new System.Windows.Forms.LinkClickedEventHandler(this.richTextBox_LinkClicked);
			this.richTextBox.TextChanged += new System.EventHandler(this.richTextBox_TextChanged);
			// 
			// Container_Text
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.Controls.Add(this.richTextBox);
			this.Controls.Add(this.buttonEdit);
			this.Controls.Add(this.label);
			this.Name = "Container_Text";
			this.Size = new System.Drawing.Size(572, 237);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label;
		private System.Windows.Forms.Button buttonEdit;
		private System.Windows.Forms.RichTextBox richTextBox;
	}
}
