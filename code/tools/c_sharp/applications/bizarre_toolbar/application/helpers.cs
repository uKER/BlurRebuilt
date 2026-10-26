using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace application
{
	class Helpers
	{
		static public RectangleF GravitateBounds(RectangleF from, Rectangle to, double gravity, double snapVal)
		{
			double newX = Gravitate(from.X, to.X, gravity, snapVal);
			double newY = Gravitate(from.Y, to.Y, gravity, snapVal);
			double newW = Gravitate(from.Width, to.Width, gravity, snapVal);
			double newH = Gravitate(from.Height, to.Height, gravity, snapVal);

			return new RectangleF((float)newX, (float)newY, (float)newW, (float)newH);
		}

		static public double Gravitate(double from, double to, double gravity, double snapVal)
		{
			double diff = to - from;

			double value = from + diff * gravity;

			if(Math.Abs(to - value) < snapVal)
				return to;

			return value;
		}
	}
}
