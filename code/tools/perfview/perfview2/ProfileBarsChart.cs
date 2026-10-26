using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;
using System.Drawing;
using System.ComponentModel;

namespace ProfilingApplication
{
    public class ProfileBarsChart : Panel
    {
        public ProfileBarsChart()
        {
            // Activates double buffering 
            SetStyle(ControlStyles.UserPaint, true);
            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            SetStyle(ControlStyles.DoubleBuffer, true);
            m_bCaptureDataSet = false;
            m_bColoursSet = false;
            m_fHorizontalScale = 1;
            m_iFocusCore = 0;

            m_iIndentation = 60;
            m_iBarHeight = 21;
            m_iBarSpacing = 24;
            m_iHeaderHeight = 20;
            m_Background = System.Drawing.Brushes.White;
            m_CoreSelection = System.Drawing.Pens.DarkBlue;
            m_Bars = new List<Brush>();

            int h = 255;
            int m = 180;
            int l = 130;

            for (int i=0; i<2; i++)
            {
                int a = (i==0)?170:255;

                // unknown
                m_Bars.Add(new LinearGradientBrush(new Point(0, m_iHeaderHeight + 11), new Point(0, m_iHeaderHeight + 11 + m_iBarSpacing),
                            System.Drawing.Color.FromArgb(a-60, 170, 170, 170), System.Drawing.Color.FromArgb(a-60, 170, 170, 170)));
                // blue
                m_Bars.Add(new LinearGradientBrush(new Point(0, m_iHeaderHeight + 11), new Point(0, m_iHeaderHeight + 11 + m_iBarSpacing),
                            System.Drawing.Color.FromArgb(a, m, m, h), System.Drawing.Color.FromArgb(a, 0, 0, l)));
                // green
                m_Bars.Add(new LinearGradientBrush(new Point(0, m_iHeaderHeight + 11), new Point(0, m_iHeaderHeight + 11 + m_iBarSpacing),
                            System.Drawing.Color.FromArgb(a, m, h, m), System.Drawing.Color.FromArgb(a, 0, l, 0)));
                // orange
                m_Bars.Add(new LinearGradientBrush(new Point(0, m_iHeaderHeight + 11), new Point(0, m_iHeaderHeight + 11 + m_iBarSpacing),
                            System.Drawing.Color.FromArgb(a, h, h, m), System.Drawing.Color.FromArgb(a, l, l, 0)));
                // red
                m_Bars.Add(new LinearGradientBrush(new Point(0, m_iHeaderHeight + 11), new Point(0, m_iHeaderHeight + 11 + m_iBarSpacing),
                            System.Drawing.Color.FromArgb(a, h, m, m), System.Drawing.Color.FromArgb(a, l, 0, 0)));
                // silver
                m_Bars.Add(new LinearGradientBrush(new Point(0, m_iHeaderHeight + 11), new Point(0, m_iHeaderHeight + 11 + m_iBarSpacing),
                            System.Drawing.Color.FromArgb(a, 230, 230, 230), System.Drawing.Color.FromArgb(a, 80, 80, 80)));
            }

            m_BarSelection = new SolidBrush(System.Drawing.Color.FromArgb(40, 0, 0, 255));

            m_HardGridLine = System.Drawing.Pens.LightGray;
            m_SoftGridLine = System.Drawing.Pens.WhiteSmoke;
        }

        public void SetParent(CaptureView parent)
        {
            m_Parent = parent;
        }

        public void SetCaptureData(CaptureData capture)
        {
            m_bCaptureDataSet = true;
            m_CaptureData = capture;

            m_fHorizontalScale = 1;
            if (capture.m_EventList.Count == 0)
                return;

            // work out the total capture length and scale the view accordingly
            uint iMaxTime = capture.m_EventList[0].iTime;
            for (int i = 1; i < capture.m_EventList.Count; i++)
            {
                uint x = capture.m_EventList[i].iTime;
                if (x > iMaxTime)
                    iMaxTime = x;
            }
            m_iMaxTime = (int)iMaxTime;
            m_fHorizontalScale = (float)iMaxTime / (float)(Width - m_iIndentation);
            if (m_fHorizontalScale < 1)
                m_fHorizontalScale = 1;

        }

