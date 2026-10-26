namespace ProfilingApplication
{
	partial class GraphViewConfigDlg
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(GraphViewConfigDlg));
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.marginCheckBox = new System.Windows.Forms.CheckBox();
			this.label6 = new System.Windows.Forms.Label();
			this.highlightEveryNumeric = new System.Windows.Forms.NumericUpDown();
			this.label5 = new System.Windows.Forms.Label();
			this.gridSpacingNumeric = new System.Windows.Forms.NumericUpDown();
			this.showGridCheck = new System.Windows.Forms.CheckBox();
			this.graphChartPreview = new ProfilingApplication.GraphChart(null);
			this.label4 = new System.Windows.Forms.Label();
			this.gridLightColourButton = new ProfilingApplication.ColorButton();
			this.gridDarkColourButton = new ProfilingApplication.ColorButton();
			this.label3 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.bgLightColourButton = new ProfilingApplication.ColorButton();
			this.bgDarkColourButton = new ProfilingApplication.ColorButton();
			this.resetButton = new System.Windows.Forms.Button();
			this.okButton = new System.Windows.Forms.Button();
			this.cancelButton = new System.Windows.Forms.Button();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.numValuesNumeric = new System.Windows.Forms.NumericUpDown();
			this.updatesPerSecNumeric = new System.Windows.Forms.NumericUpDown();
			this.label10 = new System.Windows.Forms.Label();
			this.label9 = new System.Windows.Forms.Label();
			this.maxNumeric = new System.Windows.Forms.NumericUpDown();
			this.label8 = new System.Windows.Forms.Label();
			this.label7 = new System.Windows.Forms.Label();
			this.minNumeric = new System.Windows.Forms.NumericUpDown();
			this.multiGraphCheck = new System.Windows.Forms.CheckBox();
			this.autoScaleCheck = new System.Windows.Forms.CheckBox();
			this.groupBox1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.highlightEveryNumeric)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.gridSpacingNumeric)).BeginInit();
			this.groupBox2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.numValuesNumeric)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.updatesPerSecNumeric)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.maxNumeric)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.minNumeric)).BeginInit();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.marginCheckBox);
			this.groupBox1.Controls.Add(this.label6);
			this.groupBox1.Controls.Add(this.highlightEveryNumeric);
			this.groupBox1.Controls.Add(this.label5);
			this.groupBox1.Controls.Add(this.gridSpacingNumeric);
			this.groupBox1.Controls.Add(this.showGridCheck);
			this.groupBox1.Controls.Add(this.graphChartPreview);
			this.groupBox1.Controls.Add(this.label4);
			this.groupBox1.Controls.Add(this.gridLightColourButton);
			this.groupBox1.Controls.Add(this.gridDarkColourButton);
			this.groupBox1.Controls.Add(this.label3);
			this.groupBox1.Controls.Add(this.label2);
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.bgLightColourButton);
			this.groupBox1.Controls.Add(this.bgDarkColourButton);
			this.groupBox1.Location = new System.Drawing.Point(12, 12);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(372, 188);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Colours";
			// 
			// marginCheckBox
			// 
			this.marginCheckBox.AutoSize = true;
			this.marginCheckBox.Location = new System.Drawing.Point(115, 157);
			this.marginCheckBox.Name = "marginCheckBox";
			this.marginCheckBox.Size = new System.Drawing.Size(58, 17);
			this.marginCheckBox.TabIndex = 13;
			this.marginCheckBox.Text = "Margin";
			this.marginCheckBox.UseVisualStyleBackColor = true;
			this.marginCheckBox.CheckedChanged += new System.EventHandler(this.valueChanged);
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(6, 129);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(81, 13);
			this.label6.TabIndex = 12;
			this.label6.Text = "Highlight Every:";
			// 
			// highlightEveryNumeric
			// 
			this.highlightEveryNumeric.Location = new System.Drawing.Point(92, 127);
			this.highlightEveryNumeric.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
			this.highlightEveryNumeric.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.highlightEveryNumeric.Name = "highlightEveryNumeric";
			this.highlightEveryNumeric.Size = new System.Drawing.Size(84, 20);
			this.highlightEveryNumeric.TabIndex = 11;
			this.highlightEveryNumeric.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.highlightEveryNumeric.ValueChanged += new System.EventHandler(this.valueChanged);
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(6, 103);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(71, 13);
			this.label5.TabIndex = 10;
			this.label5.Text = "Grid Spacing:";
			// 
			// gridSpacingNumeric
			// 
			this.gridSpacingNumeric.Location = new System.Drawing.Point(92, 101);
			this.gridSpacingNumeric.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
			this.gridSpacingNumeric.Name = "gridSpacingNumeric";
			this.gridSpacingNumeric.Size = new System.Drawing.Size(84, 20);
			this.gridSpacingNumeric.TabIndex = 9;
			this.gridSpacingNumeric.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
			this.gridSpacingNumeric.ValueChanged += new System.EventHandler(this.valueChanged);
			// 
			// showGridCheck
			// 
			this.showGridCheck.AutoSize = true;
			this.showGridCheck.Location = new System.Drawing.Point(9, 157);
			this.showGridCheck.Name = "showGridCheck";
			this.showGridCheck.Size = new System.Drawing.Size(75, 17);
			this.showGridCheck.TabIndex = 7;
			this.showGridCheck.Text = "Show Grid";
			this.showGridCheck.UseVisualStyleBackColor = true;
			this.showGridCheck.CheckedChanged += new System.EventHandler(this.valueChanged);
			// 
			// graphChartPreview
			// 
			this.graphChartPreview.AccentSpacing = 4;
			this.graphChartPreview.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.graphChartPreview.BackColor = System.Drawing.SystemColors.AppWorkspace;
			this.graphChartPreview.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.graphChartPreview.GridXSpacing = 20;
			this.graphChartPreview.GridYSpacing = 20;
			this.graphChartPreview.HeaderHeight = -1;
			this.graphChartPreview.Indentation = 0;
			this.graphChartPreview.Location = new System.Drawing.Point(190, 16);
			this.graphChartPreview.Name = "graphChartPreview";
			this.graphChartPreview.Size = new System.Drawing.Size(167, 163);
			this.graphChartPreview.TabIndex = 8;
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(6, 70);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(29, 13);
			this.label4.TabIndex = 6;
			this.label4.Text = "Grid:";
			// 
			// gridLightColourButton
			// 
			this.gridLightColourButton.CenterColor = System.Drawing.Color.Empty;
			this.gridLightColourButton.Location = new System.Drawing.Point(133, 65);
			this.gridLightColourButton.Name = "gridLightColourButton";
			this.gridLightColourButton.Size = new System.Drawing.Size(43, 23);
			this.gridLightColourButton.TabIndex = 5;
			this.gridLightColourButton.UseVisualStyleBackColor = true;
			this.gridLightColourButton.Click += new System.EventHandler(this.colourButton_Click);
			// 
			// gridDarkColourButton
			// 
			this.gridDarkColourButton.CenterColor = System.Drawing.Color.Empty;
			this.gridDarkColourButton.Location = new System.Drawing.Point(80, 65);
			this.gridDarkColourButton.Name = "gridDarkColourButton";
			this.gridDarkColourButton.Size = new System.Drawing.Size(43, 23);
			this.gridDarkColourButton.TabIndex = 4;
			this.gridDarkColourButton.UseVisualStyleBackColor = true;
			this.gridDarkColourButton.Click += new System.EventHandler(this.colourButton_Click);
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(137, 16);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(30, 13);
			this.label3.TabIndex = 3;
			this.label3.Text = "Light";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(87, 16);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(30, 13);
			this.label2.TabIndex = 2;
			this.label2.Text = "Dark";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(6, 39);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(68, 13);
			this.label1.TabIndex = 1;
			this.label1.Text = "Background:";
			// 
			// bgLightColourButton
			// 
			this.bgLightColourButton.CenterColor = System.Drawing.Color.Empty;
			this.bgLightColourButton.Location = new System.Drawing.Point(133, 34);
			this.bgLightColourButton.Name = "bgLightColourButton";
			this.bgLightColourButton.Size = new System.Drawing.Size(43, 23);
			this.bgLightColourButton.TabIndex = 0;
			this.bgLightColourButton.UseVisualStyleBackColor = true;
			this.bgLightColourButton.Click += new System.EventHandler(this.colourButton_Click);
			// 
			// bgDarkColourButton
			// 
			this.bgDarkColourButton.CenterColor = System.Drawing.Color.Empty;
			this.bgDarkColourButton.Location = new System.Drawing.Point(80, 34);
			this.bgDarkColourButton.Name = "bgDarkColourButton";
			this.bgDarkColourButton.Size = new System.Drawing.Size(43, 23);
			this.bgDarkColourButton.TabIndex = 0;
			this.bgDarkColourButton.UseVisualStyleBackColor = true;
			this.bgDarkColourButton.Click += new System.EventHandler(this.colourButton_Click);
			// 
			// resetButton
			// 
			this.resetButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.resetButton.Location = new System.Drawing.Point(13, 320);
			this.resetButton.Name = "resetButton";
			this.resetButton.Size = new System.Drawing.Size(75, 23);
			this.resetButton.TabIndex = 8;
			this.resetButton.Text = "Reset";
			this.resetButton.UseVisualStyleBackColor = true;
			this.resetButton.Click += new System.EventHandler(this.resetButton_Click);
			// 
			// okButton
			// 
			this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.okButton.Location = new System.Drawing.Point(229, 320);
			this.okButton.Name = "okButton";
			this.okButton.Size = new System.Drawing.Size(75, 23);
			this.okButton.TabIndex = 9;
			this.okButton.Text = "OK";
			this.okButton.UseVisualStyleBackColor = true;
			// 
			// cancelButton
			// 
			this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelButton.Location = new System.Drawing.Point(310, 320);
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.Size = new System.Drawing.Size(75, 23);
			this.cancelButton.TabIndex = 10;
			this.cancelButton.Text = "Cancel";
			this.cancelButton.UseVisualStyleBackColor = true;
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.numValuesNumeric);
			this.groupBox2.Controls.Add(this.updatesPerSecNumeric);
			this.groupBox2.Controls.Add(this.label10);
			this.groupBox2.Controls.Add(this.label9);
			this.groupBox2.Controls.Add(this.maxNumeric);
			this.groupBox2.Controls.Add(this.label8);
			this.groupBox2.Controls.Add(this.label7);
			this.groupBox2.Controls.Add(this.minNumeric);
			this.groupBox2.Controls.Add(this.multiGraphCheck);
			this.groupBox2.Controls.Add(this.autoScaleCheck);
			this.groupBox2.Location = new System.Drawing.Point(12, 206);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(372, 108);
			this.groupBox2.TabIndex = 11;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Layout";
			// 
			// numValuesNumeric
			// 
			this.numValuesNumeric.Location = new System.Drawing.Point(105, 70);
			this.numValuesNumeric.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
			this.numValuesNumeric.Minimum = new decimal(new int[] {
            20,
            0,
            0,
            0});
			this.numValuesNumeric.Name = "numValuesNumeric";
			this.numValuesNumeric.Size = new System.Drawing.Size(68, 20);
			this.numValuesNumeric.TabIndex = 8;
			this.numValuesNumeric.Value = new decimal(new int[] {
            20,
            0,
            0,
            0});
			this.numValuesNumeric.ValueChanged += new System.EventHandler(this.valueChanged);
			// 
			// updatesPerSecNumeric
			// 
			this.updatesPerSecNumeric.Location = new System.Drawing.Point(277, 70);
			this.updatesPerSecNumeric.Maximum = new decimal(new int[] {
            15,
            0,
            0,
            0});
			this.updatesPerSecNumeric.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.updatesPerSecNumeric.Name = "updatesPerSecNumeric";
			this.updatesPerSecNumeric.Size = new System.Drawing.Size(80, 20);
			this.updatesPerSecNumeric.TabIndex = 8;
			this.updatesPerSecNumeric.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.updatesPerSecNumeric.ValueChanged += new System.EventHandler(this.valueChanged);
			// 
			// label10
			// 
			this.label10.AutoSize = true;
			this.label10.Location = new System.Drawing.Point(6, 72);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(93, 13);
			this.label10.TabIndex = 7;
			this.label10.Text = "Number of values:";
			// 
			// label9
			// 
			this.label9.AutoSize = true;
			this.label9.Location = new System.Drawing.Point(187, 72);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(71, 13);
			this.label9.TabIndex = 6;
			this.label9.Text = "Updates/Sec";
			// 
			// maxNumeric
			// 
			this.maxNumeric.Location = new System.Drawing.Point(277, 44);
			this.maxNumeric.Name = "maxNumeric";
			this.maxNumeric.Size = new System.Drawing.Size(80, 20);
			this.maxNumeric.TabIndex = 4;
			this.maxNumeric.ValueChanged += new System.EventHandler(this.valueChanged);
			// 
			// label8
			// 
			this.label8.AutoSize = true;
			this.label8.Location = new System.Drawing.Point(187, 46);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(30, 13);
			this.label8.TabIndex = 3;
			this.label8.Text = "Max:";
			// 
			// label7
			// 
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(187, 20);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(27, 13);
			this.label7.TabIndex = 3;
			this.label7.Text = "Min:";
			// 
			// minNumeric
			// 
			this.minNumeric.Location = new System.Drawing.Point(277, 18);
			this.minNumeric.Name = "minNumeric";
			this.minNumeric.Size = new System.Drawing.Size(80, 20);
			this.minNumeric.TabIndex = 2;
			this.minNumeric.ValueChanged += new System.EventHandler(this.valueChanged);
			// 
			// multiGraphCheck
			// 
			this.multiGraphCheck.AutoSize = true;
			this.multiGraphCheck.Location = new System.Drawing.Point(9, 45);
			this.multiGraphCheck.Name = "multiGraphCheck";
			this.multiGraphCheck.Size = new System.Drawing.Size(123, 17);
			this.multiGraphCheck.TabIndex = 1;
			this.multiGraphCheck.Text = "Multiple-Graph mode";
			this.multiGraphCheck.UseVisualStyleBackColor = true;
			this.multiGraphCheck.CheckedChanged += new System.EventHandler(this.valueChanged);
			// 
			// autoScaleCheck
			// 
			this.autoScaleCheck.AutoSize = true;
			this.autoScaleCheck.Location = new System.Drawing.Point(9, 20);
			this.autoScaleCheck.Name = "autoScaleCheck";
			this.autoScaleCheck.Size = new System.Drawing.Size(78, 17);
			this.autoScaleCheck.TabIndex = 0;
			this.autoScaleCheck.Text = "Auto-Scale";
			this.autoScaleCheck.UseVisualStyleBackColor = true;
			this.autoScaleCheck.CheckedChanged += new System.EventHandler(this.valueChanged);
			// 
			// GraphViewConfigDlg
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(396, 352);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.cancelButton);
			this.Controls.Add(this.okButton);
			this.Controls.Add(this.resetButton);
			this.Controls.Add(this.groupBox1);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "GraphViewConfigDlg";
			this.ShowInTaskbar = false;
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "GraphView Configuration";
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.highlightEveryNumeric)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.gridSpacingNumeric)).EndInit();
			this.groupBox2.ResumeLayout(false);
			this.groupBox2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.numValuesNumeric)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.updatesPerSecNumeric)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.maxNumeric)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.minNumeric)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.GroupBox groupBox1;
		private ColorButton bgDarkColourButton;
		private ColorButton gridLightColourButton;
		private ColorButton gridDarkColourButton;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label1;
		private ColorButton bgLightColourButton;
		private System.Windows.Forms.CheckBox showGridCheck;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Button resetButton;
		private System.Windows.Forms.NumericUpDown gridSpacingNumeric;
		private GraphChart graphChartPreview;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.NumericUpDown highlightEveryNumeric;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Button okButton;
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.CheckBox multiGraphCheck;
		private System.Windows.Forms.CheckBox autoScaleCheck;
		private System.Windows.Forms.NumericUpDown maxNumeric;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.NumericUpDown minNumeric;
		private System.Windows.Forms.NumericUpDown numValuesNumeric;
		private System.Windows.Forms.NumericUpDown updatesPerSecNumeric;
		private System.Windows.Forms.Label label10;
		private System.Windows.Forms.Label label9;
		private System.Windows.Forms.CheckBox marginCheckBox;
	}
}