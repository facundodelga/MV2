#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Operaciones.h"

int getOp(TMV *mv,TOperando o){
    char t_reg = 0x02, t_mem = 0x00, t_inm = 0x01;

    if(o.tipo == t_reg){
        return getReg(mv,o);

    }else if(o.tipo == t_mem){
        return getMem(mv,o);

    }else if(o.tipo == t_inm){
        //printf("desplazamiento %d\n",o.desplazamiento);
        return o.desplazamiento;
    }

}

int getReg(TMV *mv,TOperando o){
    int num;
    if(o.segmentoReg == 0x03){ //segmento X
        num = mv->registros[(int)o.registro];
        num = num << 16;
        num = num >> 16;
    }else{
        if(o.segmentoReg == 0x02){ // segmento H
            num = mv->registros[(int)o.registro];
            num = num << 16;
            num = num >> 24;
        }else{
            if(o.segmentoReg == 0x01){ //segmento L
                num = mv->registros[(int)o.registro];
                num = num << 24;
                num = num >> 24;
            }else
                if(o.segmentoReg == 0x00)
                    num = mv->registros[(int)o.registro];
        }
    }

    return num;
}


int getMem(TMV *mv,TOperando o){
    int num = 0;
    if(mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento >= mv->TDD[1] && mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento < 16383){

        if(o.segmentoReg == 0x02){ //segmento 2 bytes
                //      "|=" me haces re mal abuela la concha de tu madre
            num |= mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento] << 8;
            num |= mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento + 1];
        }else{
            if(o.segmentoReg == 0x00){ //segmento de 4 bytes

                num |= mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento] << 24;
                num |= mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento + 1] << 16;
                num |= mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento + 2] << 8;
                num |= mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento + 3];

            }else //segmento de 1 byte
                num = mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento];
        }

    }else{
        printf("ERROR DE FALLO DE SEGEMENTO!... BYE BYE\n");
        STOP(mv,&o);
    }

    return num;
}

void recuperaOperandos(TMV *mv,TOperando *o,int ip){
    char aux;
    int auxInt = 0;
    switch(o[0].tipo){

        case 0x00: //tipo memoria
            aux = mv->memoria[++ip];  //leo en un auxiliar el byte que dice el registro en el que se va a almacenar
            aux = aux & 0x0F;
            o[0].registro = (int)aux;
            //printf("registro t mem op 1 %d\n",aux);
            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el desplazamiento de bytes
            auxInt |= mv->memoria[++ip];
            o[0].desplazamiento = auxInt;
            //printf("desplazamiento t mem op 1 %d\n",aux);
            break;

        case 0x01: //tipo inmediato
            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el numero inmediato
            auxInt |= mv->memoria[++ip];
            o[0].desplazamiento = auxInt;
            break;

        case 0x02: //tipo registro
            aux = mv->memoria[++ip];  //leo en un auxiliar el byte que dice el registro que voy a usar
            aux = aux & 0x0F;
            o[0].registro = aux;
            //printf("registro op %d\n",aux);
            aux = mv->memoria[ip]; //leo en un auxiliar el byte y saco el segmento de registro
            aux = aux >> 4;
            o[0].segmentoReg = aux & 0x03;
            //printf("segmento t reg op 1 %d\n",aux);
            break;
    }
    auxInt = 0;
    switch(o[1].tipo){

        case 0x00: //tipo memoria
            aux = mv->memoria[++ip];  //leo en un auxiliar el byte que dice el registro en el que se va a almacenar
            aux = aux & 0x0F;
            o[1].registro = aux;

            if(o[0].tipo == 0x02) //le asigno el segmento para saber que cantidad de bytes voy a leer de memoria
                o[1].segmentoReg = o[0].segmentoReg;

            //printf("segmento op 2 %d\n",o[0].segmentoReg);
            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el desplazamiento de bytes
            auxInt |= mv->memoria[++ip];
            o[1].desplazamiento = auxInt;
            break;

        case 0x01: //tipo inmediato
            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el numero inmediato
            auxInt |= mv->memoria[++ip];
            o[1].desplazamiento = auxInt;

            break;

        case 0x02: //tipo registro
            aux = mv->memoria[++ip];  //leo en un auxiliar el byte que dice el registro que voy a usar
            aux = aux & 0x0F;
            o[1].registro = aux;
            aux = mv->memoria[ip]; //leo en un auxiliar el byte y saco el segmento de registro
            aux = aux >> 4;
            o[1].segmentoReg = aux & 0x03;
            break;
    }
}

