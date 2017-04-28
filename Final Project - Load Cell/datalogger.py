import time
import threading
import sys
import os
from Tkinter import *
import tkMessageBox
try:
    import matplotlib.pyplot as plt
    import numpy as np
    import serial
except:
    print 'Required Packages not found.'
    print 'Please run: install.cmd'
    print 'Exiting in:',
    for i in range(5,0,-1):
        print str(i),
        time.sleep(1)
    exit(1)

class Application(Frame):

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
            del t[-1]
            del d[-1]
            plt.plot(t,d)
            plt.ylabel('Force [N]')
            plt.xlabel('Time [s]')
            plt.title('Calculated impulse = '+str(np.sum(d))+'Ns')
            plt.show()
        except:
            tkMessageBox.showwarning("Plotting Error","Invalid Data\n\nSee console for more information.")


    # Start calibration
    def calibrate(self):
        try:
            self.port = serial.Serial(self.comPort.get())
        except:
            tkMessageBox.showwarning("Serial Error","Could not open serial port:\n\n"+self.comPort.get())
            return
        tkMessageBox.showinfo("Calibration Step 1","Make sure load cell is unloaded.")
        self.statusLabel["text"] = "Calibrating Unloaded..."
    
        self.calibrating = 0
        self.lowEnd = 0
        while self.calibrating<5:
            self.calibrating += 1
            temp = float(self.port.readline())
            self.lowEnd += temp/50.
            print "Calibrating Unloaded",self.calibrating,":",str(temp)
            self.statusLabel["text"] = "Calibrating Unloaded: "+str(self.calibrating*2)+"%"

        tkMessageBox.showinfo("Calibration Step 2","Make sure load cell is loaded with "+self.maxKg.get()+"Kg.")
        
        self.calibrating = 0
        self.upperEnd = 0
        while self.calibrating<5:
            self.calibrating += 1
            temp = float(self.port.readline())
            self.upperEnd += temp/50.
            print "Calibrating Loaded",self.calibrating,":",str(temp)
            self.statusLabel["text"] = "Calibrating Loaded: "+str(self.calibrating*2)+"%"
        
        self.enableButtons()
        self.statusLabel["text"] = "Calibration Complete"
        tkMessageBox.showinfo("Calibration","Calibration Complete")
        
        self.gain = 9.81*float(self.maxKg.get())/(self.upperEnd-self.lowEnd)
        self.offset = self.gain*self.lowEnd
        calStr = "Calibration function: Force=("+str(self.gain)+")*input-("+str(self.offset)+")"
        self.statusLabel["text"] = calStr
        self.port.close()
        print calStr


    def clearCalibration(self):
        self.gain = 1
        self.offset = 0
        self.statusLabel["text"] = "Calibration Cleared. Recorded data will be raw input."
        self.enableButtons()
         
            
    # Start logging thread
    def start_logging(self):
        try:
            self.port = serial.Serial(self.comPort.get())
        except:
            tkMessageBox.showwarning("Serial Error","Could not open serial port:\n\n"+self.comPort.get())
            return
        try:
            self.fName = self.fileName.get()
            f = open(self.fName,'w')
            f.write('')
            f.close()
        except:
            tkMessageBox.showwarning("File Error","Could not open file:\n\n"+self.fName)
            return
        self.statusLabel["text"] = "Logging..."
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
            
        self.statusLabel["text"] = "Data Logging Complete. Raw data saved to:",self.fName

    def enableButtons(self):
        self.start['state'] = 'normal'
        self.stop['state'] = 'normal'
        self.plot['state'] = 'normal'
        self.calUnLoaded['state'] = 'normal'

    def disableButtons(self):
        self.start['state'] = 'disabled'
        self.stop['state'] = 'disabled'
        self.plot['state'] = 'disabled'
        self.calUnLoaded['state'] = 'disabled'

    # Create GUI elements
    def createWidgets(self):

        self.comLabel = Label(self)
        self.comLabel["text"] = "Serial port:"
        self.comLabel.grid(row=0,column=1,pady=20)
        self.comLabel["anchor"] = "e"
        self.comPort = Entry(self)
        self.comPort.insert(0,"COM1")
        self.comPort.grid(row=0,column=2,pady=20,columnspan=2, padx=10)

        self.fnameL = Label(self)
        self.fnameL["text"] = "Data log filename (.csv):\n**Will overwrite files**"
        self.fnameL.grid(row=1,column=1,pady=20)
        self.fileName = Entry(self)
        self.fileName.insert(0,"log.csv")
        self.fileName.grid(row=1,column=2,pady=20, columnspan=2, padx=10)

        self.plot = Button(self)
        self.plot["text"] = "Plot\nData"
        self.plot["fg"]   = "blue"
        self.plot["command"] = self.plotData
        self.plot.grid(row=1,column=4,pady=20,padx=20)

        self.maxLabel = Label(self)
        self.maxLabel["text"] = "Calibration Mass [Kg]:"
        self.maxLabel.grid(row=2,column=1,pady=20)
        self.maxLabel["anchor"] = "e"
        self.maxKg = Entry(self)
        self.maxKg.insert(0,"5")
        self.maxKg.grid(row=2,column=2,pady=20, columnspan=2)
        
        self.calUnLoaded = Button(self)
        self.calUnLoaded["text"] = "Calibrate"
        self.calUnLoaded["command"] = self.calibrate
        self.calUnLoaded.grid(row=2,column=4,pady=20,padx=20)

        self.start = Button(self)
        self.start["text"] = "Start\nLogging"
        self.start["fg"]   = "green"
        self.start["command"] = self.start_logging
        self.start.grid(row=3,column=1,pady=20,padx=20) 

        self.stop = Button(self)
        self.stop["text"] = "Stop\nLogging"
        self.stop["fg"]   = "red"
        self.stop["command"] = self.stop_logging
        self.stop.grid(row=3,column=2,pady=20,padx=20)

        self.clearCal = Button(self)
        self.clearCal["text"] = "Clear\nCalibration"
        self.clearCal["command"] = self.clearCalibration
        self.clearCal.grid(row=3,column=4,pady=20,padx=20)

        self.statusLabel = Label(self)
        self.statusLabel["text"] = "Calibration is required before data acquisition.\n'Clear Calibration' to run without calibrating and record raw data."
        self.statusLabel["anchor"] = "w"
        self.statusLabel.grid(row=4,column=1,pady=20,columnspan=4)

        self.start['state'] = 'disabled'
        self.stop['state'] = 'disabled'


    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.pack()
        self.createWidgets()
        



root = Tk()
def on_closing():
    if tkMessageBox.askokcancel("Quit", "Are you sure you want to quit?\nAny calibration data and unsaved plots will be lost."):
        plt.close()
        root.destroy()

root.protocol("WM_DELETE_WINDOW", on_closing)
app = Application(master=root)
app.master.title("Rocket Engine Curve and Impulse DAQ - v1.0")
app.master.minsize(600,250)
app.mainloop()

print "datalogger.py quit sucessfully"


