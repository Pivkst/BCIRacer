import socket
import keyboard
import threading
from time import sleep

#Start game
import os, sys
scriptLocation = os.path.dirname(os.path.abspath(sys.argv[0]))
executableLocation = os.path.join(os.path.dirname(scriptLocation), "debug")
os.chdir(executableLocation)
gameThread = threading.Thread(target=os.system, args=("BCIGAME.exe aligned",))
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
gameStarted = False

def buttonState():
    if left and right:
        return "both"
    elif left:
        return "left"
    elif right:
        return "right"
    else:
        return "none"

#Set up key listener
def checkKeys(event):
    global left
    global right
    global gameStarted
    #Wait for SPACE to start the game
    if not gameStarted:
        if event.name == 'space':
            gameStarted = True
            send("start")
            print("Starting game")
    #Send key state based on input
    else:
        if event.name == 'left':
            if event.event_type == keyboard.KEY_DOWN:
                left = True
            else:
                left = False
        elif event.name == 'right':
            if event.event_type == keyboard.KEY_DOWN:
                right = True
            else:
                right = False
        else:
            return
        send(buttonState())

keyboard.hook(checkKeys)

#Set up response reciever
def reciever():
    while True:
        global sock
        global gameStarted
        try:
            #print("GAME: "+recieve())
            recieve();
        except ConnectionResetError:
            gameStarted = False;
            print("Couldn't connect, retrying in 10 seconds")
            sleep(10)
            send("hello")
            #print("Connected?")

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
