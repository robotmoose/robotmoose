% Clayton Auld
% ITEST development: plotting LiPo battery percentage curve

clear;clc;

time=dlmread('batterytest3.csv',',','A2..A3144')/3600;
% Cell0_3=dlmread('batterytest3.csv',',','B2..B3144');
% Cell1_3=dlmread('batterytest3.csv',',','C2..C3144');
% Cell2_3=dlmread('batterytest3.csv',',','D2..D3144');
Percent=dlmread('batterytest3.csv',',','E2..E3144');

figure(1)
plot(time,Percent)
xlabel('Time (hours)');
ylabel('Percent');