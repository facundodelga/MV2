
#include "Operaciones.h"

int get(TMV *mv,TOperando o){
    char t_reg = 0x02, t_mem = 0x00, t_inm = 0x01;

    if(o.tipo == t_reg){
        return getReg(mv,o);

    }else if(o.tipo == t_mem){
        return getMem(mv,o);

    }else if(o.tipo == t_inm){
        return o.desplazamiento;
    }

}

int getReg(TMV *mv,TOperando o){
    int num;
    if(o.segmentoReg == 0x11){ //segmento X
        num = mv->registros[o.registro];
        num = num << 16;
        num = num >> 16;
    }else{
        if(o.segmentoReg == 0x10){ // segmento H
            num = mv->registros[o.registro];
            num = num << 16;
            num = num >> 24;
        }else{
            if(o.segmentoReg == 0x01){ //segmento L
                num = mv->registros[o.registro];
                num = num << 24;
                num = num >> 24;
            }else
                if(o.segmentoReg == 0x00)
                    num = mv->registros[o.registro];
        }
    }

    return num;
}


int getMem(TMV *mv,TOperando o){
    int num = 0;

    if(o.segmentoReg == 0x11){ //segmento 2 bytes
            //      "|=" me haces re mal abuela la concha de tu madre
        num |= mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento] << 8;
        num |= mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento + 1];
    }else{
        if(o.segmentoReg == 0x00){ //segmento de 4 bytes

            num |= mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento] << 24;
            num |= mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento + 1] << 16;
            num |= mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento + 2] << 8;
            num |= mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento + 3];

        }else //segmento de 1 byte
            num = mv->registros[mv->TDD[1] + o.registro + o.desplazamiento];
    }

    return num;
}

void recuperaOperandos(TMV *mv,TOperando *o,int ip){
    char aux;
    int auxInt = 0;
    switch(o[0].tipo){

        case 0x00: //tipo memoria
            aux = mv->memoria[++ip];  //leo en un auxiliar el byte que dice el registro en el que se va a almacenar
            aux = aux << 4;
            aux = aux >> 4;
            o[0].registro = aux;

            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el desplazamiento de bytes
            auxInt |= mv->memoria[++ip];
            o[0].desplazamiento = auxInt;
            break;

        case 0x01: //tipo inmediato
            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el numero inmediato
            auxInt |= mv->memoria[++ip];
            o[0].desplazamiento = auxInt;
            break;

        case 0x10: //tipo registro
            aux = mv->memoria[++ip];  //leo en un auxiliar el byte que dice el registro que voy a usar
            aux = aux << 4;
            aux = aux >> 4;
            o[0].registro = aux;

            aux = mv->memoria[ip]; //leo en un auxiliar el byte y saco el segmento de registro
            aux = aux << 4;
            aux = aux >> 6;
            o[0].segmentoReg = aux;
            break;
    }

    switch(o[1].tipo){

        case 0x00: //tipo memoria
            aux = mv->memoria[++ip];  //leo en un auxiliar el byte que dice el registro en el que se va a almacenar
            aux = aux << 4;
            aux = aux >> 4;
            o[1].registro = aux;

            if(o[0].tipo == 0x10) //le asigno el segmento para saber que cantidad de bytes voy a leer de memoria
                o[1].segmentoReg = o[0].segmentoReg;

            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el desplazamiento de bytes
            auxInt |= mv->memoria[++ip];
            o[1].desplazamiento = auxInt;
            break;

        case 0x01: //tipo inmediato
            auxInt |= mv->memoria[++ip] << 8; //leo en un int auxiliar los 2 bytes que representan el numero inmediato
            auxInt |= mv->memoria[++ip];
            o[1].desplazamiento = auxInt;
            break;

        case 0x10: //tipo registro
            aux = mv->memoria[++ip];  //leo en un auxiliar el byte que dice el registro que voy a usar
            aux = aux << 4;
            aux = aux >> 4;
            o[1].registro = aux;

            aux = mv->memoria[ip]; //leo en un auxiliar el byte y saco el segmento de registro
            aux = aux << 4;
            aux = aux >> 6;
            o[1].segmentoReg = aux;
            break;
    }
}

void setOp(TMV *mv,TOperando o,int num){
    switch(o.tipo){
        case 0x00: //tipo memoria

            if(o.segmentoReg == 0x00){ //segmento de 4 bytes
                mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento] = (char)((num >> 24) & 0xFF);
                mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento + 1] = (char)((num >> 16) & 0xFF);
                mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento + 2] = (char)((num >> 8) & 0xFF);
                mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento + 3] = (char)(num & 0xFF);

            }else{
                if(o.segmentoReg == 0x11){ //segmento 2 bytes
                    mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento] = (char) (num >> 8) & 0xFF;
                    mv->memoria[mv->TDD[1] + o.registro + o.desplazamiento + 1] = (char) num & 0xFF;

                }else //segmento de 1 byte
                    mv->registros[mv->TDD[1] + o.registro + o.desplazamiento] = (char) num;
            }
            break;

        case 0x10: // tipo registro

            if(o.segmentoReg == 0x11){ //segmento X 2 ultimos bytes

                mv->registros[o.registro] &= 0xFFFF0000; //limpia los 2 últimos bytes del registro
                mv->registros[o.registro] |= (num & 0x0000FFFF); //asigna los 2 últimos bytes del entero al registro

            } else if(o.segmentoReg == 0x10){ // segmento H 3er byte

                mv->registros[o.registro] &= 0x0000FFFF; //limpia los 2 primeros bytes del registro
                mv->registros[o.registro] |= ((num & 0x00FF0000) >> 8); //asigna el 3er byte del entero al registro

            } else if(o.segmentoReg == 0x01){ //segmento L 4to byte

                mv->registros[o.registro] &= 0xFFFFFF00; //limpia el último byte del registro
                mv->registros[o.registro] |= (num & 0x000000FF); //asigna el último byte del entero al registro

            } else if(o.segmentoReg == 0x00) {

                mv->registros[o.registro] = num; //registro completo

            }

            break;

    }
}

void cargaVectorDeFunciones(TOperaciones *v){
    //operaciones de 2 operandos
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
    //operaciones de 1 operandos
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
    //operaciones sin operandos
    v[0xF0] = STOP;
}

void MOV(TMV *mv,TOperando *op){

}
void ADD(TMV *mv, TOperando *op){

}
void SUB(TMV *mv, TOperando *op){

}
void MUL(TMV *mv, TOperando *op){

}
void DIV(TMV *mv, TOperando *op){

}
void SWAP(TMV *mv, TOperando *op){

}
void CMP(TMV *mv, TOperando *op){

}
void SHL(TMV *mv, TOperando *op){

}
void SHR(TMV *mv, TOperando *op){

}
void AND(TMV *mv, TOperando *op){

}
void OR(TMV *mv, TOperando *op){

}
void XOR(TMV *mv, TOperando *op){

}
void SYS(TMV *mv, TOperando *op){

}
void JMP(TMV *mv, TOperando *op){

}
void JZ(TMV *mv, TOperando *op){

}
void JP(TMV *mv, TOperando *op){

}
void JN(TMV *mv, TOperando *op){

}
void JNZ(TMV *mv, TOperando *op){

}
void JNP(TMV *mv, TOperando *op){

}
void JNN(TMV *mv, TOperando *op){

}
void LDL(TMV *mv, TOperando *op){

}
void LDH(TMV *mv, TOperando *op){

}
void RND(TMV *mv, TOperando *op){

}
void NOT(TMV *mv, TOperando *op){

}
void STOP(TMV *mv, TOperando *op){

}

