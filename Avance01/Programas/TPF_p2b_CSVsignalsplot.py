import csv
import matplotlib.pyplot as plt
import numpy as np

csv_filename = 'rcrcDatos4.csv'

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
input = np.array(input)
output = np.array(output)

# t=t[:(int)(len(t)/4)]
# input = input[:(int)(len(input)/4)]
# output = output[:(int)(len(output)/4)]

plt.figure(figsize=(10, 6))
plt.plot(t, input, marker='', linestyle='-', color='b', label='Input')
plt.plot(t, output, marker='', linestyle='-', color='r', label='Output')

plt.xlabel('Tiempo (segundos)')
plt.ylabel('Valor')
plt.title('Plot de la Señal en función del Tiempo')
plt.legend()

plt.grid(True)

plt.show()
