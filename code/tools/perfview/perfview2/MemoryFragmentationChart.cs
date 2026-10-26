using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;
using System.Drawing;

namespace ProfilingApplication
{
	class MemoryFragmentationChart : Panel
	{
		// member variables
		MemoryTrackerData	m_MemoryDump;
        private Brush		m_Background;
		List<Brush>			m_Blocks;
		List<UInt32>		m_GridContents; // for each grid cell this contains the number of bytes of allocated memory 
		List<bool>			m_GridSelected;	// for each grid cell, whether it contains the currently selected memory tracker id
		MemoryFragmentationView m_Parent;
		Font				m_Font;
		SolidBrush			m_TextBrush;
		SolidBrush			m_SelectedBrush;

		int					m_iFilterTag;
		int					m_iBlocksPerLine;
		int					m_nBlocks;
		int					m_nLines;
		UInt32 				m_nBytesPerBlock;
		int					m_iBlockWidth;
		int					m_iBlockSpacing;
		int					m_iCurrentWidth;
		int					m_iLeftMargin;
		int					m_iSelectedBlock;
		int					m_iSelectedTrackerId;

		// functions
		public MemoryFragmentationChart()
		{
            // Activates double buffering 
            SetStyle(ControlStyles.UserPaint, true);
            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            SetStyle(ControlStyles.DoubleBuffer, true);

            m_Background = new SolidBrush(System.Drawing.Color.FromArgb(197,213,250));

			m_nBytesPerBlock = 1024 * 256;
			m_iBlockWidth = 13;
			m_iBlockSpacing = 14;
			m_iLeftMargin = 80;
			m_iFilterTag = -1;
			m_iSelectedBlock = -1;
			m_iSelectedTrackerId = -1;

			m_Blocks = new List<Brush>();
			m_iCurrentWidth = -1;

			int a = 255;
			{
				// blue
				m_Blocks.Add(new LinearGradientBrush(new Point(0, m_iBlockSpacing), new Point(0, m_iBlockSpacing + m_iBlockSpacing),
							System.Drawing.Color.FromArgb(a, 0, 102, 204), System.Drawing.Color.FromArgb(a, 118, 172, 236)));

				// light blue
				m_Blocks.Add(new LinearGradientBrush(new Point(0, m_iBlockSpacing), new Point(0, m_iBlockSpacing + m_iBlockSpacing),
							System.Drawing.Color.FromArgb(a, 0, 155, 255), System.Drawing.Color.FromArgb(a, 108, 211, 255)));

				// pale
				m_Blocks.Add(new LinearGradientBrush(new Point(0, m_iBlockSpacing), new Point(0, m_iBlockSpacing + m_iBlockSpacing),
							System.Drawing.Color.FromArgb(a, 134, 116, 148), System.Drawing.Color.FromArgb(a, 185, 180, 206)));

				// pale red
				m_Blocks.Add(new LinearGradientBrush(new Point(0, m_iBlockSpacing), new Point(0, m_iBlockSpacing + m_iBlockSpacing),
							System.Drawing.Color.FromArgb(a, 254, 113, 77), System.Drawing.Color.FromArgb(a, 249, 179, 169)));

				// red
				m_Blocks.Add(new LinearGradientBrush(new Point(0, m_iBlockSpacing), new Point(0, m_iBlockSpacing + m_iBlockSpacing),
							System.Drawing.Color.FromArgb(a, 254, 57, 8), System.Drawing.Color.FromArgb(a, 249, 149, 131)));

				// dark red
				m_Blocks.Add(new LinearGradientBrush(new Point(0, m_iBlockSpacing), new Point(0, m_iBlockSpacing + m_iBlockSpacing),
							System.Drawing.Color.FromArgb(a, 151, 0, 0), System.Drawing.Color.FromArgb(a, 194, 118, 127)));

				// white
				m_Blocks.Add(new LinearGradientBrush(new Point(0, m_iBlockSpacing), new Point(0, m_iBlockSpacing + m_iBlockSpacing),
							System.Drawing.Color.FromArgb(a, 255, 255, 255), System.Drawing.Color.FromArgb(a, 255, 255, 255)));
			}

			m_SelectedBrush = new SolidBrush(System.Drawing.Color.FromArgb(100,0,0,0));

			this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.ProcessMouseClick);

