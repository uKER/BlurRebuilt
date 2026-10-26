namespace ProfilerTool.Controls
{
    partial class Log
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
            this.lvLog = new System.Windows.Forms.ListView();
            this.cbxPause = new System.Windows.Forms.CheckBox();
            this.btnClear = new System.Windows.Forms.Button();
            this.splLog = new System.Windows.Forms.SplitContainer();
            this.splLog.Panel1.SuspendLayout();
            this.splLog.Panel2.SuspendLayout();
            this.splLog.SuspendLayout();
            this.SuspendLayout();
            // 
            // lvLog
            // 
            this.lvLog.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lvLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvLog.FullRowSelect = true;
            this.lvLog.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.lvLog.Location = new System.Drawing.Point(0, 0);
            this.lvLog.Name = "lvLog";
            this.lvLog.Size = new System.Drawing.Size(482, 450);
            this.lvLog.TabIndex = 1;
            this.lvLog.UseCompatibleStateImageBehavior = false;
            this.lvLog.View = System.Windows.Forms.View.Details;
            // 
            // cbxPause
            // 
            this.cbxPause.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cbxPause.Location = new System.Drawing.Point(427, 2);
            this.cbxPause.Name = "cbxPause";
            this.cbxPause.Size = new System.Drawing.Size(55, 20);
            this.cbxPause.TabIndex = 0;
            this.cbxPause.Text = "pause";
            this.cbxPause.UseVisualStyleBackColor = true;
            // 
            // btnClear
            // 
            this.btnClear.FlatAppearance.MouseOverBackColor = System.Drawing.SystemColors.Window;
            this.btnClear.Location = new System.Drawing.Point(0, 0);
            this.btnClear.Name = "btnClear";
            this.btnClear.Size = new System.Drawing.Size(55, 24);
            this.btnClear.TabIndex = 1;
            this.btnClear.Text = "clear";
            this.btnClear.UseVisualStyleBackColor = true;
            this.btnClear.Click += new System.EventHandler(this.btnClear_Click);
            // 
            // splLog
            // 
            this.splLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splLog.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splLog.IsSplitterFixed = true;
            this.splLog.Location = new System.Drawing.Point(0, 0);
            this.splLog.Name = "splLog";
            this.splLog.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splLog.Panel1
            // 
            this.splLog.Panel1.Controls.Add(this.lvLog);
            // 
            // splLog.Panel2
            // 
            this.splLog.Panel2.Controls.Add(this.btnClear);
            this.splLog.Panel2.Controls.Add(this.cbxPause);
            this.splLog.Size = new System.Drawing.Size(482, 482);
            this.splLog.SplitterDistance = 450;
            this.splLog.TabIndex = 3;
            // 
            // Log
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.splLog);
            this.Name = "Log";
            this.Size = new System.Drawing.Size(482, 482);
            this.splLog.Panel1.ResumeLayout(false);
            this.splLog.Panel2.ResumeLayout(false);
            this.splLog.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lvLog;
        private System.Windows.Forms.CheckBox cbxPause;
        private System.Windows.Forms.Button btnClear;
        private System.Windows.Forms.SplitContainer splLog;
    }
}
