namespace ProfilingApplication
{
	partial class CounterPropertiesDlg
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CounterPropertiesDlg));
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.inactiveCounterView = new System.Windows.Forms.TreeView();
			this.button1 = new System.Windows.Forms.Button();
			this.button2 = new System.Windows.Forms.Button();
			this.addButton = new System.Windows.Forms.Button();
			this.removeButton = new System.Windows.Forms.Button();
			this.moveUpButton = new System.Windows.Forms.Button();
			this.moveDownButton = new System.Windows.Forms.Button();
			this.saveButton = new System.Windows.Forms.Button();
			this.loadButton = new System.Windows.Forms.Button();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.activeCounterView = new System.Windows.Forms.ListView();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.scaleTextBox = new System.Windows.Forms.NumericUpDown();
			this.colorButton = new ProfilingApplication.ColorButton();
			this.customCounterButton = new System.Windows.Forms.Button();
			this.deleteCustomCounterButton = new System.Windows.Forms.Button();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.scaleTextBox)).BeginInit();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.groupBox1.Controls.Add(this.inactiveCounterView);
			this.groupBox1.Location = new System.Drawing.Point(13, 19);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(251, 324);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Inactive Counters";
			// 
			// inactiveCounterView
			// 
			this.inactiveCounterView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.inactiveCounterView.HideSelection = false;
			this.inactiveCounterView.Location = new System.Drawing.Point(6, 19);
			this.inactiveCounterView.Name = "inactiveCounterView";
			this.inactiveCounterView.Size = new System.Drawing.Size(239, 297);
			this.inactiveCounterView.TabIndex = 0;
			// 
			// button1
			// 
			this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.button1.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.button1.Location = new System.Drawing.Point(525, 350);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(75, 23);
			this.button1.TabIndex = 0;
			this.button1.Text = "OK";
			this.button1.UseVisualStyleBackColor = true;
			// 
			// button2
			// 
			this.button2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.button2.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.button2.Location = new System.Drawing.Point(606, 350);
			this.button2.Name = "button2";
			this.button2.Size = new System.Drawing.Size(75, 23);
			this.button2.TabIndex = 1;
			this.button2.Text = "Cancel";
			this.button2.UseVisualStyleBackColor = true;
			// 
			// addButton
			// 
			this.addButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.addButton.Location = new System.Drawing.Point(270, 151);
			this.addButton.Name = "addButton";
			this.addButton.Size = new System.Drawing.Size(89, 23);
			this.addButton.TabIndex = 2;
			this.addButton.Text = "&Add -->";
			this.addButton.UseVisualStyleBackColor = true;
			this.addButton.Click += new System.EventHandler(this.addButton_Click);
			// 
			// removeButton
			// 
			this.removeButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.removeButton.Location = new System.Drawing.Point(270, 180);
			this.removeButton.Name = "removeButton";
			this.removeButton.Size = new System.Drawing.Size(89, 23);
			this.removeButton.TabIndex = 3;
			this.removeButton.Text = "<-- &Remove";
			this.removeButton.UseVisualStyleBackColor = true;
			this.removeButton.Click += new System.EventHandler(this.removeButton_Click);
			// 
			// moveUpButton
			// 
			this.moveUpButton.Location = new System.Drawing.Point(220, 47);
			this.moveUpButton.Name = "moveUpButton";
			this.moveUpButton.Size = new System.Drawing.Size(90, 23);
			this.moveUpButton.TabIndex = 1;
			this.moveUpButton.Text = "Move &Up";
			this.moveUpButton.UseVisualStyleBackColor = true;
			this.moveUpButton.Click += new System.EventHandler(this.moveUpButton_Click);
			// 
			// moveDownButton
			// 
			this.moveDownButton.Location = new System.Drawing.Point(220, 76);
			this.moveDownButton.Name = "moveDownButton";
			this.moveDownButton.Size = new System.Drawing.Size(90, 23);
			this.moveDownButton.TabIndex = 2;
			this.moveDownButton.Text = "Move &Down";
			this.moveDownButton.UseVisualStyleBackColor = true;
			this.moveDownButton.Click += new System.EventHandler(this.moveDownButton_Click);
			// 
			// saveButton
			// 
			this.saveButton.Location = new System.Drawing.Point(220, 218);
			this.saveButton.Name = "saveButton";
			this.saveButton.Size = new System.Drawing.Size(90, 23);
			this.saveButton.TabIndex = 4;
			this.saveButton.Text = "&Save As...";
			this.saveButton.UseVisualStyleBackColor = true;
			this.saveButton.Click += new System.EventHandler(this.saveButton_Click);
			// 
			// loadButton
			// 
			this.loadButton.Location = new System.Drawing.Point(220, 189);
			this.loadButton.Name = "loadButton";
			this.loadButton.Size = new System.Drawing.Size(90, 23);
			this.loadButton.TabIndex = 3;
			this.loadButton.Text = "&Load...";
			this.loadButton.UseVisualStyleBackColor = true;
			this.loadButton.Click += new System.EventHandler(this.loadButton_Click);
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(218, 269);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(37, 13);
			this.label2.TabIndex = 4;
			this.label2.Text = "Scale:";
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(218, 298);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(34, 13);
			this.label3.TabIndex = 4;
			this.label3.Text = "Color:";
			// 
			// activeCounterView
			// 
			this.activeCounterView.HideSelection = false;
			this.activeCounterView.Location = new System.Drawing.Point(6, 19);
			this.activeCounterView.Name = "activeCounterView";
			this.activeCounterView.Size = new System.Drawing.Size(208, 297);
			this.activeCounterView.TabIndex = 0;
			this.activeCounterView.UseCompatibleStateImageBehavior = false;
			this.activeCounterView.View = System.Windows.Forms.View.List;
			this.activeCounterView.SelectedIndexChanged += new System.EventHandler(this.activeCounterView_SelectedIndexChanged);
			// 
			// groupBox2
			// 
			this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.groupBox2.Controls.Add(this.scaleTextBox);
			this.groupBox2.Controls.Add(this.activeCounterView);
			this.groupBox2.Controls.Add(this.label3);
			this.groupBox2.Controls.Add(this.label2);
			this.groupBox2.Controls.Add(this.loadButton);
			this.groupBox2.Controls.Add(this.colorButton);
			this.groupBox2.Controls.Add(this.saveButton);
			this.groupBox2.Controls.Add(this.moveDownButton);
			this.groupBox2.Controls.Add(this.moveUpButton);
			this.groupBox2.Location = new System.Drawing.Point(365, 19);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(316, 324);
			this.groupBox2.TabIndex = 1;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Active Counters";
			// 
			// scaleTextBox
			// 
			this.scaleTextBox.DecimalPlaces = 4;
			this.scaleTextBox.Enabled = false;
			this.scaleTextBox.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
			this.scaleTextBox.Location = new System.Drawing.Point(254, 267);
			this.scaleTextBox.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
			this.scaleTextBox.Name = "scaleTextBox";
			this.scaleTextBox.Size = new System.Drawing.Size(56, 20);
			this.scaleTextBox.TabIndex = 7;
			this.scaleTextBox.ValueChanged += new System.EventHandler(this.scaleTextBox_ValueChanged);
			// 
			// colorButton
			// 
			this.colorButton.CenterColor = System.Drawing.Color.Empty;
			this.colorButton.Enabled = false;
			this.colorButton.Location = new System.Drawing.Point(254, 293);
			this.colorButton.Name = "colorButton";
			this.colorButton.Size = new System.Drawing.Size(56, 23);
			this.colorButton.TabIndex = 6;
			this.colorButton.UseVisualStyleBackColor = true;
			this.colorButton.Click += new System.EventHandler(this.colorButton_Click);
			// 
			// customCounterButton
			// 
			this.customCounterButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.customCounterButton.Location = new System.Drawing.Point(151, 350);
			this.customCounterButton.Name = "customCounterButton";
			this.customCounterButton.Size = new System.Drawing.Size(75, 23);
			this.customCounterButton.TabIndex = 1;
			this.customCounterButton.Text = "Custom...";
			this.customCounterButton.UseVisualStyleBackColor = true;
			this.customCounterButton.Click += new System.EventHandler(this.customCounterButton_Click);
			// 
			// deleteCustomCounterButton
			// 
			this.deleteCustomCounterButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.deleteCustomCounterButton.Image = ((System.Drawing.Image)(resources.GetObject("deleteCustomCounterButton.Image")));
			this.deleteCustomCounterButton.Location = new System.Drawing.Point(232, 350);
			this.deleteCustomCounterButton.Name = "deleteCustomCounterButton";
			this.deleteCustomCounterButton.Size = new System.Drawing.Size(32, 23);
			this.deleteCustomCounterButton.TabIndex = 4;
			this.deleteCustomCounterButton.UseVisualStyleBackColor = true;
			this.deleteCustomCounterButton.Click += new System.EventHandler(this.deleteCustomCounterButton_Click);
			// 
			// CounterPropertiesDlg
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(693, 385);
			this.Controls.Add(this.deleteCustomCounterButton);
			this.Controls.Add(this.removeButton);
			this.Controls.Add(this.addButton);
			this.Controls.Add(this.customCounterButton);
			this.Controls.Add(this.button2);
			this.Controls.Add(this.button1);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.groupBox1);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "CounterPropertiesDlg";
			this.ShowInTaskbar = false;
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Counter Properties";
			this.groupBox1.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			this.groupBox2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.scaleTextBox)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.TreeView inactiveCounterView;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Button button2;
		private System.Windows.Forms.Button addButton;
		private System.Windows.Forms.Button removeButton;
		private System.Windows.Forms.Button moveUpButton;
		private System.Windows.Forms.Button moveDownButton;
		private System.Windows.Forms.Button saveButton;
		private ColorButton colorButton;
		private System.Windows.Forms.Button loadButton;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.ListView activeCounterView;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.NumericUpDown scaleTextBox;
		private System.Windows.Forms.Button customCounterButton;
		private System.Windows.Forms.Button deleteCustomCounterButton;
	}
}