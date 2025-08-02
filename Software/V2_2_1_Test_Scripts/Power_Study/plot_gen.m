clear; clc; close all;

%data = csvread("Lora power study.csv",1);
% data = csvread("Lora Power Study.csv",1);
%data = csvread("Power_Study_1.csv");
data = csvread("lora_lp_ps_3.csv");

datatrim = data(1:length(data)-1 ,:);

% datatrim = datatrim(1500:length(datatrim),:);
datatrim = datatrim(2075:2150, :);
%small 2600:4200
%medium 2000:8000
% data taken 5 ms apart
x = linspace(0,  (length(datatrim)-1)*0.005, length(datatrim))';
limx = x(length(x));

subplot(2,2,1)
plot(x, datatrim(:,1)/1000)
xlim([0,limx])

title("voltage (V)")
xlabel("Time (s)")
ylabel("voltage (V)")


subplot(2,2,2)
plot(x, datatrim(:,2)/1000)
xlim([0,limx])

title("Current (A)")
xlabel("Time (s)")
ylabel("Current (A)")

subplot(2,2,[3 4])
hold on
power = (datatrim(:,1)/1000).*(datatrim(:,2)/1000);
plot(x, power)
xlim([0,limx])
[pks,pks_idx] = findpeaks(power,'MinPeakHeight',0.035);
scatter(x(pks_idx),pks);
E = trapz(x,power);

title( sprintf("Power, total energy = %.3fJ",E) )
xlabel("Time (s)")
ylabel("Power (W)")
hold off

sgtitle("Power Plot for LoRa Transmission on Low Power Wio")
%sgtitle("Power Plot for LoRa Transmission on High Power Wio")


%%
clear; clc; close all;

%data = csvread("Lora power study.csv",1);
% data = csvread("Lora Power Study.csv",1);
%data = csvread("Power_Study_1.csv");
data = csvread("lora_lp_ps_3.csv");

datatrim = data(1:length(data)-1 ,:);

% datatrim = datatrim(1500:length(datatrim),:);
datatrim = datatrim(2097:2126, :);

x = linspace(0,  (length(datatrim)-1)*0.005, length(datatrim))';
limx = x(length(x));

hold on
power = (datatrim(:,1)/1000).*(datatrim(:,2)/1000);
plot(x, power)
xlim([0,limx])
[pks,pks_idx] = findpeaks(power,'MinPeakHeight',0.1);
scatter(x(pks_idx),pks);
E = trapz(x,power);

%Average E Per Transmission (72 Total)
E_per_transmission = trapz(x(pks_idx), pks)/72

title( sprintf("Power, total energy = %.3fJ",E) )
xlabel("Time (s)")
ylabel("Power (W)")
hold off

%%