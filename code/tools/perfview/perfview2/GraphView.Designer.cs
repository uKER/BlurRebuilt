namespace ProfilingApplication
{
    partial class GraphView
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(GraphView));
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.pauseButton = new System.Windows.Forms.CheckBox();
			this.continuousCaptureCheckbox = new System.Windows.Forms.CheckBox();
			this.button1 = new System.Windows.Forms.Button();
			this.addCounterButton = new System.Windows.Forms.Button();
			this.counterListView = new ProfilingApplication.DoubleBufferedListView();
			this.colourHeader = new System.Windows.Forms.ColumnHeader();
			this.nameHeader = new System.Windows.Forms.ColumnHeader();
			this.valueHeader = new System.Windows.Forms.ColumnHeader();
			this.minHeader = new System.Windows.Forms.ColumnHeader();
			this.maxHeader = new System.Windows.Forms.ColumnHeader();
			this.scaleHeader = new System.Windows.Forms.ColumnHeader();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			this.SuspendLayout();
			// 
			// splitContainer1
			// 
			this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
			this.splitContainer1.Location = new System.Drawing.Point(0, 0);
			this.splitContainer1.Name = "splitContainer1";
			this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.pauseButton);
			this.splitContainer1.Panel2.Controls.Add(this.continuousCaptureCheckbox);
			this.splitContainer1.Panel2.Controls.Add(this.button1);
			this.splitContainer1.Panel2.Controls.Add(this.addCounterButton);
			this.splitContainer1.Panel2.Controls.Add(this.counterListView);
			this.splitContainer1.Size = new System.Drawing.Size(597, 538);
			this.splitContainer1.SplitterDistance = 343;
			this.splitContainer1.TabIndex = 2;
			this.splitContainer1.TabStop = false;
			// 
			// pauseButton
			// 
			this.pauseButton.Appearance = System.Windows.Forms.Appearance.Button;
			this.pauseButton.AutoSize = true;
			this.pauseButton.Image = ((System.Drawing.Image)(resources.GetObject("pauseButton.Image")));
			this.pauseButton.ImageAlign = System.Drawing.ContentAlignment.BottomRight;
			this.pauseButton.Location = new System.Drawing.Point(567, 162);
			this.pauseButton.Name = "pauseButton";
			this.pauseButton.Size = new System.Drawing.Size(22, 22);
			this.pauseButton.TabIndex = 3;
			this.pauseButton.UseVisualStyleBackColor = true;
			this.pauseButton.CheckedChanged += new System.EventHandler(this.pauseButton_CheckedChanged);
			// 
			// continuousCaptureCheckbox
			// 
			this.continuousCaptureCheckbox.AutoSize = true;
			this.continuousCaptureCheckbox.Location = new System.Drawing.Point(12, 165);
			this.continuousCaptureCheckbox.Name = "continuousCaptureCheckbox";
			this.continuousCaptureCheckbox.Size = new System.Drawing.Size(119, 17);
			this.continuousCaptureCheckbox.TabIndex = 2;
			this.continuousCaptureCheckbox.Text = "Continuous Capture";
			this.continuousCaptureCheckbox.UseVisualStyleBackColor = true;
			this.continuousCaptureCheckbox.CheckedChanged += new System.EventHandler(this.continuousCaptureCheckbox_CheckedChanged);
			// 
			// button1
			// 
			this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.button1.Location = new System.Drawing.Point(474, 162);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(87, 22);
			this.button1.TabIndex = 1;
			this.button1.Text = "&Configure...";
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler(this.graphViewConfigButton_Click);
			// 
			// addCounterButton
			// 
			this.addCounterButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.addCounterButton.Location = new System.Drawing.Point(381, 162);
			this.addCounterButton.Name = "addCounterButton";
			this.addCounterButton.Size = new System.Drawing.Size(87, 22);
			this.addCounterButton.TabIndex = 1;
			this.addCounterButton.Text = "&Edit Counters...";
			this.addCounterButton.UseVisualStyleBackColor = true;
			this.addCounterButton.Click += new System.EventHandler(this.addCounterButton_Click);
			// 
			// counterListView
			// 
			this.counterListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.counterListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colourHeader,
            this.nameHeader,
            this.valueHeader,
            this.minHeader,
            this.maxHeader,
            this.scaleHeader});
			this.counterListView.Location = new System.Drawing.Point(3, -1);
			this.counterListView.Name = "counterListView";
			this.counterListView.Size = new System.Drawing.Size(591, 160);
			this.counterListView.TabIndex = 0;
			this.counterListView.UseCompatibleStateImageBehavior = false;
			this.counterListView.View = System.Windows.Forms.View.Details;
			// 
			// colourHeader
			// 
			this.colourHeader.Text = "Colour";
			this.colourHeader.Width = 44;
			// 
			// nameHeader
			// 
			this.nameHeader.Text = "Name";
			this.nameHeader.Width = 229;
			// 
			// valueHeader
			// 
			this.valueHeader.Text = "Current";
			this.valueHeader.Width = 79;
			// 
			// minHeader
			// 
			this.minHeader.Text = "Min";
			this.minHeader.Width = 76;
			// 
			// maxHeader
			// 
			this.maxHeader.Text = "Max";
			this.maxHeader.Width = 74;
			// 
			// scaleHeader
			// 
			this.scaleHeader.Text = "Scale";
			// 
			// GraphView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(597, 538);
			this.Controls.Add(this.splitContainer1);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "GraphView";
			this.TabText = "GraphView";
			this.Text = "GraphView";
			this.SizeChanged += new System.EventHandler(this.GraphView_SizeChanged);
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.GraphView_FormClosing);
			this.splitContainer1.Panel2.ResumeLayout(false);
			this.splitContainer1.Panel2.PerformLayout();
			this.splitContainer1.ResumeLayout(false);
			this.ResumeLayout(false);

        }

        #endregion

		private System.Windows.Forms.SplitContainer splitContainer1;
		private System.Windows.Forms.Button addCounterButton;
		private DoubleBufferedListView counterListView;
		private System.Windows.Forms.ColumnHeader nameHeader;
		private System.Windows.Forms.ColumnHeader valueHeader;
		private System.Windows.Forms.ColumnHeader colourHeader;
		private System.Windows.Forms.ColumnHeader scaleHeader;
		private System.Windows.Forms.ColumnHeader minHeader;
		private System.Windows.Forms.ColumnHeader maxHeader;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.CheckBox pauseButton;
		private System.Windows.Forms.CheckBox continuousCaptureCheckbox;
    }
}