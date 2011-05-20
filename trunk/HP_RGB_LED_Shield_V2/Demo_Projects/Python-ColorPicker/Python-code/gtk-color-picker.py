import gtk, sys, serial

#--\\ Serial Config //--#
ser = serial.Serial()
ser.setPort("COM8")
ser.baudrate = 57600
#ser.baudrate = 115200
ser.open()
#--\\ GTK Config //--#
w = gtk.Window()
c = gtk.ColorSelection()
w.add(c)
w.show_all()

#--\\ Main Code //--#
def callback(*args):
    color=c.get_current_color()
    ser.write("r" + chr(int(color.red/257)))
    ser.write("g" + chr(int(color.green/257)))
    ser.write("b" + chr(int(color.blue/257)))

c.connect('color-changed', callback)
w.connect('destroy', sys.exit)

gtk.main()

 
