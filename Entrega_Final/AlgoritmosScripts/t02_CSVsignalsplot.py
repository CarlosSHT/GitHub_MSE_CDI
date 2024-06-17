import csv
import matplotlib.pyplot as plt
import numpy as np

csv_filename = '../rec_data/TPFrcrcDatos_005.csv'

data_rows = []

input = []
output1 = []
output2 = []

with open(csv_filename, mode='r') as file:
    reader = csv.reader(file)
    for row in reader:
        input.append((int)(row[0]))
        output1.append((int)(row[1]))
        output2.append((int)(row[2]))

data_rows = [int(x) for x in data_rows]

fs = 200

tiempo = [i / fs for i in range(len(data_rows))]

t= np.arange(len(input))/fs
input = np.array(input)*3.3/4095
output1 = np.array(output1)*3.3/4095
output2 = np.array(output2)*3.3/4095


plt.figure(figsize=(10, 6))
plt.plot(t, input, marker='', linestyle='-', color='b', label='Input')
plt.plot(t, output1, marker='', linestyle='-', color='g', label='OutputC1')
plt.plot(t, output2, marker='', linestyle='-', color='r', label='OutputC2')

plt.xlabel('Tiempo (segundos)')
plt.ylabel('Valor')
plt.title('Plot de la Señal en función del Tiempo')
plt.legend()

plt.grid(True)

plt.show()
