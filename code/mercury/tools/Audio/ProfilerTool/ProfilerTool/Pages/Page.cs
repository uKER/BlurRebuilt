using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace ProfilerTool.Pages
{
    public class PageWindow : Form
    {
        Page _page;

        public PageWindow(Pages.Page page)
        {
            _page = page;

            Name = page.Name;
            Controls.Add(page);
            page.Location = new Point(0, 0);
            ClientSize = new Size(page.Size.Width, page.Size.Height);
            FormBorderStyle = FormBorderStyle.SizableToolWindow;

            Resize += new EventHandler(This_Resize);
        }

        public Page Page
        {
            get { return _page; }
            set { _page = value; }
        }

        private void This_Resize(object sender, System.EventArgs e)
        {
            _page.Size = _page.Window.ClientSize;

            _page.AlignControls();
        }
    }

    public class Page : Panel
    {
        protected Windows.InstanceWnd _instWnd;
        bool _minimized = true;

        //  Prefs
        protected Point _topleft;
        protected Point _border;
        protected Point _spacing;
        protected Size _size;
        protected int _numColumns;
        protected int _height;
        protected bool _windowed = false;
        protected PageWindow _window;

        protected List<Control> _controlItems;

        public Windows.InstanceWnd InstanceWindow
        {
            get { return _instWnd; }
            set { _instWnd = value; }
        }

        public Point Border
        {
            get { return _border; }
            set { _border = value; }
        }

        public Point Spacing
        {
            get { return _spacing; }
            set { _spacing = value; }
        }

        public Size ItemSize
        {
            get { return _size; }
            set { _size = value; }
        }

        public int PageHeight
        {
            get { return _height; }
        }

        public PageWindow Window
        {
            get { return _window; }
            set { _window = value; }
        }

        public bool Windowed
        {
            get { return _windowed; }
            set { _windowed = value; }
        }

        public bool Minimized
        {
            get { return _minimized; }
            set { _minimized = value; }
        }

        public Page(Windows.InstanceWnd instView)
        {
            //  init member vars
            _instWnd = instView;

            _controlItems = new List<Control>();

            //  prefs
            Border = new Point(25, 5);
            Spacing = new Point(5, 5);
            _size = new Size(565, 100);
            _numColumns = 10;

            SuspendLayout();

            //  style
            Dock = DockStyle.Fill;

            ResumeLayout(false);
        }

        public virtual Control CreateControl(string name, Controls.ComponentTreeNode comNode)
        {
            MessageBox.Show("CreateControl called on Page base class", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

            return null;
        }

        protected Point ItemPosition(int index)
        {
            _numColumns = (Size.Width - 2 * Border.X) / (_size.Width + _spacing.X);

            //  must be at least one col, even if no space
            if (_numColumns == 0)
                _numColumns = 1;

            int row = index / _numColumns;
            int col = index % _numColumns;

            int x = _topleft.X + _border.X + col * (_size.Width + _spacing.X);
            int y = _topleft.Y + _border.Y + row * (_size.Height + _spacing.Y);

            return new Point(x, y);
        }

        public virtual void AlignControls()
        {
            int i = 0;
            foreach (Control ctrl in _controlItems)
            {
                if (ctrl.Visible)
                {
                    ctrl.Location = ItemPosition(i);
                    i++;
                }
            }

            int numActive = 0;
            foreach (Control c in _controlItems)
            {
                if (c.Visible)
                    numActive++;
            }

            //if (numActive == 0 || _minimized)
            if (numActive == 0)
                _height = 0;
            else
            {
                _height = _topleft.Y + (2 * _border.Y) + ((numActive + _numColumns - 1) / _numColumns) * (_size.Height + _spacing.Y);
            }

            Height = _height;
        }

        public virtual void AddControl(Controls.ComponentTreeNode comNode)
        {
            string text;
            if (comNode.ComponentOutput == "")
                text = comNode.ComponentName;
            else
                text = string.Format("{0:s}.{1:s}", comNode.ComponentName, comNode.ComponentOutput);

            Control ctrl = CreateControl(text, comNode);

            ToolTip toolTip1 = new ToolTip();

            // Set up the delays for the ToolTip.
            toolTip1.AutoPopDelay = 5000;
            toolTip1.InitialDelay = 1000;
            toolTip1.ReshowDelay = 500;
            // Force the ToolTip text to be displayed whether or not the form is active.
            toolTip1.ShowAlways = true;

            // Set up the ToolTip text for the Button and Checkbox.
            toolTip1.SetToolTip(ctrl, text);

            if (ctrl == null)
                throw new Exception("Page did not create a control - using base Page class?");

            ctrl.Size = ItemSize;
            ctrl.Hide();

            _controlItems.Add(ctrl);

            Controls.Add(ctrl);
        }

        protected void Clear()
        {
            foreach (Control ctrl in _controlItems)
                ctrl.Dispose();

            Controls.Clear();
        }

        public void Activate(string name)
        {
            foreach (Control ctrl in _controlItems)
            {
                if (ctrl.Text == name)
                    ctrl.Show();
            }

            AlignControls();
        }

        public void Deactivate(string name)
        {
            foreach (Control ctrl in _controlItems)
            {
                if (ctrl.Text == name)
                    ctrl.Hide();
            }

            AlignControls();
        }

        public void Maximize()
        {
            _minimized = false;

            AlignControls();
        }

        public void Minimize()
        {
            _minimized = true;

            AlignControls();
        }
    }
}
