using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    public class Graph : UserControl
    {
        private Timer _clock;

        private float _minValue = 0.0f;
        private float _maxValue = 1.0f;
        private float _currValue = 0.0f;
        private UInt32 _frame = 1;

        private float[] _currValues;
        private UInt32[] _currFrames;
        private int _dataWidth = 580;
        private int _currIndex = 0;
        private int _selStart = 479;
        private int _selWidth = 0;
        private bool _isSelecting = false;
        private float _minSelValue = 0.0f;
        private float _maxSelValue = 1.0f;

        private bool _isAutoScale = true;
        private bool _isActive = true;

        private System.Drawing.Color _backColour = Color.Black;
        private System.Drawing.Color _lineColour = Color.White;
        private System.Drawing.Color _labelColour = Color.White;
        private System.Drawing.Color _selectColor = Color.FromArgb(0x80, 0x20, 0x40, 0x60);

        private float smooth = 1.0f;

        #region Initialize
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        public Graph()
        {
            InitializeComponent();

            _clock = new Timer();
            _clock.Interval = 50;
            _clock.Start();
            _clock.Tick += new EventHandler(Timer_Tick);

            ResizeBuffer(_dataWidth + 1);

            Text = "unknown";

            //  double buffer to stop flicker
            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.UserPaint | ControlStyles.DoubleBuffer, true);
        }

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
        #endregion

        #region Component Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            components = new System.ComponentModel.Container();
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;

            this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.Graph_MouseMove);
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Graph_MouseDown);
            this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.Graph_MouseUp);
        }

        #endregion

        #region Public Properties
        [Category("Appearance"), DefaultValue(typeof(Color), "System.Drawing.Color.White"), Description("The default line colour")]
        public Color LineColour
        {
            get { return _lineColour; }
            set { _lineColour = value; }
        }

        [Category("Appearance"), DefaultValue(typeof(Color), "System.Drawing.Color.Black"), Description("The default background colour")]
        public Color Colour
        {
            get { return _backColour; }
            set { _backColour = value; }
        }

        [Category("Appearance"), DefaultValue(typeof(Color), "System.Drawing.Color.FromArgb(0x80, 0x20, 0x40, 0x60)"), Description("The default selection colour with alpha")]
        public Color SelectColour
        {
            get { return _selectColor; }
            set { _selectColor = value; }
        }

        [Category("Behavior"), DefaultValue(0.0f), Description("The current Y value of the graph")]
        public float Value
        {
            get
            {
                if (_selWidth == 0)
                    return _currValue;
                else
                    return _currValues[(_currIndex + _selStart + _selWidth) % _dataWidth];
            }
            set { _currValue = value; }
        }

        [Category("Behavior"), DefaultValue(0.0f), Description("The default min Y value of the graph")]
        public float Min
        {
            get
            {
                if (_selWidth == 0)
                    return _minValue;
                else
                    return _minSelValue;
            }
            set
            {
                if (float.IsNaN(value))
                {
                    Console.WriteLine("Trying to set Graph.Min to NaN");
                    _minValue = 0;
                }
                else
                    _minValue = value;
            }
        }

        [Category("Behavior"), DefaultValue(1.0f), Description("The default max Y value of the graph")]
        public float Max
        {
            get
            {
                if (_selWidth == 0)
                    return _maxValue;
                else
                    return _maxSelValue;
            }
            set
            {
                if (float.IsNaN(value))
                {
                    Console.WriteLine("Trying to set Graph.Min to NaN");
                    _maxValue = 0;
                }
                else
                    _maxValue = value;
            }
        }

        [Category("Behavior"), DefaultValue(true), Description("The active state of the graph")]
        public bool Active
        {
            get { return _isActive; }
            set { _isActive = value; }
        }

        [Category("Behavior"), DefaultValue(true), Description("The auto scale Y state of the graph")]
        public bool AutoScaleY
        {
            get { return _isAutoScale; }
            set { _isAutoScale = value; }
        }

        [Category("Behavior"), DefaultValue(50), Description("The default redraw interval in ms")]
        public int Interval
        {
            get { return _clock.Interval; }
            set
            {
                if (value < 20)
                    _clock.Interval = 20;
                else
                    _clock.Interval = value;
            }
        }

        public bool HasSelection
        {
            get { return _selWidth > 0; }
        }

        public UInt32 Frame
        {
            get
            {
                if (_selWidth == 0)
                    return _frame;
                else
                    return _currFrames[(_currIndex + _selStart + _selWidth) % _dataWidth];
            }
            set { _frame = value; }
        }
        #endregion

        #region Overridden events
        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);

            Invalidate();
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            //  clear background
            e.Graphics.Clear(_backColour);

            //  create pens
            Pen penLine = new Pen(_lineColour, 1);
            Pen penSel = new Pen(_selectColor, 1);

            float diff = (_maxValue - _minValue);

            //  if graph has no y range, force to 0.1
            if (diff == 0.0f)
                diff = 0.1f;

            //  1st point
            float y0 = (_currValues[_currIndex] - _minValue) / diff;
            Point p0 = new Point(0, Convert.ToInt32((1.0f - y0) * Convert.ToSingle(this.Height)));

            //  draw lines
            int curr = _currIndex;
            for (int i = 1; i < _dataWidth; i++)
            {
                float y = ((float)_currValues[curr] - _minValue) / diff;
                float x = Convert.ToSingle(i) / (_dataWidth - 1);

                Point p1 = new Point(Convert.ToInt32(x * Convert.ToSingle(this.Width)), Convert.ToInt32((1.0f - y) * Convert.ToSingle(this.Height)));

                e.Graphics.DrawLine(penLine, p0, p1);

                p0 = p1;

                if (++curr == _dataWidth)
                    curr = 0;
            }

            float xsel = Convert.ToSingle(_selStart) / Convert.ToSingle(_dataWidth);
            float width = Convert.ToSingle(_selWidth) / Convert.ToSingle(_dataWidth);

            //  draw cursor
            e.Graphics.FillRectangle(penSel.Brush, xsel * Convert.ToSingle(this.Width), 0, width * Convert.ToSingle(this.Width), this.Height);

            //  draw label
            System.Drawing.SolidBrush brush = new System.Drawing.SolidBrush(_labelColour);
            //Font font = new Font("Microsoft Sans Serif", 10);
            e.Graphics.DrawString(Text, Font, brush, 0, 0);
        }

        private void Graph_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                //  set flag for mouse drag
                _isSelecting = true;

                //  set selection start
                _selStart = Convert.ToInt32(Convert.ToSingle(_dataWidth) * Convert.ToSingle(e.X) / Convert.ToSingle(this.Width));
                _selWidth = 1;
            }
            else if (e.Button == MouseButtons.Right)
            {
                //  clear selection
                _isSelecting = false;

                _selStart = _dataWidth - 1;
                _selWidth = 0;
            }
        }

        private void Graph_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (_isSelecting)
            {
                //  get selection width
                _selWidth = Convert.ToInt32(Convert.ToSingle(_dataWidth) * Convert.ToSingle(e.X) / Convert.ToSingle(this.Width)) - _selStart;

                //  get new min/max values for selection
                if (_selWidth > 0)
                    UpdateSelection();
            }
        }

        private void Graph_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            //  release selection
            _isSelecting = false;
        }
        #endregion

        #region Public Events
        [Category("Action"), Description("Occurs when the graph updates")]
        public event EventHandler ValueChanged;
        #endregion

        public void Timer_Tick(object sender, EventArgs eArgs)
        {
            Timer timer = sender as Timer;

            if (Visible)
            {
                if (sender == _clock)
                {
                    if (_isActive)
                    {
                        int prevIndex = _currIndex - 1;
                        if (prevIndex < 0)
                            prevIndex = _dataWidth - 1;

                        float prevValue = _currValues[prevIndex];

                        //  update newest value
                        //_currValues[_currIndex] = _currValue;
                        _currValues[_currIndex] = smooth * _currValue + (1.0f - smooth) * prevValue;
                        _currFrames[_currIndex] = _frame;

                        //  increment window view cursor
                        _currIndex++;

                        if (_currIndex == _dataWidth)
                            _currIndex = 0;
                    }

                    bool update = false;

                    //  get new min/max values for window
                    if (_isActive)
                    {
                        if (_isAutoScale)
                        {
                            _maxValue = _currValues[0];
                            _minValue = _currValues[0];

                            for (int i = 1; i < _dataWidth; i++)
                            {
                                _maxValue = Math.Max(_maxValue, _currValues[i]);
                                _minValue = Math.Min(_minValue, _currValues[i]);
                            }
                        }

                        update = true;
                    }

                    if (_isSelecting)
                        update = true;

                    if (_selWidth > 0)
                        UpdateSelection();

                    if (update)
                    {
                        Invalidate();

                        // Raise the ValueChanged event
                        if (ValueChanged != null)
                            ValueChanged(this, new EventArgs());
                    }
                }
            }
        }

        public void UpdateSelection()
        {
            //  update selection values
            _maxSelValue = -1000000.0f;
            _minSelValue = 1000000.0f;
            int index = (_currIndex + _selStart) % _dataWidth;
            for (int i = 0; i < _selWidth; i++)
            {
                _maxSelValue = Math.Max(_maxSelValue, _currValues[index]);
                _minSelValue = Math.Min(_minSelValue, _currValues[index]);

                if (++index == _dataWidth)
                    index = 0;
            }
        }

        public void ResizeBuffer(int dataWidth)
        {
            if (dataWidth < 2)
                _dataWidth = 2;
            else
                _dataWidth = dataWidth;

            _currValues = new float[_dataWidth];
            _currFrames = new UInt32[_dataWidth];

            _minValue = 0.0f;
            _maxValue = 1.0f;
            _currValue = 0.5f;

            _currIndex = 0;
            _selStart = _dataWidth - 1;
            _selWidth = 0;
            _isSelecting = false;
        }
    }
}