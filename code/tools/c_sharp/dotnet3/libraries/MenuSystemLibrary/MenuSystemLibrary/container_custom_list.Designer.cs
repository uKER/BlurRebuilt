namespace MenuSystemLibrary
{
	partial class Container_CustomList
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
            this.textBoxContainerValue = new System.Windows.Forms.TextBox();
            this.listBox = new System.Windows.Forms.ListBox();
            this.but_remove = new System.Windows.Forms.Button();
            this.labelControlName = new System.Windows.Forms.GroupBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label_target = new System.Windows.Forms.Label();
            this.but_add = new System.Windows.Forms.Button();
            this.labelControlName.SuspendLayout();
            this.SuspendLayout();
            // 
            // textBoxContainerValue
            // 
            this.textBoxContainerValue.Location = new System.Drawing.Point(16, 44);
            this.textBoxContainerValue.Name = "textBoxContainerValue";
            this.textBoxContainerValue.Size = new System.Drawing.Size(150, 20);
            this.textBoxContainerValue.TabIndex = 1;
            this.textBoxContainerValue.KeyUp += new System.Windows.Forms.KeyEventHandler(this.textBoxContainerValue_KeyUp);
            // 
            // listBox
            // 
            this.listBox.FormattingEnabled = true;
            this.listBox.Location = new System.Drawing.Point(296, 44);
            this.listBox.MaximumSize = new System.Drawing.Size(150, 69);
            this.listBox.Name = "listBox";
            this.listBox.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.listBox.Size = new System.Drawing.Size(150, 69);
            this.listBox.TabIndex = 3;
            this.listBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.listBox_MouseDown);
            // 
            // but_remove
            // 
            this.but_remove.Image = global::MenuSystemLibrary.Properties.Resources.Remove;
            this.but_remove.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.but_remove.Location = new System.Drawing.Point(195, 77);
            this.but_remove.Name = "but_remove";
            this.but_remove.Size = new System.Drawing.Size(75, 23);
            this.but_remove.TabIndex = 4;
            this.but_remove.Text = "Remove";
            this.but_remove.UseVisualStyleBackColor = true;
            this.but_remove.Click += new System.EventHandler(this.but_remove_Click);
            // 
            // labelControlName
            // 
            this.labelControlName.Controls.Add(this.label2);
            this.labelControlName.Controls.Add(this.label_target);
            this.labelControlName.Controls.Add(this.textBoxContainerValue);
            this.labelControlName.Controls.Add(this.but_add);
            this.labelControlName.Controls.Add(this.but_remove);
            this.labelControlName.Controls.Add(this.listBox);
            this.labelControlName.Location = new System.Drawing.Point(3, 3);
            this.labelControlName.Name = "labelControlName";
            this.labelControlName.Size = new System.Drawing.Size(464, 134);
            this.labelControlName.TabIndex = 6;
            this.labelControlName.TabStop = false;
            this.labelControlName.Text = "groupBox1";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(16, 25);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(75, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Target Name :";
            this.label2.Click += new System.EventHandler(this.label2_Click);
            // 
            // label_target
            // 
            this.label_target.AutoSize = true;
            this.label_target.Location = new System.Drawing.Point(296, 25);
            this.label_target.Name = "label_target";
            this.label_target.Size = new System.Drawing.Size(49, 13);
            this.label_target.TabIndex = 5;
            this.label_target.Text = "Targets :";
            // 
            // but_add
            // 
            this.but_add.Image = global::MenuSystemLibrary.Properties.Resources.Add;
            this.but_add.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.but_add.Location = new System.Drawing.Point(195, 44);
            this.but_add.Name = "but_add";
            this.but_add.Size = new System.Drawing.Size(75, 23);
            this.but_add.TabIndex = 2;
            this.but_add.Text = "Add";
            this.but_add.UseVisualStyleBackColor = true;
            this.but_add.Click += new System.EventHandler(this.but_add_Click);
            // 
            // Container_CustomList
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.Controls.Add(this.labelControlName);
            this.Name = "Container_CustomList";
            this.Size = new System.Drawing.Size(475, 143);
            this.labelControlName.ResumeLayout(false);
            this.labelControlName.PerformLayout();
            this.ResumeLayout(false);

		}

		#endregion

        private System.Windows.Forms.TextBox textBoxContainerValue;
        private System.Windows.Forms.Button but_add;
        private System.Windows.Forms.ListBox listBox;
        private System.Windows.Forms.Button but_remove;
        private System.Windows.Forms.GroupBox labelControlName;
        private System.Windows.Forms.Label label_target;
        private System.Windows.Forms.Label label2;
	}
}
