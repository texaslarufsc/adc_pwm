#include "Perifericos_Setup.h"

uint32_t count = 0, index = 0;
uint16_t sinetable[400];

uint16_t adc1 = 0;
uint16_t adc2 = 0;

uint16_t plot[400];
uint16_t *adc = &adc1;

_interrupt isr_adc(void);

int main(void){

    //SETANDO OS REGISTRADORES
    InitSysCtrl();      //INICIALIZA O SISTEMA DO MICROCONTROLADOR (CLOCKS, MEMÓRIA...)

    DINT; //DESABILITA INTERRUPÇÕES
    InitPieCtrl(); //RESETA TODOS OS REGISTRADORES DA TABELA DE INTERRUPÇÕES (COLUNAS)
    //DESABLITANDO AS INTERRUPÇÕES
    IER = 0x0000; //RESETA O REGISTRADOR DE INTERRUPÇÕES (LINHAS)
    IFR = 0x0000; //RESETA O FLAG DAS INTERRUPÇÕES
    InitPieVectTable(); //PREENCHE OS REGISTRADORES DE INTERRUPÇÃO COM O ENDEREÇO DAS FUNÇÕES DOS PERIFÉRICOS

    Setup_GPIO();//CONFIGURA ENTRADAS E SAÍDAS
    Setup_ePWM();//CONFIGURA PWM

    EALLOW;
    PieVectTable.TIMER0_INT = &isr_adc;
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;  //ADC está na coluna 1, linha 1 da tabela de interrpções
    EDIS;
    IER |= M_INT1; //Habilita interrupção da linha 1


    //HABILITA NOVAMENTE AS INTERRUPÇÕES
    EINT; //INTERRUPÇÃO GLOBAL
    ERTM; //INTERRUPÇÃO EM TEMPO REAL

    while(1){//LOOP INFINITO
    }
    return 0;
}

_interrupt void isr_adc(void){
    GpioDataRegs.GPADAT.bit.GPIO14 = 1; //Pino de verificação de entrada da função

    adc1 = AdcaResultRegs.ADCRESULT0; //Valor amostrado é alocado nas variáveis
    adc2 = AdcaResultRegs.ADCRESULT1;

    index = (index == 400) ? 0 : (index + 1);

    EPwm7Regs.CMPA.bit.CMPA = sinetable[index];
    EPwm8Regs.CMPA.bit.CMPA = sinetable[index];

    plot[index] = *adc; //Plota o gráfico para mostrar a forma de onda que está sendo amostrada

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //Limpa o flag da interrupção
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; //Limpa o flag para todas interrupções
    GpioDataRegs.GPADAT.bit.GPIO14 = 0;
}