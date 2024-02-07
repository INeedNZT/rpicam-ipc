from flask import Flask, render_template
from flask_sock import Sock
from simple_websocket import ConnectionClosed
from time import sleep
from threading import Thread
import json
from ctypes import *
import ctypes
import os

# load the dynamic library
rpicam_ipc = CDLL('build/rpicam_ipc.so')
rpicam_ipc.GetFrameBuffer.restype=POINTER(c_uint8)
ctl = rpicam_ipc.Create()



app = Flask(__name__)
sock = Sock(app)

# shared variables for rpicam
rpicam_socks = []


@app.route('/')
def home_template():
    return render_template('index.html')


@app.route('/rpicam')
def rpicam_worker_template():
    return render_template('rpicam.html')


@sock.route('/rpicam')
def rpicam_sock(sock):
    global rpicam_socks

    options = {
        "width": 1920,
        "height": 1080,
    }

    def broadcast_frame():
        while True:
            print("Broadcasting frame...")
            size = c_int()
            frame_buffer = rpicam_ipc.GetFrameBuffer(ctl, byref(size))
            frame_data = [frame_buffer[i] for i in range(size.value)]
            frame_bytearray = bytearray(frame_data)
            frame_bytes = bytes(frame_bytearray)
            broadcast(rpicam_socks, frame_bytes)
    
    def start_feed():
        print("Start Feeding...")
        broadcast_frame()
    
    new_client(sock, rpicam_socks, options, start_feed)


def broadcast(socks, data):
    for sock in socks:
        try:
            if not sock.pause:
                sock.send(data)
        except Exception as e:
            print(e)
            # ignore the disconnected clients or other issues
            pass

def new_client(sock, socks, options, start_feed):
    try:
        # add an attribute to pause the socket
        # default is False
        sock.pause = False
        socks.append(sock)
        print("New client connected!")

        sock.send(json.dumps({
            "action": "init",
            "width": options["width"],
            "height": options["height"]
        }))

        while True:
            # waiting for message from ws client
            message = sock.receive()
            print(message)
            action = message.split(' ')[0]
            
            if action == "REQUESTSTREAM":
                sock.pause = False
                start_feed()
            elif action == "STOPSTREAM":
                # pause just for this socket
                sock.pause = True

    except ConnectionClosed as e:
        socks.remove(sock)
        print("Client disconnected!")
            
def start_rpicam():
    rpicam_ipc.Start(ctl)

if __name__ == "__main__":
    thread = Thread(target=start_rpicam)
    thread.start()
    # You can add host='0.0.0.0' to make it accessible from other devices
    app.run(host='0.0.0.0')