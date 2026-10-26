namespace ProfilerTool.Controls
{
    partial class XYControl
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
            this.picStick = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.picStick)).BeginInit();
            this.SuspendLayout();
            // 
            // picStick
            // 
            this.picStick.Image = global::ProfilerTool.Properties.Resources.picStick;
            this.picStick.Location = new System.Drawing.Point(42, 42);
            this.picStick.Name = "picStick";
            this.picStick.Size = new System.Drawing.Size(16, 16);
            this.picStick.TabIndex = 0;
            this.picStick.TabStop = false;
            this.picStick.MouseMove += new System.Windows.Forms.MouseEventHandler(this.picStick_MouseMove);
            this.picStick.MouseDown += new System.Windows.Forms.MouseEventHandler(this.picStick_MouseDown);
            this.picStick.MouseUp += new System.Windows.Forms.MouseEventHandler(this.picStick_MouseUp);
            // 
            // XYControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.WindowText;
            this.Controls.Add(this.picStick);
            this.Name = "XYControl";
            this.Size = new System.Drawing.Size(100, 100);
            ((System.ComponentModel.ISupportInitialize)(this.picStick)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox picStick;
    }
}
