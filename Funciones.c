#include <stdio.h>
#include <stdlib.h>
#include "Funciones.h"
#include <string.h>

void sumaIP(int *ip,char operando1,char operando2){
    *ip += 1 + !operando1 + !operando2;
}


//recibo primer byte de la instruccion, asigno tipos de operandos y operacion
void leePrimerByte(char instruccion,char *operando1,char *operando2,char *operacion){

    if((instruccion & 0xF0) == 0xF0)
        *operacion = instruccion;
    else{
        *operando1 = (instruccion & 0xc0) >> 6;
        *operando2 = instruccion & 0x30;

        if(*operando2 == 0x30)
            *operacion=instruccion & 0x3F;
        else{
            *operacion=instruccion & 0x0F;
            *operando2 = *operando2 >> 6;
        }
    }
}

char* intToHex(int n) {
    char* hex = (char*)malloc(sizeof(char) * 9);
    sprintf(hex, "%08X", n);  // Convierte el entero en hexadecimal
    if (n < 0) {
        // Si el número es negativo, se le agrega el signo "-"
        memmove(hex+1, hex, sizeof(char) * 8);
        hex[0] = '-';
    }
    return hex;
}

//char* instruccionDissasembler(int ip,int );
