#include <stdio.h>
#include <stdlib.h>
#include "Funciones.h"
#include <string.h>

//recibo primer byte de la instruccion, asigno tipos de operandos y operacion
void leePrimerByte(char instruccion,char *operando1,char *operando2,char *operacion){

    if((instruccion & 0xF0) == 0xF0)
        *operacion = 240;
    else{
        *operando1 = (instruccion >> 6) & 0x03;
        *operando2 = instruccion & 0x30;

        if(*operando2 == 0x30){
            *operacion = instruccion & 0x3F;
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

//char* intToHex(int n) {
//    char* hex = (char*)malloc(sizeof(char) * 9);
//    sprintf(hex, "%08X", n);  // Convierte el entero en hexadecimal
////    if (n < 0) {
////        // Si el número es negativo, se le agrega el signo "-"
////        memmove(hex+1, hex, sizeof(char) * 8);
////        hex[0] = '-';
////    }
//    return hex;
//}

//char* instruccionDissasembler(int ip,int );
