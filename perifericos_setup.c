#include "Perifericos_setup.h"

void Setup_GPIO(void){
    EALLOW;

    //GRUPO A: PINOS 0, 15 (1) E 16,31 (2)
    //GRUPO B: PINOS 32, 47 (1) E 48,64 (2)

    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = 0;   //CONFIGURANDO GRUPO DO MUX COMO 0
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;    //CONFIGURANDO O MUX COMO PWM

    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = 0;   //CONFIGURANDO GRUPO DO MUX COMO 0
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;    //CONFIGURANDO O MUX COMO PWM


    EDIS;
}

void Setup_ePWM(void){
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0; //DESABILITA O CLOCK DE SINCRONISMO DO CRYSTAL DO MIC PARA REALIZAR CONFIGURAÇÃO DO PWM | DESLIGA O PWM
    //PWM COM FREQUÊNCIA DE 25GHz
    //PWM 1A = GPIO 0
    //PWM 1B = GPIO 1
    EPwm1Regs.TBPRD = 2000; //REGISTRADOR QUE DEFINE O PERIODO | PRD = FREQ(CLOCK)/2*2*FREQ(PWM) | PRD = 200MHz/4*25GHz = 2000 | CTR CONTA ATÉ 2000 E RETORNA A 0 NO TEMPO DE (1/25G) SEGUNDOS
    EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD >> 1; //DUTY CICLE EM 50% DO PERIODO
    EPwm1Regs.TBPHS.bit.TBPHS = 0; //USADO PARA CONVERSOR CC/CA | FASE = 0
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE; //USADO PARA CONVERSOR CC/CA | SINCRONIZA O PWM DO SISTEMA TRIFÁSICO
    EPwm1Regs.TBCTR = 0; //REGISTRADOR QUE MARCA O INÍCIO DA CONTAGEM
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; //CONFIGURA PWM COMO UPDOWN (0x02)
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE; //USADO PARA CONVERSOR CC/CA | DESABILITA DEFASAGEM
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1; //DIVIDE A FREQUÊNCIA DO CLOCK PARA O PWM (NESTE CASO, FREQ(CLOCK)/1) | DIMINUI A FREQUÊNCIA DO PWM
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW; //CONFIGURA ATUALIZAÇÃO DO DUTY CICLE PARA VALORES DE CTR = 0 OU  CTR = 2000 OU CTR = 0 E CTR = 2000
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD; //ATUALIZA DUTY CICLE EM CTR = 2000 E EM CTR = 0
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW; //CONFIGURA ATUALIZAÇÃO DO DUTY CICLE PARA VALORES DE CTR = 0 OU  CTR = 2000 OU CTR = 0 E CTR = 2000
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD; //ATUALIZA DUTY CICLE EM CTR = 2000 E EM CTR = 0

    EPwm1Regs.AQCTLA.bit.PRD = AQ_NO_ACTION; //GPIO NÃO SOFRE ALTERAÇÃO EM CTR = PRD = 2000
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION; //GPIO NÃO SOFRE ALTERAÇÃO EM CTR = 0
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR; //GPIO = 0, QUANDO CTR = CMPA (SUBINDO)
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET; //GPIO = 1, QUANDO CTR = CMPA (DESCENDO)

    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; //PWM 1B É INVERTIDO EM RELAÇÃO AO PWM 1A
    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE; //HABILITA TEMPO DE DELAY DE DEAD BAND DOS TRANSISTORES
    EPwm1Regs.DBFED.bit.DBFED = 100; //TEMPO (EM ms) DO TRANSISTOR
    EPwm1Regs.DBRED.bit.DBRED = 100; //TEMPO (EM ms) DO TRANSISTOR

    //Geração do sinal de Trigger: f(ADC) = 2*f(PWM)
    EPwm1Regs.ETSEL.bit.SOCAEN = 1; //Habilita a conversão
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_PRDZERO; //Dispara o evento em 0 e em PRD
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST; //Dispara o Trigger no primeiro evento

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; //HABILITA O CLOCK DE SINCRONISMO DO CRYSTAL DO MIC DEPOIS DE REALIZADA A CONFIGURAÇÃO DO PWM

    EDIS;
}

void Setup_ADC(void){
    Uint16 acqps;
    if(ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION) //Tamanho da janela de amostragem: Quanto tempo o ADC ficará 
        //habilitado para capturar o sinal
        //Depende da resolução, 12bits ou 16bits
        acqps = 14; //14 pulsos = 72ns
    else
        acqps = 63; //63 pulsos = 63ns

    EALLOW;

    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //Numero de pulsos de clock do ADC, setando ADCCLK/4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE); //Configura O ADC A, baseado na resolução de 12 bits.
    //Amostragem tipo single (1 vez)
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1; //Seta o pulso um ciclo antes do resultado
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1; //Liga o ADC
    DELAY_US(1000); //Delay de 1ms para esperar o ADC ser ligado

    //Configuração dos canais 3 e 4 para o ADC
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 3; //Pino ADC = 3
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps; //Tamanho da janela de amostragem = 14 + 1
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = TRIG_SEL_ePWM1_SOCA; //Seleciona qual evento irá disparar o ADC (ePWM1) - Tabela 11-33 do Manual

    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 4; //Pino ADC = 4
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = acqps; //Tamanho da janela de amostragem = 14 + 1
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = TRIG_SEL_ePWM1_SOCA; //Seleciona qual evento irá disparar o ADC (ePWM1) - Tabela 11-33 do Manual

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0x01; //Habilita interrupção do ADC depois do SOC 1 ser acionado
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1; //Habilita interrupção
    AdcaRegs.ADCINTGLFCLR.bit.ADCINT1 = 1; //Limpa o flag da interrupção

    EDIS;
}