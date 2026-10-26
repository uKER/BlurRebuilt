using System;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace Bizarre.Test
{
    /// <summary>
    /// Holds a regex for splitting up arguments.
    /// </summary>
    internal static class ArgumentParser
    {
        // Regex to split args; splits on word boundaries and "quoted phrases"
        private const string NonQuotedWord = @"([^"" \n\r\t\f]+)";
        private const string QuotedPhrase = @"(""[^""]+"")";
        private static Regex regex = new Regex(NonQuotedWord + "|" + QuotedPhrase, RegexOptions.Compiled);

        /// <summary>
        /// Gets the regex used for splitting arguments.
        /// </summary>
        internal static Regex Regex
        {
            get { return regex; }
        }
    }
}