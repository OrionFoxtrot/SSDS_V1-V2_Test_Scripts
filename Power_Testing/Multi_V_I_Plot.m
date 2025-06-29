
%V2 Charging Testing - Multi_Test_BAT_SOLAR_SAT

clear; clc; 
close all;
% figure()
save = 0;

name = "V2 Charging Testing - Multi_Test_BAT_SOLAR_SAT.csv";
% name = "V2 Charging Testing - Multi_Test_BAT_SOLAR_SAT_DISCHARGE.csv"

data = csvread(name,1);
n = 30; % num to trim off end

datatrim = data(1:length(data)-n ,:); % Trim data

% in seconds *0.250 for delay of 250ms between measurements
x = linspace(1,length(datatrim)*0.250, length(datatrim))';
x = x/60; % to min

% voltage/current for INA 1
subplot(3,2,1);
hold on
plot(x,datatrim(:,1)*10^-3,'DisplayName','Battery Voltage')

% f=fit(x,datatrim(:,1)*10^-3,'poly5');
% plot(f)

xlabel('Time (min)')
ylabel('Voltage (V)')
title('Voltage across Battery')
subplot(3,2,2);
hold on
plot(x,datatrim(:,2)*10^-3,'DisplayName','Battery Current')
xlabel('Time (min)')
ylabel('Current (A)')
title('Current through Battery')

% voltage/current for INA 2
subplot(3,2,3);
hold on
plot(x,datatrim(:,3)*10^-3,'DisplayName','Battery Voltage')
xlabel('Time (min)')
ylabel('Voltage (V)')
title('Voltage across Solar Cell')
subplot(3,2,4);
hold on
plot(x,datatrim(:,4)*10^-3,'DisplayName','Battery Current')
xlabel('Time (min)')
ylabel('Current (A)')
title('Current through Solar Cell')

% voltage/current for INA 3
subplot(3,2,5);
hold on
plot(x,datatrim(:,5)*10^-3,'DisplayName','Battery Voltage')
xlabel('Time (min)')
ylabel('Voltage (V)')
title('Voltage across ChipSat (Post Buck)')
subplot(3,2,6);
hold on
plot(x,datatrim(:,6)*10^-3,'DisplayName','Battery Current')
xlabel('Time (min)')
ylabel('Current (A)')
title('Current through ChipSat (Post Buck)')


% legend()
titleStr = sprintf('VI Cycle for: %s', name); 
sgtitle(titleStr,'Interpreter','None')


saveStr = char(name)
saveStr = saveStr(1:end-4)
saveStr = strcat(saveStr,'.png')

if save
    saveas(gcf,saveStr);
end


%% 
function y = simple_regress(t,x)
    % [x,t] = simplefit_dataset;
    net = feedforwardnet(10);
    net = train(net,x,t);
    y = net(x);
    %plotregression(t,y,'Regression')
end