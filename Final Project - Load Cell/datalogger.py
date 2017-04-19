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
    
    def start_logging(self):
        self.logging = 1
        while self.logging:
            self.logging = self.logging +1
            print "Data Logging: ",self.logging
        print "Data Logging Stopped"
        

    def start_log(self):
        threading.Thread(target=self.start_logging).start()

    def createWidgets(self):
        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["fg"]   = "red"
        self.QUIT["command"] =  self.quitApp
        self.QUIT.grid(row=4,column=2)

        self.start = Button(self)
        self.start["text"] = "Start",
        self.start["command"] = self.start_log
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
app.master.minsize(400,400)
app.mainloop()
root.destroy()

print "Load-Cell Datalogger quit sucessfully"
