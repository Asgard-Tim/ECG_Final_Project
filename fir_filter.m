function Hd = fir_filter
%FIR_FILTER 返回离散时间滤波器对象。

% MATLAB Code
% Generated by MATLAB(R) 9.13 and Signal Processing Toolbox 9.1.

% FIR Window Lowpass filter designed using the FIR1 function.

% All frequency values are in Hz.
Fs = 500;  % Sampling Frequency

N             = 160;      % Order
Fc            = 35;       % Cutoff Frequency
flag          = 'scale';  % Sampling Flag
SidelobeAtten = 100;      % Window Parameter

% Create the window vector for the design algorithm.
win = chebwin(N+1, SidelobeAtten);

% Calculate the coefficients using the FIR1 function.
b  = fir1(N, Fc/(Fs/2), 'low', win, flag);
Hd = dfilt.dffir(b);

% [EOF]