void setOp(TMV *mv,TOperando o,int num){
    switch(o.tipo){
        case 0x00: //tipo memoria

            if(o.segmentoReg == 0x00){ //segmento de 4 bytes
                mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento] = (char)((num >> 24) & 0xFF);
                mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento + 1] = (char)((num >> 16) & 0xFF);
                mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento + 2] = (char)((num >> 8) & 0xFF);
                mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento + 3] = (char)(num & 0xFF);
            }else{
                if(o.segmentoReg == 0x02){ //segmento 2 bytes
                    mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento] = (char) (num >> 8) & 0xFF;
                    mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento + 1] = (char) num & 0xFF;

                }else //segmento de 1 byte
                    mv->memoria[mv->TDD[1] + mv->registros[o.registro] + o.desplazamiento] = (char) num;
            }
            break;

        case 0x02: // tipo registro

            if(o.segmentoReg == 0x03){ //segmento X 2 ultimos bytes

                mv->registros[(int)o.registro] &= 0xFFFF0000; //limpia los 2 últimos bytes del registro
                mv->registros[(int)o.registro] |= (num & 0x0000FFFF); //asigna los 2 últimos bytes del entero al registro

            } else if(o.segmentoReg == 0x02){ // segmento H 3er byte

                mv->registros[(int)o.registro] &= 0x0000FFFF; //limpia los 2 primeros bytes del registro
                mv->registros[(int)o.registro] |= ((num & 0x00FF0000) >> 8); //asigna el 3er byte del entero al registro

            } else if(o.segmentoReg == 0x01){ //segmento L 4to byte

                mv->registros[(int)o.registro] &= 0xFFFFFF00; //limpia el último byte del registro
                mv->registros[(int)o.registro] |= (num & 0x000000FF); //asigna el último byte del entero al registro

            } else if(o.segmentoReg == 0x00) {

                mv->registros[(int)o.registro] = num; //registro completo

            }

            break;
    }
}

void cargaVectorDeFunciones(TOperaciones *v){
    //operaciones de 2 op
    v[0x0] = MOV;
    v[0x1] = ADD;
    v[0x2] = SUB;
    v[0x3] = SWAP;
    v[0x4] = MUL;
    v[0x5] = DIV;
    v[0x6] = CMP;
    v[0x7] = SHL;
    v[0x8] = SHR;
    v[0x9] = AND;
    v[0xA] = OR;
    v[0xB] = XOR;
    //operaciones de 1 op
    v[0x30] = SYS;
    v[0x31] = JMP;
    v[0x32] = JZ;
    v[0x33] = JP;
    v[0x34] = JN;
    v[0x35] = JNZ;
    v[0x36] = JNP;
    v[0x37] = JNN;
    v[0x38] = LDL;
    v[0x39] = LDH;
    v[0x3A] = RND;
    v[0x3B] = NOT;
    //operaciones sin op
    v[0xF0] = STOP;
}

char* intToHex2B(int n) {
    char* hex = (char*)malloc(sizeof(char) * 9);
    sprintf(hex, "%04X", n);  // Convierte el entero en hexadecimal
    return hex;
}

char* intToHex4B(int n) {
    char* hex = (char*)malloc(sizeof(char) * 9);
    sprintf(hex, "%08X", n);  // Convierte el entero en hexadecimal
    return hex;
}

