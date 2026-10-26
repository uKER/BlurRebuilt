//
// NAntContrib
// Copyright (C) 2002 Tomas Restrepo (tomasr@mvps.org)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//

// [Keith] Addendum. Cut out everything except the function ToRegexPattern. 080905.

using System;
using System.Collections.Specialized;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;



namespace RegexpHelpers
{
	/// <summary>
	///     Lifted/Modified from <see cref="NAnt.Core.DirectoryScanner"/> to convert patterns to match filenames to regularexpressions.
	/// </summary>
	/// <param name="nantPattern">Search pattern - meant to be just a filename with no path info</param>
	/// <remarks>The directory seperation code in here most likely is overkill.</remarks>
	/// <returns>Regular expresssion for searching matching file names</returns>
	/// 

	public class FilemaskHandler
	{
		protected FilemaskHandler()
		{
		}

		public static string ToRegexFromList(string strList)
		{
			string result = null;

			string delimStr = ";";
			char [] delimiter = delimStr.ToCharArray();

			string[] masks = strList.Split(delimiter);

			bool first = true;

			foreach (string s in masks) 
			{
				if(first == false)
				{
					result += "|";
				}
				first = false;

				if (s.Trim() != "")
					result += "(" + ToRegexPattern(s.ToLower()) + ")";		
			}

			return result;
		}

		public static string ToRegexPattern(string nantPattern)
		{
			StringBuilder pattern = new StringBuilder(nantPattern);

			// NAnt patterns can use either / \ as a directory seperator.
			// We must replace both of these characters with Path.DirectorySeperatorChar
			pattern.Replace('/',  Path.DirectorySeparatorChar);
			pattern.Replace('\\', Path.DirectorySeparatorChar);

			// The '\' character is a special character in regular expressions
			// and must be escaped before doing anything else.
			pattern.Replace(@"\", @"\\");

			// Escape the rest of the regular expression special characters.
			// NOTE: Characters other than . $ ^ { [ ( | ) * + ? \ match themselves.
			// TODO: Decide if ] and } are missing from this list, the above
			// list of characters was taking from the .NET SDK docs.
			pattern.Replace(".", @"\.");
			pattern.Replace("$", @"\$");
			pattern.Replace("^", @"\^");
			pattern.Replace("{", @"\{");
			pattern.Replace("[", @"\[");
			pattern.Replace("(", @"\(");
			pattern.Replace(")", @"\)");
			pattern.Replace("+", @"\+");

			// Special case directory seperator string under Windows.
			string seperator = Path.DirectorySeparatorChar.ToString();
			if (seperator == @"\") 
			{
				seperator = @"\\";
			}

			// Convert NAnt pattern characters to regular expression patterns.

			// SPECIAL CASE: to match subdirectory OR current directory.  If
			// we don't do this then we can write something like 'src/**/*.cs'
			// to match all the files ending in .cs in the src directory OR
			// subdirectories of src.
			pattern.Replace(seperator + "**", "(" + seperator + ".|)|");

			// | is a place holder for * to prevent it from being replaced in next line
			pattern.Replace("**", ".|");
			pattern.Replace("*", "[^" + seperator + "]*");
			//			pattern.Replace("?", "[^" + seperator + "]?");
						pattern.Replace("?", ".");
			pattern.Replace('|', '*'); // replace place holder string

			// Help speed up the search
			//pattern.Insert(0, '^'); // start of line
			//pattern.Append('$'); // end of line

			return pattern.ToString();
		}
	}
}