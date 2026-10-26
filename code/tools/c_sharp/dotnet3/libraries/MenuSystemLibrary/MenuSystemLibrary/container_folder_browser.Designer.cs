namespace MenuSystemLibrary
{
	partial class Container_FolderBrowser
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.textBox = new System.Windows.Forms.TextBox();
			this.labelPrefix = new System.Windows.Forms.Label();
			this.buttonBrowse = new System.Windows.Forms.Button();
			this.label = new System.Windows.Forms.Label();
			this.buttonOptions = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// textBox
			// 
			this.textBox.AllowDrop = true;
			this.textBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.textBox.Cursor = System.Windows.Forms.Cursors.IBeam;
			this.textBox.HideSelection = false;
			this.textBox.Location = new System.Drawing.Point(38, 21);
			this.textBox.Name = "textBox";
			this.textBox.Size = new System.Drawing.Size(214, 20);
			this.textBox.TabIndex = 10;
			this.textBox.TextChanged += new System.EventHandler(this.textBox_TextChanged);
			// 
			// labelPrefix
			// 
			this.labelPrefix.AutoSize = true;
			this.labelPrefix.Location = new System.Drawing.Point(4, 24);
			this.labelPrefix.Name = "labelPrefix";
			this.labelPrefix.Size = new System.Drawing.Size(33, 13);
			this.labelPrefix.TabIndex = 12;
			this.labelPrefix.Text = "Prefix";
			// 
			// buttonBrowse
			// 
			this.buttonBrowse.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonBrowse.Location = new System.Drawing.Point(253, 21);
			this.buttonBrowse.Name = "buttonBrowse";
			this.buttonBrowse.Size = new System.Drawing.Size(24, 23);
			this.buttonBrowse.TabIndex = 11;
			this.buttonBrowse.Text = "...";
			this.buttonBrowse.UseVisualStyleBackColor = true;
			this.buttonBrowse.Click += new System.EventHandler(this.button_Click);
			// 
			// label
			// 
			this.label.AutoSize = true;
			this.label.Location = new System.Drawing.Point(4, 4);
			this.label.Name = "label";
			this.label.Size = new System.Drawing.Size(29, 13);
			this.label.TabIndex = 9;
			this.label.Text = "label";
			// 
			// buttonOptions
			// 
			this.buttonOptions.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonOptions.Image = global::MenuSystemLibrary.Properties.Resources.ExecuterToolButton;
			this.buttonOptions.Location = new System.Drawing.Point(277, 21);
			this.buttonOptions.Name = "buttonOptions";
			this.buttonOptions.Size = new System.Drawing.Size(23, 23);
			this.buttonOptions.TabIndex = 13;
			this.buttonOptions.UseVisualStyleBackColor = true;
			this.buttonOptions.Click += new System.EventHandler(this.buttonOptions_Click);
			// 
			// Container_FolderBrowser
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.buttonOptions);
			this.Controls.Add(this.textBox);
			this.Controls.Add(this.labelPrefix);
			this.Controls.Add(this.buttonBrowse);
			this.Controls.Add(this.label);
			this.Name = "Container_FolderBrowser";
			this.Size = new System.Drawing.Size(308, 50);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Button buttonOptions;
		private System.Windows.Forms.TextBox textBox;
		private System.Windows.Forms.Label labelPrefix;
		private System.Windows.Forms.Button buttonBrowse;
		private System.Windows.Forms.Label label;
	}
}
