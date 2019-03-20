import pygatt
adapter = pygatt.GATTToolBackend()
try:
    adapter.start()
    device = adapter.connect("00:1D:40:0B:00:45")
    while(True):
        value = device.char_read("00002001-0000-1000-8000-00805f9b34fb")
        print(value)
finally:
    adapter.stop()
