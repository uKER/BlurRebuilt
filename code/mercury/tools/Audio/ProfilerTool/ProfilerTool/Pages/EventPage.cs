using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace ProfilerTool.Pages
{
    public class EventPage : Page
    {
        private Controls.Log _log;

        public EventPage(Windows.InstanceWnd instWnd, string strItem)
            : base(instWnd)
        {
            //  prefs
            ItemSize = new Size(630, 320);

            _log = new Controls.Log(strItem);

            Controls.Add(_log);

            _height = _topleft.Y + (2 * Border.Y) + ItemSize.Height;

            AlignControls();
        }

        public override void AlignControls()
        {
            _log.Location = new Point(_topleft.X + Border.X, _topleft.Y + Border.Y);
            _log.Width = Width - 2 * Border.X;

            if (Windowed)
                _log.Height = _window.ClientSize.Height - (2 * Border.Y);
            else
                _log.Height = ItemSize.Height;

            if (Minimized)
                _height = 0;
            else
                _height = _topleft.Y + (2 * Border.Y) + ItemSize.Height;
        }

        public void LogMessage(UInt32 frame, string msg, string par1, string par2, Color textCol, Color backCol)
        {
            _log.AddLine(frame, msg, par1, par2, textCol, backCol);
        }
    }
}
