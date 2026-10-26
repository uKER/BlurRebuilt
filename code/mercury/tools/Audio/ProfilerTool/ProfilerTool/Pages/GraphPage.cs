using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace ProfilerTool.Pages
{
    class GraphPage : ProfilerTool.Pages.Page
    {
        private CheckBox _cbPauseAll;

        public GraphPage(Windows.InstanceWnd instWnd)
            : base(instWnd)
        {
            //  prefs
            ItemSize = new Size(630, 125);

            _cbPauseAll = new CheckBox();
            _cbPauseAll.Location = new Point(Border.X, 0);
            _cbPauseAll.Text = "pause all";

            _cbPauseAll.CheckedChanged += new EventHandler(PauseAll_CheckedChanged);

            Controls.Add(_cbPauseAll);

            _topleft.Y = _cbPauseAll.Height;
        }

        private void PauseAll_CheckedChanged(object sender, System.EventArgs e)
        {
            foreach (Control ctrl in _controlItems)
            {
                Controls.GraphProps graph = ctrl as Controls.GraphProps;

                if (graph != null)
                    graph.graph.Active = !_cbPauseAll.Checked;
            }
        }

        public override Control CreateControl(string name, Controls.ComponentTreeNode comNode)
        {
            Controls.GraphProps graph1 = new Controls.GraphProps();

            graph1.graph.Colour = System.Drawing.Color.MidnightBlue;
            graph1.graph.SelectColour = System.Drawing.Color.FromArgb(0x80, 0x40, 0x80, 0xc0);
            graph1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            graph1.graph.LineColour = System.Drawing.Color.White;
            graph1.Location = new System.Drawing.Point(0, 0);
            graph1.Text = name;
            graph1.Size = new System.Drawing.Size(200, 100);
            graph1.TabIndex = 1;
            graph1.Value = 0.5F;

            //  hack to draw cpu counters 0-100, no autoscale y
            if (name.Contains("CPU."))
            {
                graph1.LockScale = true;
                graph1.Max = 100.0f;
                graph1.Min = 0.0f;
            }

            return graph1 as Control;
        }

        public void SetValue(string name, UInt32 frame, float value)
        {
            foreach (Control ctrl in _controlItems)
            {
                ProfilerTool.Controls.GraphProps graph = ctrl as ProfilerTool.Controls.GraphProps;

                if (graph != null && graph.Text == name)
                {
                    graph.Value = value;
                    graph.Frame = frame;
                    //graph.Tick();
                    break;
                }
            }
        }
    }
}
