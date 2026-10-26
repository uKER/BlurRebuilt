using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace ProfilerTool.Pages
{
    class ActionsPage : Page
    {
        public ActionsPage(Windows.InstanceWnd instWnd)
            : base(instWnd)
        {
            //  prefs
            ItemSize = new Size(125, 25);
        }

        public override Control CreateControl(string name, Controls.ComponentTreeNode comNode)
        {
            Button button = new Button();
            button.FlatStyle = FlatStyle.Flat;
            button.Name = name;
            button.Text = name;
            button.Size = ItemSize;

            button.Click += new EventHandler(button_Click);

            return button;
        }

        void button_Click(object sender, EventArgs e)
        {
            Button button = sender as Button;

            InstanceWindow.SendActionTrigger(button.Text);
        }
    }
}
