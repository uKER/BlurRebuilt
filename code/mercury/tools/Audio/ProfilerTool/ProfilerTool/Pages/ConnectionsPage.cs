using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace ProfilerTool.Pages
{
    class ConnectionsPage : Page
    {
        public ConnectionsPage(Windows.InstanceWnd instWnd)
            : base(instWnd)
        {
            //  prefs
            ItemSize = new Size(265, 45);
            Spacing = new Point(25, 5);
        }

        public override Control CreateControl(string name, Controls.ComponentTreeNode comNode)
        {
            Controls.Connection connection = new Controls.Connection();
            connection.Text = name;
            //Button button = new Button();
            //button.FlatStyle = FlatStyle.Flat;
            //button.Name = name;
            //button.Text = name;
            //button.Size = ItemSize;

            //button.Click += new EventHandler(button_Click);

            return connection;
        }

        public void SetInfo(string thisInst, string thisComType, string thisComName, string thatInst, string thatComType, string thatComName)
        {
            foreach (Control ctrl in Controls)
            {
                Controls.Connection con = ctrl as Controls.Connection;

                if (con.Text == thisComName)
                {
                    con.LeftObject = thisInst;
                    con.LeftComponentType = thisComType;
                    con.LeftComponentName = thisComName;
                    con.RightObject = thatInst;
                    con.RightComponentType = thatComType;
                    con.RightComponentName = thatComName;
                    break;
                }
            }
        }
    }
}
