import pygatt
adapter = pygatt.GATTToolBackend()
try:
    adapter.start()
    # connect to Hexiwear through MAC address
    device = adapter.connect("00:1D:40:0B:00:45")
    while(True):
        # keep reading data from UUID of acceleration serivce 
        value = device.char_read("00002001-0000-1000-8000-00805f9b34fb")
        print(value)
finally:
    # disconnect from Hexiwear 
    adapter.stop()