        private int CalculateColourFromString(UserSettings settings, string s)
        {
            while (true)
            {
                if (settings.m_ColourSettings.ContainsKey(s))
                    return settings.m_ColourSettings[s];

                int x = s.LastIndexOf('.');
                if (x == -1)
                {
                    if (settings.m_ColourSettings.ContainsKey("[$root]"))
                        return settings.m_ColourSettings["[$root]"];
                    else
                        return 0;
                }

                s = s.Substring(0, x);
            }
        }

        public void UpdateBarColours(UserSettings settings)
        {
            m_CachedColours.Clear();

            for (int x = 0; x < m_CaptureData.m_StringList.Count; x++)
                m_CachedColours.Add(CalculateColourFromString(settings, m_CaptureData.m_StringList[x]));

            m_bColoursSet = true;
            Invalidate();
        }

        public void UpdateSize()
        {
            SetBounds(Left, Top, (int)(m_iMaxTime / m_fHorizontalScale + m_iIndentation), Height);
            Invalidate(true);
        }

        public void ZoomIn()
        {
            m_fHorizontalScale *= 2;
            UpdateSize();
        }

        public void ZoomOut()
        {
            m_fHorizontalScale /= 2;
            if (m_fHorizontalScale < 1)
                m_fHorizontalScale = 1;
            UpdateSize();
        }

        private string ConvertTimeToString(double time)
        {
            double fScale;
            double fExp = Math.Log(time) / Math.Log(10.0);
            string units;

            if (fExp < 3)
            {
                units = "us";
                fScale = 1.0;
            }
            else if (fExp < 7)
            {
                units = "ms";
                fScale = 0.001;
            }
            else
            {
                units = "s";
                fScale = 0.000001;
            }

            time = time * fScale * 100.0;
            int x = (int)time;
            time = (double)x / 100.0;
            return time.ToString() + units;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            Graphics g = e.Graphics;

            g.FillRectangle(m_Background, e.ClipRectangle);

            DrawGrid(g);

            DrawCoreSelection(g);

            DrawBars(g);
        }

        private void DrawCoreSelection(Graphics g)
        {
            int y = (m_iHeaderHeight + 7) + m_iFocusCore * m_iBarSpacing;
            g.DrawLine(m_CoreSelection, 6, y+1, this.Right - this.Left, y+1);
            g.DrawLine(m_CoreSelection, 6, y+m_iBarSpacing+1, this.Right - this.Left, y+m_iBarSpacing+1);
        }

        private int CalculateBrushColour(CoreBarsInfo bar)
        {
            if (!m_bColoursSet)
                return 0;

            if (bar.iString != 0xfffffffe)
                return m_CachedColours[(int)bar.iString];

            return 0;
        }

        private int HeightToCore(int iY)
        {
            return (iY - (m_iHeaderHeight + 7)) / m_iBarSpacing;
        }

        public int TimeToWidth(int iX)
        {
            return m_iIndentation + (int)(iX / m_fHorizontalScale);
        }

        private int WidthToTime(int iX)
        {
            return (int)((float)(iX - m_iIndentation) * m_fHorizontalScale);
        }

