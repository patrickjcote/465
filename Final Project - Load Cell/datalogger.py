import threading
from Tkinter import *
import serial
import sys

class Application(Frame):
    
    def quitApp(self):
        self.logging = 0;
        self.quit()

    def stop_logging(self):
        self.logging = 0

    # Start logging thread
    def start_logging(self):
        threading.Thread(target=self._start_logging).start()

    # Logging thread
    def _start_logging(self):
        self.logging = 1
        f = open('current.csv','w')
        f.write('')
        f.close()
        #port = serial.Serial("/dev/ttyUSB0")
        self.fName = self.fileName.get()

        while self.logging:
            self.logging += 1
            #self.dataIn = port.readline()
            self.dataIn = 'Test,'
            f = open('current.csv','a+')
            f2 = open(self.fName,'a+')
            f.write(self.dataIn)
            f2.write(self.dataIn)
            f.close()
            f.close()
            print "Data Logging ",self.logging,": ",self.dataIn
            
        print "Data Logging Stopped to:",self.fName

    # Create GUI elements
    def createWidgets(self):
        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["fg"]   = "red"
        self.QUIT["command"] =  self.quitApp
        self.QUIT.grid(row=4,column=2)

        self.start = Button(self)
        self.start["text"] = "Start",
        self.start["command"] = self.start_logging
        self.start.grid(row=2,column=1,pady=20)

        self.stop = Button(self)
        self.stop["text"] = "Stop",
        self.stop["command"] = self.stop_logging
        self.stop.grid(row=2,column=3,pady=20)

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
app.master.minsize(400,200)
app.mainloop()
root.destroy()

print "'Load-Cell Datalogger' quit sucessfully"
