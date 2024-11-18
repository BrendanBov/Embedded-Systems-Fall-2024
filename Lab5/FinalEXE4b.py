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

app = Flask(__name__)


server_address_1 =  ('127.0.0.2', 8001)
sock_1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_1.bind(server_address_1)



server_address =  ('127.0.0.3', 8002)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(server_address)
sock.listen(5)
connection, address = sock.accept()


#Find the IP Address of your device
#Use the 'ifconfig' terminal command, the address should be in
#the format  "XX.XXX.XXX.XXX"
IP_Address = '10.227.112.139'
PORT = 8080
#Connect the *.html page to the server and run as the default page


@app.route('/')
def index():
    if request.headers.get('accept') == 'text/event-stream':
        def events():
            for i, c in enumerate(itertools.cycle('\|/-')):
                yield "data: %s\n\n" % (info)
        return Response(events(), content_type='text/event-stream')
    return render_template('FinalEXE3.html')


    
def gen(camera):
    max_len = 65507
    frame = ''
    while True:
        # receive image to the client: frame = .....
        frame,_ = sock_1.recvfrom(max_len)
        yield (b'--frame\r\n'
            b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')


@app.route('/video_feed')
def video_feed():
    return Response(gen(Camera()),mimetype='multipart/x-mixed-replace; boundary=frame')


def launch_socket_server(connection):
    global info, frame
    print('Listening...')
    a='b0c0d0'
    while True:        
        info = connection.recv(6).decode("utf-8")
        print('info:', info)
        if info != a and len(info)>0:
            a = info

@app.route('/UpFunction')
def UpFunction():
    print('In UpFunction')
    cmd = 'u'
    connection.send(cmd.encode('utf-8'))  
    return "None"

# define four funtions to handle the left, right, down and stop buttons
@app.route('/LeftFunction')
def LeftFunction():
    print('In LeftFunction')
    cmd = 'l'
    connection.send(cmd.encode('utf-8'))  
    return "None"

@app.route('/RightFunction')
def RightFunction():
    print('In RightFunction')
    cmd = 'r'
    connection.send(cmd.encode('utf-8'))  
    return "None"

@app.route('/DownFunction')
def DownFunction():
    print('In DownFunction')
    cmd = 'd'
    connection.send(cmd.encode('utf-8'))  
    return "None"

@app.route('/StopFunction')
def StopFunction():
    print('In StopFunction')
    cmd = 's'
    connection.send(cmd.encode('utf-8'))  
    return "None"


    

#Start the server
if __name__ == "__main__":
    t = Thread(target=launch_socket_server,args=(connection,))
    t.daemon = True
    t.start()

    app.run(debug=True, host=IP_Address, port=PORT, use_reloader=False)
