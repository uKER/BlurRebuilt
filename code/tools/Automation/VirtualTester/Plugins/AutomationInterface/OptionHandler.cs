using System;
using System.Collections.Generic;
using System.Threading;
//using System.Windows.Forms;

namespace Bizarre.Test
{
    public class OptionHandler<T>
    {
        Random random;
        List<T> optionNames = new List<T>();
        List<int> weightings = new List<int>();
        int lastChoice;

        public OptionHandler()
        {
            random = new Random();
            lastChoice = -1;
        }

        /**
         *  Copy options from an existing handler
         **/
        public OptionHandler(OptionHandler<T> handlerToCopy)
        {
            random = new Random();
            lastChoice = -1;
            this.optionNames.AddRange(handlerToCopy.optionNames);
            this.weightings.AddRange(handlerToCopy.weightings);
        }

        public List<T> GetOptions()
        {
            return optionNames;
        }

        public void AddOption(T name, int weighting)
        {
            optionNames.Add(name);
            weightings.Add(weighting);
        }

        /**
         *  Add options from another list with the given weighting  
         */
        public void AddOptions(List<T> options, int weighting)
        {
            foreach (T option in options)
            {
                optionNames.Add(option);
                weightings.Add(weighting);
            }
        }

        /**
         *  Remove a chosen option and its weighting  
         */
        public void RemoveOptions(T option)
        {
            int index = optionNames.IndexOf(option);
            optionNames.RemoveAt(index);
            weightings.RemoveAt(index);
        }

        public void Clear()
        {
            optionNames.Clear();
            weightings.Clear();
            lastChoice = -1;
        }

        public int numOptions
        {
            get
            {
                return optionNames.Count;
            }
        }

        public int TotalWeightings
        {
            get
            {
                int tw = 0;
                foreach (int i in weightings)
                    tw += i;
                return tw;
            }
        }

        public T GetSelectedOption
        {
            get
            {
                return optionNames[lastChoice];
            }
        }

        public int GetSelectedOptionNumber
        {
            get
            {
                return lastChoice;
            }
        }

        public void GetRandomOption()
        {
            int number = random.Next(0, this.TotalWeightings - 1);
            int option = 0;
            int weightingsSoFar = 0;

            while (number > weightingsSoFar + weightings[option])
            {
                weightingsSoFar += weightings[option];
                option++;
            }

            lastChoice = option;
        }
    }
}
