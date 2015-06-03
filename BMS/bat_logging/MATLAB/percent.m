% Clayton Auld
% ITEST development: plotting LiPo battery percentage curve

clear;clc;

time=dlmread('batterytest3.csv',',','A3..A756')/3600;
Cell0=dlmread('batterytest4.csv',',','B3..B756');
Cell1=dlmread('batterytest4.csv',',','C3..C756');
Cell2=dlmread('batterytest4.csv',',','D3..D756');
Percent=dlmread('batterytest4.csv',',','E3..E756');

figure(1)
plot(time,Percent)
xlabel('Time (hours)');
ylabel('Percent');

figure(2)
plot(time,Cell0,time,Cell1,time,Cell2);
xlabel('Time (hours)');
ylabel('Cell Voltage (V)');

charge=Cell0+Cell1+Cell2;
figure(3)
plot(time,charge)
xlabel('Time (hours)');
ylabel('Battery Voltage (V)');