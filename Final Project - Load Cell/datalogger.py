import threading
from Tkinter import *
import serial
import sys
import matplotlib.pyplot as plt
import numpy as np

class Application(Frame):

    portName = "/dev/ttyUSB0"
    
    def quitApp(self):
        self.logging = 0;
        self.quit()

    def stop_logging(self):
        self.logging = 0

    def plotData(self):
        d = np.genfromtxt('current.csv',delimiter=',',dtype=int).tolist()
        t = (np.arange(0.,len(d),1)/10).tolist()
        del t[-1]
        del d[-1]
        d = (self.upperEnd/self.lowEnd)*d - self.lowEnd
        plt.plot(t,d)
        plt.ylabel('Force')
        plt.xlabel('Time [s]')
        plt.show()

    # Start calibration thread
    def calibrate(self, mode):
        threading.Thread(target=self._calibrate, args=(mode,)).start()

    # Calibration thread
    def _calibrate(self, mode):
        self.calibrating = 1
        if mode==1:
            modeName = "unloaded"
        if mode==2:
            modeName = "loaded"
        f = open(modeName+'.cal','w')
        f.write('')
        #port = serial.Serial(self.portName)

        while self.calibrating<50:
            self.calibrating += 1
            #self.dataIn = port.readline()
            self.dataIn = str(self.calibrating)+','
            f.write(self.dataIn)
            print "Calibrating",modeName,self.calibrating,":",self.dataIn
        
        f.close() 
        
        print 'Opening '+modeName+'.cal'
        if mode==1:
            d = np.genfromtxt('unloaded.cal',delimiter=',',dtype=int).tolist()
            del d[-1]
            self.lowEnd = np.mean(d)
            print "Unloaded calibration factor: ",self.lowEnd
        if mode==2:
            d = np.genfromtxt('loaded.cal',delimiter=',',dtype=int).tolist()
            del d[-1]
            self.upperEnd = np.mean(d)
            print "Loaded calibration factor: ",self.upperEnd


         
            
    # Start logging thread
    def start_logging(self):
        threading.Thread(target=self._start_logging).start()

    # Logging thread
    def _start_logging(self):
        self.logging = 1
        f = open('current.csv','w')
        f.write('')
        f.close()
        #port = serial.Serial(self.portName)
        self.fName = self.fileName.get()

        while self.logging:
            self.logging += 1
            #self.dataIn = port.readline()
            self.dataIn = str(self.logging)+','
            f = open('current.csv','a+')
            f2 = open(self.fName,'a+')
            f.write(self.dataIn)
            f2.write(self.dataIn)
            f.close()
            f2.close()
            print "Data Logging ",self.logging,": ",self.dataIn
            
        print "Data Logging Stopped to:",self.fName

    # Create GUI elements
    def createWidgets(self):
        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["command"] =  self.quitApp
        self.QUIT.grid(row=5,column=2,pady=20)

        self.start = Button(self)
        self.start["text"] = "Start"
        self.start["fg"]   = "green"
        self.start["command"] = self.start_logging
        self.start.grid(row=1,column=1,pady=20) 

        self.stop = Button(self)
        self.stop["text"] = "Stop"
        self.stop["fg"]   = "red"
        self.stop["command"] = self.stop_logging
        self.stop.grid(row=1,column=3,pady=20)

        self.calUnLoaded = Button(self)
        self.calUnLoaded["text"] = "Calibrate Unloaded"
        self.calUnLoaded["command"] = lambda: self.calibrate(1)
        self.calUnLoaded.grid(row=3,column=3,pady=20)

        self.calLoaded = Button(self)
        self.calLoaded["text"] = "Calibrate Loaded"
        self.calLoaded["command"] = lambda: self.calibrate(2)
        self.calLoaded.grid(row=3,column=1,pady=20)

        self.plot = Button(self)
        self.plot["text"] = "Plot"
        self.plot["command"] = self.plotData
        self.plot.grid(row=3,column=2,pady=20)

        self.fileName = Entry(self)
        self.fileName.insert(0,"File Name.csv")
        self.fileName.grid(row=1,column=2,pady=20)

    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.pack()
        self.createWidgets()
        

root = Tk()
app = Application(master=root)
app.master.title("Load-Cell Datalogger")
app.master.minsize(450,250)
app.mainloop()
root.destroy()

print "datalogger.py quit sucessfully"
