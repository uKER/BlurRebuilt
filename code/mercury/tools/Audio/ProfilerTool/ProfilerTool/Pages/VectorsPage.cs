using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace ProfilerTool.Pages
{
    class VectorsPage : Page
    {
        public VectorsPage(Windows.InstanceWnd instWnd)
            : base(instWnd)
        {
            //  prefs
            ItemSize = new Size(100, 80);
        }

        public override Control CreateControl(string name, Controls.ComponentTreeNode comNode)
        {
            Controls.Vector vec = new Controls.Vector();
            vec.Text = name;

            //vec.ValueChanged += new EventHandler(fader_ValueChanged);
            //vec.OverrideChanged += new EventHandler(fader_OverrideChanged);

            return vec;
        }

        public void SetValue(string name, float value)
        {
            //foreach (Control ctrl in Controls)
            //{
            //    Controls.Fader fader = ctrl as Controls.Fader;

            //    if (fader.Text == name)
            //    {
            //        if (fader.Override)
            //        {
            //            //fader.Value = value;
            //            break;
            //        }
            //        else
            //        {
            //            fader.Value = value;
            //            break;
            //        }
            //    }
            //}
        }

        void fader_ValueChanged(object sender, EventArgs e)
        {
            //Controls.Fader fader = sender as Controls.Fader;

            //if (fader.Override)
            //    InstanceWindow.SendParameterValue(fader.Text, fader.Value, true);
            //else // cheap trick to enable or disable game setting of parameters
            //    InstanceWindow.SendParameterValue(fader.Text, fader.Value, false);
        }

        void fader_OverrideChanged(object sender, EventArgs e)
        {
            //Controls.Fader fader = sender as Controls.Fader;

            //InstanceWindow.SendParameterValue(fader.Text, fader.Value, fader.Override);
        }
    }
}
