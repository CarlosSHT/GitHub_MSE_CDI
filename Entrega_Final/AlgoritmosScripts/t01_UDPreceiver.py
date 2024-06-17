import socket
import threading
import struct
import time
import queue
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import csv
import os
import re

dest_ip = '192.168.5.229'
dest_port = 61454

fs_sys = 0 ## 200Hz fs
tplo_secs = 3 ## 2 segundos ploteo


struct_format = '=4sIBHH4s'
header_size = struct.calcsize(struct_format)

def next_file_number(folder_path, base_name="rcrcDatos_", extension=".csv"):
    files = os.listdir(folder_path)
    pattern = re.compile(rf"{base_name}(\d{{3}}){extension}$")
    
    max_number = 0
    for file in files:
        match = pattern.match(file)
        if match:
            number = int(match.group(1))
            if number > max_number:
                max_number = number
    
    next_number = max_number + 1
    return f"{base_name}{next_number:03d}{extension}"

new_file_name = next_file_number(folder_path = ".", base_name= "v3TPFrcrcDatos_")

csv_file = new_file_name

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
while True:
    try:
        sock.bind((dest_ip, dest_port))
        print(f"Socket successfully bound to {dest_ip}:{dest_port}")
        break

    except:
        print("Failed to bind socket")
        time.sleep(0.3)


fig = plt.figure(1)
fig.suptitle('Señal en tiempo', fontsize=16)

adcAxe = fig.add_subplot(1, 1, 1)
adcAxe.set_title("RC-RC Circuit", rotation=0, fontsize=10, va="center")
adc1s, = plt.plot([], [], 'b-', linewidth=1, alpha=0.8, label="Input")
adc2s, = plt.plot([], [], 'g-', linewidth=1, alpha=0.8, label="Output C1")
adc3s, = plt.plot([], [], 'r-', linewidth=1, alpha=0.8, label="Output C2")
adcLg = adcAxe.legend()
adcAxe.grid(True)
# adcAxe.set_ylim(1100, 2600)
adcAxe.set_ylim(0, 3.3)
adcAxe.xaxis.grid(True, which='both', linestyle='--', linewidth=0.5, color='gray')
adcAxe.yaxis.grid(True, which='both', linestyle='--', linewidth=0.5, color='gray')

cola_data_udp = queue.Queue()
data_udp = []
data_buffer = []

data_adcN = []
# tData = np.arange(0, fs * tplo_secs, 1) / fs
tData = []

paused = False


def init_plot():
    return adc1s,adc2s,adc3s,adcAxe

def update_plot(f):
    global data_udp, fs_sys, data_adcN, tData

    adc_s1 = []
    cola_dataUDP_size = cola_data_udp.qsize()
    if cola_dataUDP_size >0 :
        data_udp = cola_data_udp.get()
        
        header_data = data_udp[:header_size]
        unpacked_header = struct.unpack(struct_format, header_data)

        pre = unpacked_header[0].decode('utf-8').rstrip('\x00')
        id = unpacked_header[1]
        s = unpacked_header[2]
        N = unpacked_header[3]
        fs_uc = unpacked_header[4]
        pos = unpacked_header[5].decode('utf-8').rstrip('\x00')

        data_signals = data_udp[header_size:header_size + len(data_udp)] 
        
        # print(f"headersize {header_size}, pre: {pre}, id: {id}, s: {s}, N: {N}, fs: {fs_uc}, pos: {pos}")
        
        if fs_sys != fs_uc:
            fs_sys=fs_uc
            tData = np.arange(0, fs_uc * tplo_secs, 1) / fs_uc

            for n in range (s):
                data_adcX = np.zeros(fs_uc * tplo_secs, dtype=np.uint16)
                data_adcN.append(data_adcX)
            adcAxe.set_xlim(0, (fs_sys * tplo_secs) / fs_sys)
                    
                
        adc_sX = []
        for nsig in range ((int)(len(data_signals)/(2*N))):
            adc_sn = []
            for i in range(0, 2*N, 2):
                # print(f"inicio {i+nsig*2*N} fin {i+2+nsig*2*N}")
                value = struct.unpack('<H ', data_signals[i+nsig*2*N:i+2+nsig*2*N])[0]  
                adc_sn.append(value)
                
            adc_sX.append(adc_sn)
            data_adcN[nsig] = data_adcN[nsig][len(adc_sn):]

            data_adcN[nsig] = np.concatenate((data_adcN[nsig], np.array(np.array(adc_sn), dtype=np.uint16)))

        with open(csv_file, mode='a', newline='') as file:
            writer = csv.writer(file)
            writer.writerows(np.array(adc_sX).transpose())


        adc1s.set_data(tData, data_adcN[0]*3.3/4095)
        adc2s.set_data(tData, data_adcN[1]*3.3/4095)
        adc3s.set_data(tData, data_adcN[2]*3.3/4095)



    return adc1s, adc2s, adc3s, adcAxe

def toggle_pause(event):
    global animation, paused
    paused = not paused
    if paused:
        animation.pause()
    else:
        pass
        # animation.resume()

def recibir_datos():
    global data_buffer
    while True:
        data, addr = sock.recvfrom(1024)
        cola_data_udp.put(data)





hilo_receptor = threading.Thread(target=recibir_datos)
hilo_receptor.start()

fig.canvas.mpl_connect('button_press_event', toggle_pause)
animation = FuncAnimation(fig, update_plot, 2, init_func=init_plot, blit=True, interval=1, repeat=True)
plt.show()