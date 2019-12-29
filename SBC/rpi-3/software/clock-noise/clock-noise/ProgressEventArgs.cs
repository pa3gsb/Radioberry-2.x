using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ClockNoise
{
    public class ProgressEventArgs
    {
            string message;
            

            public string Message
            {
                get { return message; }
                set { message = value; }
            }

          

            public ProgressEventArgs(string message)
            {
                this.message = message;
            }
           


           
    }
}
