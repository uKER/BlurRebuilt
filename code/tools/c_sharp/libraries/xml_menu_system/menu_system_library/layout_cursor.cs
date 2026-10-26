using System;
using System.Collections.Generic;
using System.Text;

namespace MenuSystemLibrary
{
	class LayoutCursor
	{		
		const int minX = 3;
		const int minY = 13;

		internal LayoutCursor()
		{
			Reset();
		}

		internal void Reset()
		{
			_X = minX;
			_Y = minY;
			_maxX = minX;
			_maxY = minY;
		}

		int _X = minX;
		int _Y = minY;
		internal int _maxX = minX;
		internal int _maxY = minY;

		internal int X { get { return _X; } set { _X = value; if(_X > _maxX)_maxX = _X; } }
		internal int Y { get { return _Y; } set { _Y = value; if(_Y > _maxY)_maxY = _Y; } }

		internal void CR()
		{
			X = minX;
			Y = _maxY;
		}

		internal void xtab(int xval)
		{
			X += xval;
		}

		internal void ytab(int yval)
		{
			Y += yval;
		}

		internal void Print(int width, int height)
		{
			this.X += width;
			if(this.Y + height > this._maxY)
				this._maxY = this.Y + height;

		}
	}
}
