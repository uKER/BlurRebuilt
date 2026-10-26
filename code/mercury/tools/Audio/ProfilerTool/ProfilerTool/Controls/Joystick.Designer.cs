namespace ProfilerTool.Controls
{
    partial class Joystick
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
            this.sldY = new Faders.MixFader();
            this.cbxOverride = new System.Windows.Forms.CheckBox();
            this.lblComponent = new System.Windows.Forms.Label();
            this.lblScale = new System.Windows.Forms.Label();
            this.lblCoords = new System.Windows.Forms.Label();
            this.tbxScale = new System.Windows.Forms.TextBox();
            this.xyCtrl = new ProfilerTool.Controls.XYControl();
            this.SuspendLayout();
            // 
            // sldY
            // 
            this.sldY.Disabled = false;
            this.sldY.Location = new System.Drawing.Point(160, 20);
            this.sldY.Name = "sldY";
            this.sldY.Size = new System.Drawing.Size(36, 150);
            this.sldY.TabIndex = 1;
            this.sldY.TickColor = System.Drawing.Color.Black;
            this.sldY.TickFrequency = 10;
            this.sldY.ValueChanged += new System.EventHandler(this.sldY_ValueChanged);
            // 
            // cbxOverride
            // 
            this.cbxOverride.AutoSize = true;
            this.cbxOverride.Location = new System.Drawing.Point(185, 3);
            this.cbxOverride.Name = "cbxOverride";
            this.cbxOverride.Size = new System.Drawing.Size(15, 14);
            this.cbxOverride.TabIndex = 2;
            this.cbxOverride.UseVisualStyleBackColor = true;
            // 
            // lblComponent
            // 
            this.lblComponent.Location = new System.Drawing.Point(5, 5);
            this.lblComponent.Name = "lblComponent";
            this.lblComponent.Size = new System.Drawing.Size(174, 15);
            this.lblComponent.TabIndex = 3;
            this.lblComponent.Text = "name";
            this.lblComponent.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // lblScale
            // 
            this.lblScale.Location = new System.Drawing.Point(0, 175);
            this.lblScale.Name = "lblScale";
            this.lblScale.Size = new System.Drawing.Size(35, 15);
            this.lblScale.TabIndex = 4;
            this.lblScale.Text = "scale:";
            this.lblScale.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // lblCoords
            // 
            this.lblCoords.Location = new System.Drawing.Point(60, 175);
            this.lblCoords.Name = "lblCoords";
            this.lblCoords.Size = new System.Drawing.Size(140, 15);
            this.lblCoords.TabIndex = 5;
            this.lblCoords.Text = "0.0, 0.0, 0.0";
            this.lblCoords.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // tbxScale
            // 
            this.tbxScale.BackColor = System.Drawing.SystemColors.Control;
            this.tbxScale.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbxScale.Location = new System.Drawing.Point(35, 175);
            this.tbxScale.Name = "tbxScale";
            this.tbxScale.Size = new System.Drawing.Size(35, 13);
            this.tbxScale.TabIndex = 6;
            this.tbxScale.Text = "100";
            this.tbxScale.TextChanged += new System.EventHandler(this.tbxScale_TextChanged);
            // 
            // xyCtrl
            // 
            this.xyCtrl.BackColor = System.Drawing.SystemColors.WindowText;
            this.xyCtrl.Location = new System.Drawing.Point(5, 20);
            this.xyCtrl.Name = "xyCtrl";
            this.xyCtrl.Size = new System.Drawing.Size(150, 150);
            this.xyCtrl.TabIndex = 0;
            this.xyCtrl.X = 0F;
            this.xyCtrl.Y = 0F;
            this.xyCtrl.ValueChanged += new System.EventHandler(this.xyCtrl_ValueChanged);
            // 
            // Joystick
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Control;
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.Controls.Add(this.tbxScale);
            this.Controls.Add(this.lblCoords);
            this.Controls.Add(this.lblScale);
            this.Controls.Add(this.lblComponent);
            this.Controls.Add(this.cbxOverride);
            this.Controls.Add(this.sldY);
            this.Controls.Add(this.xyCtrl);
            this.Name = "Joystick";
            this.Size = new System.Drawing.Size(200, 195);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private Controls.XYControl xyCtrl;
        private Faders.MixFader sldY;
        private System.Windows.Forms.CheckBox cbxOverride;
        private System.Windows.Forms.Label lblComponent;
        private System.Windows.Forms.Label lblScale;
        private System.Windows.Forms.Label lblCoords;
        private System.Windows.Forms.TextBox tbxScale;
    }
}