int hextoint(char* hex) {
    int len = strlen(hex);
    int decimal = 0;
    int base = 1;
    int i;
    char sign = '0';

    if (hex[0] >= 'a' && hex[0] <= 'f') {
        sign = 'f';
    }

    for (i = len - 1; i >= 0; i--) {
        if (hex[i] >= '0' && hex[i] <= '9') {
            decimal += (hex[i] - '0') * base;
        }
        else if (hex[i] >= 'a' && hex[i] <= 'f') {
            decimal += (hex[i] - 'a' + 10) * base;
        }
        else if (hex[i] >= 'A' && hex[i] <= 'F') {
            decimal += (hex[i] - 'A' + 10) * base;
        }
        base *= 16;
    }

    if (sign == 'f') {
        int sign_bits = (sizeof(decimal) * 8) - log2(decimal) - 1;
        decimal |= (1 << sign_bits) - 1;
    }

    return decimal;
}

char* decToOctal(int num) {
    int octalNum[100];
    int i = 0;
    int sign = (num < 0) ? -1 : 1; // Guarda el signo del número.

    num = abs(num); // Convierte el número a su valor absoluto.

    // Divide el número sucesivamente por 8 y almacena el resto en un array.
    while (num != 0) {
        octalNum[i] = num % 8;
        num = num / 8;
        i++;
    }

    // Agrega el signo al resultado si es negativo.
    if (sign == -1) {
        i++; // Aumenta el contador para incluir el signo negativo.
    }

    // Crea la cadena de caracteres que representa el número octal.
    char* result = (char*)malloc((i+1) * sizeof(char)); // Reserva memoria para la cadena resultante.
    int j = 0;

    if (sign == -1) {
        result[j++] = '-';
    }

    // Agrega los dígitos del número octal a la cadena.
    for (int k = i - 1; k >= 0; k--) {
        result[j++] = octalNum[k] + '0';
    }
    result[j] = '\0'; // Agrega el caracter nulo al final de la cadena.

    return result;
}

int octtoint(char* oct) {
    int len = strlen(oct);
    int decimal = 0;
    int base = 1;
    int i;
    char sign = '0';

    if (oct[0] == '-') {
        sign = '7';
    }

    for (i = len - 1; i >= 0; i--) {
        if (oct[i] >= '0' && oct[i] <= '7') {
            decimal += (oct[i] - '0') * base;
        }
        base *= 8;
    }

    if (sign == '7') {
        int sign_bits = (sizeof(decimal) * 8) - log2(decimal) - 1;
        decimal |= (1 << sign_bits) - 1;
    }

    return decimal;
}


void readSys(TMV *mv,TSistema aux){
    int i = 0;
    int dato;
    char c;
    char stOct[12],stHex[9];
    TOperando auxOp;
    auxOp.registro = 13;
    auxOp.desplazamiento = 0;

    if(aux.tamanio == 4)
        auxOp.segmentoReg = 0; // si es 4 bytes asigno 0 por que asi va xd
    else
        auxOp.segmentoReg = aux.tamanio;

    while(i<aux.cantidad){
        switch (aux.formato){
        case 1:
            printf("[%s] ",intToHex2B(mv->registros[13] - mv->TDD[1] + auxOp.desplazamiento));
            scanf("%d",&dato);
            setOp(mv,auxOp,dato);
            break;
        case 2:
            printf("[%s] ",intToHex2B(mv->registros[13] - mv->TDD[1] + auxOp.desplazamiento));
            scanf("%c",&c);
            setOp(mv,auxOp,c);
            break;
        case 4:
            printf("[%s] ",intToHex2B(mv->registros[13] - mv->TDD[1] + auxOp.desplazamiento));
            scanf("%s",stOct);
            setOp(mv,auxOp,hextoint(stOct));
            break;
        case 8:
            printf("[%s] ",intToHex2B(mv->registros[13] - mv->TDD[1] + auxOp.desplazamiento));
            scanf("%s",stHex);
            setOp(mv,auxOp,hextoint(stHex));
            break;
        }
        i++;
        auxOp.desplazamiento = i * aux.tamanio;
    }
}

