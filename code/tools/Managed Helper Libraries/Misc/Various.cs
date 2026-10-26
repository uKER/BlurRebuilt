using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Bizarre.Misc
{
    public class Various
    {
        public static string ConvertSizeToString(uint size)
        {
            if (size >= 1024 * 1024)
            {
                return string.Format("{0:0.00} MB", size / (1024.0 * 1024.0));
            }
            else if (size >= 1024)
            {
                return string.Format("{0:0.0}k", size / 1024.0);
            }
            else
            {
                return size + " bytes";
            }
        }
    }
}
