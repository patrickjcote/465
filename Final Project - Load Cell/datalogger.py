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

   
    def quitApp(self):
        self.logging = 0;
        self.quit()
        

    def stop_logging(self):
        self.logging = 0

    def plotData(self):
        try:
            d = np.genfromtxt("Logs\\"+self.fileName.get(),delimiter="\n", autostrip=True).tolist()
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
            d = [9.81*x for x in d] # Kg to Newtons
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
                        "After a 'Clear calibration' plotting will display the raw data.\n\n"
                        "'Plot Data' will plot the data from the filename in the text box. "
                        "Data is assumed to be sampled at 10sps (The default frequency "
                        "of the A/D).")
        tkMessageBox.showinfo("Help",self.helpMessage)

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
        
        self.gain = float(self.maxKg.get())/(self.upperEnd-self.lowEnd)
        self.offset = self.gain*self.lowEnd
        calStr = "Calibration function: Force=("+str(self.gain)+")*input-("+str(self.offset)+")"
        self.statusLabel["text"] = calStr
        self.port.close()
        print calStr


    def clearCalibration(self):
        self.gain = 1
        self.offset = 0
        print "Calibration factor: y=",self.gain,"x -",self.offset
        self.statusLabel["text"] = "Calibration Cleared. Recorded output will be raw data."
        self.enableButtons()
         
            
    # Start logging thread
    def start_logging(self):
        if not os.path.exists('Logs\\'):
            os.makedirs('Logs\\')
        try:
            self.port = serial.Serial(self.comPort.get())
        except:
            tkMessageBox.showwarning("Serial Error","Could not open serial port:\n\n"+self.comPort.get())
            return
        try:
            self.fName = "Logs\\"+self.fileName.get()
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
        self.comPort.grid(row=0,column=2,pady=20,columnspan=2)
        
        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["command"] =  self.quitApp
        self.QUIT.grid(row=0,column=4,pady=20)

        self.fnameL = Label(self)
        self.fnameL["text"] = "Data log file name (.csv):\n**Will overwrite files**"
        self.fnameL.grid(row=1,column=1,pady=20)
        self.fileName = Entry(self)
        self.fileName.insert(0,"Filename.csv")
        self.fileName.grid(row=1,column=2,pady=20, columnspan=2)
        
        self.helpB = Button(self)
        self.helpB["text"] = "Help"
        self.helpB["command"] = self.helpBox
        self.helpB.grid(row=1,column=4,pady=20,padx=20)

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

        self.plot = Button(self)
        self.plot["text"] = "Plot\nData"
        self.plot["fg"]   = "blue"
        self.plot["command"] = self.plotData
        self.plot.grid(row=3,column=3,pady=20,padx=20)

        self.clearCal = Button(self)
        self.clearCal["text"] = "Clear\nCalibration"
        self.clearCal["command"] = self.clearCalibration
        self.clearCal.grid(row=3,column=4,pady=20,padx=20)

        self.statusLabel = Label(self)
        self.statusLabel["text"] = "Calibration is required before data acquisition."
        self.statusLabel["anchor"] = "w"
        self.statusLabel.grid(row=4,column=1,pady=20,columnspan=4)

        self.start['state'] = 'disabled'
        self.stop['state'] = 'disabled'
        self.plot['state'] = 'disabled'


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