void writeSys(TMV *mv,TSistema aux){
    int i = 0;
    TOperando auxOp; //creo una variable auxiliar para pedir a memoria

    auxOp.registro = 13;
    auxOp.desplazamiento = 0;

    if(aux.tamanio == 4)
        auxOp.segmentoReg = 0; // si es 4 bytes asigno 0 por que asi va xd
    else
        auxOp.segmentoReg = aux.tamanio;
    //printf("formato de print %d\n",aux.formato);
    while(i < aux.cantidad){
        switch (aux.formato){
        case 1:
            printf("[%s] %d\n",intToHex2B(mv->registros[13] - mv->TDD[1] + auxOp.desplazamiento),getMem(mv,auxOp));
            break;
        case 2:
            printf("[%s] %c\n",intToHex2B(mv->registros[13] - mv->TDD[1] + auxOp.desplazamiento),getMem(mv,auxOp));
            break;
        case 4:
            printf("[%s] %s\n",intToHex2B(mv->registros[13] - mv->TDD[1] + auxOp.desplazamiento),decToOctal(getMem(mv,auxOp)));
            break;
        case 8:
            printf("[%s] %s\n",intToHex2B(mv->registros[13] - mv->TDD[1] + auxOp.desplazamiento),intToHex4B(getMem(mv,auxOp)));
            break;
        }
        i++;
        auxOp.desplazamiento = i * aux.tamanio;
    }
}

void sumaIP(int *ip,char operando1,char operando2){
    int op1,op2;
    if(operando1 == 0x0){
        op1 = 3;
    }else{
        if(operando1 == 0x1){
            op1 = 2;
        }else{
            if(operando1 == 0x2)
                op1 = 1;
            else
                op1 = 0;
        }
    }
    if(operando2 == 0x0){ // memoria
        op2 = 3;
    }else{
        if(operando2 == 0x1){ // inmediato
            op2 = 2;
        }else{
            if(operando2 == 0x2) // registro
                op2= 1;

            else
                op2 = 0;
        }
    }
    *ip += 1 + op1 + op2;
}

