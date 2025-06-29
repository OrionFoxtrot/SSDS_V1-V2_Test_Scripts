clear; clc; 
% close all;
figure()
save = 0;
% name = "V2 Charging Testing - Charge.csv";
name = "V2 Charging Testing - Drain.csv";
% name = "V2 Charging Testing - V1.csv"
% name = "V2 Charging Testing - V1_With_Delay.csv"
% name = "V2 Charging Testing - V1_Low_PWR_Delay_Sleep.csv"
% name = "V2 Charging Testing - V1_Long_Charge.csv"
% name = "V2 Charging Testing - V1_Super_Long_Charge.csv"

data = csvread(name,1);
datatrim = data(1:length(data)-1 ,:); %get rid of last failed link

x = linspace(1,length(datatrim), length(datatrim))';

subplot(1,2,1);
hold on
plot(x,datatrim(:,1)*10^-3,'DisplayName','Battery Voltage')
xlabel('Time (s)')
ylabel('Voltage (V)')
title('Voltage')
subplot(1,2,2);
hold on
plot(x,datatrim(:,2)*10^-3,'DisplayName','Battery Current')
xlabel('Time (s)')
ylabel('Current (A)')
title('Current')

% legend()
titleStr = sprintf('VI Cycle for: %s', name); 
sgtitle(titleStr,'Interpreter','None')


saveStr = char(name)
saveStr = saveStr(1:end-4)
saveStr = strcat(saveStr,'.png')

if save
    saveas(gcf,'akjjkfnsfjldnjlsd.png');
end
