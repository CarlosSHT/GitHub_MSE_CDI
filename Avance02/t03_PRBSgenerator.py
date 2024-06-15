import numpy as np
import matplotlib.pyplot as plt

def create_prbs(ValUinit, ValAmpli, ValDecal, ValLgReg, ValDivi, Nsamp, Tappli):
    # "Entry parameters" are :
    # ValUinit  : Initial steady state
    # ValAmpli  : Magnitude
    # ValDecal  : Add-on DC component
    # ValLgReg  : Register length
    # ValDivi   : Frequency divider
    # samp      : Number of samples
    # Tappli    : Application instant 
    
#                   ____  Valdecal + ValAmpli         __________      ____
#                  |    |                            |          |    |
#  Valdecal       -|----|--------                    |          |    |
#                  |    |____________________________|          |____|
#                  |
#                  |
#  ini ____________|
#                                                    |--------->|
#      |-Tappli -->|                        ValReg * ValDivi 
#      
# 
#      |---------- samp ------------------------------------------------->|
#                              
    
    # the initialization is performed
    k1 = ValLgReg - 1
    k2 = ValLgReg
    
    if ValLgReg == 5:
        k1 = 3
    elif ValLgReg == 7:
        k1 = 4
    elif ValLgReg == 9:
        k1 = 5
    elif ValLgReg == 10:
        k1 = 7
    elif ValLgReg == 11:
        k1 = 9

    sbpa = np.ones(11)

    # After init-phase PRBS algo is running

    # Output set to init-value until the PRBS application istant
    prbs = np.empty(Nsamp)
    prbs[:Tappli] = ValUinit

    # PRBS sequence generation 
    i = Tappli
    while (i < Nsamp):
        uiu = -sbpa[k1] * sbpa[k2]
        if (ValLgReg == 8):
            uiu = -sbpa[2] * sbpa[3] * sbpa[5] * sbpa[8]
        j = 1
        while (j <= ValDivi):
            prbs[i] = uiu * ValAmpli + ValDecal
            i += 1
            j += 1
        for j in range(ValLgReg, 1, -1):
            sbpa[j] = sbpa[j-1]
        sbpa[1] = uiu
    
    return prbs

# "Entry parameters" are :
# ValUinit  : Initial steady state
# ValAmpli  : Magnitude              (U)
# ValDecal  : Add-on DC component    
# ValLgReg  : Register length        (N)
# ValDivi   : Frequency divider      (p) --> fprbs = fs/p
# samp      : Number of samples      (L)
# Tappli    : Application instant 

# create_prbs(ValUinit, ValAmpli, ValDecal, ValLgReg, ValDivi, Nsamp, Tappli)

U = 0.5               # La amplitud del PRBS puede ser pequeña, pero debe ser tal de tener una buena relación señal/ruido  
# U tampoco debe ser tan grande como para poner de manifiesto la dinámica no-lineal del sistema
tr = 0.173
Tprbs1 = 0.173/9
Tprbs = round(Tprbs1,2)+0.0
# Tprbs = Tprbs1
p = 1               # Divisor de la frecuencia de muestreo, solamente necesaria si N se hace demasiado grande
N = int(tr/Tprbs)+2 # esto es así si p=1, si p>1 --> N=tr/(p*Tprbs)
L = 2**N-1
D = L*Tprbs         # Indicación del tiempo que debe durar el ensayo como mínimo

cAdd = 1.5
print('*'*20)
print(f"Magnitud de la señal U : {U}")
print(f"Componente Add on DC : {cAdd}")
print(f"Periodo de muestreo proyectado PRBS: {Tprbs1}")
print(f"Periodo de muestreo mínimo seleccionado PRBS: {Tprbs}")
print(f"Máximo número de muestras por PULSO: {N}")
print(f"Tiempo maximo de pulso {Tprbs*N} > tiempo de subida {tr}")
print(f"Frecuencia de muestreo PRBS: {1/Tprbs}")
print(f"Longitud total de la secuencia (n° muestras): {L}")
print(f"Tiempo total del ensayo: {D}")
print('*'*20)



# def create_prbs(ValUinit, ValAmpli, ValDecal, ValLgReg, ValDivi, Nsamp, Tappli):
#     # "Entry parameters" are :
#     # ValUinit  : Initial steady state
#     # ValAmpli  : Magnitude
#     # ValDecal  : Add-on DC component
#     # ValLgReg  : Register length
#     # ValDivi   : Frequency divider
#     # samp      : Number of samples
#     # Tappli    : Application instant 


prbs = create_prbs(ValUinit = 0, ValAmpli= U, ValDecal = cAdd, ValLgReg = N, ValDivi = p, Nsamp = L, Tappli = 0)

variable_name = "prbs_1"
size_define = "SIZE_PRBS_1"
size = len(prbs)

# prbs = np.where(prbs == 1.2, 1.2*4095/3.3, prbs)
# prbs = np.where(prbs == 1.8, 1.8*4095/3.3, prbs)
prbs = np.where(prbs == 1, 1241, prbs)
prbs = np.where(prbs == 2, 2480, prbs)
prbs = prbs.astype(np.uint16)


# Crear el contenido del archivo de texto en formato C
# c_declaration = f"#define {size_define} {size}\n\n"
c_declaration = f"uint16_t {variable_name}[{size_define}] = {{\n"

# Añadir los valores del array al contenido
for i, value in enumerate(prbs):
    c_declaration += f"    {value},"
    if (i + 1) % 10 == 0:  # Añadir una nueva línea cada 10 valores para mejor legibilidad
        c_declaration += "\n"
c_declaration = c_declaration.rstrip(',\n') + "\n};"

# Escribir el contenido en un archivo de texto
with open("array_data.h", "w") as file:
    file.write(c_declaration)

# print("PRBS")
# print(len(prbs))
# print(prbs)

plt.stem(prbs)
# plt.plot(prbs)
plt.show()