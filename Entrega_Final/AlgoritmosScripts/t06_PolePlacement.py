import control.matlab as cntm
import matplotlib.pyplot as plt
import numpy as np

###########################################################################
###########################################################################
##### NO FUNCIONA - MATRICES A B C D DIFERENTES ENTRE MATLAB Y PYTHON #####
###########################################################################
###########################################################################
# Descripcion de la Planta
# Componentes planta RCRC
R1 = 10*1e3
C1 = 1*1e-6
R2 = 39*1e3
C2 = 1*1e-6

# Frecuencia / periodo de muestreo
fs = 200
h = 1/fs

# % Función de transferencia continua
nums = [1]
dens = [(R1*C1*R2*C2), (R1*C1+R1*C2+R2*C2), 1]
Hs = cntm.tf(nums, dens)
print(Hs)

# % Función de transferencia discreta
Hz = cntm.c2d(Hs, h, 'zoh')
[numz, denz] = cntm.tfdata(Hz)
print(Hz)

# % A, B, C, D Matrices espacio estado
ssS_sys = cntm.ss(Hs)
ssZ_sys = cntm.c2d(ssS_sys, h)


# % Polos del sistema en espacio continuo
pS_sys = cntm.pole(Hs)
print(f"Los polos del sistema son  {pS_sys[0]}, {pS_sys[1]}")


plt.figure(1)
cntm.pzmap(Hs, grid = True, title = 'Polos y ceros de H(s)')  # % Grafica los polos y ceros

plt.figure(2)
cntm.pzmap(Hz, grid = True, title = 'Polos y ceros de H(z)') # % Grafica los polos y ceros


###########################################################################
#  REQUERIMIENTOS

# % Aplicando requerimiento tr menor en 30%
poles_t = np.exp(pS_sys*1.3*h)

# % Obteniendo K
K = cntm.place(ssZ_sys.A, ssZ_sys.B, [poles_t[0],poles_t[1]])
print(f"Valor de K {K}")

# % Obteniendo K0
ssZcl_sys = cntm.ss(ssZ_sys.A - ssZ_sys.B*K, ssZ_sys.B, ssZ_sys.C, ssZ_sys.D)
gain_pp =  np.dot(np.dot(ssZcl_sys.C, np.linalg.inv((np.eye(2) - ssZcl_sys.A))), ssZcl_sys.B) 
K0 = 1/gain_pp
print(f"Valor de K0 {K0}")

# **********************************************
# Definir parámetros
t = np.arange(0, 2 + h, h)  # Intervalo de tiempo de 0 a 2 segundos con paso h

# Inicializar matrices para guardar resultados
x = np.zeros((ssZ_sys.A.shape[0], len(t)))
y = np.zeros(len(t))
u = np.ones(len(t))

print("Matrices Espacio estado necesarios")
print(ssZ_sys.A)
print(ssZ_sys.B)
print(ssZ_sys.C)
print(ssZ_sys.D)
 
# Bucle de simulación en espacio de estado
for i in range(len(t) - 1):
    x[:, i + 1] = (np.dot(ssZ_sys.A, x[:, i].reshape(-1, 1))  + np.dot(ssZ_sys.B, u[i]))[:,0]
    y[i] = np.dot(ssZ_sys.C, x[:, i].reshape(-1, 1))[0][0]

# Graficar resultados
plt.figure(3)
plt.plot(t[:-1], y[:-1])  # Se usa t[:-1] y y[:-1] para ajustar la longitud de t e y
plt.xlabel('Tiempo')
plt.ylabel('Salida y')
plt.title('Respuesta del sistema en espacio de estado')
plt.grid(True)


plt.show()