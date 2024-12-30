% 读取心电信号数据
dataLen = 4096;  % 数据长度
fs = 500;        % 采样频率 (Hz)
fid = fopen('serial_data_raw2000.dat', 'rb');
raw_data = fread(fid, [2, dataLen], 'uint8');
fclose(fid);

% 重新组合16位数据（低位在前，高位在后）
data = raw_data(1, :) + raw_data(2, :) * 256;
data(data > 32768) = data(data > 32768) - 65536;  % 转换为补码表示

% 绘制原始信号波形
figure;
plot(data);
title('原始心电信号时域波形');
xlabel('采样点');
ylabel('幅值');
grid on;

% 分析原始数据频谱
N = length(data);  % 数据长度
freq = (0:N-1) * fs / N;  % 频率坐标 (Hz)
spectrum = abs(fft(data));  % 计算频谱
figure;
plot(freq(1:N/2), spectrum(1:N/2));
title('原始信号频谱');
xlabel('频率 (Hz)');
ylabel('幅值');
grid on;

% 设计数字直流陷波器
% 系统函数 H(z) = (1 - z^(-1)) / (1 - a * z^(-1))
a = 0.992;  % 陷波器极点参数
b_dc = [1, -1];
a_dc = [1, -a];
filtered_data_dc = filter(b_dc, a_dc, data);

% 绘制直流陷波器后的频谱
spectrum_dc = abs(fft(filtered_data_dc));
figure;
plot(freq(1:N/2), spectrum_dc(1:N/2));
title('直流陷波器滤波后频谱');
xlabel('频率 (Hz)');
ylabel('幅值');
grid on;

%设计 FIR 数字低通滤波器
cutoff = 35;         % 截止频率 (Hz)
transition_width = 10;  % 过渡带宽度 (Hz)
numtaps = 64;        % 滤波器阶数
fir_coeff = fir1(numtaps, cutoff / (fs / 2), hamming(numtaps + 1));  % Hamming窗设计
filtered_data_lp = filter(fir_coeff, 1, filtered_data_dc);

% 绘制 FIR 滤波器频率响应
[h, w] = freqz(fir_coeff, 1, 1024, fs);
figure;
plot(w, 20*log10(abs(h)));
title('FIR 低通滤波器频率响应');
xlabel('频率 (Hz)');
ylabel('增益 (dB)');
grid on;

% 绘制低通滤波后的频谱
spectrum_lp = abs(fft(filtered_data_lp));
figure;
plot(freq(1:N/2), spectrum_lp(1:N/2));
title('低通滤波后频谱');
xlabel('频率 (Hz)');
ylabel('幅值');
grid on;

% 绘制滤波后波形并限制动态范围
% 动态范围限制
dynamic_range = 120;
min_val = min(filtered_data_lp);
max_val = max(filtered_data_lp);
scaled_data = dynamic_range * (filtered_data_lp - min_val) / (max_val - min_val);

% 绘制归一化后的波形
figure;
plot(scaled_data);
title('滤波后心电信号波形（归一化）');
xlabel('采样点');
ylabel('幅值');
grid on;

% 比较滤波前后频谱
figure;
plot(freq(1:N/2), spectrum(1:N/2), 'b-', 'DisplayName', '原始频谱');
hold on;
plot(freq(1:N/2), spectrum_lp(1:N/2), 'r-', 'DisplayName', '低通滤波后频谱');
hold off;
title('频谱对比');
xlabel('频率 (Hz)');
ylabel('幅值');
legend('show');
grid on;

% 估算心率
[pks, locs] = findpeaks(scaled_data, 'MinPeakHeight', 0.6 * max(scaled_data), 'MinPeakDistance', fs * 0.6);  % 检测波峰
rr_intervals = diff(locs) / fs;  % R-R 间期（秒）
heart_rate = 60 / mean(rr_intervals);  % 平均心率 (bpm)
fprintf('估算的心率：%.2f bpm\n', heart_rate);

% 可视化
figure;
plot(scaled_data);
hold on;
plot(locs, scaled_data(locs), 'rx', 'DisplayName', 'R 波');
hold off;
title('心电信号波形及 R 波检测');
xlabel('采样点');
ylabel('幅值');
legend('show');
grid on;