        private void DrawBars(Graphics g)
        {
            if (!m_bCaptureDataSet)
                return;
            if (m_CaptureData.m_EventList.Count == 0)
                return;

            
            int iStartCore = HeightToCore((int)g.ClipBounds.Top);
            int iEndCore = HeightToCore((int)g.ClipBounds.Bottom);
            int iStartTime = WidthToTime((int)g.ClipBounds.Left);
            int iEndTime = WidthToTime((int)g.ClipBounds.Right);

            int iSelBar = m_Parent.GetSelectedBar();
			bool bSelArea = m_Parent.GetSelectedArea();
			int iSelMin = m_Parent.GetSelectedMin();
			int iSelMax = m_Parent.GetSelectedMax();

            if (iStartCore<0)
                iStartCore = 0;
            if (iEndCore>m_CaptureData.m_CoreList.Count-1)
                iEndCore = m_CaptureData.m_CoreList.Count-1;

            // for each core that needs drawing
            for (int iCore=iStartCore; iCore<=iEndCore; iCore++)
            {
                int iCurrentColour = -1;
                int iCurrentStart = -1;
                int iCurrentEnd = -1;
                int iSelThread = m_Parent.GetSelectedThread();
                List<CoreBarsInfo> barList = m_CaptureData.m_CoreBarsList[iCore];

                for (int iBar=0; iBar<barList.Count; iBar++)
                {
                    CoreBarsInfo bar = barList[iBar];

                    // search to start-time for this core
                    if (bar.iEnd < iStartTime)
                        continue;

                    // skip end-time
                    if (bar.iStart > iEndTime)
                        break;

                    int x1 = TimeToWidth((int)bar.iStart);
                    int x2 = TimeToWidth((int)bar.iEnd);

                    // if this bar would be drawn over the top of the last one, then ignore
                    if (x2 <= iCurrentEnd)
                        continue;

                    // if this bar is the same colour as the previous one, and if the time is continuous, then combine them
                    int iColour = CalculateBrushColour(bar);
                    if (bar.iThread == iSelThread)
                        iColour += 6;
					//if (false)//iCurrentColour == iColour && x1<=iCurrentEnd)
					//{
					//    iCurrentEnd = x2;
					//}
					//else
                    {
                        // draw the cached bar and start a new cache using the current bar
                        if (iCurrentColour != -1)
                        {
                            g.FillRectangle(m_Bars[iCurrentColour],
                                iCurrentStart, m_iHeaderHeight + 10 + (iCore * m_iBarSpacing), 1 + (iCurrentEnd - iCurrentStart), m_iBarHeight);
                        }

                        iCurrentColour = iColour;
                        iCurrentStart = x1;
                        if (iCurrentStart<=iCurrentEnd)
                            iCurrentStart = iCurrentEnd+1;
                        iCurrentEnd = x2;
                    }
                }

                // flush last cached bar
                if (iCurrentColour != -1)
                {
                    // draw the cached bar and start a new cache using the current bar
                    g.FillRectangle(m_Bars[iCurrentColour],
                        iCurrentStart, m_iHeaderHeight + 10 + (iCore * m_iBarSpacing), 1 + (iCurrentEnd - iCurrentStart), m_iBarHeight);
                }
            }

            // highlight the currently selected bar
            if (iSelBar != -1 || bSelArea)
            {
				int iCore = m_Parent.GetSelectedCore();
				if (!bSelArea)
				{
					CoreBarsInfo bar = m_CaptureData.m_CoreBarsList[iCore][iSelBar];
					iSelMin = (int)bar.iStart;
					iSelMax = (int)bar.iEnd;
				}
                int x1 = TimeToWidth(iSelMin);
                int x2 = TimeToWidth(iSelMax);

                g.FillRectangle(m_BarSelection, 
                    x1, m_iHeaderHeight + 10 + (iCore * m_iBarSpacing), 1 + (x2 - x1), m_iBarHeight);
            }

            int iSelCore = m_Parent.GetSelectedCore();
            if (iSelCore != -1)
            {
                // draw a small blue vertical line on the current time selection
                int x = TimeToWidth(m_Parent.GetCurrentTime());
                int y = m_iHeaderHeight + 10 + (iSelCore * m_iBarSpacing);
                
                g.DrawLine(m_CoreSelection, x, y, x, y + m_iBarHeight - 1);
            }
        }

