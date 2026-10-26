using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace ProfilerTool.Pages
{
    class ParametersPage : Page
    {
        public ParametersPage(Windows.InstanceWnd instWnd)
            : base(instWnd)
        {
            //  prefs
            ItemSize = new Size(95, 145);
        }

        public override Control CreateControl(string name, Controls.ComponentTreeNode comNode)
        {
            Controls.Fader fader = new Controls.Fader();
            fader.Text = name;

            fader.ValueChanged += new EventHandler(fader_ValueChanged);
            fader.OverrideChanged += new EventHandler(fader_OverrideChanged);

            return fader;
        }

        public void SetValue(string name, float value)
        {
            foreach (Control ctrl in Controls)
            {
                Controls.Fader fader = ctrl as Controls.Fader;

                if (fader.Text == name)
                {
                    if (fader.Override)
                    {
                        //fader.Value = value;
                        break;
                    }
                    else
                    {
                        fader.Value = value;
                        break;
                    }
                }
            }
        }

        public bool GetValue(string name, ref float value)
        {
            foreach (Control ctrl in Controls)
            {
                Controls.Fader fader = ctrl as Controls.Fader;

                if (fader.Text == name)
                {
                    value = fader.Value;
                    return fader.Override;
                }
            }

            return false;
        }

        void fader_ValueChanged(object sender, EventArgs e)
        {
            Controls.Fader fader = sender as Controls.Fader;

            if (fader.Override)
                InstanceWindow.SendParameterValue(fader.Text, fader.Value, true);
            else // cheap trick to enable or disable game setting of parameters
                InstanceWindow.SendParameterValue(fader.Text, fader.Value, false);
        }

        void fader_OverrideChanged(object sender, EventArgs e)
        {
            Controls.Fader fader = sender as Controls.Fader;

            InstanceWindow.SendParameterValue(fader.Text, fader.Value, fader.Override);
        }
    }
}
