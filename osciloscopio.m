% Gabriel Alexandre de Souza Braga
clear;
close all;
clc;

%% Parâmetros %%
instrreset
% Config. a conexão serial (mude a porta COMx se necessário)
s = serialport('/dev/ttyUSB0', 9600); 
N = 128; % Numero de amostras por quadro

write(s, 'x',"uint8");    % Envia comando 'x' para a bluepill enviar um quadro
rx = read(s, N,'uint8');  % Carrega o quadro
clear s;    % Deleta a porta serial

rx = rx*3.3/256;

Fs = 1e3;   % [Hz]
Ts = 1/Fs;  % período de amostragem
t = 0:Ts:(length(rx)-1)*Ts;

%% Plota o quadro capturado %%
% Cria figura do quadro capturado
figure1 = figure('PaperOrientation', 'landscape', 'PaperUnits', 'centimeters',...
    'PaperType', 'A4',...
    'WindowState', 'maximized',...
    'Color', [1 1 1],...
    'Renderer', 'painters');

% Cria plot do quadro capturado
axes1 = axes('Parent', figure1);
hold(axes1, 'on');

% Cria plot
plot1 = plot(t, rx, 'LineWidth', 3, 'Color', [0.00,0.45,0.74]);

% Cria rotulo y e x
ylabel('s(t)', 'FontWeight', 'bold', 'FontName', 'Times New Roman');
xlabel('t   [s]', 'FontWeight', 'bold', 'FontName', 'Times New Roman');

% Cria titulo
title('Quadro Capturado pelo micro-controlador');

% Define limites do plot, para x e y
xlim(axes1, [0 ((length(rx)-1)*Ts)]);
ylim(axes1, [0 3.3]);

% Liga as grades e etc
box(axes1, 'on');
grid(axes1, 'on');
hold(axes1, 'off');

% Define as propriedades restantes dos eixos
set(axes1, 'AlphaScale', 'log', 'ColorScale', 'log', 'FontName',...
    'Times New Roman', 'FontSize', 16, 'FontWeight', 'bold', 'GridAlpha', 0.5,...
    'LineWidth', 1.5, 'MinorGridAlpha', 0.5);
