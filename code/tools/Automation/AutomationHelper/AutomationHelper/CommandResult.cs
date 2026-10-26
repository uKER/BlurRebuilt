namespace Automation
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text.RegularExpressions;

    /// <summary>
    /// Type of command result; use to determine how to read result.
    /// </summary>
    public enum ResultType { NotSet, SingleLine, MultiLine, Error, Timeout, Canceled };

    /// <summary>
    /// Contains the result of a command sent back from an Xbox.
    /// </summary>
    public class CommandResult
    {
        private ResultType resultType = ResultType.NotSet;
        private List<string> lines = new List<string>(1);

        /// <summary>
        /// Gets the type of result.
        /// </summary>
        public ResultType Type
        {
            get { return resultType; }
            internal set { resultType = value; }
        }

        /// <summary>
        /// Gets if the command failed, returns true if it did. Check Error for details.
        /// </summary>
        public bool Failed
        {
            get
            {
                return (resultType == ResultType.Error ||
                    resultType == ResultType.Timeout ||
                    resultType == ResultType.Canceled);
            }
        }

        /// <summary>
        /// Gets the a single line result; ResultType must be Type.SingleLine.
        /// </summary>
        public string SingleLine
        {
            get
            {
                System.Diagnostics.Debug.Assert(resultType == ResultType.SingleLine);
                System.Diagnostics.Debug.Assert(lines.Count == 1);

                return lines[0];
            }

            internal set
            {
                lines.Clear();
                lines.Add(value);
            }
        }

        /// <summary>
        /// Gets the multi-line result; ResultType must be Type.MultiLine.
        /// </summary>
        public List<string> MultiLine
        {
            get
            {
                System.Diagnostics.Debug.Assert(resultType == ResultType.MultiLine, "Check that the message is actually multi-line when we try and get the data out");

                return lines;
            }
        }

        /// <summary>
        /// Gets the error; ResultType must be Type.Error.
        /// </summary>
        public string Error
        {
            get
            {
                System.Diagnostics.Debug.Assert(resultType == ResultType.Error || resultType == ResultType.Timeout);
                System.Diagnostics.Debug.Assert(lines.Count == 1);

                return lines[0];
            }

            internal set
            {
                lines.Clear();
                lines.Add(value);
            }
        }

        /// <summary>
        /// Splits arguments up for a single-line response.
        /// </summary>
        /// <returns>Split arguments.</returns>
        //public string[] GetSingleLineArgs()
        //{
        //    return GetArgs(SingleLine);
        //}

        /// <summary>
        /// Splits arguments up for a multi-line response.
        /// </summary>
        /// <param name="line">Line number.</param>
        /// <returns>Split arguments.</returns>
        //public string[] GetMultiLineArgs(int line)
        //{
        //    System.Diagnostics.Debug.Assert(0 <= line && line < MultiLine.Count);
        //    return GetArgs(MultiLine[line]);
        //}

        //private string[] GetArgs(string text)
        //{
        //    MatchCollection matches = ArgumentParser.Regex.Matches(text);

        //    string[] args = new string[matches.Count];

        //    for (int i = 0; i < matches.Count; ++i)
        //        args[i] = matches[i].Value;

        //    return args;
        //}
    }
}