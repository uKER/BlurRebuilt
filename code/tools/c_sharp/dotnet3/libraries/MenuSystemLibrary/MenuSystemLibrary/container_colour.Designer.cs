namespace MenuSystemLibrary
{
    partial class Container_Colour
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
            this.name = new System.Windows.Forms.Label();
            this.red = new System.Windows.Forms.Label();
            this.redScroll = new System.Windows.Forms.NumericUpDown();
            this.greenScroll = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.blueScroll = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.colourButton = new System.Windows.Forms.Button();
            this.alphaScroll = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.redSlider = new System.Windows.Forms.TrackBar();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.greenSlider = new System.Windows.Forms.TrackBar();
            this.label6 = new System.Windows.Forms.Label();
            this.blueSlider = new System.Windows.Forms.TrackBar();
            this.label7 = new System.Windows.Forms.Label();
            this.alphaSlider = new System.Windows.Forms.TrackBar();
            ((System.ComponentModel.ISupportInitialize)(this.redScroll)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.greenScroll)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.blueScroll)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.alphaScroll)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.redSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.greenSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.blueSlider)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.alphaSlider)).BeginInit();
            this.SuspendLayout();
            // 
            // name
            // 
            this.name.AutoSize = true;
            this.name.Location = new System.Drawing.Point(6, 6);
            this.name.Name = "name";
            this.name.Size = new System.Drawing.Size(37, 13);
            this.name.TabIndex = 0;
            this.name.Text = "Colour";
            // 
            // red
            // 
            this.red.AutoSize = true;
            this.red.Location = new System.Drawing.Point(192, 6);
            this.red.Name = "red";
            this.red.Size = new System.Drawing.Size(15, 13);
            this.red.TabIndex = 1;
            this.red.Text = "R";
            // 
            // redScroll
            // 
            this.redScroll.DecimalPlaces = 2;
            this.redScroll.Increment = new decimal(new int[] {
            1,
            0,
            0,
            131072});
            this.redScroll.Location = new System.Drawing.Point(213, 3);
            this.redScroll.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.redScroll.Name = "redScroll";
            this.redScroll.Size = new System.Drawing.Size(47, 20);
            this.redScroll.TabIndex = 2;
            this.redScroll.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.redScroll.ValueChanged += new System.EventHandler(this.colourScroll_ValueChanged);
            // 
            // greenScroll
            // 
            this.greenScroll.DecimalPlaces = 2;
            this.greenScroll.Increment = new decimal(new int[] {
            1,
            0,
            0,
            131072});
            this.greenScroll.Location = new System.Drawing.Point(288, 3);
            this.greenScroll.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.greenScroll.Name = "greenScroll";
            this.greenScroll.Size = new System.Drawing.Size(47, 20);
            this.greenScroll.TabIndex = 4;
            this.greenScroll.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.greenScroll.ValueChanged += new System.EventHandler(this.colourScroll_ValueChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(267, 6);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(15, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "G";
            // 
            // blueScroll
            // 
            this.blueScroll.DecimalPlaces = 2;
            this.blueScroll.Increment = new decimal(new int[] {
            1,
            0,
            0,
            131072});
            this.blueScroll.Location = new System.Drawing.Point(364, 3);
            this.blueScroll.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.blueScroll.Name = "blueScroll";
            this.blueScroll.Size = new System.Drawing.Size(47, 20);
            this.blueScroll.TabIndex = 6;
            this.blueScroll.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.blueScroll.ValueChanged += new System.EventHandler(this.colourScroll_ValueChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(343, 6);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(14, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "B";
            // 
            // colourButton
            // 
            this.colourButton.Location = new System.Drawing.Point(364, 29);
            this.colourButton.Name = "colourButton";
            this.colourButton.Size = new System.Drawing.Size(47, 22);
            this.colourButton.TabIndex = 7;
            this.colourButton.Text = "Pick";
            this.colourButton.UseVisualStyleBackColor = true;
            this.colourButton.Click += new System.EventHandler(this.colourButton_Click);
            // 
            // alphaScroll
            // 
            this.alphaScroll.DecimalPlaces = 2;
            this.alphaScroll.Increment = new decimal(new int[] {
            1,
            0,
            0,
            131072});
            this.alphaScroll.Location = new System.Drawing.Point(139, 3);
            this.alphaScroll.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.alphaScroll.Name = "alphaScroll";
            this.alphaScroll.Size = new System.Drawing.Size(47, 20);
            this.alphaScroll.TabIndex = 9;
            this.alphaScroll.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.alphaScroll.ValueChanged += new System.EventHandler(this.colourScroll_ValueChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(119, 6);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(14, 13);
            this.label3.TabIndex = 8;
            this.label3.Text = "A";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(9, 29);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(47, 22);
            this.button1.TabIndex = 10;
            this.button1.Text = "more...";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // redSlider
            // 
            this.redSlider.LargeChange = 10;
            this.redSlider.Location = new System.Drawing.Point(80, 86);
            this.redSlider.Maximum = 100;
            this.redSlider.Name = "redSlider";
            this.redSlider.Size = new System.Drawing.Size(331, 45);
            this.redSlider.TabIndex = 32;
            this.redSlider.TickFrequency = 10;
            this.redSlider.Value = 100;
            this.redSlider.Scroll += new System.EventHandler(this.redSlider_Scroll);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(13, 95);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(27, 13);
            this.label4.TabIndex = 12;
            this.label4.Text = "Red";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(13, 122);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(36, 13);
            this.label5.TabIndex = 33;
            this.label5.Text = "Green";
            // 
            // greenSlider
            // 
            this.greenSlider.LargeChange = 10;
            this.greenSlider.Location = new System.Drawing.Point(80, 115);
            this.greenSlider.Maximum = 100;
            this.greenSlider.Name = "greenSlider";
            this.greenSlider.Size = new System.Drawing.Size(331, 45);
            this.greenSlider.TabIndex = 34;
            this.greenSlider.TickFrequency = 10;
            this.greenSlider.Value = 100;
            this.greenSlider.Scroll += new System.EventHandler(this.greenSlider_Scroll);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(13, 152);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(28, 13);
            this.label6.TabIndex = 35;
            this.label6.Text = "Blue";
            // 
            // blueSlider
            // 
            this.blueSlider.LargeChange = 10;
            this.blueSlider.Location = new System.Drawing.Point(80, 143);
            this.blueSlider.Maximum = 100;
            this.blueSlider.Name = "blueSlider";
            this.blueSlider.Size = new System.Drawing.Size(331, 45);
            this.blueSlider.TabIndex = 36;
            this.blueSlider.TickFrequency = 10;
            this.blueSlider.Value = 100;
            this.blueSlider.Scroll += new System.EventHandler(this.blueSlider_Scroll);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(13, 66);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(34, 13);
            this.label7.TabIndex = 37;
            this.label7.Text = "Alpha";
            // 
            // alphaSlider
            // 
            this.alphaSlider.LargeChange = 10;
            this.alphaSlider.Location = new System.Drawing.Point(80, 57);
            this.alphaSlider.Maximum = 100;
            this.alphaSlider.Name = "alphaSlider";
            this.alphaSlider.Size = new System.Drawing.Size(331, 45);
            this.alphaSlider.TabIndex = 38;
            this.alphaSlider.TickFrequency = 10;
            this.alphaSlider.Value = 100;
            this.alphaSlider.Scroll += new System.EventHandler(this.alphaSlider_Scroll);
            // 
            // Container_Colour
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScroll = false;
            this.Controls.Add(this.blueSlider);
            this.Controls.Add(this.greenSlider);
            this.Controls.Add(this.redSlider);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.alphaSlider);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.alphaScroll);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.colourButton);
            this.Controls.Add(this.blueScroll);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.greenScroll);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.redScroll);
            this.Controls.Add(this.red);
            this.Controls.Add(this.name);
            this.Name = "Container_Colour";
            this.Size = new System.Drawing.Size(414, 184);
            ((System.ComponentModel.ISupportInitialize)(this.redScroll)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.greenScroll)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.blueScroll)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.alphaScroll)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.redSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.greenSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.blueSlider)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.alphaSlider)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label name;
        private System.Windows.Forms.Label red;
        private System.Windows.Forms.NumericUpDown redScroll;
        private System.Windows.Forms.NumericUpDown greenScroll;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown blueScroll;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button colourButton;
        private System.Windows.Forms.NumericUpDown alphaScroll;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TrackBar redSlider;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TrackBar greenSlider;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TrackBar blueSlider;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TrackBar alphaSlider;
    }
}
