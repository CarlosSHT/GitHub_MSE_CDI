clc, close all, clear all;
%% Valores teóricos
% Valores de componentes

pkg load signal
pkg load control

%%
function [Theta] = identificacionLS(n, u, y)
  Y = y(n+1:length(y));
  Phi = [];
  for i=n:-1:1
    Phi = [Phi y(i:(length(y)+i-n-1))];
  end
  for j=(n+1):-1:1
    Phi = [Phi u(j:(length(y)+j-n-1))];
  end
  Theta = (Phi'*Phi)^(-1)*Phi'*Y;
end

R1 = 10*1e3;
C1 = 1*1e-6;
R2 = 39*1e3;
C2 = 1*1e-6;

fs = 200; % frecuencia de muestreo
h = 1/fs;

num = [ 1 ];
den = [(R1*C1*R2*C2) (R1*C1+R1*C2+R2*C2) 1];

Hs= tf(num, den)

Hz = c2d(Hs, h, 'zoh')
disp(Hz);
[numz, denz] = tfdata(Hz, 'v');
%% Parte experimental

data_csv = csvread('TPFrcrcDatos_001.csv');

u= data_csv(:,1);
y= data_csv(:,2);

u = u * 3.3 / 4095;
y = y * 3.3 / 4095;

t = 1:1:length(u);
t = t / fs;

% y = filter(numz, denz, u);

[Theta_LS_1] = identificacionLS(1, u, y);
numz_LS_1 = [Theta_LS_1(2) Theta_LS_1(3)];
denz_LS_1 = [1 -Theta_LS_1(1)];

[Theta_LS_2] = identificacionLS(2, u, y);
numz_LS_2 = [Theta_LS_2(3) Theta_LS_2(4) Theta_LS_2(5)]
denz_LS_2 = [1 -Theta_LS_2(1) -Theta_LS_2(2)]

[Theta_LS_3] = identificacionLS(3, u, y);
numz_LS_3 = [Theta_LS_3(4) Theta_LS_3(5) Theta_LS_3(6) Theta_LS_3(7)];
denz_LS_3 = [1 -Theta_LS_3(1) -Theta_LS_3(2) -Theta_LS_3(3)];


y_LS_1 = filter(numz_LS_1, denz_LS_1, u);
J_1 = (y-y_LS_1)'*(y-y_LS_1)/2

y_LS_2 = filter(numz_LS_2, denz_LS_2, u);
J_2 = (y-y_LS_2)'*(y-y_LS_2)/2

y_LS_3 = filter(numz_LS_3, denz_LS_3, u);
J_3 = (y-y_LS_3)'*(y-y_LS_3)/2

%% Plot


figure;
hold on;
plot(t, u)
plot(t, y, 'LineWidth', 3)
% plot(t, y_LS_1, 'LineWidth', 3)
% plot(t, y_LS_2, '--', 'LineWidth', 3)
% legend('u', 'y', 'y_LS_1', 'y_LS_2')


