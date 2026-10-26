using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    public partial class XYControl : UserControl
    {
        private bool _disabled = false;
        private bool _dragModeEnabled = false;
        private Point _startDragPoint;
        private float _fX;
        private float _fY;

        public XYControl()
        {
            InitializeComponent();

            picStick.Left = (Width / 2) - (picStick.Width / 2);
            picStick.Top = (Height / 2) - (picStick.Height / 2);
        }

        public float X
        {
            get { return _fX; }
            set
            {
                if (value < 0)
                    _fX = 0;
                else if (value > 1)
                    _fX = 1;
                else
                    _fX = value;

                picStick.Left = (int)(_fX * (float)(Width - picStick.Width));
            }
        }

        public float Y
        {
            get { return _fY; }
            set
            {
                if (value < 0)
                    _fY = 0;
                else if (value > 1)
                    _fY = 1;
                else
                    _fY = value;

                picStick.Top = (int)(_fY * (float)(Height - picStick.Height));
            }
        }

        #region Public Events
        [Category("Action"), Description("Occurs when the stick is moved")]
        public event EventHandler ValueChanged;
        #endregion

        private void picStick_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (!_disabled)
            {
                // Set the drag flag for the mousemove event
                _dragModeEnabled = true;

                // Record the start point for the slider movement
                _startDragPoint = new Point(e.X, e.Y);
            }
        }

        private void picStick_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // User isn't dragging the slider, so dont bother moving it
            if (_dragModeEnabled == false)
                return;

            if (_disabled == true)
                return;

            // Calculate the distance the mouse moved
            int deltaX = e.X - _startDragPoint.X;
            int deltaY = e.Y - _startDragPoint.Y;

            if (deltaX == 0 && deltaY == 0)
                return;

            MoveStick(deltaX, deltaY);
        }

        private void picStick_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // Reset the drag flag
            _dragModeEnabled = false;
        }

        private void MoveStick(int deltaX, int deltaY)
        {
            // Move the stick and make sure it stays in the bounds of the control

            if (deltaX < 0 && (picStick.Left + deltaX) <= 0)
                picStick.Left = 0;
            else if (deltaX > 0 && (picStick.Left + picStick.Width + deltaX) >= Width)
                picStick.Left = Width - picStick.Width;
            else
                picStick.Left += deltaX;

            if (deltaY < 0 && (picStick.Top + deltaY) <= 0)
                picStick.Top = 0;
            else if (deltaY > 0 && (picStick.Top + picStick.Height + deltaY) >= Height)
                picStick.Top = Height - picStick.Height;
            else
                picStick.Top += deltaY;

            CalculateValues();
        }

        private void CalculateValues()
        {
            _fX = (float)picStick.Left / (float)(Width - picStick.Width);
            _fY = (float)picStick.Top / (float)(Height - picStick.Height);

            // Raise the ValueChanged event
            if (ValueChanged != null)
                ValueChanged(this, new EventArgs());
        }
    }
}
