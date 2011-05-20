import pygtk
import gtk
import serial
import time

pygtk.require('2.0')

#########################################
#
#  This program was written by John Meichle on 2/25/2010
#  as a GTK GUI Front end to an Arduino Sketch that was written
#  to control an RGB LED. This could be a single LED, or a 
#  multiple LED Array. This program interfaces with the Arduino
#  via a serial connection, and uses simple byte commands to 
#  set the color values in the Arduino.
#
#  The Python code is not commented well, as it is old code I had that 
#  uses the UIManager to build the GTK GUI. If anyone has any questions
#  I can provide a commented copy to use. 
#
#  The HSV Loop function is built into the Arduino Sketch, and
#  has a simple serial command called to start the loop. The Sketch
#  activly monitors the serial connection during the loop to check for 
#  new RGB values.
#
#  Serial Command set for the Arduino Sketch:
#
#  (Ascii r)(Red Value)                  - Sets the Red RGB value
#  (Ascii g)(Green Value)                - Sets the Green RGB value
#  (Ascii b)(Blue Value)                 - Sets the Blue RGB value
#  (Ascii f)(ms Delay)                   - Starts the HSV loop with internal delay (ms) passed
#
#########################################
class ArduinoRGB:
    serialPort = "COM8"
    serialBaud = 57600
    ui = '''<ui>
    <menubar name="MenuBar">
      <menu action="mnuFile">
        <menuitem action="funConnect"/>
        <menuitem action="funQuit"/>
      </menu>
      <menu action="mnuRGB">
        <menuitem action="funLoop"/>
        <menuitem action="funSetRed"/>
        <menuitem action="funSetGreen"/>
        <menuitem action="funSetBlue"/>
        <separator/>
        <menuitem action="funSetWhite"/>
        <menuitem action="funSetSoftWhite"/>
      </menu>
      <menu action="mnuHelp" expand="true">
        <menuitem action="funAbout"/>
      </menu>
    </menubar>
    <toolbar name="Toolbar">
      <toolitem action="funConnect"/>
      <toolitem action="funQuit"/>
      <separator/>
      <toolitem action="funLoop"/>
      <separator/>
      <toolitem action="funSetRed"/>
      <toolitem action="funSetGreen"/>
      <toolitem action="funSetBlue"/>
      <separator/>
      <toolitem action="funSetWhite"/>
      <toolitem action="funSetSoftWhite"/>
    </toolbar>
    </ui>'''
    serialConnect = False
      
    def aboutButton(self,widget):
        about = gtk.AboutDialog()
        about.set_name("Arduino-RGB")
        about.set_version("0.1")
        about.set_copyright("(c) John Meichle")
        about.set_comments("Arduino-RGB is a python frontend for an Arduino based RGB LED Controller.")
        about.set_website("http://www.Jkmcustoms.com/arduino")
        about.run()
        about.destroy() 
        
    def __init__(self):
   
        self.window = gtk.Window()
        self.window.connect('destroy', lambda w: gtk.main_quit())
        self.window.set_size_request(600, 325)
        self.window.set_title("Arduino-RGB")
        icon = self.window.render_icon(gtk.STOCK_CONVERT, gtk.ICON_SIZE_BUTTON)
        self.window.set_icon(icon)

        self.vbox = gtk.VBox()
        self.window.add(self.vbox)
        self.uimanager = gtk.UIManager()
        accelgroup = self.uimanager.get_accel_group()
        self.window.add_accel_group(accelgroup)


        genericactions = gtk.ActionGroup('GenericActions')
        self.genericactions = genericactions
        genericactions.add_actions([

                                 ('mnuRGB', None, '_Image'),
                                 ('funLoop', gtk.STOCK_REFRESH, 'HSV _Loop', None, 'HSV Loop', self.hsvLoop),
                                 ('funSetRed', gtk.STOCK_ADD, 'Set _Red', None, 'Set Red', self.setRed),
                                 ('funSetGreen', gtk.STOCK_ADD, 'Set _Green', None, 'Set Green', self.setGreen),
                                 ('funSetBlue', gtk.STOCK_ADD, 'Set _Blue', None, 'Set Blue', self.setBlue),
                                 ('funSetWhite', gtk.STOCK_ADD, 'Set _White', None, 'Set White', self.setWhite),
                                 ('funSetSoftWhite', gtk.STOCK_ADD  , 'Set _Soft White', None, 'Set Soft White', self.setSoftWhite),

                                 ])
        self.uimanager.insert_action_group(genericactions, 0)
        
        serialactions = gtk.ActionGroup('SerialActions')
        self.serialactions = serialactions
        serialactions.add_actions([
                                 ('mnuHelp', None, '_Help'),                             
                                 ('funAbout', gtk.STOCK_ABOUT, '_About', None, 'About', self.aboutButton),
                                 ('funQuit', gtk.STOCK_QUIT, '_Quit', None, 'Quit the Program', self.quit),
                                 ('mnuFile', None, '_File'),
                                 ('funConnect', gtk.STOCK_REFRESH, 'Serial _Connect', None, 'Serial Connect', self.setup_serial)
                                 ])
        self.uimanager.insert_action_group(serialactions, 0)
        
        
        
        self.uimanager.add_ui_from_string(self.ui)
        self.menubar = self.uimanager.get_widget('/MenuBar')
        self.vbox.pack_start(self.menubar, False)
        self.toolbar = self.uimanager.get_widget('/Toolbar')
        self.vbox.pack_start(self.toolbar, False)

        self.content = gtk.VBox()

        headerVbox = gtk.VBox(True,0)
        headerLabel1 = gtk.Label("RGB Control App for Arduino")
        headerVbox.pack_start(headerLabel1)
      
        mainHbox = gtk.HBox(True, 0)

        loopVbox = gtk.VBox(True, 0)
        fadeLabel = gtk.Label("Fade Delay (variable on arduino (ms): ")  
        self.fadeScale = gtk.HScale()
        self.fadeScale.set_name("fade")
        self.fadeScale.set_range(5, 60)
        self.fadeScale.set_digits(0)
        self.fadeScale.set_size_request(160, 35)
        self.fadeScale.set_value(35)
        
        self.hsvButton = gtk.Button("HSV Loop")
        self.hsvButton.set_size_request(80, 35)
        self.hsvButton.connect("clicked", self.hsvLoop)
        loopVbox.pack_start(fadeLabel)
        loopVbox.pack_start(self.fadeScale)
        loopVbox.pack_end(self.hsvButton)
        
        

        rHbox = gtk.HBox(True,0)
        rLabel = gtk.Label("Red: ")
        rHbox.pack_start(rLabel, expand=False, fill=False)   
             
        self.rScale = gtk.HScale()
        self.rScale.set_name("red")
        self.rScale.set_range(0, 255)
        self.rScale.set_increments(1, 10)
        self.rScale.set_digits(0)
        self.rScale.set_size_request(160, 35)
        self.rScale.connect("value-changed", self.slider_changed)
        rHbox.pack_end(self.rScale)
        
        gHbox = gtk.HBox(True,0)
        gLabel = gtk.Label("Green: ")
        gHbox.pack_start(gLabel, expand=False, fill=False)   
        
        self.gScale = gtk.HScale()
        self.gScale.set_name("green")
        self.gScale.set_range(0, 255)
        self.gScale.set_increments(1, 10)
        self.gScale.set_digits(0)
        self.gScale.set_size_request(160, 35)
        self.gScale.connect("value-changed", self.slider_changed)
        gHbox.pack_end(self.gScale)
        
        bHbox = gtk.HBox(True,0)       
        bLabel = gtk.Label("Blue: ")
        bHbox.pack_start(bLabel, expand=False, fill=False)   
        
        self.bScale = gtk.HScale()
        self.bScale.set_name("blue")
        self.bScale.set_range(0, 255)
        self.bScale.set_increments(1, 10)
        self.bScale.set_digits(0)
        self.bScale.set_size_request(160, 35)
        self.bScale.connect("value-changed", self.slider_changed)
        bHbox.pack_end(self.bScale)
               
        rgbVbox = gtk.VBox(True,0)

        rgbVbox.pack_start(headerVbox)
        rgbVbox.pack_start(rHbox)
        rgbVbox.pack_start(gHbox)
        rgbVbox.pack_end(bHbox)
        
        mainHbox.pack_start(loopVbox)
        mainHbox.pack_end(rgbVbox)

        self.content.pack_end(mainHbox)
        self.disableAll()
        self.vbox.pack_start(self.content)
        self.window.show_all()
        return
        
    def disableAll(self):
        self.rScale.set_sensitive(False)
        self.gScale.set_sensitive(False)
        self.bScale.set_sensitive(False)
        self.fadeScale.set_sensitive(False)
        self.hsvButton.set_sensitive(False)
        self.genericactions.set_sensitive(False)
        
    def enableAll(self):
        self.rScale.set_sensitive(True)
        self.gScale.set_sensitive(True)
        self.bScale.set_sensitive(True)
        self.fadeScale.set_sensitive(True)
        self.hsvButton.set_sensitive(True)
        self.genericactions.set_sensitive(True)
        
    def setRed(self, b):
        self.rScale.set_value(255)
        self.gScale.set_value(0)
        self.bScale.set_value(0)
        self.serial_write(255,0,0)
        
    def setGreen(self, b):
        self.rScale.set_value(0)
        self.gScale.set_value(255)
        self.bScale.set_value(0)
        self.serial_write(0,255,0)
        
    def setBlue(self, b):
        self.rScale.set_value(0)
        self.gScale.set_value(0)
        self.bScale.set_value(255)
        self.serial_write(0,0,255)
    
    def setWhite(self, b):
        self.rScale.set_value(255)
        self.gScale.set_value(255)
        self.bScale.set_value(255)
        self.serial_write(255,255,255)
        
    def setSoftWhite(self, b):
        self.rScale.set_value(223)
        self.gScale.set_value(161)
        self.bScale.set_value(59)
        self.serial_write(223,161,59)
        
    def slider_changed(self, widget):
        val = widget.get_value()
        name = widget.get_name()
        if name == "red":
            self.ser.write("r" + chr(int(val)))
        elif name == "green":
            self.ser.write("g" + chr(int(val)))
        elif name == "blue":
            self.ser.write("b" + chr(int(val)))
        else: 
            print "ERROR: Invalid widget name, in on_changed function"
    
    def serial_write(self, rval, gval, bval):
        self.ser.write("r" + chr(int(rval)))
        self.ser.write("g" + chr(int(gval)))
        self.ser.write("b" + chr(int(bval)))
    
    def hsvLoop(self, b): 
      delay = self.fadeScale.get_value()
      self.ser.write("f" + chr(int(delay)))
      time.sleep(.015)
      self.ser.write("f" + chr(int(delay)))


    def setup_serial(self, widget):
      if (self.serialConnect == False):
        self.ser = serial.Serial()
        self.ser.setPort(self.serialPort)
        self.ser.baudrate = self.serialBaud
        self.ser.open()
        if (self.ser.isOpen()):
          self.serialConnect = True
          self.enableAll()
        else:
          self.disableAll()
          self.serialConnect = False
          print "ERROR: Serial Unable to connect on " + ser.portstr
        
    def quit(self, b):
        print 'Closing Serial Connection...'
        self.ser.close()
        print 'Quitting program'
        gtk.main_quit()

if __name__ == '__main__':
    main = ArduinoRGB()
    gtk.main()
