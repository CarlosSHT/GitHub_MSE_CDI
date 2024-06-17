%% Control Digital en Sistemas Embebidos - MSE - PRACTICA 4

% pkg load signal
% pkg load control

clc;
clear;
close all;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Descripcion de la Planta
% Componentes planta RCRC
R1 = 10*1e3;
C1 = 1*1e-6;
R2 = 39*1e3;
C2 = 1*1e-6;

% Frecuencia / periodo de muestreo
fs = 200;
h = 1/fs;

% Función de transferencia continua
nums = [ 1 ];
dens = [(R1*C1*R2*C2) (R1*C1+R1*C2+R2*C2) 1];

% Valores obtenidos experimentalmente
nums = [ 1 ];
dens = [0.000216 0.08105 1.0004];
Hs = tf(nums, dens)

% Función de transferencia discreta
Hz = c2d(Hs, h, 'zoh')
[numz, denz] = tfdata(Hz, 'v');

% A, B, C, D Matrices espacio estado
ssS_sys = ss(Hs);
ssZ_sys = c2d(ssS_sys, h);
% [ssS_Phi, ssS_Gamma, ssS_C, ssS_D] = tf2ss(nums, dens); % <<<< esta mal 

% Polos del sistema en espacio continuo
pS_sys = pole(Hs);
msg = sprintf('Los polos del sistema son %f, %f',pS_sys(1),pS_sys(2));
disp(msg);

figure(1);
pzmap(Hs)   % Grafica los polos (y ceros si los hubiera)
grid on 
title('Diagrama de Polos del Sistema');

%% REQUERIMIENTOS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Aplicando requerimiento tr menor en 30%
poles_t = exp(pS_sys*1.3*h);

% Obteniendo K
K = place(ssZ_sys.A, ssZ_sys.B, [poles_t(1),poles_t(2)]);

% Obteniendo K0
ssZcl_sys = ss(ssZ_sys.A - ssZ_sys.B*K, ssZ_sys.B, ssZ_sys.C, ssZ_sys.D);
gain_pp = ssZcl_sys.C * (eye(2) - ssZcl_sys.A) ^ (-1) * ssZcl_sys.B;
K0 = 1/gain_pp;

% Obteniendo Integral

Ae  = [ssZ_sys.A zeros(length(ssZ_sys.A),1);-ssZ_sys.C 1];
Beu = [ssZ_sys.B;0];
Ber = [zeros(length(ssZ_sys.B),1);1];
Ce  = [ssZ_sys.C 0];

pdi = exp(-10*max(abs(pS_sys))*h);  

K1 = place(Ae,Beu,[poles_t;pdi]);

K1_a = K1(1:2)
K1_b = K1(3)

%%
% APLICACION - SIMULACION LAZO CERRRADO CON GANANCIA

t = 0:h:5;
fsig = 1;

x = zeros(length(ssZ_sys.A),length(t));
y = zeros(1,length(t));
u = ones(1,length(t));

for i=1:length(t)
    x(:,i+1) = ssZ_sys.A*x(:,i) + ssZ_sys.B*u(i);
    y(i) = ssZ_sys.C*x(:,i);
end

%
r    = ones(1,length(t));

r = square(2 * pi * fsig * t);
r = (r + 1) / 2 + 1;


x_lcg = zeros(length(ssZ_sys.A),length(t));
y_lcg = zeros(1,length(t));

for i=1:length(t)
    x_lcg(:,i+1) = ssZ_sys.A * x_lcg(:,i) - ssZ_sys.B * K * x_lcg(:,i) + ssZ_sys.B * K0 * r(i);
    y_lcg(i) = ssZ_sys.C * x_lcg(:,i);
end

figure(2)
plot(t,r,t,y_lcg,'LineWidth',1)