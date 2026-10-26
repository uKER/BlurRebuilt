using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    public partial class Joystick : UserControl
    {
        float _fX;
        float _fY;
        float _fZ;
        float _fScale = 100.0f;

        public Joystick()
        {
            InitializeComponent();

            X = 0;
            Y = 0;
            Z = 0;
        }

        public float X
        {
            get { return _fX; }
            set
            {
                xyCtrl.X = 0.5f - 0.5f * (_fX / _fScale);
                _fX = value;
            }
        }

        public float Y
        {
            get { return _fY; }
            set
            {
                sldY.Value = (int)(50.0f + value * 50.0f);
                _fY = value;
            }
        }

        public float Z
        {
            get { return _fZ; }
            set
            {
                xyCtrl.Y = 0.5f - 0.5f * (_fZ / _fScale);
                _fZ = value;
            }
        }

        #region Public Events
        [Category("Action"), Description("Occurs when the stick is moved")]
        public event EventHandler ValueChanged;
        #endregion

        private void sldY_ValueChanged(object sender, System.EventArgs e)
        {
            _fY = _fScale * (-1.0f + 2.0f * (float)sldY.Value / 100.0f);

            UpdateDisplay();

            if (ValueChanged != null)
                ValueChanged(this, e);
        }

        private void xyCtrl_ValueChanged(object sender, System.EventArgs e)
        {
            _fX = _fScale * (-1.0f + 2.0f * xyCtrl.X);
            _fZ = _fScale * (1.0f - 2.0f * xyCtrl.Y);

            UpdateDisplay();

            if (ValueChanged != null)
                ValueChanged(this, e);
        }

        void UpdateDisplay()
        {
            lblCoords.Text = string.Format("{0:0.0}, {1:0.0}, {2:0.0}", _fX, _fY, _fZ );
        }

        private void tbxScale_TextChanged(object sender, EventArgs e)
        {
            try
            {
                _fScale = Convert.ToSingle(tbxScale.Text);

                if (ValueChanged != null)
                    ValueChanged(this, e);
            }
            catch // non numeric entries
            {
                tbxScale.Text = string.Format("{0:0}", 100.0f);
            }
        }
    }
}
