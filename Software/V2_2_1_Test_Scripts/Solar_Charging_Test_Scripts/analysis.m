clear; clc; close all;


data = csvread("simple_ps_on_off.csv",1);

dat = data(1:length(data)-1 ,:);

% low = 26035+400;
% high = 26035+400+50;

% datatrim = dat(low:high,:);
% dat_trim_wide = dat(low-500: high+500,:);

dat(:,1) = dat(:,1)+300; % +300 for calibration offset
datatrim = dat;
dat_trim_wide = dat;


% 50u is 0.25s
% 400u is 2s
% 500u is 2.5s
%2-3k is 5s
%2-4k is 10s
%2-5k is 15s, each k is 5s

% data taken 5 ms apart
x = linspace(0,  (length(datatrim)-1)*0.005, length(datatrim))';
x_lon = linspace(0,  (length(dat_trim_wide)-1)*0.005, length(dat_trim_wide))';
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

E = trapz(x,power);

title( sprintf("Power, total energy = %.3fJ",E) )
xlabel("Time (s)")
ylabel("Power (W)")
hold off

% subplot(3,2,[5 6])
% power_wide = (dat_trim_wide(:,1)/1000).*(dat_trim_wide(:,2)/1000);
% plot(x_lon, power_wide)
% xline(2.5)
% xline(x_lon(length(x_lon))-2.5)
% xlim([0 x_lon(length(dat_trim_wide))])
% title("Wider View")
% xlabel("Time (s)")
% ylabel("Power (W)")
% sgtitle(head)
% set(gcf,'WindowState','maximized')
% % saveas(gcf,savename)


%%
clear; clc; close all;

% data = csvread
% data = csvread("Lora power study.csv",1);
% data = csvread("Lora Power Study.csv",1);
% data = csvread("Power_Study_1.csv");
% data = csvread("lora_lp_ps_3.csv");
data = csvread("New_HP_Wio_IMU_Transmit\2025-08-22_11-24-04_log2.csv",4, 1)

datatrim = data(1:length(data)-1 ,:);
% datatrim = data(3000:10000,:);
power = (datatrim(:,1)/1000).*(datatrim(:,2)/1000);

x = linspace(0,  (length(datatrim)-1)*0.005, length(datatrim))';

hold on
xlim([0 x(length(x))])
plot(x, power)

[pks,pks_idx] = findpeaks(power,'MinPeakHeight',0.1);
scatter(x(pks_idx),pks);

maxGap = 100;                            
g = cumsum([true; diff(pks_idx) > maxGap]);  % group id per index
groups = splitapply(@(v){v}, pks_idx, g);   % cell array, one cell per pulse


seq = [];

for i = 1:length(groups)
    seq = [seq groups{i}(1)];

end
seq = seq'