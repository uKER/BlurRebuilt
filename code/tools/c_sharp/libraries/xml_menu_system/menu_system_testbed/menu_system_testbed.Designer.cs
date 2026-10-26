namespace MenuSystemTestbed
{
    partial class MenuWrapper
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
			this.setString = new System.Windows.Forms.Button();
			this.menuStringTextBox = new System.Windows.Forms.RichTextBox();
			this.menuSystemLibrary = new MenuSystemLibrary.MenuSystem();
			this.SuspendLayout();
			// 
			// setString
			// 
			this.setString.Location = new System.Drawing.Point(13, 13);
			this.setString.Name = "setString";
			this.setString.Size = new System.Drawing.Size(75, 23);
			this.setString.TabIndex = 1;
			this.setString.Text = "Set String";
			this.setString.UseVisualStyleBackColor = true;
			this.setString.Click += new System.EventHandler(this.setString_Click);
			// 
			// menuStringTextBox
			// 
			this.menuStringTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.menuStringTextBox.Location = new System.Drawing.Point(94, 13);
			this.menuStringTextBox.Name = "menuStringTextBox";
			this.menuStringTextBox.Size = new System.Drawing.Size(862, 229);
			this.menuStringTextBox.TabIndex = 2;
			this.menuStringTextBox.Text = "";
			// 
			// menuSystemLibrary
			// 
			this.menuSystemLibrary.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.menuSystemLibrary.AutoScroll = true;
			this.menuSystemLibrary.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
			this.menuSystemLibrary.Location = new System.Drawing.Point(12, 248);
			this.menuSystemLibrary.Name = "menuSystemLibrary";
			this.menuSystemLibrary.Size = new System.Drawing.Size(945, 202);
			this.menuSystemLibrary.TabIndex = 0;
			// 
			// MenuWrapper
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(969, 462);
			this.Controls.Add(this.menuStringTextBox);
			this.Controls.Add(this.setString);
			this.Controls.Add(this.menuSystemLibrary);
			this.Name = "MenuWrapper";
			this.Text = "MenuWrapper";
			this.ResumeLayout(false);

        }

        #endregion

        private MenuSystemLibrary.MenuSystem menuSystemLibrary;
		private System.Windows.Forms.Button setString;
		private System.Windows.Forms.RichTextBox menuStringTextBox;

    }
}

