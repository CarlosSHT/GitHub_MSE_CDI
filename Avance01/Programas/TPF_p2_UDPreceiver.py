import socket
import threading
import struct
import time
import queue
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import csv

dest_ip = '192.168.5.229'
dest_port = 61454

fs = 200 ## 200Hz fs
tplo_secs = 3 ## 2 segundos ploteo

csv_file = "rcrcDatos1.csv"

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
adc2s, = plt.plot([], [], 'r-', linewidth=2, alpha=0.8, label="Output")
adcLg = adcAxe.legend()
adcAxe.grid(True)
adcAxe.set_ylim(1100, 2600)
adcAxe.xaxis.grid(True, which='both', linestyle='--', linewidth=0.5, color='gray')
adcAxe.yaxis.grid(True, which='both', linestyle='--', linewidth=0.5, color='gray')

cola_data_udp = queue.Queue()
data_udp = []
data_buffer = []
data_adc1 = np.zeros(fs * tplo_secs, dtype=np.uint16)
data_adc2 = np.zeros(fs * tplo_secs, dtype=np.uint16)

tData = np.arange(0, fs * tplo_secs, 1) / fs

paused = False


def init_plot():
    return adc1s,adc2s,

def update_plot(f):
    global data_udp, data_adc1, data_adc2

    adc_s1 = []
    cola_dataUDP_size = cola_data_udp.qsize()
    if cola_dataUDP_size >0 :
        data_udp = cola_data_udp.get()
        
        adc_s1 = data_udp[:(int)(len(data_udp)/2)]
        adc_s2 = data_udp[(int)(len(data_udp)/2):]

        data_adc1 = data_adc1[len(adc_s1):]
        data_adc1 = np.concatenate((data_adc1, np.array(np.array(adc_s1), dtype=np.uint16)))

        data_adc2 = data_adc2[len(adc_s2):]
        data_adc2 = np.concatenate((data_adc2, np.array(np.array(adc_s2), dtype=np.uint16)))
        
        adcAxe.set_xlim(0, (fs * tplo_secs) / fs)
        
        with open(csv_file, mode='w', newline='') as file:
            writer = csv.writer(file)
            for i, val in enumerate (data_adc1):
                writer.writerow([data_adc1[i], data_adc2[i]])

        adc1s.set_data(tData, data_adc1)
        adc2s.set_data(tData, data_adc2)
    return adc1s,adc2s,

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
        data_buffer = []

        for i in range(0, len(data), 2):
            value = struct.unpack('<H ', data[i:i+2])[0]  
            data_buffer.append(value)
        cola_data_udp.put(data_buffer)



hilo_receptor = threading.Thread(target=recibir_datos)
hilo_receptor.start()

fig.canvas.mpl_connect('button_press_event', toggle_pause)
animation = FuncAnimation(fig, update_plot, 2, init_func=init_plot, blit=True, interval=1, repeat=True)
plt.show()