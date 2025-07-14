clear; clc; close all;

name = "V2 Spreadsheets - LoRA_Tune_No_Tune_RSSI_Log.csv";

data = csvread(name,1);

n = 180; % num to trim off end

datatrim = data(1:length(data)-n ,:); % Trim data


subplot(1,2,1);
hold on
plot(datatrim(:,1),'DisplayName','RSSI (dB)')
plot(datatrim(:,2), 'DisplayName','SNR (dB)')
title("Partially Tuned Antenna Cap only")
ylim([-100 20]);

legend();
hold off
subplot(1,2,2);
hold on
plot(datatrim(:,3),'DisplayName','RSSI (dB)')
plot(datatrim(:,4),'DisplayName','RSSI (dB)')
title("Fully Tuned Antenna Res+Cap")
ylim([-100 20]);
legend();
hold off