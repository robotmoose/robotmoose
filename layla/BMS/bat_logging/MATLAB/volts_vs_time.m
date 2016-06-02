% Clayton Auld
% ITEST development: plotting LiPo battery curve

clear;clc;
time1=dlmread('batterytest1.csv',',','A2..A3467')/3600;
Cell0_1=dlmread('batterytest1.csv',',','B2..B3467');
Cell1_1=dlmread('batterytest1.csv',',','C2..C3467');
Cell2_1=dlmread('batterytest1.csv',',','D2..D3467');

time2=dlmread('batterytest2.csv',',','A2..A1661')/3600;
Cell0_2=dlmread('batterytest2.csv',',','B2..B1661');
Cell1_2=dlmread('batterytest2.csv',',','C2..C1661');
Cell2_2=dlmread('batterytest2.csv',',','D2..D1661');

% time3=dlmread('batterytest3.csv',',','A2..A3144')/3600;
% Cell0_3=dlmread('batterytest3.csv',',','B2..B3144');
% Cell1_3=dlmread('batterytest3.csv',',','C2..C3144');
% Cell2_3=dlmread('batterytest3.csv',',','D2..D3144');
% %Percent=dlmread('batterytest3.csv',',','E2..E3144');

figure(1)
%subplot(3,1,3)
plot(time1,Cell0_1,time1,Cell1_1,time1,Cell2_1,time2,Cell0_2,time2,Cell1_2,time2,Cell2_2)
xlabel('Time (hours)')
ylabel('Cell Voltage')
title('Cell Voltage vs Time')
% figure(3)
% plot(time3,Cell0_3,time3,Cell1_3,time3,Cell2_3)
% xlabel('Time (hours)')
% ylabel('Cell Voltage')
% title('Cell Voltage vs Time')
legend('Cell0_1','Cell1_1','Cell2_1','Cell0_2','Cell1_2','Cell2_2')
%legend('Cell0_1','Cell1_1','Cell2_1','Cell0_2','Cell1_2','Cell2_2','Cell0_3','Cell1_3','Cell2_3')

figure(2)
Bat_Volts1=Cell0_1+Cell1_1+Cell2_1;
Bat_Volts2=Cell0_2+Cell1_2+Cell2_2;
% Bat_Volts3=Cell0_3+Cell1_3+Cell2_3;
plot(time1,Bat_Volts1,time2,Bat_Volts2)%,time3,Bat_Volts3)
xlabel('Time (hours)')
ylabel('Battery Voltage')
title('Battery Voltage vs Time')
legend('Trial 1','Trial 2')%,'Trial 3')

