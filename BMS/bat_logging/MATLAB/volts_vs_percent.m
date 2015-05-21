% Clayton Auld
% ITEST development: plotting LiPo battery curve

clear;clc;
time=dlmread('batterytest1.csv',',','A7..A1333');
time1=dlmread('batterytest1.csv',',','A7..A1202');
time2=dlmread('batterytest1.csv',',','A1203..A1333');
max_time=time(length(time));
percent=abs(time-max_time)/max_time*100;
percent1=percent(1:size(time1),1);
percent2=percent(size(time1)+1:end,1);
Cell0=dlmread('batterytest1.csv',',','B7..B1333');
Cell1=dlmread('batterytest1.csv',',','C7..C1333');
Cell2=dlmread('batterytest1.csv',',','D7..D1333');
Cell0_1=dlmread('batterytest1.csv',',','B7..B1202');
Cell0_2=dlmread('batterytest1.csv',',','B1203..B1333');
Cell1_1=dlmread('batterytest1.csv',',','C7..C1202');
Cell1_2=dlmread('batterytest1.csv',',','C1203..C1333');
Cell2_1=dlmread('batterytest1.csv',',','D7..D1202');
Cell2_2=dlmread('batterytest1.csv',',','D1203..D1333');

figure(1)
subplot(2,2,1)
Bat_Volts=Cell0+Cell1+Cell2;
plot(Bat_Volts,percent)
ylabel('Percentage')
xlabel('Battery Voltage')
title('Raw Data')

subplot(2,2,2)
Bat_Volts2=Cell0_2+Cell1_2+Cell2_2;
Bat_Volts1=Cell0_1+Cell1_1+Cell2_1;
plot(Bat_Volts2,percent2,Bat_Volts1,percent1)
ylabel('Percentage')
xlabel('Battery Voltage')
title('Raw Data in Two Sections')
legend('Section 1','Section 2','location','southeast')

% figure(3)
% Bat_Volts1=Cell0_1+Cell1_1+Cell2_1;
% plot(Bat_Volts1,percent1)
% ylabel('Percentage')
% xlabel('Battery Voltage')
% title('Battery Voltage vs Percentage')

% Section 1: Battery Voltage < 11 V
p1 =      0.0668;
p2 =      0.3141;
p3 =      0.3531;
p4 =     -0.2128;
p5 =     -0.3324;
p6 =      0.6422;
p7 =       2.936;
p8 =       4.184;

mean=10.64;
std=0.314;

x1=[9.919:.001:11.014];
x = (x1 - mean)/std;
     
fx1 = p1*x.^7 + p2*x.^6 + p3*x.^5 + p4*x.^4 + p5*x.^3 + p6*x.^2 + p7*x + p8;

% Section 2: Battery Voltage > 11 V
p1 =      0.5245;
p2 =      -1.722;
p3 =      -1.428;
p4 =       7.848;
p5 =     -0.7242;     
p6 =      -13.69;
p7 =        29.7;
p8 =       60.95;

mean=11.49;
std=0.3152;

x2=[11.014:.001:12.2];
x = (x2 - mean)/std;
     
fx2 = p1*x.^7 + p2*x.^6 + p3*x.^5 + p4*x.^4 + p5*x.^3 + p6*x.^2 + p7*x + p8;

% Plot and compare
subplot(2,2,3)
Bat_Volts2=Cell0_2+Cell1_2+Cell2_2;
Bat_Volts1=Cell0_1+Cell1_1+Cell2_1;
plot(x1,fx1,x2,fx2)
ylabel('Percentage')
xlabel('Battery Voltage')
title('Curve Fits')
legend('Curve Fit1','Curve Fit2','location','southeast')

subplot(2,2,4)
plot(Bat_Volts2,percent2,Bat_Volts1,percent1,x1,fx1,x2,fx2)
ylabel('Percentage')
xlabel('Battery Voltage')
title('Raw Rata and Curve Fits')
legend('Data1','Data2','Curve Fit1','Curve Fit2','location','southeast')