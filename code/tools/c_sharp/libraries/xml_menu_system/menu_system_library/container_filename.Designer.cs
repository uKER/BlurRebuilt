namespace MenuSystemLibrary
{
	partial class Container_Filename
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
			this.textBox = new System.Windows.Forms.TextBox();
			this.buttonBrowse = new System.Windows.Forms.Button();
			this.labelPrefix = new System.Windows.Forms.Label();
			this.buttonOptions = new System.Windows.Forms.Button();
			this.panel1 = new System.Windows.Forms.Panel();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// label
			// 
			this.label.AutoSize = true;
			this.label.Location = new System.Drawing.Point(3, 0);
			this.label.Name = "label";
			this.label.Size = new System.Drawing.Size(29, 13);
			this.label.TabIndex = 0;
			this.label.Text = "label";
			// 
			// textBox
			// 
			this.textBox.AllowDrop = true;
			this.textBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.textBox.Cursor = System.Windows.Forms.Cursors.IBeam;
			this.textBox.HideSelection = false;
			this.textBox.Location = new System.Drawing.Point(38, 7);
			this.textBox.Name = "textBox";
			this.textBox.Size = new System.Drawing.Size(267, 20);
			this.textBox.TabIndex = 1;
			this.textBox.TextChanged += new System.EventHandler(this.textBox_TextChanged);
			this.textBox.DragDrop += new System.Windows.Forms.DragEventHandler(this.textBox_DragDrop);
			this.textBox.DragEnter += new System.Windows.Forms.DragEventHandler(this.textBox_DragEnter);
			// 
			// buttonBrowse
			// 
			this.buttonBrowse.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonBrowse.Location = new System.Drawing.Point(306, 5);
			this.buttonBrowse.Name = "buttonBrowse";
			this.buttonBrowse.Size = new System.Drawing.Size(24, 23);
			this.buttonBrowse.TabIndex = 2;
			this.buttonBrowse.Text = "...";
			this.buttonBrowse.UseVisualStyleBackColor = true;
			this.buttonBrowse.Click += new System.EventHandler(this.button_Click);
			// 
			// labelPrefix
			// 
			this.labelPrefix.AutoSize = true;
			this.labelPrefix.Location = new System.Drawing.Point(4, 10);
			this.labelPrefix.Name = "labelPrefix";
			this.labelPrefix.Size = new System.Drawing.Size(33, 13);
			this.labelPrefix.TabIndex = 4;
			this.labelPrefix.Text = "Prefix";
			// 
			// buttonOptions
			// 
			this.buttonOptions.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonOptions.Image = global::MenuSystemLibrary.Properties.Resources.ExecuterToolButton;
			this.buttonOptions.Location = new System.Drawing.Point(330, 5);
			this.buttonOptions.Name = "buttonOptions";
			this.buttonOptions.Size = new System.Drawing.Size(23, 23);
			this.buttonOptions.TabIndex = 8;
			this.buttonOptions.UseVisualStyleBackColor = true;
			this.buttonOptions.Click += new System.EventHandler(this.buttonOptions_Click);
			// 
			// panel1
			// 
			this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.panel1.Controls.Add(this.textBox);
			this.panel1.Controls.Add(this.buttonOptions);
			this.panel1.Controls.Add(this.buttonBrowse);
			this.panel1.Controls.Add(this.labelPrefix);
			this.panel1.Location = new System.Drawing.Point(0, 16);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(356, 32);
			this.panel1.TabIndex = 9;
			// 
			// Container_Filename
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.Controls.Add(this.panel1);
			this.Controls.Add(this.label);
			this.Name = "Container_Filename";
			this.Size = new System.Drawing.Size(356, 51);
			this.panel1.ResumeLayout(false);
			this.panel1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label;
		private System.Windows.Forms.TextBox textBox;
		private System.Windows.Forms.Button buttonBrowse;
		private System.Windows.Forms.Label labelPrefix;
		private System.Windows.Forms.Button buttonOptions;
		private System.Windows.Forms.Panel panel1;
	}
}