void MOV(TMV *mv,TOperando *op){
//    printf("operando 1 %d = ",getOp(mv,op[0]));
//    printf("operando 2 %d\n",getOp(mv,op[1]));
    setOp(mv,op[0],getOp(mv,op[1]));
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
//    printf("operando 1  = %d\n",getOp(mv,op[0]));
}
void ADD(TMV *mv, TOperando *op){
    int aux;
    aux = getOp(mv,op[0]) + getOp(mv,op[1]);

//    printf("suma operando 1 %d + ",getOp(mv,op[0]));
//    printf(" operando 2 %d = %d\n",getOp(mv,op[1]),aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
    setOp(mv,op[0],aux);
    setCC(mv,aux);

}
void SUB(TMV *mv, TOperando *op){
    int aux;
    aux = getOp(mv,op[0]) - getOp(mv,op[1]);
//    printf("sub operando 1 %d - ",getOp(mv,op[0]));
//    printf("operando 2 %d = %d\n",getOp(mv,op[1]),aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
    setOp(mv,op[0],aux);
    setCC(mv,aux);

}
void MUL(TMV *mv, TOperando *op){
    int aux;
    aux = getOp(mv,op[0]) * getOp(mv,op[1]);
//    printf("mul operando 1 %d * ",getOp(mv,op[0]));
//    printf("operando 2 %d = %d\n",getOp(mv,op[1]),aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
    setOp(mv,op[0],aux);
    setCC(mv,aux);

}
void DIV(TMV *mv, TOperando *op){
    int aux;
    if(getOp(mv,op[1])){
        aux=getOp(mv,op[0]) / getOp(mv,op[1]);
        mv->registros[9] = getOp( mv,op[0]) % getOp( mv,op[1]);
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
        setOp(mv,op[0],aux);
        setCC(mv,aux);
    }else{
        printf("Error fatal, esta intentando dividir por 0\n");
        printf("El programa se detendra");
        STOP(mv,op);
    }
}
void SWAP(TMV *mv, TOperando *op){
    int aux;
    aux=getOp(mv,op[0]);
    setOp(mv,op[0],getOp(mv,op[1]));
    setOp(mv,op[1],aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void CMP(TMV *mv, TOperando *op){
    int aux;
    aux = getOp(mv,op[0]) - getOp(mv,op[1]);
//    printf("CMP operando 1 %d == ",getOp(mv,op[0]));
//    printf("operando 2 %d\n",getOp(mv,op[1]));
    setCC(mv,aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void SHL(TMV *mv, TOperando *op){
    setOp(mv,op[0],getOp(mv,op[0]) << getOp(mv,op[1]));
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void SHR(TMV *mv, TOperando *op){
    int aux;
    aux = getOp(mv,op[0]) >> getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void AND(TMV *mv, TOperando *op){
    int aux;
    aux=getOp(mv,op[0]) & getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void OR(TMV *mv, TOperando *op){
    int aux;
    aux=getOp(mv,op[0]) | getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void XOR(TMV *mv, TOperando *op){
    int aux;
    aux=getOp(mv,op[0]) ^ getOp(mv,op[1]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void SYS(TMV *mv, TOperando *op){
    t_funcionSys vecLlamadas[3];
    vecLlamadas[1] = readSys;
    vecLlamadas[2] = writeSys;
    TSistema aux;

    aux.posicion = mv->registros[13];
    aux.cantidad = mv->registros[12] & 0x000000FF;
    aux.tamanio = (mv->registros[12] >> 8) & 0x000000FF;
    aux.formato = mv->registros[10] & 0x000000FF;

    vecLlamadas[getOp(mv,op[0])](mv,aux);

    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JMP(TMV *mv, TOperando *op){
    mv->registros[5] = getOp(mv,op[0]);
}
void JZ(TMV *mv, TOperando *op){
    if((mv->registros[8] & 0x40000000) == 0x40000000){
       mv->registros[5] = getOp(mv,op[0]);
//       //sumaIP(&(mv->registros[5]),0x3,0x3);
    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JP(TMV *mv, TOperando *op){
    if((mv->registros[8] & 0xc000) == 0xc000){
        mv->registros[5] = getOp(mv,op[0]);
        //sumaIP(&(mv->registros[5]),0x3,0x3);
    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JN(TMV *mv, TOperando *op){
    if((mv->registros[8] & 0x8000) == 0x8000){
        mv->registros[5] = getOp(mv,op[0]);
        //sumaIP(&(mv->registros[5]),0x3,0x3);
    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JNZ(TMV *mv, TOperando *op){
    if(mv->registros[8] != 0x4){
        mv->registros[5] = getOp(mv,op[0]);
        //sumaIP(&(mv->registros[5]),0x3,0x3);
    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JNP(TMV *mv, TOperando *op){
    if(mv->registros[8] != 0x0){
        mv->registros[5] = getOp(mv,op[0]);
        //sumaIP(&(mv->registros[5]),0x3,0x3);
    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JNN(TMV *mv, TOperando *op){
    if(mv->registros[8] != 0x8){
        mv->registros[5] = getOp(mv,op[0]);
        //sumaIP(&(mv->registros[5]),0x3,0x3);
    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void LDL(TMV *mv, TOperando *op){
    mv->registros[9]&=0xFF00;
    mv->registros[9]=(getOp(mv,op[0])&0x00FF) | mv->registros[9];
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void LDH(TMV *mv, TOperando *op){
    mv->registros[9]&=0x00FF;
    mv->registros[9]=(getOp(mv,op[0])<<16) | mv->registros[9];
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void RND(TMV *mv, TOperando *op){
    mv->registros[9]= rand() % (getOp(mv,op[0]) - 1);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void NOT(TMV *mv, TOperando *op){
    int aux;
    aux=getOp(mv,op[0]);
    setOp(mv,op[0],aux);
    setCC(mv,aux);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}

void STOP(TMV *mv, TOperando *op){
     mv->registros[5] = mv->TDD[1];
}

void setCC(TMV *mv,int numero){
    if(numero==0)
        mv->registros[8] = 0x40000000;
    if(numero<0)
        mv->registros[8] = 0x80000000;
    if(numero>0)
        mv->registros[8] = 0x00000000;
}
