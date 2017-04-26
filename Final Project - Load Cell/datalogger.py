import threading
from Tkinter import *
import tkMessageBox
import serial
import sys
import matplotlib.pyplot as plt
import numpy as np

class Application(Frame):

    portName = 'COM26'
    maxKg = 5
    
    def quitApp(self):
        self.logging = 0;
        self.quit()
        

    def stop_logging(self):
        self.logging = 0

    def plotData(self):
        try:
            d = np.genfromtxt(self.fileName.get(),delimiter="\n", autostrip=True).tolist()
        except:
            tkMessageBox.showwarning("Plotting Error",self.fileName.get()+" does not exist or is being used by another program")
            return
        
        t = (np.arange(0.,len(d),1)/10).tolist()
        
        try:
            d = [self.gain*x-self.offset for x in d]
        except:
            tkMessageBox.showwarning("Calibration Error","Calibration must be set before "
                                             " plotting.\n\nSee 'Help' for more information.")
            return
        try:
            del t[-1]
            del d[-1]
            d = [9.81*x for x in d] # Kd to Newtons
            plt.plot(t,d)
            plt.ylabel('Force [N]')
            plt.xlabel('Time [s]')
            plt.show()
        except:
            tkMessageBox.showwarning("Plotting Error","Invalid Data\n\nSee console for more information.")

    def helpBox(self):
        self.helpMessage = (
                        "Before plotting, calibration must be set.\n\n"
                        "The calibrate buttons store 5 seconds of data "
                        "in a corresponding .cal file.\n\n"
                        "'Calculate Calibration' "
                        "averages the calibration data and calculates a gain and "
                        "offset to convert raw data to Newtons of force.\n\n"
                        "'Clear calibration' sets the gain to 1 and offset to 0. "
                        "This results in seeing the raw data from the ADC.\n\n"
                        "'Plot Data' will plot the data from the filename in the text box. "
                        "Data is assumed to be sampled at 10sps (The default frequency "
                        "of the HX711 ADC).")
        tkMessageBox.showinfo("Help",self.helpMessage)

    # Start calibration thread
    def calibrate(self, mode):
        try:
            self.port = serial.Serial(self.portName)
        except:
            tkMessageBox.showwarning("Serial Error","Could not open serial port:\n\n"+self.portName)
            return
        threading.Thread(target=self._calibrate, args=(mode,)).start()

    # Calibration thread
    def _calibrate(self, mode):
        self.calibrating = 0
        if mode==1:
            modeName = "unloaded"
        if mode==2:
            modeName = "loaded"
        f = open(modeName+'.cal','w')
        f.write('')

        while self.calibrating<50:
            self.calibrating += 1
            self.dataIn = self.port.readline()
            f.write(self.dataIn)
            print "Calibrating",modeName,self.calibrating,":",str(self.dataIn).rstrip('\n')
        
        f.close()
        print modeName,"Calibration Complete"
        
    def setCalibrationFactor(self):
        # Unloaded Calibration
        try:
            d = np.genfromtxt('unloaded.cal',delimiter='\n',dtype=int).tolist()
        except:
            tkMessageBox.showwarning("Calibration Error","unloaded.cal does not exist or is being used by another program.\n\n"
                                                         "Try running 'Calibrate Unloaded'.")
            return
        try:
            del d[-1]
            self.lowEnd = np.mean(d)
            print "Unloaded calibration factor: ",self.lowEnd
        except:
            tkMessageBox.showwarning("Calibration Error","Invalid calibration data file.\n\n"
                                                         "Try running 'Calibrate Unloaded'.")
            return
        # Loaded Calibration
        try:
            d = np.genfromtxt('loaded.cal',delimiter=',',dtype=int).tolist()
        except:
            tkMessageBox.showwarning("Calibration Error","unloaded.cal does not exist or is being used by another program\n\n"
                                                         "Try running 'Calibrate Loaded'.")
            return
        try:
            del d[-1]
            self.upperEnd = np.mean(d)
            print "Loaded calibration factor: ",self.upperEnd
        except:
            tkMessageBox.showwarning("Calibration Error","Invalid calibration data file.\n\n"
                                                         "Try running 'Calibrate Loaded'.")
            return
        self.gain = self.maxKg/(self.upperEnd-self.lowEnd)
        self.offset = self.gain*self.lowEnd
        print "Calibration function: y =",self.gain,"x - (",self.offset,")"


    def clearCalibration(self):
        self.gain = 1
        self.offset = 0
        print "Calibration factor: y=",self.gain,"x -",self.offset
         
            
    # Start logging thread
    def start_logging(self):
        self.fName = self.fileName.get()
        try:
            f = open(self.fName,'w')
            f.write('')
            f.close()
        except:
            tkMessageBox.showwarning("File Error","Could not open file:\n\n"+self.fName)
            return
        try:
            self.port = serial.Serial(self.portName)
        except:
            tkMessageBox.showwarning("Serial Error","Could not open serial port:\n\n"+self.portName)
            return
        threading.Thread(target=self._start_logging).start()

    # Logging thread
    def _start_logging(self):
        self.logging = 1
        self.loggingCount = 0

        while self.logging:
            self.loggingCount += 1
            self.dataIn = self.port.readline()
            f = open(self.fName,'a+')
            f.write(self.dataIn)
            f.close()
            logMessage = "("+str(self.loggingCount/10.)+"s) - Raw: "+str(self.dataIn).rstrip('\n')
            print logMessage
            
        print "Data Logging Stopped to:",self.fName

    # Create GUI elements
    def createWidgets(self):

        self.fnameL = Label(self)
        self.fnameL["text"] = "Data log file name (.csv):\n**Will overwrite files**"
        self.fnameL.grid(row=1,column=1,pady=20)
        self.fileName = Entry(self)
        self.fileName.insert(0,"Filename.csv")
        self.fileName.grid(row=1,column=2,pady=20, columnspan=2)

        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["command"] =  self.quitApp
        self.QUIT.grid(row=1,column=4,pady=20)

        self.start = Button(self)
        self.start["text"] = "Start\nLogging"
        self.start["fg"]   = "green"
        self.start["command"] = self.start_logging
        self.start.grid(row=2,column=1,pady=20,padx=20) 

        self.stop = Button(self)
        self.stop["text"] = "Stop\nLogging"
        self.stop["fg"]   = "red"
        self.stop["command"] = self.stop_logging
        self.stop.grid(row=2,column=2,pady=20,padx=20)

        self.plot = Button(self)
        self.plot["text"] = "Plot\nData"
        self.plot["fg"]   = "blue"
        self.plot["command"] = self.plotData
        self.plot.grid(row=2,column=3,pady=20,padx=20)

        self.helpB = Button(self)
        self.helpB["text"] = "Help"
        self.helpB["command"] = self.helpBox
        self.helpB.grid(row=2,column=4,pady=20,padx=20)

        self.calUnLoaded = Button(self)
        self.calUnLoaded["text"] = "Calibrate\nUnloaded"
        self.calUnLoaded["command"] = lambda: self.calibrate(1)
        self.calUnLoaded.grid(row=4,column=1,pady=20,padx=20)

        self.calLoaded = Button(self)
        self.calLoaded["text"] = "Calibrate\nLoaded"
        self.calLoaded["command"] = lambda: self.calibrate(2)
        self.calLoaded.grid(row=4,column=2,pady=20,padx=20)

        self.calcCal = Button(self)
        self.calcCal["text"] = "Calculate\nCalibration"
        self.calcCal["command"] = self.setCalibrationFactor
        self.calcCal.grid(row=4,column=3,pady=20,padx=20)

        self.clearCal = Button(self)
        self.clearCal["text"] = "Clear\nCalibration"
        self.clearCal["command"] = self.clearCalibration
        self.clearCal.grid(row=4,column=4,pady=20,padx=20)


    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.pack()
        self.createWidgets()
        

root = Tk()
app = Application(master=root)
app.master.title("Load-Cell Datalogger")
app.master.minsize(600,250)
app.mainloop()
root.destroy()

print "datalogger.py quit sucessfully"
