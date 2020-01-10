from flask import Flask
from flask import Flask, request, render_template, redirect, url_for
import serial
import threading as th
import time

ser = serial.Serial()
ser.baudrate = 115200
ser.port = 'COM8'       #Check your LoRa port,or it will not work!

status_fly = 0
response = ""
bty="-1"
app = Flask(__name__, template_folder='template')
def thread_receive():
    global bty
    while(True):
        if ser.isOpen() != True:
            ser.open()
                    
        while not ser.in_waiting:
            pass
        
        while ser.in_waiting:
            data_raw = ser.readline()
            try:
                data = data_raw.decode()
            except:
                thread1 = th.Thread(target=thread_receive)
                thread1.start()
            if data.find("+RCV=1006,10,") != -1:
                print('Battery:', data[13:])
                bty=data[13:]
                
def thread_askforbty():
    global askforbry
    while askforbry == 1:
        askforbry = 0
        Send_Command('6')
        #response = '問電量已送出.'
        thread2 = th.Timer(5,thread_askforbty)
        thread2.start()
        
def Send_Command(cmd=0):
    print("Command send: "+cmd)
    ser.write(b'AT+SEND=0,10,' + cmd.encode('utf-8') + b'\r\n')

@app.route('/')
def execute():
    return render_template('post2.html')

@app.route('/<cmd>')
def command(cmd=None):
    global response
    global bty
    if cmd == "take":
        global status_fly
        Send_Command(('2','1')[status_fly==0])
        response = ("降落","起飛")[status_fly==0] + '已送出.'
        status_fly = (0,1)[status_fly==0]
    elif cmd == "seta":
        Send_Command('3')
        response = '指令A已送出.'
    elif cmd == "setb":
        Send_Command('5')
        response = '指令B已送出.'
    elif cmd == "battery":
        Send_Command('6')
        response = bty
    elif cmd == "setfor":
        Send_Command('7')
        response = '前進已送出.'
    elif cmd == "setleft":
        Send_Command('8')
        response = '左轉已送出.'
    elif cmd == "setright":
        Send_Command('9')
        response = '右轉已送出.'
    elif cmd == "setback":
        Send_Command('4')
        response = '後退已送出.'
    elif cmd == "setup":
        Send_Command('11')
        response = '上升已送出.'
    elif cmd == "setdown":
        Send_Command('12')
        response = '下降已送出.'

    return response, 200, {'Content-Type': 'text/plain'}

thread1 = th.Thread(target=thread_receive)
if __name__ == '__main__':
    app.debug = False
    app.config['TEMPLATES_AUTO_RELOAD'] = True      
    app.jinja_env.auto_reload = True
    thread1.start()
    app.run(host="0.0.0.0",port=3333)