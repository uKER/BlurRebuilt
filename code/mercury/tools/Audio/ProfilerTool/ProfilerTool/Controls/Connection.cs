using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    public partial class Connection : UserControl
    {
        public Connection()
        {
            InitializeComponent();
        }

        public string LeftObject
        {
            get { return lblLObject.Text; }
            set { lblLObject.Text = value; }
        }

        public string RightObject
        {
            get { return lblRObject.Text; }
            set { lblRObject.Text = value; }
        }

        public string LeftComponentType
        {
            get { return lblLType.Text; }
            set { lblLType.Text = value; }
        }

        public string RightComponentType
        {
            get { return lblRType.Text; }
            set { lblRType.Text = value; }
        }

        public string LeftComponentName
        {
            get { return lblLName.Text; }
            set { lblLName.Text = value; }
        }

        public string RightComponentName
        {
            get { return lblRName.Text; }
            set { lblRName.Text = value; }
        }

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
            this.pnlLeft = new System.Windows.Forms.Panel();
            this.pnlRight = new System.Windows.Forms.Panel();
            this.lblLObject = new System.Windows.Forms.Label();
            this.lblLType = new System.Windows.Forms.Label();
            this.lblLName = new System.Windows.Forms.Label();
            this.lblRType = new System.Windows.Forms.Label();
            this.lblRObject = new System.Windows.Forms.Label();
            this.lblRName = new System.Windows.Forms.Label();
            this.panel1 = new System.Windows.Forms.Panel();
            this.pnlLeft.SuspendLayout();
            this.pnlRight.SuspendLayout();
            this.SuspendLayout();
            // 
            // pnlLeft
            // 
            this.pnlLeft.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pnlLeft.Controls.Add(this.lblLName);
            this.pnlLeft.Controls.Add(this.lblLType);
            this.pnlLeft.Controls.Add(this.lblLObject);
            this.pnlLeft.Location = new System.Drawing.Point(0, 0);
            this.pnlLeft.Name = "pnlLeft";
            this.pnlLeft.Size = new System.Drawing.Size(120, 45);
            this.pnlLeft.TabIndex = 0;
            // 
            // pnlRight
            // 
            this.pnlRight.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pnlRight.Controls.Add(this.lblRName);
            this.pnlRight.Controls.Add(this.lblRObject);
            this.pnlRight.Controls.Add(this.lblRType);
            this.pnlRight.Location = new System.Drawing.Point(145, 0);
            this.pnlRight.Name = "pnlRight";
            this.pnlRight.Size = new System.Drawing.Size(120, 45);
            this.pnlRight.TabIndex = 0;
            // 
            // lblLObject
            // 
            this.lblLObject.AutoSize = true;
            this.lblLObject.Location = new System.Drawing.Point(0, 0);
            this.lblLObject.Name = "lblLObject";
            this.lblLObject.Size = new System.Drawing.Size(38, 13);
            this.lblLObject.TabIndex = 1;
            this.lblLObject.Text = "Object";
            // 
            // lblLType
            // 
            this.lblLType.AutoSize = true;
            this.lblLType.Location = new System.Drawing.Point(0, 15);
            this.lblLType.Name = "lblLType";
            this.lblLType.Size = new System.Drawing.Size(31, 13);
            this.lblLType.TabIndex = 1;
            this.lblLType.Text = "Type";
            // 
            // lblLName
            // 
            this.lblLName.AutoSize = true;
            this.lblLName.Location = new System.Drawing.Point(0, 30);
            this.lblLName.Name = "lblLName";
            this.lblLName.Size = new System.Drawing.Size(35, 13);
            this.lblLName.TabIndex = 1;
            this.lblLName.Text = "Name";
            // 
            // lblRType
            // 
            this.lblRType.AutoSize = true;
            this.lblRType.Location = new System.Drawing.Point(0, 15);
            this.lblRType.Name = "lblRType";
            this.lblRType.Size = new System.Drawing.Size(31, 13);
            this.lblRType.TabIndex = 1;
            this.lblRType.Text = "Type";
            // 
            // lblRObject
            // 
            this.lblRObject.AutoSize = true;
            this.lblRObject.Location = new System.Drawing.Point(0, 0);
            this.lblRObject.Name = "lblRObject";
            this.lblRObject.Size = new System.Drawing.Size(38, 13);
            this.lblRObject.TabIndex = 1;
            this.lblRObject.Text = "Object";
            // 
            // lblRName
            // 
            this.lblRName.AutoSize = true;
            this.lblRName.Location = new System.Drawing.Point(0, 30);
            this.lblRName.Name = "lblRName";
            this.lblRName.Size = new System.Drawing.Size(35, 13);
            this.lblRName.TabIndex = 1;
            this.lblRName.Text = "Name";
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Location = new System.Drawing.Point(120, 15);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(25, 15);
            this.panel1.TabIndex = 1;
            // 
            // Connection
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.pnlRight);
            this.Controls.Add(this.pnlLeft);
            this.Name = "Connection";
            this.Size = new System.Drawing.Size(265, 45);
            this.pnlLeft.ResumeLayout(false);
            this.pnlLeft.PerformLayout();
            this.pnlRight.ResumeLayout(false);
            this.pnlRight.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel pnlLeft;
        private System.Windows.Forms.Panel pnlRight;
        private System.Windows.Forms.Label lblLName;
        private System.Windows.Forms.Label lblLType;
        private System.Windows.Forms.Label lblLObject;
        private System.Windows.Forms.Label lblRName;
        private System.Windows.Forms.Label lblRObject;
        private System.Windows.Forms.Label lblRType;
        private System.Windows.Forms.Panel panel1;
    }
}
