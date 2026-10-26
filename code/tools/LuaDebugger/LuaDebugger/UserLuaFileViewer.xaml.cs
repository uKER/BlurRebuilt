using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace LuaDebugger
{
    /// <summary>
    /// Interaction logic for UserLuaFileViewer.xaml
    /// </summary>
    public partial class UserLuaFileViewer : UserControl
    {
        static double textSize = 15;
        static double lineHeight = 17;
        static double leftBarWidth = 18;

        public string Filename { get; set; }

        public class LuaLine
        {
            public LuaLine(string t)
            {
                text = t;
            }

            public string text;
            public double offset;
        }

        List<LuaLine> lines = new List<LuaLine>();
        
        public UserLuaFileViewer()
        {
            InitializeComponent();
        }

        public void LoadFile(string root, string filename)
        {
            this.Filename = filename;

            System.IO.StreamReader sr = new System.IO.StreamReader(System.IO.Path.Combine(root, filename));

            double maxLen = 0;

            while (!sr.EndOfStream)
            {
                string line = sr.ReadLine().TrimEnd();

                string shortLine = line.TrimStart();

                string blanks = line.Substring(0, line.Length - shortLine.Length);

                FormattedText ftBlank = new FormattedText(blanks, System.Globalization.CultureInfo.CurrentCulture, FlowDirection.LeftToRight, new Typeface("Courier New"), textSize, Brushes.White);
                FormattedText ft = new FormattedText(line, System.Globalization.CultureInfo.CurrentCulture, FlowDirection.LeftToRight, new Typeface("Courier New"), textSize, Brushes.White);

                maxLen = System.Math.Max(ft.Width, maxLen);

                lines.Add(new LuaLine(shortLine) { offset = ftBlank.WidthIncludingTrailingWhitespace });
            }

            MrCanvas.Width = maxLen + 30;
            MrCanvas.Height = textSize * (lines.Count + 20);
        }

        protected override void OnRender(DrawingContext drawingContext)
        {
            base.OnRender(drawingContext);

            double xOffset = MrScroll.HorizontalOffset;
            double yOffset = MrScroll.VerticalOffset;

            bool isFileCurrent = (Debugger.debugger.CurrentExecutionFile.ToLower().CompareTo(Filename.ToLower()) == 0);
            int currentLine = Debugger.debugger.CurrentExecutionLine;

            drawingContext.DrawRectangle(Brushes.LightGray, new Pen(Brushes.LightGray, 1), new Rect(0, 0, leftBarWidth, base.ActualHeight));
            drawingContext.DrawRectangle(Brushes.White, new Pen(Brushes.White, 1), new Rect(leftBarWidth, 0, base.ActualWidth, base.ActualHeight));

            List<Debugger.Breakpoint> breakpoints = Debugger.debugger.Breakpoints;

            var bpInFile = from bp in breakpoints where (bp.filename.CompareTo(this.Filename) == 0) select bp.line-1;

            for (int i = 0; i < lines.Count; i++)
            {
                LuaLine ll = lines[i];

                double yPos = i * lineHeight - yOffset;

                if (yPos < -lineHeight)
                    continue;
                else if (yPos > ActualHeight)
                    break;

                if (bpInFile.Contains(i))
                {
                    drawingContext.DrawEllipse(Brushes.DarkRed, new Pen(Brushes.DarkRed, 1), new Point(8, yPos + 8), 4, 4);
                }

                drawingContext.PushClip(new RectangleGeometry(new Rect(leftBarWidth, yPos, ActualWidth - leftBarWidth, yPos + lineHeight)));

                if (isFileCurrent && i == currentLine)
                {
                    FormattedText ft = new FormattedText(ll.text, System.Globalization.CultureInfo.CurrentCulture, FlowDirection.LeftToRight, new Typeface("Courier New"), textSize, Brushes.Black);

                    drawingContext.DrawRectangle(Brushes.Yellow, new Pen(Brushes.Yellow, 1), new Rect(ll.offset + 20 - xOffset, yPos, ft.Width, lineHeight));
                    drawingContext.DrawText(ft, new Point(ll.offset + 20 - xOffset, yPos));
                }
                else if (false) // (ll.breakpoint)
                {
                    FormattedText ft = new FormattedText(ll.text, System.Globalization.CultureInfo.CurrentCulture, FlowDirection.LeftToRight, new Typeface("Courier New"), textSize, Brushes.White);

                    drawingContext.DrawRectangle(Brushes.DarkRed, new Pen(Brushes.DarkRed, 1), new Rect(ll.offset + 20 - xOffset, yPos, ft.Width, lineHeight));
                    drawingContext.DrawText(ft, new Point(ll.offset + 20 - xOffset, yPos));
                }
                else
                {
                    FormattedText ft = new FormattedText(ll.text, System.Globalization.CultureInfo.CurrentCulture, FlowDirection.LeftToRight, new Typeface("Courier New"), textSize, Brushes.Black);

                    drawingContext.DrawText(ft, new Point(ll.offset + 20 - xOffset, yPos));
                }
                
                drawingContext.Pop();
            }
        }

        private void MrScroll_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            this.InvalidateVisual();
        }

        protected override void OnMouseDoubleClick(MouseButtonEventArgs e)
        {
            Point p = e.GetPosition(this);

            double yOffset = MrScroll.VerticalOffset;

            int line = (int) ((p.Y + yOffset) / lineHeight);

            if (line < lines.Count)
            {
                if (lines[line].text.Length > 0)
                {
                    Debugger.debugger.ToggleBreakpoint(this.Filename, line);

                    InvalidateVisual();
                }
            }

            base.OnMouseDoubleClick(e);
        }

        public void GotoLine(int line)
        {
            double yOffset = MrScroll.VerticalOffset;

            int linesOnScreen = (int)(ActualHeight / lineHeight);

            int topScreen = (int)(MrScroll.VerticalOffset / lineHeight);
            int botScreen = (int)((MrScroll.VerticalOffset + ActualHeight) / lineHeight);

            if (line < topScreen || line > botScreen)
            {
                MrScroll.ScrollToVerticalOffset(line * lineHeight);
            }

            InvalidateVisual();
        }
    }
}
