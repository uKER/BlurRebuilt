namespace SyncTool
{
    partial class InitialScan
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
            this.labelSrc = new System.Windows.Forms.Label();
            this.labelDst = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // labelSrc
            // 
            this.labelSrc.Cursor = System.Windows.Forms.Cursors.AppStarting;
            this.labelSrc.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelSrc.Location = new System.Drawing.Point(13, 13);
            this.labelSrc.Name = "labelSrc";
            this.labelSrc.Size = new System.Drawing.Size(471, 32);
            this.labelSrc.TabIndex = 0;
            // 
            // labelDst
            // 
            this.labelDst.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelDst.Location = new System.Drawing.Point(13, 51);
            this.labelDst.Name = "labelDst";
            this.labelDst.Size = new System.Drawing.Size(471, 32);
            this.labelDst.TabIndex = 1;
            // 
            // InitialScan
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(496, 92);
            this.Controls.Add(this.labelDst);
            this.Controls.Add(this.labelSrc);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "InitialScan";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Bizarre Sync Tool";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form1_FormClosed);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label labelSrc;
        private System.Windows.Forms.Label labelDst;
    }
}