			// Create font and brush.
			m_Font = new Font("Arial", 8);
			m_TextBrush = new SolidBrush(Color.Black);
		}

		public void SetParent(MemoryFragmentationView parent)
		{
			m_Parent = parent;
		}

		internal void SetMemoryTrackerData(MemoryTrackerData memoryDump)
		{
			m_MemoryDump = memoryDump;
		}

        public void ProcessMouseClick(object sender, MouseEventArgs e)
        {
			int X = e.X - m_iLeftMargin;
			if (X<0)
				return;

            // determine which gridbox we clicked
			UInt32 iBlock = (UInt32)(((X)/m_iBlockSpacing) + (e.Y/m_iBlockSpacing)*m_iBlocksPerLine);

			m_iSelectedBlock = (int)iBlock;

			// bring up a window containing a list of all the allocations inside this block
			m_Parent.SelectBlock(iBlock*m_nBytesPerBlock + m_MemoryDump.m_iMemoryStart, m_nBytesPerBlock, false);

			Invalidate();
        }

		void RecalcGrid()
		{
			if (m_MemoryDump == null)
				return;

			// calculate grid dimensions
			m_nBlocks = 1+(int)((m_MemoryDump.m_iMemoryEnd - m_MemoryDump.m_iMemoryStart) / m_nBytesPerBlock);
			m_iBlocksPerLine = (this.Width-m_iLeftMargin) / m_iBlockSpacing;
			m_nLines = (m_nBlocks / m_iBlocksPerLine)+1;

			if (m_iBlocksPerLine < 1)
				return;

			m_GridContents = new List<UInt32>(m_nBlocks);
			m_GridSelected = new List<bool>(m_nBlocks);

			// zero the grid
			for (int i=0; i<m_nBlocks; i++)
			{
				m_GridContents.Add(0);
				m_GridSelected.Add(false);
			}

			// check each allocation and add up the number of allocated bytes for each grid cell
			for (int i=0; i<m_MemoryDump.m_Allocations.Count; i++)
			{
				AllocationInfo item = m_MemoryDump.m_Allocations[i];
				if (m_iFilterTag != -1 && item.iTag != m_iFilterTag)
					continue;

				// mark used space
				UInt32 iAddrStart  = item.iAddress - m_MemoryDump.m_iMemoryStart;
				UInt32 iAddrEnd    = item.iAddress - m_MemoryDump.m_iMemoryStart + item.cSize;
				UInt32 iStart = iAddrStart/m_nBytesPerBlock;
				UInt32 iEnd = iAddrEnd/m_nBytesPerBlock;

				bool bSelected = (item.iContextId == m_iSelectedTrackerId);

				for (UInt32 iBlock = iStart; iBlock<=iEnd; iBlock++)
				{
					UInt32  iBlockStart = iBlock*m_nBytesPerBlock;
					UInt32  iBlockEnd   = iBlockStart+m_nBytesPerBlock;

					UInt32 d = ((iBlockEnd>iAddrEnd)?iAddrEnd:iBlockEnd) - ((iBlockStart>iAddrStart)?iBlockStart:iAddrStart);
					m_GridContents[(int)iBlock] += d;

					if (bSelected)
						m_GridSelected[(int)iBlock] = true;
				}
			}

            SetBounds(Left, Top, Width, m_nLines * m_iBlockSpacing);

			m_iCurrentWidth = this.Width;
			m_iSelectedBlock = -1;
		}

