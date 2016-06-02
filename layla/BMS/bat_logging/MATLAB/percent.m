% Clayton Auld
% ITEST development: plotting LiPo battery percentage curve

clear;clc;

time=dlmread('batterytest5.csv',',','A2..A736')/3600;
Cell0=dlmread('batterytest5.csv',',','B2..B736');
Cell1=dlmread('batterytest5.csv',',','C2..C736');
Cell2=dlmread('batterytest5.csv',',','D2..D736');
Percent=dlmread('batterytest5.csv',',','E2..E736');

figure(1)
plot(time,Percent)
xlabel('Time (hours)');
ylabel('Percent');
title('Battery Percentage vs Time')

figure(2)
plot(time,Cell0,time,Cell1,time,Cell2);
xlabel('Time (hours)');
ylabel('Cell Voltage (V)');
legend('Cell 0','Cell 1','Cell 2');
title('Cell Voltage vs Time')

charge=Cell0+Cell1+Cell2;
figure(3)
plot(time,charge)
xlabel('Time (hours)');
ylabel('Battery Voltage (V)');
title('Battery Voltage vs Time');

figure(4)
subplot(2,1,1)
plot(time,charge)
xlabel('Time (hours)');
ylabel('Battery Voltage (V)');
title('Battery Voltage vs Time');
subplot(2,1,2)
plot(time,Percent)
xlabel('Time (hours)');
ylabel('Percent');
title('Battery Percentage vs Time')