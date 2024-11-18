#This is a basic script to host a webpage at the IP specified
# By the 'IP_Address' variable

# Import library to create webserver to host webpage
from flask import Flask, render_template
from flask import Flask, render_template, Response,redirect,request, url_for
import itertools
# import time
from camera_pi import Camera
import socket
from threading import Thread
# import sqlite3




app = Flask(__name__)
server_address_1 =  ('127.0.0.2', 8001)
sock_1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_1.bind(server_address_1)



server_address =  ('127.0.0.1', 8000)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(server_address)
sock.listen(5)
connection, address = sock.accept()


#Find the IP Address of your device
#Use the 'ifconfig' terminal command, the address should be in
#the format  "XX.XXX.XXX.XXX"
IP_Address = '10.227.33.73'
PORT = 8080
#Connect the *.html page to the server and run as the default page

info = "2"

@app.route('/')
def index():
    if request.headers.get('accept') == 'text/event-stream':
        def events():
            for i, c in enumerate(itertools.cycle('\|/-')):
                yield "data: %s\n\n" % ("b0c0d0")
                
        return Response(events(), content_type='text/event-stream')
    return render_template('FinalB1.html')


def launch_socket_server(connection, address ):
    global info, frame
    print('Listening...')
    a='b0c0d0'
    while True:        
        info = connection.recv(6).decode("utf-8")
        if info != a and len(info)>0:
            a = info



def gen(camera):
    max_len = 65507
    while True:
        
        # print(len(frame))
        yield (b'--frame\r\n'
            b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
@app.route('/video_feed')
def video_feed():
    return Response(gen(Camera()),mimetype='multipart/x-mixed-replace; boundary=frame')




@app.route('/joydata',methods = ['POST', 'GET'])
def JoystickFunction():
    content_type = request.headers.get('Content-Type')
    if (content_type == 'application/json'):
        json = request.get_json()
        connection.send(str(json).encode('utf-8'))
        return "Content supported\n"
    else:
        return "Content not supported\n"



#Start the server
if __name__ == "__main__":


    t = Thread(target=launch_socket_server,args=(connection,address))
    t.daemon = True
    t.start()


    app.run(debug=True, host=IP_Address, port=PORT, use_reloader=False)
