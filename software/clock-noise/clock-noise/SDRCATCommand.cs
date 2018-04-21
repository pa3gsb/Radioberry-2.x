using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ClockNoise
{
    class SDRCATCommand
    {

        public String setFrequency(int freq)
        {
            String freq11 = String.Format("{0:00000000000}", freq);

            return "ZZFA" + freq11 + ";";    //"ZZFA00014320151;"
        }

        public String setAGCSlow()
        {
            return "ZZGT2;";        
        }

        public String setCWLMode()
        {
            return "ZZMD03;";       
        }

        public String setVar1Width()
        {
            String low = String.Format("{0:0000}", 0);
            String high = String.Format("{0:0000}", 5000);

            return "ZZFI10;";    //"ZZFA00014320151;"
        }

        public String setLow()
        {
            String low = String.Format("{0:00000}", 0);
            return "ZZFL" + low + ";";
        }
        public String setHigh()
        {
            String high = String.Format("{0:00000}", 5000);
            return "ZZFH" + high + ";";
        }


        public String setDIGUMode()
        {
            return "ZZMD07;";
        }

        public String setRXMeterToSignalAverage()
        {
            return "ZZMR1;";
        }

        public String getRxMeter()
        {
            return "ZZRM1;";
        }

    }
}
