#This is a basic script to host a webpage at the IP specified
# By the 'IP_Address' variable

# Import library to create webserver to host webpage
from flask import Flask, render_template
import os
app = Flask(__name__)



#Find the IP Address of your device
#Use the 'ifconfig' terminal command, the address should be in
#the format  "XX.XXX.XXX.XXX"
IP_Address = '10.227.117.20'
PORT = 8080
#Connect the *.html page to the server and run as the default page
@app.route('/')
def index():
    return render_template('FinalEXE1b.html')

#Start the server
if __name__ == "__main__":
    app.run(debug=True, host=IP_Address, port=PORT, use_reloader=False)
