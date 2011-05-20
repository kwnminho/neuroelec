import serial

# Setup the Serial Object
ser = serial.Serial()
# Set the Serial Port to use
ser.setPort("COM5")
# Set the Baudrate (Arduino Sketch is expecting 57600 for smooth transitions in the GUI)
ser.baudrate = 57600
# Open the Serial Connection
ser.open()
loopVar = True

if (ser.isOpen()):
  # Start a main loop
  while (loopVar):
    # Prompt for Loop Delay  (Internal loop delay, used to control the speed of the HSV loop)
    delayVal=input('Delay value:')
    ser.write("f" + chr(int(delayVal)))
    # Check if user wants to end
    loopCheck=raw_input('Loop (y/N):')
    if (loopCheck == "N"):
      loopVar = False
  # After loop exits, close serial connection
  ser.close()