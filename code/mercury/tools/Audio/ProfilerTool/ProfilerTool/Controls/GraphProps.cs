using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    public partial class GraphProps : UserControl
    {
        public Graph graph;
        private Label lblValueVal;
        private Label lblTick;
        private CheckBox cbxPause;
        private CheckBox cbxLockScale;
        private TextBox tbxTickVal;
        private TextBox tbMaxValue;
        private Label lblFrame;
        private Label lblFrameVal;
        private TextBox tbMinValue;

        public GraphProps()
        {
            InitializeComponent();
        }

        public GraphProps(string name, Size size)
        {
            InitializeComponent();

            Name = name;
            Size = size;
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
            this.cbxPause = new System.Windows.Forms.CheckBox();
            this.cbxLockScale = new System.Windows.Forms.CheckBox();
            this.tbxTickVal = new System.Windows.Forms.TextBox();
            this.lblValueVal = new System.Windows.Forms.Label();
            this.lblTick = new System.Windows.Forms.Label();
            this.tbMinValue = new System.Windows.Forms.TextBox();
            this.tbMaxValue = new System.Windows.Forms.TextBox();
            this.graph = new ProfilerTool.Controls.Graph();
            this.lblFrame = new System.Windows.Forms.Label();
            this.lblFrameVal = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // cbxPause
            // 
            this.cbxPause.AutoSize = true;
            this.cbxPause.Location = new System.Drawing.Point(105, 106);
            this.cbxPause.Name = "cbxPause";
            this.cbxPause.Size = new System.Drawing.Size(55, 17);
            this.cbxPause.TabIndex = 3;
            this.cbxPause.Text = "pause";
            this.cbxPause.UseVisualStyleBackColor = true;
            this.cbxPause.CheckedChanged += new System.EventHandler(this.cbxPause_CheckedChanged);
            // 
            // cbxLockScale
            // 
            this.cbxLockScale.AutoSize = true;
            this.cbxLockScale.Location = new System.Drawing.Point(5, 106);
            this.cbxLockScale.Name = "cbxLockScale";
            this.cbxLockScale.Size = new System.Drawing.Size(74, 17);
            this.cbxLockScale.TabIndex = 3;
            this.cbxLockScale.Text = "lock scale";
            this.cbxLockScale.UseVisualStyleBackColor = true;
            this.cbxLockScale.CheckedChanged += new System.EventHandler(this.cbxLockScale_CheckedChanged);
            // 
            // tbxTickVal
            // 
            this.tbxTickVal.BackColor = System.Drawing.SystemColors.Control;
            this.tbxTickVal.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbxTickVal.Location = new System.Drawing.Point(585, 107);
            this.tbxTickVal.Name = "tbxTickVal";
            this.tbxTickVal.Size = new System.Drawing.Size(30, 13);
            this.tbxTickVal.TabIndex = 2;
            this.tbxTickVal.Text = "50";
            this.tbxTickVal.TextChanged += new System.EventHandler(this.tbxTickVal_TextChanged);
            // 
            // lblValueVal
            // 
            this.lblValueVal.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblValueVal.Location = new System.Drawing.Point(585, 47);
            this.lblValueVal.Name = "lblValueVal";
            this.lblValueVal.Size = new System.Drawing.Size(40, 15);
            this.lblValueVal.TabIndex = 0;
            this.lblValueVal.Text = "0.5";
            this.lblValueVal.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblTick
            // 
            this.lblTick.Location = new System.Drawing.Point(485, 107);
            this.lblTick.Name = "lblTick";
            this.lblTick.Size = new System.Drawing.Size(100, 15);
            this.lblTick.TabIndex = 0;
            this.lblTick.Text = "redraw rate (ms):";
            this.lblTick.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // tbMinValue
            // 
            this.tbMinValue.BackColor = System.Drawing.SystemColors.Control;
            this.tbMinValue.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbMinValue.Location = new System.Drawing.Point(590, 90);
            this.tbMinValue.Name = "tbMinValue";
            this.tbMinValue.Size = new System.Drawing.Size(35, 13);
            this.tbMinValue.TabIndex = 15;
            this.tbMinValue.Text = "0.0";
            this.tbMinValue.TextChanged += new System.EventHandler(this.tbMinValue_TextChanged);
            // 
            // tbMaxValue
            // 
            this.tbMaxValue.BackColor = System.Drawing.SystemColors.Control;
            this.tbMaxValue.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbMaxValue.Location = new System.Drawing.Point(590, 5);
            this.tbMaxValue.Name = "tbMaxValue";
            this.tbMaxValue.Size = new System.Drawing.Size(35, 13);
            this.tbMaxValue.TabIndex = 15;
            this.tbMaxValue.Text = "1.0";
            this.tbMaxValue.TextChanged += new System.EventHandler(this.tbMaxValue_TextChanged);
            // 
            // graph
            // 
            this.graph.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.graph.Colour = System.Drawing.Color.Black;
            this.graph.LineColour = System.Drawing.Color.White;
            this.graph.Location = new System.Drawing.Point(5, 5);
            this.graph.Max = 0.5F;
            this.graph.Min = 0.5F;
            this.graph.Name = "graph";
            this.graph.SelectColour = System.Drawing.Color.White;
            this.graph.Size = new System.Drawing.Size(580, 100);
            this.graph.TabIndex = 14;
            this.graph.Value = 0.5F;
            this.graph.ValueChanged += new System.EventHandler(this.graph_ValueChanged);
            // 
            // lblFrame
            // 
            this.lblFrame.Location = new System.Drawing.Point(205, 107);
            this.lblFrame.Name = "lblFrame";
            this.lblFrame.Size = new System.Drawing.Size(50, 15);
            this.lblFrame.TabIndex = 16;
            this.lblFrame.Text = "frame:";
            this.lblFrame.TextAlign = System.Drawing.ContentAlignment.TopRight;
            // 
            // lblFrameVal
            // 
            this.lblFrameVal.Location = new System.Drawing.Point(255, 107);
            this.lblFrameVal.Name = "lblFrameVal";
            this.lblFrameVal.Size = new System.Drawing.Size(50, 15);
            this.lblFrameVal.TabIndex = 16;
            this.lblFrameVal.Text = "0";
            // 
            // GraphProps
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.Controls.Add(this.lblFrameVal);
            this.Controls.Add(this.lblFrame);
            this.Controls.Add(this.tbMaxValue);
            this.Controls.Add(this.tbMinValue);
            this.Controls.Add(this.cbxPause);
            this.Controls.Add(this.graph);
            this.Controls.Add(this.cbxLockScale);
            this.Controls.Add(this.tbxTickVal);
            this.Controls.Add(this.lblValueVal);
            this.Controls.Add(this.lblTick);
            this.Name = "GraphProps";
            this.Size = new System.Drawing.Size(630, 123);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        #region Public Properties

        [Category("Behaviour"), DefaultValue(0.0), Description("The default line colour")]
        public float Value
        {
            set { graph.Value = value; }
        }

        public UInt32 Frame
        {
            set { graph.Frame = value; }
        }

        public override string Text
        {
            get { return graph.Text; }
            set { graph.Text = value; }
        }

        public float Max
        {
            set { tbMaxValue.Text = Convert.ToString(value); }
        }

        public float Min
        {
            set { tbMinValue.Text = Convert.ToString(value); }
        }

        public bool LockScale
        {
            set { cbxLockScale.Checked = value; }
        }

        #endregion

        private void graph_ValueChanged(object sender, System.EventArgs e)
        {
            if (!cbxLockScale.Checked)
            {
                tbMaxValue.Text = string.Format("{0:0.00}", graph.Max);
                tbMinValue.Text = string.Format("{0:0.00}", graph.Min);
            }

            lblValueVal.Text = string.Format("{0:0.00}", graph.Value);

            lblFrameVal.Text = string.Format("{0:0}", graph.Frame);
        }

        private void tbxTickVal_TextChanged(object sender, EventArgs e)
        {
            try
            {
                graph.Interval = (Convert.ToInt32(tbxTickVal.Text));
            }
            catch
            {
            }
        }

        private void cbxPause_CheckedChanged(object sender, EventArgs e)
        {
            if (cbxPause.Checked)
                graph.Active = false;
            else
                graph.Active = true;
        }

        private void cbxLockScale_CheckedChanged(object sender, EventArgs e)
        {
            if (cbxLockScale.Checked)
                graph.AutoScaleY = false;
            else
                graph.AutoScaleY = true;
        }

        private void tbMaxValue_TextChanged(object sender, EventArgs e)
        {
            try
            {
                if (!graph.HasSelection)
                    graph.Max = Convert.ToSingle(tbMaxValue.Text);
            }
            catch
            {
            }
        }

        private void tbMinValue_TextChanged(object sender, EventArgs e)
        {
            try
            {
                if (!graph.HasSelection)
                    graph.Min = Convert.ToSingle(tbMinValue.Text);
            }
            catch
            {
            }
        }
    }
}
