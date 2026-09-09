clear; clc; close all;

% filename = "V2_6_0_R0_CSID_3_PowerProfile.csv";
filename = "V2_5_0_R0_CSID_2_PowerProfile.csv";
df = readtable(filename);

Time = df.Time;
MET = seconds(Time - Time(1));
fprintf("Max I %.3f \n", max(df.MaxCurrent))
maskLen = length(MET);
% or 
maskLen = 500;

MET = MET(1:maskLen);
V = df.Voltage(1:maskLen);
I = df.Current(1:maskLen);
maxI = df.MaxCurrent(1:maskLen);

subplot(1,2,1);
plot(MET, V)
ylim([0,4000])

subplot(1,2,2)
hold on
plot(MET,I)
plot(MET,maxI)
xlabel("MET")
ylim([0,120]);

hold off

%%
clear; clc; close all;

% filename = "V2_6_0_R0_CSID_3_PowerProfile.csv";
filename = "V2_5_0_R0_CSID_2_PowerProfile.csv";
filename = "ESTIMATED_V2_5_0_R0_CSID_2_Wio_E5_LE_14dBm_PowerProfile.csv"
df = readtable(filename);

Time = df.Time;
MET = seconds(Time - Time(1));
fprintf("Max I %.3f \n", max(df.MaxCurrent))
maskLen = length(MET);
% or 
idx = find(MET<15.3);

MET = MET(idx);
V = df.Voltage(idx)/1000; % V
I = df.Current(idx)/1000; % A
maxI = df.MaxCurrent(idx);

subplot(1,3,1);
plot(MET, V)
ylabel("Voltage (V)")
xlabel("MET (s)");
ylim([0,4000/1000])

subplot(1,3,2)
hold on
plot(MET,I)
% plot(MET,maxI)
xlabel("MET (s)")
ylabel("Current (A)");
ylim([0,120/1000]);

subplot(1,3,3)
hold on
plot(MET,I.*V)
% plot(MET,maxI)
xlabel("MET (s)")
ylabel("Power (W)")
hold off

sgtitle("Power Profile of 2 events from a LP Wio")

P = I.*V;
E = trapz(MET,P);
fprintf("Total Energy is %.3f \n",E);