		void DrawBlock(Graphics g, int X, int Y)
		{
			bool bSelected = false;

			int iBrush;
			if (m_GridContents != null)
			{
				int iBlock = X + Y*m_iBlocksPerLine;
				if (iBlock >= m_nBlocks)
					return;
				float u = (100*(float)m_GridContents[iBlock]/(float)m_nBytesPerBlock);

				if (u == 100)	// completely full
					iBrush = 0;
				else if (u > 90)	
					iBrush = 1;
				else if (u > 70)	
					iBrush = 2;
				else if (u > 50)	
					iBrush = 3;
				else if (u > 20)	
					iBrush = 4;
				else if (u > 0)
					iBrush = 5;
				else				
					iBrush = 6;

				bSelected = m_GridSelected[iBlock];
			}
			else
				iBrush = 6;

			g.FillRectangle(m_Blocks[iBrush], m_iLeftMargin+X*m_iBlockSpacing, Y*m_iBlockSpacing, m_iBlockWidth, m_iBlockWidth);

			if (bSelected)
			{
				g.FillRectangle(m_SelectedBrush, m_iLeftMargin+X*m_iBlockSpacing-1, Y*m_iBlockSpacing-1, m_iBlockWidth+2, m_iBlockWidth+2);
				g.DrawRectangle(System.Drawing.Pens.White, m_iLeftMargin+X*m_iBlockSpacing+1, Y*m_iBlockSpacing+1, m_iBlockWidth-1, m_iBlockWidth-1);
			}
		}

		int PointToGridX(int x)
		{
			int rtn = (x-m_iLeftMargin)/m_iBlockSpacing;
			if (rtn<0)
				rtn=0;
			else if (rtn >= m_iBlocksPerLine)
				rtn = m_iBlocksPerLine-1;
			return rtn;
		}

		int PointToGridY(int y)
		{
			int rtn = y/m_iBlockSpacing;
			if (rtn<0)
				rtn=0;
			if (rtn >= m_nLines)
				rtn = m_nLines-1;
			return rtn;
		}

		void DrawGrid(Graphics g, Rectangle rect)
		{
			if (m_iCurrentWidth != this.Width)
				RecalcGrid();

			int Ystart = PointToGridY(rect.Top);
			int Yend = PointToGridY(rect.Bottom);
			int Xstart = PointToGridX(rect.Left);
			int Xend = PointToGridX(rect.Right);

			for (int Y=Ystart; Y<=Yend; Y++)
			{
				// draw the address in the margin
				if ((Y&3) == 0)
				{
					long iAddr = Y * m_iBlocksPerLine * m_nBytesPerBlock + m_MemoryDump.m_iMemoryStart;
					g.DrawString(iAddr.ToString("X"), m_Font, m_TextBrush, new PointF(4, Y*m_iBlockSpacing));
				}

				for (int X=Xstart; X<=Xend; X++)
				{
					DrawBlock(g, X, Y);
				}
			}
	
			if (m_iSelectedBlock != -1)
			{
				int X = m_iSelectedBlock % m_iBlocksPerLine;
				int Y = m_iSelectedBlock / m_iBlocksPerLine;
				g.DrawRectangle(System.Drawing.Pens.White, m_iLeftMargin+X*m_iBlockSpacing-1, Y*m_iBlockSpacing, m_iBlockWidth+1, m_iBlockWidth);
				g.DrawRectangle(System.Drawing.Pens.Black, m_iLeftMargin+X*m_iBlockSpacing-2, Y*m_iBlockSpacing-1, m_iBlockWidth+3, m_iBlockWidth+2);
			} 
		}

		protected override void OnSizeChanged(EventArgs e)
		{
			if (this.Width != m_iCurrentWidth)
				Invalidate();

			base.OnSizeChanged(e);
		}

		public void SetFilterTag(int p)
		{
			if (p != m_iFilterTag)
			{
				m_iFilterTag = p;
				m_iCurrentWidth = -1;
				Invalidate();
			}					
		}

        protected override void OnPaint(PaintEventArgs e)
        {
            Graphics g = e.Graphics;

            g.FillRectangle(m_Background, e.ClipRectangle);

            DrawGrid(g, e.ClipRectangle);
        }

		internal void SetBlockSize(int i)
		{
			m_nBytesPerBlock = (UInt32)i;
			m_iCurrentWidth = -1;
			Invalidate();
		}

		internal void SetSelectedTrackerId(int iNewNode)
		{
			m_iSelectedTrackerId = iNewNode;
			m_iCurrentWidth = -1;
			Invalidate();
		}
	}
}
