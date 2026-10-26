using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;
using System.Drawing;
using System.ComponentModel;

namespace ProfilingApplication
{
	class GraphChart : Panel
	{
        Brush		m_Background;
		Pen			m_HardGridLine, m_SoftGridLine;
		int			m_iIndentation;
		int			m_iHeaderHeight;
		int			m_iGridXSpacing;
		int			m_iGridYSpacing;
		int			m_iAccentSpacing;
		int			m_iHorizontalGridOffset;
		List<CounterGraphInfo> m_Counters;
		Colour		m_bgColourDark, m_bgColourLight;
		bool		m_bDrawGrid;
		bool		m_bAutoScale;
		int			m_iSelection;
		float		m_fMin, m_fMax;
		float		m_fLastFrameBiggestValue;
		GraphView	m_Parent;

        // designer properties
        [ Category("Appearance"), Description("Grid X Spacing"), DefaultValue(40.0) ]
        public int GridXSpacing { get { return m_iGridXSpacing; } set { m_iGridXSpacing = value; Invalidate(true); } }

        [ Category("Appearance"), Description("Grid Y Spacing"), DefaultValue(40.0) ]
        public int GridYSpacing { get { return m_iGridYSpacing; } set { m_iGridYSpacing = value; Invalidate(true); } }

        [ Category("Appearance"), Description("Header Height"), DefaultValue(40.0) ]
        public int HeaderHeight { get { return m_iHeaderHeight; } set { m_iHeaderHeight = value; Invalidate(true); } }

        [ Category("Appearance"), Description("Indentation"), DefaultValue(40.0) ]
        public int Indentation { get { return m_iIndentation; } set { m_iIndentation = value; Invalidate(true); } }

        [ Category("Appearance"), Description("Accent Spacing"), DefaultValue(5) ]
        public int AccentSpacing { get { return m_iAccentSpacing; } set { m_iAccentSpacing = value; Invalidate(true); } }


