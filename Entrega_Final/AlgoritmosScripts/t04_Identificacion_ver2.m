clear all, close all, clc;


R1 = 10*1e3;
C1 = 1*1e-6;
R2 = 39*1e3;
C2 = 1*1e-6;

fs = 500; % frecuencia de muestreo
hs = 1/fs;

num = [ 1 ];
den = [(R1*C1*R2*C2) (R1*C1+R1*C2+R2*C2) 1];

Hs= tf(num, den)

Hz = c2d(Hs, hs, 'zoh')
data_csv = csvread('../rec_data/v3TPFrcrcDatos_001.csv');

u= data_csv(:,1);
y= data_csv(:,3);


u = u * 3.3 / 4095;
y = y * 3.3 / 4095;


t = 1:1:length(u);
t = t / fs;

% filtro de media movil
n_order = 3;
h = ones (1,n_order);
yf = filter(h, 1, y)/n_order;

figure(1);
plot(t,u,t,y)
hold on 
plot(t,yf);

systemIdentification
