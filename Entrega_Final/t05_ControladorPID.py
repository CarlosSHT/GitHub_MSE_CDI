import numpy as np
import csv
import control as cnt
import control.matlab  as cntm
import matplotlib.pyplot as plt



csv_filename = 'TPFrcrcDatos_006.csv'

data_rows = []

input = []
output = []

with open(csv_filename, mode='r') as file:
    reader = csv.reader(file)
    for row in reader:
        input.append((int)(row[0]))
        output.append((int)(row[1]))

data_rows = [int(x) for x in data_rows]

fs = 500

tiempo = [i / fs for i in range(len(data_rows))]

t= np.arange(len(input))/fs
input = np.array(input) *3.3/4095
output = np.array(output)*3.3/4095


input_src = input
t_src = t


plt.figure(1)
plt.plot(t, input, marker='', linestyle='-', color='b', label='Input')
plt.plot(t, output, marker='', linestyle='-', color='r', label='Output')
plt.xlabel('Tiempo (segundos)')
plt.ylabel('Valor')
plt.title('Plot de la Señal en función del Tiempo')
plt.legend()
plt.grid(True)

#############################################################################
#############################################################################
#############################################################################


x1 = 0.0414
y1 = 1.034

x2 = 0.1352
y2 = 2

# Ecuación de la recta
t1 = np.linspace(x1, x2, 5000)
y_r = ((y2-y1)/(x2-x1))*(t1-x1)+y1


plt.plot(t1, y_r, 'g-', label='Recta')
plt.legend(loc='best')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')



#############################################################################
#############################################################################
#############################################################################

csv_filename = 'TPFrcrcDatos_001.csv'

data_rows = []

input = []
output = []

with open(csv_filename, mode='r') as file:
    reader = csv.reader(file)
    for row in reader:
        input.append((int)(row[0]))
        output.append((int)(row[1]))

data_rows = [int(x) for x in data_rows]

fs = 100

tiempo = [i / fs for i in range(len(data_rows))]

t= np.arange(len(input))/fs
input = np.array(input) *3.3/4095
output = np.array(output) *3.3/4095


# Ziegler-Nichols Parameters
L = x1
T = x2-x1

A = B = 2.0

# PID con Z-N

K  = 1.2*T/L
Ti = 2*L
Td = 0.5*L

K  = 1.2*T/L*6
Ti = 2*L*0.3
Td = 0.5*L*0.8


pid = cnt.tf([K*Td, K, K/Ti], [1, 0])

print(f"El valor de K es {round(K,4)}")
print(f"El valor de Ti es {round(Ti,4)}")
print(f"El valor de Td es {round(Td,4)}")
print("*"*10)
print(f"El valor de Kp es {round(K,4)}")
print(f"El valor de Ki es {round(K/Ti,4)}")
print(f"El valor de Kd es {round(K*Td,4)}")


R1 = 10*1e3
C1 = 1*1e-6
R2 = 39*1e3
C2 = 1*1e-6

num = [1]
den = [(R1*C1*R2*C2), (R1*C1+R1*C2+R2*C2), 1]
sys = cnt.tf(num, den)


cl_sys  = cnt.feedback(sys*pid, 1)
t, y_cl = cnt.forced_response(cl_sys, t, input)


# print(max(y_cl))
print(f"El valor máximo de la señal controlada es {round(max(y_cl),4)}")


plt.figure(2)

plt.plot(t, input, marker='', linestyle='-', color='b', label='Input')
plt.plot(t, output, marker='', linestyle='-', color='r', label='Output')


plt.grid(color='k', linestyle='-', linewidth=0.2)
# plt.plot(t, y_ol, 'b-', label='Out_ol')
plt.plot(t, y_cl, 'y-', label='Out_cl', linewidth=2)
plt.legend(loc='best')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')

plt.show()

