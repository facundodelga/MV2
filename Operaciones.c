#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Funciones.h"
#include "Operaciones.h"

void ejecutaCicloProcesador(TMV *mv,char version){
    unsigned int condicion,operacion;
    TOperando operandos[2];
    TOperaciones vecFunciones[256];

    cargaVectorDeFunciones(vecFunciones);
    leePrimerByte(mv->memoria[mv->registros[5]],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);
    recuperaOperandos(mv,operandos,mv->registros[5]);
    switch (version){
    case 1: condicion=(operacion >= 0 && operacion < 12) || (operacion >= 0x30 && operacion < 0x3C) || (operacion == 0xF0);
        break;
    case 2: condicion=(operacion >= 0 && operacion < 12) || (operacion >= 0x30 && operacion < 0x3F) || (operacion >= 0xF0 && operacion < 0xF2);
        break;
    }
    if(condicion)
         vecFunciones[operacion](mv, operandos);
    else{
        printf("ERROR! LA FUNCION %02X NO EXISTE!... BYE BYE\n",operacion);
        mv->registros[5] = mv->TDD[2][0];
    }
}

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

void creaArchivoDeImagen(TMV mv){
    FILE *archImagen;
    unsigned short int i,tamanio=mv.header[6]<<8|mv.header[7];
    char DD; //DD=descriptor de segmento

    archImagen=fopen(mv.imagenArchivo,"wb");
    if(archImagen!=NULL){
    for(i=0;i<8;i++)
        fprintf(archImagen,"%c",mv.header[i]);
    for(i=0;i<15;i++){
        fprintf(archImagen,"%c",(mv.registros[i]&0xFF000000)>>24);
        fprintf(archImagen,"%c",(mv.registros[i]&0x00FF0000)>>16);
        fprintf(archImagen,"%c",(mv.registros[i]&0x0000FF00)>>8);
        fprintf(archImagen,"%c",mv.registros[i]&0x000000FF);
    }
    for(i=0;i<8;i++){
        DD=mv.TDD[i][0]<<4 | mv.TDD[i][1];
        fprintf(archImagen,"%c",DD);
    }
    for(i=0;i<tamanio;i++){
        fprintf(archImagen,"%c",mv.memoria[i]);
    }
}
    close(archImagen);
}

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
    unsigned int num = 0;

    //printf("get puntero %02X\n",mv->registros[o.registro]);
    if(mv->registros[o.registro] >= mv->TDD[1][0] && mv->registros[o.registro] < mv->TDD[1][1]){

        if(o.segmentoReg == 0x00){ //segmento de 4 bytes
                num |= mv->memoria[mv->registros[o.registro] + o.desplazamiento] << 24;
                num |= (0x00FF0000 & (mv->memoria[mv->registros[o.registro] + o.desplazamiento + 1] << 16));
                num |= (0x0000FF00 & (mv->memoria[mv->registros[o.registro] + o.desplazamiento + 2] << 8));
                num |= (0x000000FF & (mv->memoria[mv->registros[o.registro] + o.desplazamiento + 3]));

        }else{
            if(o.segmentoReg == 0x02){ //segmento 2 bytes
                    //      "|=" me haces re mal abuela
                num |= mv->memoria[mv->registros[o.registro] + o.desplazamiento] << 8;
                num |= mv->memoria[mv->registros[o.registro] + o.desplazamiento + 1];

            }else //segmento de 1 byte
                num = mv->memoria[mv->registros[o.registro] + o.desplazamiento];
        }
    }else{
        printf("ERROR DE FALLO DE SEGMENTO!... BYE BYE\n");
        mv->registros[5] = mv->TDD[1][0];
    }
    return num;
}
void recuperaOperandos(TMV *mv,TOperando *o,int ip){
    char aux;
    int auxInt = 0;
    switch(o[0].tipo){

        case 0x00: //tipo memoria
            aux = mv->memoria[++ip];  //leo en un auxiliar el byte que dice el registro en el que se va a almacenar
            o[1].segmentoReg = (aux >> 6) & 0x03;
            aux = aux & 0x0F;
            o[0].registro = (int)aux;
            //printf("registro t mem op 1 %d\n",aux);
            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el desplazamiento de bytes
            auxInt |= mv->memoria[++ip];
            o[0].desplazamiento = auxInt;
            o[0].segmentoReg = 0;
            //printf("desplazamiento t mem op 1 %d\n",aux);
            break;

        case 0x01: //tipo inmediato
            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el numero inmediato
//            printf("set inm %04X\n",auxInt);
            auxInt |= (mv->memoria[++ip] & 0x00FF);
//            printf("set inm %04X\n",auxInt);
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
            o[1].segmentoReg = (aux >> 6) & 0x03;
            aux = aux & 0x0F;
            o[1].registro = aux;

            //printf("segmento op 2 %d\n",o[0].segmentoReg);
            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el desplazamiento de bytes
            auxInt |= mv->memoria[++ip];
            o[1].desplazamiento = auxInt;

            break;

        case 0x01: //tipo inmediato
            auxInt = (int) mv->memoria[++ip] << 8;
            auxInt |= (mv->memoria[++ip] & 0x000000FF);
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

            if(mv->registros[o.registro] >= mv->TDD[1][0] && mv->registros[o.registro] < mv->TDD[1][0] + mv->TDD[1][1]){

                if(o.segmentoReg == 0x00){ //segmento de 4 bytes
                    mv->memoria[mv->registros[o.registro] + o.desplazamiento] = (char)((num >> 24) & 0x000000FF);
                    mv->memoria[mv->registros[o.registro] + o.desplazamiento + 1] = (char)((num >> 16) & 0x000000FF);
                    mv->memoria[mv->registros[o.registro] + o.desplazamiento + 2] = (char)((num >> 8) & 0x000000FF);
                    mv->memoria[mv->registros[o.registro] + o.desplazamiento + 3] = (char)(num & 0x000000FF);
                    if(mv->TDD[1][0] + mv->TDD[1][1] < mv->registros[o.registro] + o.desplazamiento + 3)
                        mv->TDD[1][1] += o.desplazamiento + 4;
                }else{
                    if(o.segmentoReg == 0x02){ //segmento 2 bytes
                        mv->memoria[mv->registros[o.registro] + o.desplazamiento] = (char) (num >> 8) & 0x000000FF;
                        mv->memoria[mv->registros[o.registro] + o.desplazamiento + 1] = (char) num & 0x000000FF;
                        if(mv->TDD[1][0] + mv->TDD[1][1] < mv->registros[o.registro] + o.desplazamiento + 1)
                            mv->TDD[1][1] += o.desplazamiento + 2;

                    }else{ //segmento de 1 byte
                        mv->memoria[mv->registros[o.registro] + o.desplazamiento] = (char) num >> 24;
                        if(mv->TDD[1][0] + mv->TDD[1][1] < mv->registros[o.registro] + o.desplazamiento)
                            mv->TDD[1][1] += o.desplazamiento + 1;
                    }
                }
            }else{
                printf("ERROR DE FALLO DE SEGMENTO!... BYE BYE\n");
                mv->registros[5] = mv->TDD[1][0];
            }

        break;

        case 0x02: // tipo registro

            if(o.segmentoReg == 0x03){ //segmento X 2 ultimos bytes

                mv->registros[(int)o.registro] &= 0xFFFF0000; //limpia los 2 últimos bytes del registro
                mv->registros[(int)o.registro] |= (num & 0x0000FFFF); //asigna los 2 últimos bytes del entero al registro

            } else if(o.segmentoReg == 0x02){ // segmento H 3er byte

                mv->registros[(int)o.registro] &= 0xFFFF00FF; //limpia el byte del registro
                mv->registros[(int)o.registro] |= ((num & 0x000000FF) << 8); //asigna el 3er byte del entero al registro

            } else if(o.segmentoReg == 0x01){ //segmento L 4to byte

                mv->registros[(int)o.registro] &= 0xFFFFFF00; //limpia el último byte del registro
                mv->registros[(int)o.registro] |= (num & 0x000000FF); //asigna el último byte del entero al registro

            } else if(o.segmentoReg == 0x00) {

                mv->registros[(int)o.registro] = num; //registro completo

            }

            break;
        default:
            printf("NO SE PUEDE ASIGNAR A UN TIPO INMEDIATO!... BYE BYE");
            STOP(mv,&o);
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
    v[0x3C] = PUSH;
    v[0x3D] = POP;
    v[0x3E] = CALL;
    //operaciones sin op
    v[0xF0] = STOP;
    v[0xF1] = RET;
}


void readSys(TMV *mv,TSistema aux){
    int i = 0;
    int dato;
    TOperando auxOp;
    auxOp.tipo = 0;
    auxOp.registro = 13;
    auxOp.desplazamiento = 0;

    if(aux.tamanio == 4)
        auxOp.segmentoReg = 0;
    else
        auxOp.segmentoReg = aux.tamanio;

    while(i<aux.cantidad){
         printf("[%04X] ",mv->registros[13] - mv->TDD[1][0] + auxOp.desplazamiento);
        switch (aux.formato){
        case 1:
            scanf(" %d",&dato);
            setOp(mv,auxOp,dato);
            break;
        case 2:
            scanf(" %c",&dato);
            setOp(mv,auxOp,dato);
            break;
        case 4:
            scanf(" %o",&dato);
            setOp(mv,auxOp,dato);
            break;
        case 8:
            scanf(" %X",&dato);
            setOp(mv,auxOp,dato);
            break;
        }
        i++;
        auxOp.desplazamiento = i * aux.tamanio;
    }
}
void writeSys(TMV *mv,TSistema aux){
    int i = 0;
    TOperando auxOp; //creo una variable auxiliar para pedir a memoria
    int auxint = 0;
    auxOp.registro = 13;
    auxOp.desplazamiento = 0;

    if(aux.tamanio == 4)
        auxOp.segmentoReg = 0; // si es 4 bytes asigno 0 por que asi va xd
    else
        auxOp.segmentoReg = aux.tamanio;
  //printf("formato de print %d\n",aux.formato);

    while(i < aux.cantidad){
        printf("[%04X] ",mv->registros[13] - mv->TDD[1][0] + auxOp.desplazamiento);
        switch (aux.formato){
        case 1:
            printf("%d\n",getMem(mv,auxOp));
            break;
        case 2:
            auxint = getMem(mv,auxOp);
            if(32 <= auxint && auxint < 127)
                printf("%c %d\n",getMem(mv,auxOp));
            else
                printf(". %d\n",auxint);
            break;
        case 4:
            printf("%o\n",getMem(mv,auxOp));
            break;
        case 8:
            printf("%08X\n",getMem(mv,auxOp));
            break;
        case 9:
            auxint = getMem(mv,auxOp);
            printf(" # %d HEXA %08X\n",auxint,auxint);
            break;
        case 15:
            auxint = getMem(mv,auxOp);
            if(32 <= auxint && auxint < 127)
                printf("' %c # %d @ %o  HEXA %08X\n",auxint,auxint,auxint,auxint);
            else
                printf("' . # %d @ %o  HEXA %08X\n",auxint,auxint,auxint);

            break;
        default:
            i = aux.cantidad;
            break;
        }
        i++;
        auxOp.desplazamiento = i * aux.tamanio;
    }
}
void readStringSys(TMV *mv,TSistema aux){
    char *st;
    if(aux.tamanio > 0){
        st = (char *)malloc(aux.tamanio * sizeof(char));
    }else if(aux.tamanio == -1){
        st = (char *)malloc(16384 * sizeof(char));
    }
    scanf(" %s",st);
    int i = 0;
    while(i < strlen(st)){
        mv->memoria[mv->registros[13] + i] = st[i];
        i++;
    }

}
void writeStringSys(TMV *mv,TSistema aux){

    char *st;
    int cantCaracteres = 0;

    while(mv->memoria[mv->registros[13] + cantCaracteres] != '\0'){
        cantCaracteres++;
    }

    if(cantCaracteres > 0){
        st = (char * ) malloc(cantCaracteres * (sizeof(char)));

        for(int i = 0;i <= cantCaracteres;i++){
            strcat(*st,mv->memoria[mv->registros[13] + i]);
        }

        printf("%s\n",*st);
    }
}
void breakPointSys(TMV *mv,TSistema aux){
    char quitOEnter;
    if(mv->imagenArchivo!=NULL){
        do{
          creaArchivoDeImagen(*mv);
          scanf(" %c\n",&quitOEnter);
          if(quitOEnter==13) //si es igual a enter se ejecuta el ciclo del procesador con la siguiente instruccion
            ejecutaCicloProcesador(mv,mv->header[5]);
        }while(quitOEnter!=113); //si es igual a q se detiene el breakpoint
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
    //printf("%04X * %04X = \n",getOp(mv,op[0]),getOp(mv,op[1]));
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
    setOp(mv,op[0],aux);
    //printf("====== %04X\n",getOp(mv,op[0]));
    setCC(mv,aux);

}
void DIV(TMV *mv, TOperando *op){
    int aux;
    if(getOp(mv,op[1])){
        aux = (int) getOp(mv,op[0]) / getOp(mv,op[1]);
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
//    printf("CMP operando 1 %08X == ",getOp(mv,op[0]));
//    printf("operando 2 %08X\n",getOp(mv,op[1]));
//     printf("====== %d\n",aux);
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
    t_funcionSys vecLlamadas[0xF1];
    vecLlamadas[1] = readSys;
    vecLlamadas[2] = writeSys;
    vecLlamadas[3] = readStringSys;
    vecLlamadas[4] = writeStringSys;
    vecLlamadas[0xF] = breakPointSys;
    TSistema aux;
    int llamada = getOp(mv,op[0]);
    if(llamada == 1 || llamada == 2){
    //    printf("sys registro %02X\n",mv->registros[13]);
        aux.posicion = mv->registros[13];
        aux.cantidad = mv->registros[12] & 0x000000FF;
        aux.tamanio = (mv->registros[12] >> 8) & 0x000000FF;
        aux.formato = mv->registros[10] & 0x000000FF;
    }else if(llamada == 3 || llamada == 4){
        aux.posicion = mv->registros[13];
        aux.tamanio = mv->registros[12] & 0x0000FFFF;
    }else if(llamada == 7){
        system("cls");
    }
    vecLlamadas[llamada](mv,aux);

    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JMP(TMV *mv, TOperando *op){
    int aux;
    aux = getOp(mv,op[0]);
    mv->registros[5] =(unsigned int) aux;
//    printf("JMP  %02X\n",mv->registros[5]);
}
void JZ(TMV *mv, TOperando *op){
    unsigned int aux;
    aux = getOp(mv,op[0]);
    if((mv->registros[8] & 0xF0000000) == 0x40000000){
       mv->registros[5] = aux;
//       printf("Jz  %02X\n",mv->registros[5]);
    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JP(TMV *mv, TOperando *op){
    if((mv->registros[8] & 0xF0000000) == 0x00000000){
        mv->registros[5] = getOp(mv,op[0]);
//        printf("JP  %02X\n",mv->registros[5]);

    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JN(TMV *mv, TOperando *op){
    if((mv->registros[8] & 0xF0000000) == 0x80000000){
        mv->registros[5] = getOp(mv,op[0]);
//       printf("Jn  %02X\n",mv->registros[5]);

    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JNZ(TMV *mv, TOperando *op){
    if((mv->registros[8] & 0xF0000000) != 0x40000000){
        mv->registros[5] = getOp(mv,op[0]);
//        printf("JNZ  %02X\n",mv->registros[5]);

    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JNP(TMV *mv, TOperando *op){
    if((mv->registros[8] & 0xF0000000)  != 0x00000000){
        mv->registros[5] = getOp(mv,op[0]);
//        printf("JNP  %02X\n",mv->registros[5]);

    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void JNN(TMV *mv, TOperando *op){

    if((mv->registros[8] & 0xF0000000) != 0x80000000){
        mv->registros[5] = getOp(mv,op[0]);
//        printf("JNN  %02X\n",mv->registros[5]);

    }else
        sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void LDL(TMV *mv, TOperando *op){
    mv->registros[9] &= 0xFFFF0000;
//    printf("LDL %02X\n",getOp(mv,op[0]));
    mv->registros[9] |= (getOp(mv,op[0]) & 0x0000FFFF);
    sumaIP(&(mv->registros[5]),op[0].tipo,op[1].tipo);
}
void LDH(TMV *mv, TOperando *op){
    mv->registros[9] &= 0x0000FFFF;
//    printf("LDH %02X\n",getOp(mv,op[0]));
    mv->registros[9] |= ((getOp(mv,op[0])<<16) & 0xFFFF0000);
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

void PUSH(TMV *mv,TOperando *op){
    int aux;
    mv->registros[6]-=4;
    if(mv->registros[6]<mv->registros[4]){
        printf("ERROR: STACK OVERFLOW");
        STOP(mv,op);
    }else{
        aux=getOp(mv,op[0]);
        mv->memoria[mv->registros[6]+3]=(aux & 0x000000FF);
        mv->memoria[mv->registros[6]+2]=(aux & 0x0000FF00)>>8;
        mv->memoria[mv->registros[6]+1]=(aux & 0x00FF0000)>>16;
        mv->memoria[mv->registros[6]]=(aux & 0xFF000000)>>24;
    }
}
void POP(TMV *mv,TOperando *op){
    int aux,tamanio=0x00000000 | mv->header[6]<<8 | mv->header[7];
    if(mv->registros[6]-4<tamanio){
        printf("ERROR: STACK UNDERFLOW");
        STOP(mv,op);
    }else{
        aux=(mv->memoria[mv->registros[6]]<<24) | (mv->memoria[mv->registros[6]+1]<<16) | (mv->memoria[mv->registros[6]+2]<<8) | mv->memoria[mv->registros[6]+3];
        setOp(mv,op[0],aux);
        mv->registros[6]+=4;
    }
}
void CALL(TMV *mv,TOperando *op){
    int aux;
    mv->registros[6]-=4;
    if(mv->registros[6]<mv->registros[4]){
        printf("ERROR: STACK OVERFLOW");
        STOP(mv,op);
    }else{
        aux=mv->registros[5];
        mv->memoria[mv->registros[6]+3]=(aux & 0x000000FF);
        mv->memoria[mv->registros[6]+2]=(aux & 0x0000FF00)>>8;
        mv->memoria[mv->registros[6]+1]=(aux & 0x00FF0000)>>16;
        mv->memoria[mv->registros[6]]=(aux & 0xFF000000)>>24;
        JMP(mv,op);
    }
}
void STOP(TMV *mv, TOperando *op){
     mv->registros[5] = mv->TDD[1][0];
}
void RET(TMV *mv,TOperando *op){
    int aux,tamanio=0x00000000 | mv->header[6]<<8 | mv->header[7];
    if(mv->registros[6]-4<tamanio){
        printf("ERROR: STACK UNDERFLOW");
        STOP(mv,op);
    }else{
        mv->registros[5]=(mv->memoria[mv->registros[6]]<<24) | (mv->memoria[mv->registros[6]+1]<<16) | (mv->memoria[mv->registros[6]+2]<<8) | mv->memoria[mv->registros[6]+3];
        mv->registros[6]+=4;
    }
}


void setCC(TMV *mv,int numero){
    if(numero==0)
        mv->registros[8] = 0x40000000;
    if(numero<0){
        mv->registros[8] = 0x80000000;
    }
    if(numero>0)
        mv->registros[8] = 0x00000000;
}



//DISSASEMBLER

void cargaVectorDisassembler(t_funcionDisassembler *v){
    //operaciones de 2 operandos
    v[0x0] = imprimeMOV;
    v[0x1] = imprimeADD;
    v[0x2] = imprimeSUB;
    v[0x3] = imprimeSWAP;
    v[0x4] = imprimeMUL;
    v[0x5] = imprimeDIV;
    v[0x6] = imprimeCMP;
    v[0x7] = imprimeSHL;
    v[0x8] = imprimeSHR;
    v[0x9] = imprimeAND;
    v[0xA] = imprimeOR;
    v[0xB] = imprimeXOR;
    //operaciones de 1 operandos
    v[0x30] = imprimeSYS;
    v[0x31] = imprimeJMP;
    v[0x32] = imprimeJZ;
    v[0x33] = imprimeJP;
    v[0x34] = imprimeJN;
    v[0x35] = imprimeJNZ;
    v[0x36] = imprimeJNP;
    v[0x37] = imprimeJNN;
    v[0x38] = imprimeLDL;
    v[0x39] = imprimeLDH;
    v[0x3A] = imprimeRND;
    v[0x3B] = imprimeNOT;
    v[0x3C] = imprimePUSH;
    v[0x3D] = imprimePOP;
    v[0x3E] = imprimeCALL;
    //operaciones sin operandos
    v[0xF0] = imprimeSTOP;
    v[0xF1] = imprimeRET;
}

void obtieneTAG(char reg,char segmento,char nombre[]){
    switch (reg){
    case 0x00:strcpy(nombre,"CS");
        break;
    case 0x01:strcpy(nombre,"DS");
        break;
    case 0x02:strcpy(nombre,"KS");
        break;
    case 0x03:strcpy(nombre,"ES");
        break;
    case 0x04:strcpy(nombre,"SS");
        break;
    case 0x05:strcpy(nombre,"IP");
        break;
    case 0x06:strcpy(nombre,"SP");
        break;
    case 0x07:strcpy(nombre,"BP");
        break;
    case 0x08:strcpy(nombre,"CC");
        break;
    case 0x09:strcpy(nombre,"AC");
        break;
    case 0x0A:
        switch (segmento){
                case 0:strcpy(nombre,"EAX");
                    break;
                case 1:strcpy(nombre,"AL");
                    break;
                case 2:strcpy(nombre,"AH");
                    break;
                case 3:strcpy(nombre,"AX");
                    break;
        }
        break;
    case 0x0B:
        switch (segmento){
                case 0:strcpy(nombre,"EBX");
                    break;
                case 1:strcpy(nombre,"BL");
                    break;
                case 2:strcpy(nombre,"BH");
                    break;
                case 3:strcpy(nombre,"BX");
                    break;
        }
        break;
    case 0x0C:
        switch (segmento){
                case 0:strcpy(nombre,"ECX");
                    break;
                case 1:strcpy(nombre,"CL");
                    break;
                case 2:strcpy(nombre,"CH");
                    break;
                case 3:strcpy(nombre,"CX");
                    break;
        }
        break;
    case 0x0D:
        switch (segmento){
                case 0:strcpy(nombre,"EDX");
                    break;
                case 1:strcpy(nombre,"DL");
                    break;
                case 2:strcpy(nombre,"DH");
                    break;
                case 3:strcpy(nombre,"DX");
                    break;
        }
        break;
    case 0x0E:
        switch (segmento){
            case 0:strcpy(nombre,"EEX");
                break;
            case 1:strcpy(nombre,"EL");
                break;
            case 2:strcpy(nombre,"EH");
                break;
            case 3:strcpy(nombre,"EX");
                break;
        }
        break;
    case 0x0F:
        switch (segmento){
                case 0:strcpy(nombre,"EFX");
                    break;
                case 1:strcpy(nombre,"FL");
                    break;
                case 2:strcpy(nombre,"FH");
                    break;
                case 3:strcpy(nombre,"FX");
                    break;
        }
        break;
    }
}

void imprimeOperando(TOperando op){
    char nombre[5];
    switch (op.tipo){

    case 0x00://memoria
              obtieneTAG(op.registro,op.segmentoReg,nombre);
              printf("[%s + %d]",nombre,op.desplazamiento);
        break;
    case 0x01://inmediato
              printf(" %d",op.desplazamiento);
        break;
    case 0x02://registro
              obtieneTAG(op.registro,op.segmentoReg,nombre);
              printf("%s ",nombre);
        break;
    }
}

void imprimeMOV(TInstruccionDisassembler disInstruccion){
    printf("MOV ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeADD(TInstruccionDisassembler disInstruccion){
    printf("ADD ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSUB(TInstruccionDisassembler disInstruccion){
    printf("SUB ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSWAP(TInstruccionDisassembler disInstruccion){
    printf("SWAP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeMUL(TInstruccionDisassembler disInstruccion){
    printf("MUL ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeDIV(TInstruccionDisassembler disInstruccion){
    printf("DIV ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeCMP(TInstruccionDisassembler disInstruccion){
    printf("CMP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSHL(TInstruccionDisassembler disInstruccion){
    printf("SHL ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSHR(TInstruccionDisassembler disInstruccion){
    printf("SHR ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeAND(TInstruccionDisassembler disInstruccion){
    printf("AND ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeOR(TInstruccionDisassembler disInstruccion){
    printf("OR ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeXOR(TInstruccionDisassembler disInstruccion){
    printf("XOR ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf(",");
    imprimeOperando(disInstruccion.operandos[1]);
    printf("\n");
}
void imprimeSYS(TInstruccionDisassembler disInstruccion){
    printf("SYS ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJMP(TInstruccionDisassembler disInstruccion){
    printf("JMP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJZ(TInstruccionDisassembler disInstruccion){
    printf("JZ ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJP(TInstruccionDisassembler disInstruccion){
    printf("JP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJN(TInstruccionDisassembler disInstruccion){
    printf("JN ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJNZ(TInstruccionDisassembler disInstruccion){
    printf("JNZ ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJNP(TInstruccionDisassembler disInstruccion){
    printf("JNP ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeJNN(TInstruccionDisassembler disInstruccion){
    printf("JNN ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeLDL(TInstruccionDisassembler disInstruccion){
    printf("LDL ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeLDH(TInstruccionDisassembler disInstruccion){
    printf("LDH ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeRND(TInstruccionDisassembler disInstruccion){
    printf("RND ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeNOT(TInstruccionDisassembler disInstruccion){
    printf("NOT ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimePUSH(TInstruccionDisassembler disInstruccion){
    printf("PUSH ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimePOP(TInstruccionDisassembler disInstruccion){
    printf("PULL ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeCALL(TInstruccionDisassembler disInstruccion){
    printf("CALL ");
    imprimeOperando(disInstruccion.operandos[0]);
    printf("\n");
}
void imprimeSTOP(TInstruccionDisassembler disInstruccion){
    printf("STOP ");
    printf("\n");
}
void imprimeRET(TInstruccionDisassembler disInstruccion){
    printf("RET ");
    printf("\n");
}
