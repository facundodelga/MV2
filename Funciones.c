#include <stdio.h>
#include <stdlib.h>
#include "Funciones.h"
#include <string.h>

//recibo primer byte de la instruccion, asigno tipos de operandos y operacion
void leePrimerByte(char instruccion,char *operando1,char *operando2,unsigned int *operacion){

    if((instruccion & 0xF0) == 0xF0)
        *operacion = 0xF0;
    else{
        *operando1 = (instruccion >> 6) & 0x03;
        *operando2 = instruccion & 0x30;

        if(*operando2 == 0x30){
            *operacion = instruccion & 0x3F; //aislo codigo de operacion
            *operando2 = 0x3;
        }else{
            *operacion = instruccion & 0x0F;
            *operando2 = *operando2 >> 4;
            *operando2 &= 0x03;// 0000 0011
        }
    }

//    printf("%d\n",*operando1);
//    printf("%d\n",*operando2);
}