        private void DrawGrid(Graphics g)
        {
            double fHorizontalScale = m_fHorizontalScale; // 100 means 100 units per pixel

            int iIdealBarSpacing = 50;
            double fRealUnits = (double)iIdealBarSpacing * fHorizontalScale;

            double fExp = Math.Log(fRealUnits) / Math.Log(10.0);
            int iExp = (int)fExp;
            double fRealUnits2 = Math.Exp((double)iExp * Math.Log(10.0));
            double fFraction = fRealUnits / fRealUnits2;

            double fAltFraction = fFraction;
            if (fAltFraction <= 1.7)
                fAltFraction = 1.0;
            else if (fAltFraction <= 4.0)
                fAltFraction = 2.5;
            else if (fAltFraction <= 7.5)
                fAltFraction = 5.0;
            else
                fAltFraction = 10.0;
            fRealUnits2 = fRealUnits2 * fAltFraction;
            double fScreenSpacing = fRealUnits2 / fHorizontalScale;
            fScreenSpacing = fScreenSpacing / 5;

            int x = 0;
            int iBottom = this.Bottom;
            if (m_bCaptureDataSet)
                iBottom = this.Top + m_iHeaderHeight + 7 + m_CaptureData.m_CoreList.Count * m_iBarSpacing;
            for (double f = m_iIndentation; f < Width; f += fScreenSpacing)
            {
                if (f >= g.ClipBounds.Left-20 && f <= g.ClipBounds.Right+20)
                {
                    if (x%5 != 0)
                        g.DrawLine(m_SoftGridLine, (int)f, this.Top + m_iHeaderHeight, (int)f, iBottom);
                    else
                        g.DrawLine(m_HardGridLine, (int)f, this.Top + m_iHeaderHeight, (int)f, iBottom);

                    if (x%10 == 0)
                    {
                        g.DrawString(ConvertTimeToString((f-m_iIndentation) * fHorizontalScale),
                            new Font("Arial", 8), Brushes.Black, new RectangleF((float)(f - 20.0), 0, (float)(f + 20.0), 20));
                    }
                }
                x++;
            }

            int iCore = 0;
            for (int i = this.Top + m_iHeaderHeight + 8; i < this.Bottom; i += m_iBarSpacing)
            {
                g.DrawLine(m_HardGridLine, m_iIndentation, i, this.Width, i);
                if (m_bCaptureDataSet && m_CaptureData.m_CoreList.Count <= iCore)
                    break;

                string text;
                if (m_bCaptureDataSet)
                    text = m_CaptureData.m_CoreList[iCore].sName;
                else
                    text = "CPU Name";
                g.DrawString(text,
                    new Font("Arial", 8), Brushes.Black, new RectangleF(10, i + (m_iBarSpacing/2)-8, m_iIndentation, i + (m_iBarSpacing/2)+4));

                iCore++;
            }

            g.DrawLine(m_SoftGridLine, this.Right-1, this.Top + m_iHeaderHeight, this.Right-1, iBottom);
        }

        public void ProcessMouseClick(object sender, MouseEventArgs e)
        {
            // determine where in the control we clicked
            int iCore = HeightToCore(e.Y);
            int iTime = WidthToTime(e.X);
            if (iTime < 0)
                iTime = 0;

            m_Parent.SetSelectedCoreAndTime(iCore, (UInt32)iTime);
            m_iFocusCore = iCore;
            Invalidate();

            m_Parent.Select();
        }

        // member variables
        CaptureData m_CaptureData;
        CaptureView m_Parent;
        bool m_bCaptureDataSet;
        bool m_bColoursSet;
        int m_iMaxTime;
        private List<int> m_CachedColours = new List<int>();

        // designer properties
        private double m_fHorizontalScale = 0;
        [
        Category("Behavior"),
        Description("Initial horizontal scale."),
        DefaultValue(1.0)
        ]
        public double HorizontalScale
        {
            get { return m_fHorizontalScale; }
            set { m_fHorizontalScale = value; Invalidate(true); }
        }

        private Brush   m_Background;
        private Pen     m_SoftGridLine;
        private Pen     m_HardGridLine;
        private List<Brush> m_Bars;
        private Pen     m_CoreSelection;
        private Brush   m_BarSelection;
        private int     m_iBarHeight;
        private int     m_iBarSpacing;
        private int     m_iHeaderHeight;
        private int     m_iIndentation;
        public int      m_iFocusCore;

    }
}
