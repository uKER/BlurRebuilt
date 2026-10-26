using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    public partial class Vector : UserControl
    {
        float _prevX = 0.0f;
        float _prevY = 0.0f;
        float _prevZ = 0.0f;

        public Vector()
        {
            InitializeComponent();
        }

        public override string Text
        {
            get { return label1.Text; }
            set { label1.Text = value; }
        }

        private void tbxX_GotFocus(object sender, EventArgs e)
        {
            _prevX = Convert.ToSingle(tbxX.Text);
        }
        private void tbxX_LostFocus(object sender, EventArgs e)
        {
            try
            {
                tbxX.Text = string.Format("{0:0.00}", Convert.ToSingle(tbxX.Text));
            }
            catch // non numeric entries
            {
                tbxX.Text = string.Format("{0:0.00}", _prevX);
            }
        }
        private void tbxX_CheckKeys(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            if (e.KeyChar == (char)13)
            {
                try
                {
                    float val = Convert.ToSingle(tbxX.Text);
                    tbxX.Text = string.Format("{0:0.00}", val);
                    _prevX = val;
                }
                catch // non numeric entries
                {
                    tbxX.Text = string.Format("{0:0.00}", _prevX);
                }
            }
        }

        private void tbxY_GotFocus(object sender, EventArgs e)
        {
            _prevY = Convert.ToSingle(tbxY.Text);
        }
        private void tbxY_LostFocus(object sender, EventArgs e)
        {
            try
            {
                tbxY.Text = string.Format("{0:0.00}", Convert.ToSingle(tbxY.Text));
            }
            catch // non numeric entries
            {
                tbxY.Text = string.Format("{0:0.00}", _prevY);
            }
        }
        private void tbxY_CheckKeys(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            if (e.KeyChar == (char)13)
            {
                try
                {
                    float val = Convert.ToSingle(tbxY.Text);
                    tbxY.Text = string.Format("{0:0.00}", val);
                    _prevY = val;
                }
                catch // non numeric entries
                {
                    tbxY.Text = string.Format("{0:0.00}", _prevY);
                }
            }
        }

        private void tbxZ_GotFocus(object sender, EventArgs e)
        {
            _prevZ = Convert.ToSingle(tbxZ.Text);
        }
        private void tbxZ_LostFocus(object sender, EventArgs e)
        {
            try
            {
                tbxZ.Text = string.Format("{0:0.00}", Convert.ToSingle(tbxZ.Text));
            }
            catch // non numeric entries
            {
                tbxZ.Text = string.Format("{0:0.00}", _prevZ);
            }
        }
        private void tbxZ_CheckKeys(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            if (e.KeyChar == (char)13)
            {
                try
                {
                    float val = Convert.ToSingle(tbxZ.Text);
                    tbxZ.Text = string.Format("{0:0.00}", val);
                    _prevZ = val;
                }
                catch // non numeric entries
                {
                    tbxZ.Text = string.Format("{0:0.00}", _prevZ);
                }
            }
        }
    }
}