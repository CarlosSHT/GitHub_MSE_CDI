import numpy as np
import csv
import control as cnt
import control.matlab  as cntm
import matplotlib.pyplot as plt



csv_filename = 'rcrcDatos0.csv'

data_rows = []

input = []
output = []

with open(csv_filename, mode='r') as file:
    reader = csv.reader(file)
    for row in reader:
        input.append((int)(row[0]))
        output.append((int)(row[1]))

data_rows = [int(x) for x in data_rows]

fs = 200

tiempo = [i / fs for i in range(len(data_rows))]

t= np.arange(len(input))/fs
input = np.array(input) *3.3/4095
output = np.array(output)*3.3/4095

input_src = input
t_src = t

threshold = 1.5

idx = np.argmax(input > threshold)

t = t[:-idx]
input = input [idx:]
output = output [idx:]

idx_clean = np.argmax(input < threshold)


t = t[:idx_clean]
input = input [:idx_clean]-1
output = output [:idx_clean]-1



x1 = 0.015
y1 = 1.188-1

x2 = 0.11
y2 = 2-1

# Ecuación de la recta
t1 = np.linspace(0.00, x2, 5000)
y_r = ((y2-y1)/(x2-x1))*(t1-x1)+y1

# Ziegler-Nichols Parameters
L = x1
T = x2-x1

A = B = 2.0

# PID con Z-N

K  = 1.2*T/L
Ti = 2*L
Td = 0.5*L

K  = 1.2*T/L*1.1
Ti = 2*L*0.8
Td = 0.5*L*1.8

K  = 1.2*T/L*1.1
Ti = 2*L*1.4
Td = 0.5*L*2

print(K,Ti,Td)

pid = cnt.tf([K*Td, K, K/Ti], [1, 0])



R1 = 10*1e3
C1 = 1*1e-6
R2 = 39*1e3
C2 = 1*1e-6

num = [1]
den = [(R1*C1*R2*C2), (R1*C1+R1*C2+R2*C2), 1]
sys = cnt.tf(num, den)


cl_sys  = cnt.feedback(sys*pid, 1)
t, y_cl = cnt.forced_response(cl_sys, t, input)


print(max(y_cl))

plt.figure(figsize=(10, 6))
plt.plot(t, input, marker='', linestyle='-', color='b', label='Input')
plt.plot(t, output, marker='', linestyle='-', color='r', label='Output')
plt.xlabel('Tiempo (segundos)')
plt.ylabel('Valor')
plt.title('Plot de la Señal en función del Tiempo')
plt.legend()
plt.grid(True)


plt.plot(t1, y_r, 'g-', label='Recta')
plt.legend(loc='best')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')


# Plot the results
plt.grid(color='k', linestyle='-', linewidth=0.2)
# plt.plot(t, y_ol, 'b-', label='Out_ol')
plt.plot(t, y_cl, 'y-', label='Out_cl', linewidth=2)
plt.legend(loc='best')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')

plt.show()

