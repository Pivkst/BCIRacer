import socket
import keyboard
import threading
from time import sleep

#Start game
import os, sys
scriptLocation = os.path.dirname(os.path.abspath(sys.argv[0]))
executableLocation = os.path.join(os.path.dirname(scriptLocation), "debug")
os.chdir(executableLocation)
gameThread = threading.Thread(target=os.system, args=("BCIGAME.exe",))
gameThread.start()

#Socket setup
IP = "localhost"
PORT = 1235
PORT_GAME = 1234

print("Press SPACE to start game and ESC to quit")
print("Target IP: ", IP)
print("Target port: "+str(PORT_GAME))

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((IP, PORT))

def send(text):
    sock.sendto(bytes(text, "utf-8"), (IP, PORT_GAME))

def recieve():
    data = sock.recv(1024)
    return str(data, "utf-8")

#Logic
left = False
right = False
paused = True
connected = False

def buttonState():
    if left != right:
        if left:
            return "left"
        else:
            return "right"
    else:
        return "none"

#Set up key listener
def checkKeys(event):
    global left
    global right
    global paused
    #Start or pause
    if event.name == 'space' and event.event_type == keyboard.KEY_DOWN:
        if paused:
            paused = False
            send("start")
            print("Starting")
        else:
            paused = True
            send("pause")
            print("Pausing")
    #Send arrow key state based on input
    elif event.name == 'left':
        if event.event_type == keyboard.KEY_DOWN:
            left = True
        else:
            left = False
        send(buttonState())
    elif event.name == 'right':
        if event.event_type == keyboard.KEY_DOWN:
            right = True
        else:
            right = False
        send(buttonState())
    else:
        return    

keyboard.hook(checkKeys)

#Set up response reciever
def reciever():
    global sock
    global connected
    while True:
        try:
            recieve();
            if not connected:
                print("Connected")
            connected = True
        except ConnectionResetError:
            if connected:
                print("Connection reset")
            connected = False
            sleep(1)
            send("hello")

recieveThread = threading.Thread(target=reciever, daemon=True)
recieveThread.start()
sleep(1)
send("hello")

#Wait until ESC is pressed
keyboard.wait('esc')

#Cleanup
send("end")
keyboard.unhook_all()
gameThread.join()