		// functions
		public GraphChart(GraphView parent)
		{
            // Activates double buffering 
            SetStyle(ControlStyles.UserPaint, true);
            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            SetStyle(ControlStyles.DoubleBuffer, true);

			GenerateBackgroundBrush();
            m_HardGridLine = System.Drawing.Pens.SlateBlue;
            m_SoftGridLine = System.Drawing.Pens.DarkSlateBlue;

			m_iIndentation = 60;
			m_iHeaderHeight = 0;
			m_iGridXSpacing = 40;
			m_iGridYSpacing = 40;
			m_iAccentSpacing = 5;
			m_iHorizontalGridOffset = 0;
			m_bDrawGrid = true;
			m_fMin = 0;
			m_fMax = 100;
			m_fLastFrameBiggestValue = 0;
			m_iSelection = 0;
			m_Parent = parent;

			this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.GraphChart_MouseMove);
			this.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.GraphChart_MouseDoubleClick);
		}

		// move the background grid
		public void MoveGridOffset(int iOffset)
		{
			float fD = (float)this.Width / (float)(CounterGraphInfo.iMaxValues-2);

			int iAccentGridWidth = m_iGridXSpacing * m_iAccentSpacing;

			m_iHorizontalGridOffset += (int)(iOffset * fD);
			while (m_iHorizontalGridOffset < 0)
				m_iHorizontalGridOffset += iAccentGridWidth;
			while (m_iHorizontalGridOffset > iAccentGridWidth)
				m_iHorizontalGridOffset -= iAccentGridWidth;

			if (m_bAutoScale)
			{
				float fMax = m_fLastFrameBiggestValue;
				if (fMax <= 1)
					fMax = 1;
				else if (fMax<=10)
					fMax = 10;
				else if (fMax<=100)
					fMax = 100;
				else if (fMax<=1000)
					fMax = 1000;
				else if (fMax<=10000)
					fMax = 10000;
				else if (fMax<=100000)
					fMax = 100000;
				else if (fMax<=1000000)
					fMax = 1000000;
				else if (fMax<=10000000)
					fMax = 10000000;
				m_fMax = fMax;

				m_fMin = 0;
				m_fLastFrameBiggestValue = 0;
			}
		}

		// draw the background grid
		void DrawGrid(Graphics g, Rectangle rect)
		{
            g.FillRectangle(m_Background, rect);

			int iOffset = m_iHorizontalGridOffset % m_iGridXSpacing;

            int x = m_iAccentSpacing - 1 - (m_iHorizontalGridOffset / m_iGridXSpacing);
            int iBottom = this.Height;
			if (m_bDrawGrid)
			{
				for (int f = Width-iOffset; f>m_iIndentation; f -= m_iGridXSpacing)
				{
					if (f >= g.ClipBounds.Left-20 && f <= g.ClipBounds.Right+20)
						g.DrawLine(((x%m_iAccentSpacing)==0)?m_HardGridLine:m_SoftGridLine, (int)f, m_iHeaderHeight, (int)f, iBottom);
					x++;
				}
			}

			x = 0;
			float fValue = m_fMax;
			float fDelta = m_iGridYSpacing*(m_fMax-m_fMin)/this.Height;
			fValue -= fDelta;
            for (int i = m_iHeaderHeight; i < this.Height; i += m_iGridYSpacing)
            {
				if (m_bDrawGrid)
	                g.DrawLine(((x%m_iAccentSpacing)==0)?m_HardGridLine:m_SoftGridLine, m_iIndentation, i, this.Width, i);
				x++;

				if (m_iIndentation > 30)
				{
					StringFormat fmt = new StringFormat();
					fmt.Alignment = StringAlignment.Far;
					string text;
					text = fValue.ToString("N0");
					g.DrawString(text,
						new Font("Arial", 8), Brushes.Silver, new RectangleF(0, i + (m_iGridYSpacing/3), m_iIndentation, 12), fmt);
				}

				fValue-=fDelta;
            }

		}

		// draw the graph for a single counter
		void DrawGraph(Graphics g, Rectangle rect, CounterGraphInfo info)
		{
			Pen pen = info.pen;
			int iMaxValues = CounterGraphInfo.iMaxValues -2;

			float fD = (float)(this.Width-2) / (float)iMaxValues;
			float fMin = m_fMin;
			float fMax = m_fMax;

			float fYScale = info.fScale * (((float)this.Height))/ (fMax-fMin);

			float fXO = 0, fYO = 0;
			bool bFirst = true;
			int x = info.iLastValue;
			for (int i=0; i<info.nValues; i++)
			{
				float fVal = info.fValues[x];
				float fValScale = fVal * info.fScale;
				if (fValScale > m_fLastFrameBiggestValue)
					m_fLastFrameBiggestValue = fValScale;

				float fX = (iMaxValues - i) * fD;
				float fY = this.Height - ((info.fValues[x]-fMin) * fYScale);
				if (bFirst)
					bFirst = false;
				else
					g.DrawLine(pen, fXO, fYO, fX, fY);

				x--;
				if (x < 0)
					x = iMaxValues-2;

				fXO = fX;
				fYO = fY;
			}
		}

		protected void DrawSelection(Graphics g, Rectangle rect)
		{
			int iMaxValues = CounterGraphInfo.iMaxValues -2;
			float fD = (float)(this.Width-2) / (float)iMaxValues;

			float x = (float)m_iSelection * fD;
			g.DrawLine(System.Drawing.Pens.White, x, 0, x, this.Height);
		}

        protected override void OnPaint(PaintEventArgs e)
        {
            Graphics g = e.Graphics;

            DrawGrid(g, e.ClipRectangle);

			DrawSelection(g, e.ClipRectangle);

			if (m_Counters != null)
			{
				for (int i=0; i<m_Counters.Count; i++)
					DrawGraph(g, e.ClipRectangle, m_Counters[i]);
			}
        }

		void GenerateBackgroundBrush()
		{
			int iHeight = this.Height;
			if (iHeight < 600)
				iHeight = 600;

            m_Background = new LinearGradientBrush(new Point(0,0), new Point(0,iHeight), 
					System.Drawing.Color.FromArgb(m_bgColourLight.R,m_bgColourLight.G,m_bgColourLight.B),
					System.Drawing.Color.FromArgb(m_bgColourDark.R,m_bgColourDark.G,m_bgColourDark.B));

			Invalidate(true);
		}

		protected override void OnSizeChanged(EventArgs e)
		{
			GenerateBackgroundBrush();

			base.OnSizeChanged(e);
		}

		internal void SetCounters(List<CounterGraphInfo> counters)
		{
			m_Counters = counters;			
		}

		internal void SetSettings(GraphViewConfig settings)
		{
			m_iGridXSpacing = m_iGridYSpacing = settings.iGridSpacing;
			m_iAccentSpacing = settings.iHighlightEvery;
			m_bDrawGrid = settings.bGridOn;

			m_bgColourLight = settings.bgLight;
			m_bgColourDark = settings.bgDark;

			m_fMin = settings.fMin;
			m_fMax = settings.fMax;
			m_bAutoScale = settings.bAutoScale;
			m_iIndentation = settings.bMargin?40:0;

			m_HardGridLine = new System.Drawing.Pen(System.Drawing.Color.FromArgb(255,settings.gridLight.R, settings.gridLight.G,settings.gridLight.B));
			m_SoftGridLine = new System.Drawing.Pen(System.Drawing.Color.FromArgb(255,settings.gridDark.R, settings.gridDark.G,settings.gridDark.B));
			GenerateBackgroundBrush();
			Invalidate();
		}

		// display a tracking line to show our selection
 		private void GraphChart_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left)
			{
				int iMaxValues = CounterGraphInfo.iMaxValues -2;
				float fD = (float)(this.Width-2) / (float)iMaxValues;
				m_iSelection = (int)((float)e.X / fD);
				Invalidate();
			}
		}

		// double clicking on the graph retrieves the timing capture for that frame if possible
		private void GraphChart_MouseDoubleClick(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (m_Parent == null)
				return;

			int iMaxValues = CounterGraphInfo.iMaxValues -2;
			float fD = (float)(this.Width-2) / (float)iMaxValues;
			m_iSelection = (int)((float)e.X / fD);
			Invalidate();
			m_Parent.ChartClicked(CounterGraphInfo.iMaxValues-m_iSelection);
		}
	}
}
