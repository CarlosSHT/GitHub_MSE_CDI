%% Control Digital en Sistemas Embebidos - MSE - PRACTICA 4

pkg load signal
pkg load control

clc;
clear all;
close all;


R1 = 10*1e3;
C1 = 1*1e-6;
R2 = 39*1e3;
C2 = 1*1e-6;

##PLANTA RC RC
fs = 200; % frecuencia de muestreo
h = 1/fs;

num = [ 1 ];
den = [(R1*C1*R2*C2) (R1*C1+R1*C2+R2*C2) 1];


Hs= tf(num, den)

Hz = c2d(Hs, h, 'zoh')
disp(Hz);

[numz, denz] = tfdata(Hz, 'v');
[Phi, Gamma, C, D] = tf2ss(numz, denz);



% Polos del sistema
poles_sys = pole(Hz)
msg = sprintf('Los polos del sistema son %f, %f',poles_sys(1),poles_sys(2));
disp(msg);

figure(1);
zplane([], poles_sys);  % Grafica los polos (y ceros si los hubiera)
title('Diagrama de Polos del Sistema');

## Carga de valores obtenidos con ADC


data_csv = csvread('TPFrcrcDatos_001.csv');

r= data_csv(:,1);
y= data_csv(:,2);
r = r * 3.3 / 4095;
y = y * 3.3 / 4095; ## Valores de salida sin control

t = 1:1:length(r);
t = t / fs;

figure(2);
hold on;
stairs(t, r)
stairs(t, y, "LineWidth", 3)


% En primer lugar se aplica un movimiento de los polos sin aplicar la ganancia

poles = poles_sys* (1-0.30) % 30% mas rapido

figure(3);
zplane([], poles);  % Grafica los polos (y ceros si los hubiera)
title('Diagrama de Polos Trasladados para el Sistema ');


K = place(Phi, Gamma, poles)

Phi_LC = Phi - Gamma * K


y_open = filter(numz, denz, r);
[numz_pole_placement_1, denz_pole_placement_1] = ss2tf(Phi_LC, Gamma, C, D);
y_pole_placement = filter(numz_pole_placement_1, denz_pole_placement_1, r);

figure(4);
hold on;
stairs(t, r)
stairs(t, y_open, "LineWidth", 3)
stairs(t, y_pole_placement, "LineWidth", 3)
legend("Entrada", "Respuesta natural", "Salida controlada")


% Obteneniendo K0


K = place(Phi, Gamma, poles)

Phi_LC = Phi - Gamma * K

K0 = (C * (eye(2) - Phi_LC)^(-1) * Gamma)^(-1)

Gamma_LC = Gamma * K0

y_open = filter(numz, denz, r);
[numz_pole_placement, denz_pole_placement] = ss2tf(Phi_LC, Gamma_LC, C, D);
y_pole_placement = filter(numz_pole_placement, denz_pole_placement, r);

figure(6);
hold on;
stairs(t, r)
stairs(t, y_open, "LineWidth", 3)
stairs(t, y_pole_placement, "LineWidth", 3)
legend("Entrada", "Respuesta natural", "Salida controlada")


function [data] = pole_placement_controller(data)
  data.output = data.K0 * data.R - data.K * data.X;
end

function [y] = filter_pole_placement(b, a, r, data)
  y_inicial = length(a) - 1;
  y = zeros(y_inicial + length(r), 1);
  r = [ zeros(y_inicial, 1); r ];
  u = [ zeros(y_inicial, 1); r ];

  for k = y_inicial + 1 : 1 : length(r)
    data.X = y(k-1);
    data.R = r(k);
    data = pole_placement_controller(data);
    u(k) = data.output;

    for m = 1 : 1 : length(b)
      y(k) = y(k) + b(m) * u(k-m+1);
    end

    for n = 2 : 1 : length(a)
      y(k) = y(k) - a(n) * y(k-n+1);
    end

    y(k) = y(k) / a(1);
  end

  y = y(y_inicial+1:end);
end




