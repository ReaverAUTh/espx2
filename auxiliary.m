%% SCRIPT: Visualization of delay_offset and close contacts
%
% Author: Angelos Spyrakis (9352) - aspyrakis@ece.auth.gr
%
% Date: JUL 13, 2021
%
% An Aristotle University of Thessaloniki ECE Department
% project for course 077 - Real Time Embedded Systems
% --------------------------------------------------------
clear all; %#ok

%%
% Plot delay_offset
fid = fopen('delays.bin');
delays = fread(fid, 'double');
plot(delays, '-k', 'LineWidth', 2);
fclose(fid);

% Read close contacts
fid = fopen('close_contacts_COVID.bin');
contacts = fread(fid, 'uint64');
positive_tests = nnz(contacts == 0) %#ok

delay_offset=zeros(size(delays)-1);
for i=1:1:(size(delays)-1)
    delay_offset(i) = delays(i+1) - delays(i);
end
figure
plot(delay_offset)
mean_of_delay_offset = mean(delay_offset);
range(delay_offset)
std(delay_offset)
fclose(fid);
