using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    public class Fader : UserControl
    {
        private float upper;
        private float lower;
        private TextBox tbxUpper;
        private TextBox tbxLower;
        private CheckBox cbxOverride;
        private float _prevUpper = 0.0f;
        private float _prevLower = 0.0f;
        private float _prevValue = 0.0f;

        public Fader()
        {
            InitializeComponent();

            upper = 1.0f;
            lower = 0.0f;
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
            this.avFader1 = new Faders.MixFader();
            this.tbxValue = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.tbxUpper = new System.Windows.Forms.TextBox();
            this.tbxLower = new System.Windows.Forms.TextBox();
            this.cbxOverride = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // avFader1
            // 
            this.avFader1.Disabled = false;
            this.avFader1.Location = new System.Drawing.Point(0, 15);
            this.avFader1.Maximum = 1000;
            this.avFader1.Name = "avFader1";
            this.avFader1.Size = new System.Drawing.Size(50, 125);
            this.avFader1.TabIndex = 12;
            this.avFader1.TickColor = System.Drawing.Color.Black;
            this.avFader1.TickFrequency = 100;
            this.avFader1.Value = 40;
            this.avFader1.ValueChanged += new System.EventHandler(this.avFader1_ValueChanged);
            // 
            // tbxValue
            // 
            this.tbxValue.BackColor = System.Drawing.SystemColors.Control;
            this.tbxValue.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbxValue.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbxValue.Location = new System.Drawing.Point(50, 70);
            this.tbxValue.Name = "tbxValue";
            this.tbxValue.Size = new System.Drawing.Size(40, 13);
            this.tbxValue.TabIndex = 13;
            this.tbxValue.Text = "0.0";
            this.tbxValue.GotFocus += new System.EventHandler(this.tbxValue_GotFocus);
            this.tbxValue.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.tbxValue_CheckKeys);
            this.tbxValue.LostFocus += new System.EventHandler(this.tbxValue_LostFocus);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(0, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(80, 15);
            this.label1.TabIndex = 14;
            this.label1.Text = "name";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // tbxUpper
            // 
            this.tbxUpper.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.tbxUpper.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbxUpper.Location = new System.Drawing.Point(50, 15);
            this.tbxUpper.Name = "tbxUpper";
            this.tbxUpper.Size = new System.Drawing.Size(40, 13);
            this.tbxUpper.TabIndex = 13;
            this.tbxUpper.Text = "0.0";
            this.tbxUpper.GotFocus += new System.EventHandler(this.tbxUpper_GotFocus);
            this.tbxUpper.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.tbxUpper_CheckKeys);
            this.tbxUpper.LostFocus += new System.EventHandler(this.tbxUpper_LostFocus);
            // 
            // tbxLower
            // 
            this.tbxLower.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.tbxLower.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tbxLower.Location = new System.Drawing.Point(50, 125);
            this.tbxLower.Name = "tbxLower";
            this.tbxLower.Size = new System.Drawing.Size(40, 13);
            this.tbxLower.TabIndex = 13;
            this.tbxLower.Text = "0.0";
            this.tbxLower.GotFocus += new System.EventHandler(this.tbxLower_GotFocus);
            this.tbxLower.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.tbxLower_CheckKeys);
            this.tbxLower.LostFocus += new System.EventHandler(this.tbxLower_LostFocus);
            // 
            // cbxOverride
            // 
            this.cbxOverride.AutoSize = true;
            this.cbxOverride.Checked = true;
            this.cbxOverride.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbxOverride.Location = new System.Drawing.Point(77, 3);
            this.cbxOverride.Name = "cbxOverride";
            this.cbxOverride.Size = new System.Drawing.Size(15, 14);
            this.cbxOverride.TabIndex = 15;
            this.cbxOverride.UseVisualStyleBackColor = true;
            this.cbxOverride.CheckedChanged += new System.EventHandler(this.cbxOverride_Changed);
            // 
            // Fader
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.Controls.Add(this.cbxOverride);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.tbxLower);
            this.Controls.Add(this.tbxUpper);
            this.Controls.Add(this.tbxValue);
            this.Controls.Add(this.avFader1);
            this.Name = "Fader";
            this.Size = new System.Drawing.Size(95, 145);
            this.Load += new System.EventHandler(this.Slider_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private Faders.MixFader avFader1;
        private System.Windows.Forms.TextBox tbxValue;
        private System.Windows.Forms.Label label1;
        public override string Text
        {
            get { return label1.Text; }
            set { label1.Text = value; }
        }

        public bool Override
        {
            get
            {
                return cbxOverride.Checked;
            }
            set
            {
                cbxOverride.Checked = value;
            }
        }

        public float Value
        {
            get
            {
                //return avFader1.Value / (upper - lower);
                return lower + ((float)avFader1.Value / (float)avFader1.Maximum) * (upper - lower);
            }
            set
            {
                //  adjust scale
                if (value > upper)
                {
                    upper = value;
                    tbxUpper.Text = string.Format("{0:0.00}", upper);
                }
                if (value < lower)
                {
                    lower = value;
                    tbxLower.Text = string.Format("{0:0.00}", lower);
                }
                float val = (value - lower) / (upper - lower);
                val *= (float)avFader1.Maximum;
                //  clamp
                //val = Math.Max(val, avFader1.Minimum);
                //val = Math.Min(val, avFader1.Maximum);
                avFader1.Value = Convert.ToInt32(val);
                tbxValue.Text = string.Format("{0:0.00}", lower + ((float)avFader1.Value / (float)avFader1.Maximum) * (upper - lower));
            }
        }

        public event EventHandler ValueChanged;
        public event EventHandler OverrideChanged;

        private void Slider_Load(object sender, System.EventArgs e)
        {
            UpdateText();
        }

        private void avFader1_ValueChanged(object sender, System.EventArgs e)
        {
            UpdateText();

            if (ValueChanged != null)
                ValueChanged(this, e);
        }

        private void tbxValue_GotFocus(object sender, EventArgs e)
        {
            _prevValue = Convert.ToSingle(tbxValue.Text);
        }
        private void tbxValue_LostFocus(object sender, EventArgs e)
        {
            try
            {
                float val = (Convert.ToSingle(tbxValue.Text) - lower) / (upper - lower);
                val *= (float)avFader1.Maximum;
                val = Math.Max(val, avFader1.Minimum);
                val = Math.Min(val, avFader1.Maximum);
                avFader1.Value = Convert.ToInt32(val);

                tbxValue.Text = string.Format("{0:0.00}", lower + ((float)this.avFader1.Value / (float)this.avFader1.Maximum) * (upper - lower));

                if (ValueChanged != null)
                    ValueChanged(this, e);
            }
            catch // non numeric entries
            {
                tbxValue.Text = string.Format("{0:0.00}", _prevValue);
            }
        }
        private void tbxValue_CheckKeys(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            if (e.KeyChar == (char)13)
            {
                try
                {
                    float valCurr = (Convert.ToSingle(tbxValue.Text) - lower) / (upper - lower);
                    valCurr *= (float)avFader1.Maximum;
                    valCurr = Math.Max(valCurr, avFader1.Minimum);
                    valCurr = Math.Min(valCurr, avFader1.Maximum);
                    avFader1.Value = Convert.ToInt32(valCurr);

                    tbxValue.Text = string.Format("{0:0.00}", lower + ((float)this.avFader1.Value / (float)this.avFader1.Maximum) * (upper - lower));

                    if (ValueChanged != null)
                        ValueChanged(this, e);

                    _prevValue = valCurr;
                }
                catch // non numeric entries
                {
                    tbxValue.Text = string.Format("{0:0.00}", _prevValue);
                }
            }
        }

        private void tbxUpper_GotFocus(object sender, EventArgs e)
        {
            _prevUpper = Convert.ToSingle(tbxUpper.Text);
        }
        private void tbxUpper_LostFocus(object sender, EventArgs e)
        {
            try
            {
                float val = Convert.ToSingle(tbxUpper.Text);
                if (val > lower)
                    upper = val;

                float valCurr = (Convert.ToSingle(tbxValue.Text) - lower) / (upper - lower);
                valCurr *= (float)avFader1.Maximum;
                valCurr = Math.Max(valCurr, avFader1.Minimum);
                valCurr = Math.Min(valCurr, avFader1.Maximum);
                avFader1.Value = Convert.ToInt32(valCurr);

                tbxUpper.Text = string.Format("{0:0.00}", upper);

                if (ValueChanged != null)
                    ValueChanged(this, e);
            }
            catch // non numeric entries
            {
                tbxUpper.Text = string.Format("{0:0.00}", _prevUpper);
            }
        }
        private void tbxUpper_CheckKeys(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            if (e.KeyChar == (char)13)
            {
                try
                {
                    float val = Convert.ToSingle(tbxUpper.Text);
                    if (val > lower)
                        upper = val;

                    float valCurr = (Convert.ToSingle(tbxValue.Text) - lower) / (upper - lower);
                    valCurr *= (float)avFader1.Maximum;
                    valCurr = Math.Max(valCurr, avFader1.Minimum);
                    valCurr = Math.Min(valCurr, avFader1.Maximum);
                    avFader1.Value = Convert.ToInt32(valCurr);

                    tbxUpper.Text = string.Format("{0:0.00}", upper);

                    if (ValueChanged != null)
                        ValueChanged(this, e);

                    _prevUpper = upper;
                }
                catch // non numeric entries
                {
                    tbxUpper.Text = string.Format("{0:0.00}", _prevUpper);
                }
            }
        }

        private void tbxLower_GotFocus(object sender, EventArgs e)
        {
            _prevLower = Convert.ToSingle(tbxLower.Text);
        }
        private void tbxLower_LostFocus(object sender, EventArgs e)
        {
            try
            {
                float val = Convert.ToSingle(tbxLower.Text);
                if (val < upper)
                    lower = val;

                float valCurr = (Convert.ToSingle(tbxValue.Text) - lower) / (upper - lower);
                valCurr *= (float)avFader1.Maximum;
                valCurr = Math.Max(valCurr, avFader1.Minimum);
                valCurr = Math.Min(valCurr, avFader1.Maximum);
                avFader1.Value = Convert.ToInt32(valCurr);

                tbxLower.Text = string.Format("{0:0.00}", lower);

                if (ValueChanged != null)
                    ValueChanged(this, e);
            }
            catch // non numeric entries
            {
                tbxLower.Text = string.Format("{0:0.00}", _prevLower);
            }
        }
        private void tbxLower_CheckKeys(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            if (e.KeyChar == (char)13)
            {
                try
                {
                    float val = Convert.ToSingle(tbxLower.Text);
                    if (val < upper)
                        lower = val;

                    float valCurr = (Convert.ToSingle(tbxValue.Text) - lower) / (upper - lower);
                    valCurr *= (float)avFader1.Maximum;
                    valCurr = Math.Max(valCurr, avFader1.Minimum);
                    valCurr = Math.Min(valCurr, avFader1.Maximum);
                    avFader1.Value = Convert.ToInt32(valCurr);

                    tbxLower.Text = string.Format("{0:0.00}", lower);

                    if (ValueChanged != null)
                        ValueChanged(this, e);

                    _prevLower = lower;
                }
                catch // non numeric entries
                {
                    tbxLower.Text = string.Format("{0:0.00}", _prevLower);
                }
            }
        }

        private void cbxOverride_Changed(object sender, EventArgs e)
        {
            avFader1.Disabled = !cbxOverride.Checked;

            if (OverrideChanged != null)
                OverrideChanged(this, e);
        }

        private void UpdateText()
        {
            tbxValue.Text = string.Format("{0:0.00}", lower + ((double)avFader1.Value / (double)avFader1.Maximum) * (upper - lower));
            tbxUpper.Text = string.Format("{0:0.00}", upper);
            tbxLower.Text = string.Format("{0:0.00}", lower);
        }
    }
}
