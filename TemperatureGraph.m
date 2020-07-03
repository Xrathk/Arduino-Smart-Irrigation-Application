% Template MATLAB code for visualizing data from a channel as a 2D line
% plot using PLOT function.

% Prior to running this MATLAB code template, assign the channel variables.
% Set 'readChannelID' to the channel ID of the channel to read from. 
% Also, assign the read field ID to 'fieldID1'. 

% TODO - Replace the [] with channel ID to read data from:
readChannelID = 1064859;
% TODO - Replace the [] with the Field ID to read data from:
fieldID1 = 2; % Temperature

% Channel Read API Key 
% If your channel is private, then enter the read API
% Key between the '' below: 
readAPIKey = 'J1SNQUAQS9X6IBG5';

%% Read Data %%

[data, time] = thingSpeakRead(readChannelID, 'Field', fieldID1, 'NumPoints', 50, 'ReadKey', readAPIKey);
data(isnan(data)) = 45 % NaN values (due to arduino not sending temperature and moisture data simultaneously) close to average so they don't affect  graph

%% Visualize Data %%

plot(time, data);
title('Temperature Data')
xlabel('Time')
ylabel('Temperature')
legend('Temperature')
ylim([40 50])