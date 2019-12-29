using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.IO;

namespace ClockNoise
{
    class Process
    {

        #region Used for communication outside thread
        public delegate void ProgressEventHandler(ProgressEventArgs e);
        public event ProgressEventHandler ProgressStatus;
        #endregion

        Dictionary<int, string> nf_measurement = new Dictionary<int, string>();

        private bool _stop = false;
        private int startf, stopf, stepf;
        private int actual;

        private ControlPowerSDR ctrlSDR;
        private SDRCATCommand sdrCATCommand;
        private ControlGenerator controlGenerator;

  

        public Process(int startf, int stopf, int step)
        {
            this.startf = startf;
            this.stopf = stopf;
            this.stepf = step;
        }


        public bool getIsStopped()
        {
            return _stop;
        }

        public void startWithoutSignal()
        {
            _stop = false;

            ctrlSDR = new ControlPowerSDR();
            sdrCATCommand = new SDRCATCommand();

            initializeSDRProgram();

            measureNoiseFloor();

            writeToFile();
        }

        public void startWithSignal()
        {
            controlGenerator = new ControlGenerator();
            measureRXSignals();
            writeToFile();

            _stop = true;

            ProgressStatus(new ProgressEventArgs(""));
        }

        public void stop()
        {
            ctrlSDR.stop();
            controlGenerator.stop();
            ctrlSDR = null;
            sdrCATCommand = null;
            controlGenerator = null;
        }

        private void initializeSDRProgram() 
        {
            ProgressStatus(new ProgressEventArgs("Init SDR Program"));
            ctrlSDR.setCommand(sdrCATCommand.setDIGUMode());
            ctrlSDR.setCommand(sdrCATCommand.setVar1Width());
            ctrlSDR.setCommand(sdrCATCommand.setLow());
            ctrlSDR.setCommand(sdrCATCommand.setHigh());
            ctrlSDR.setCommand(sdrCATCommand.setRXMeterToSignalAverage());
            ctrlSDR.setCommand(sdrCATCommand.setAGCSlow());
        }

        private void measureNoiseFloor()
        {
            ProgressStatus(new ProgressEventArgs("Start Measuring Noise Floor"));
            actual = startf;
                // for loop start to end with step 
                while (actual < stopf)
                {
                    ProgressStatus(new ProgressEventArgs("Measure Signal Level for frequency " + actual));
                    
                    ctrlSDR.setCommand(sdrCATCommand.setFrequency(actual));
                    
                    Thread.Sleep(1000);
                    String value = ctrlSDR.getCommand(sdrCATCommand.getRxMeter()).Replace("ZZRM1", "").Replace("dBm;", "");
                    nf_measurement[actual] = value;

                    actual = actual + stepf;
                }
                ProgressStatus(new ProgressEventArgs("nextStep"));
        }

        private void measureRXSignals()
        {
            ProgressStatus(new ProgressEventArgs("Start Measuring RX Signal Level"));

            actual = startf;
            while (actual < stopf)
            {
                ProgressStatus(new ProgressEventArgs("Measure RX Signal Level for frequency " + actual));
                ctrlSDR.setCommand(sdrCATCommand.setFrequency(actual));
                ctrlSDR.setCommand(sdrCATCommand.setCWLMode());
                controlGenerator.setFrequency(actual - 200);
                Thread.Sleep(100);
                String value = ctrlSDR.getCommand(sdrCATCommand.getRxMeter()).Replace("ZZRM1", "").Replace("dBm;", "");
               
                nf_measurement[actual] = nf_measurement[actual] + " ; " +value;

                actual = actual + stepf;
            }

        }

        private void writeToFile()
        {
            List<string> measurements = new List<string>();
            // Copy to list
            string context = "#Clock Noise measurement";
            //measurements.Add(context);
            context = "#Frequency ; Level(dBm)";
            measurements.Add(context);
            foreach(var item in nf_measurement) {
                measurements.Add(item.Key + " ; " + item.Value + " ; ");
            }
            //Convert list to array and write to file
            string path = @"c:\temp\measurement.txt";
             if (File.Exists(path)) {
                 File.Delete(path);
             }
             
             File.WriteAllLines(path, measurements.ToArray(), Encoding.ASCII);
           
        }

        private string  calculateNoisFloor(String values)
        {
            string result = "";
            
            string noSignalLevel = "";
            int index = values.IndexOf(";");
            noSignalLevel = values.Substring(0, index);
            noSignalLevel = noSignalLevel.Trim();
            double s1 = Double.Parse(noSignalLevel);

            string signalLevel = "";
            signalLevel = values.Substring(++index);
            signalLevel = signalLevel.Trim();
            double s2 = Double.Parse(signalLevel);

            double nf = s1; // - (s2 - this.power);

            result = nf.ToString();

            return result;
        }
    }
}
