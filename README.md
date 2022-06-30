# Osciloscópio
Este é o algoritmo de um osciloscópio feito utilizando o microcontrolador da placa Blue Pill (STM32F103C8T6) no VScode + Platformio + Matlab.
Este osciloscópio utiliza o ADC de 12 bits da blue pill, com uma taxa de amostragem de 1 kHz. O tamanho do quadro montado é de 128 amostras.

## Requsitos
É necessário ter o Visual Studio Code instalado, juntamente com o PlatformIO, que é uma extensão do VSCode, também será necessário possuir uma Blue Pill e o MATLAB instalado.

### Execução
Basta compilar o código .c no platformIO e fazer o upload na blue pill, em seguida rode o script .m no matlab. Também será necessário definir no script do matlab a porta USB em que a blue pill está conectada.

### Funcionamento
O script irá enviar o caracter 'x' para blue pill, ao receber do Matlab o caracter ’x’, o microcontrolador enviará 128 amostras do sinal medido, e o matlab fará o plot do sinal.
