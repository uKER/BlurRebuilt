namespace ProfilerTool.Controls
{
    partial class Vector
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
            this.cbxOverride = new System.Windows.Forms.CheckBox();
            this.lblX = new System.Windows.Forms.Label();
            this.lblY = new System.Windows.Forms.Label();
            this.lblZ = new System.Windows.Forms.Label();
            this.tbxX = new System.Windows.Forms.TextBox();
            this.tbxY = new System.Windows.Forms.TextBox();
            this.tbxZ = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // cbxOverride
            // 
            this.cbxOverride.AutoSize = true;
            this.cbxOverride.Location = new System.Drawing.Point(81, 3);
            this.cbxOverride.Name = "cbxOverride";
            this.cbxOverride.Size = new System.Drawing.Size(15, 14);
            this.cbxOverride.TabIndex = 0;
            this.cbxOverride.UseVisualStyleBackColor = true;
            // 
            // lblX
            // 
            this.lblX.Location = new System.Drawing.Point(5, 20);
            this.lblX.Name = "lblX";
            this.lblX.Size = new System.Drawing.Size(15, 15);
            this.lblX.TabIndex = 1;
            this.lblX.Text = "X:";
            // 
            // lblY
            // 
            this.lblY.Location = new System.Drawing.Point(5, 35);
            this.lblY.Name = "lblY";
            this.lblY.Size = new System.Drawing.Size(17, 13);
            this.lblY.TabIndex = 1;
            this.lblY.Text = "Y:";
            // 
            // lblZ
            // 
            this.lblZ.Location = new System.Drawing.Point(5, 50);
            this.lblZ.Name = "lblZ";
            this.lblZ.Size = new System.Drawing.Size(17, 13);
            this.lblZ.TabIndex = 1;
            this.lblZ.Text = "Z:";
            // 
            // tbxX
            // 
            this.tbxX.BackColor = System.Drawing.SystemColors.Control;
            this.tbxX.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbxX.Location = new System.Drawing.Point(25, 20);
            this.tbxX.Name = "tbxX";
            this.tbxX.Size = new System.Drawing.Size(60, 13);
            this.tbxX.TabIndex = 2;
            this.tbxX.Text = "0.00";
            this.tbxX.GotFocus += new System.EventHandler(tbxX_GotFocus);
            this.tbxX.LostFocus += new System.EventHandler(tbxX_LostFocus);
            this.tbxX.KeyPress += new System.Windows.Forms.KeyPressEventHandler(tbxX_CheckKeys);
            // 
            // tbxY
            // 
            this.tbxY.BackColor = System.Drawing.SystemColors.Control;
            this.tbxY.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbxY.Location = new System.Drawing.Point(25, 35);
            this.tbxY.Name = "tbxY";
            this.tbxY.Size = new System.Drawing.Size(60, 13);
            this.tbxY.TabIndex = 2;
            this.tbxY.Text = "0.00";
            this.tbxY.GotFocus += new System.EventHandler(tbxY_GotFocus);
            this.tbxY.LostFocus += new System.EventHandler(tbxY_LostFocus);
            this.tbxY.KeyPress += new System.Windows.Forms.KeyPressEventHandler(tbxY_CheckKeys);
            // 
            // tbxZ
            // 
            this.tbxZ.BackColor = System.Drawing.SystemColors.Control;
            this.tbxZ.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbxZ.Location = new System.Drawing.Point(25, 50);
            this.tbxZ.Name = "tbxZ";
            this.tbxZ.Size = new System.Drawing.Size(60, 13);
            this.tbxZ.TabIndex = 2;
            this.tbxZ.Text = "0.00";
            this.tbxZ.GotFocus += new System.EventHandler(tbxZ_GotFocus);
            this.tbxZ.LostFocus += new System.EventHandler(tbxZ_LostFocus);
            this.tbxZ.KeyPress += new System.Windows.Forms.KeyPressEventHandler(tbxZ_CheckKeys);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(0, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(80, 15);
            this.label1.TabIndex = 15;
            this.label1.Text = "name";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // Vector
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.Controls.Add(this.label1);
            this.Controls.Add(this.tbxZ);
            this.Controls.Add(this.tbxY);
            this.Controls.Add(this.tbxX);
            this.Controls.Add(this.lblZ);
            this.Controls.Add(this.lblY);
            this.Controls.Add(this.lblX);
            this.Controls.Add(this.cbxOverride);
            this.Name = "Vector";
            this.Size = new System.Drawing.Size(100, 80);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox cbxOverride;
        private System.Windows.Forms.Label lblX;
        private System.Windows.Forms.Label lblY;
        private System.Windows.Forms.Label lblZ;
        private System.Windows.Forms.TextBox tbxX;
        private System.Windows.Forms.TextBox tbxY;
        private System.Windows.Forms.TextBox tbxZ;
        private System.Windows.Forms.Label label1;
    }
}
