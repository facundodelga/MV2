#include <stdio.h>
#include <stdlib.h>
#include "Funciones.h"
#include "Operaciones.h"
#include <string.h>

void cargaVectorDeFunciones(TOperaciones *);
unsigned short int acomodaTamanio(unsigned short int tamanio);

int main(int argc,char *argv[]){
    FILE *archBinario;
    char version;
    unsigned int operacion;
    TOperando operandos[2];
    char *header =(char *)malloc(sizeof(char) * 6);
    TInstruccionDisassembler vecDisassembler[16384];
    TMV mv;

    TOperaciones vecFunciones[242];
    t_funcionDisassembler imprimeFuncion[242];

    unsigned short int tamanio = 0;

    //if(argc > 1){
        //archBinario=fopen(argv[1],"rb");
        archBinario=fopen("sample (1).vmx","rb");
        if(archBinario){
            fgets(header,6 * sizeof(char),archBinario); //Obtengo el header

            if(strcmp(header,"VMX23") == 0){
                fread(&version,sizeof(char),1,archBinario);

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño del codigo y asigno al CS

                tamanio = acomodaTamanio(tamanio);

                mv.TDD[0][0] = 0X0000;
                mv.TDD[0][1] = tamanio;

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño del KS

                tamanio = acomodaTamanio(tamanio);

                mv.TDD[2][0] = mv.TDD[0][1];

                if(tamanio == 0){

                    mv.TDD[2][1] = mv.TDD[0][1];
                    mv.registros[2] = -1;
                    /*
                    Antes de comenzar la ejecución, la máquina virtual debe inicializar los registros CS, KS, DS, ES y SS con
                    los punteros al comienzo de sus respectivos segmentos. Si alguno de los segmentos no existe (su tamaño
                    es igual a cero), su respectivo registro deberá quedar cargado con un -1.
                    */
                }else{
                    mv.TDD[2][1] = mv.TDD[2][0] + tamanio;
                    mv.registros[2] = 0x0000FFFF & mv.TDD[0][1];
                 }

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño DS

                tamanio = acomodaTamanio(tamanio);

                mv.TDD[1][0] = mv.TDD[2][1];

                if(tamanio == 0){
                    mv.TDD[1][1] = mv.TDD[2][1];
                    mv.registros[1] = -1;
                }else{
                    mv.TDD[1][1] = mv.TDD[1][0] + tamanio;
                    mv.registros[2] = 0x0000FFFF & mv.TDD[1][0];
                }

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño ES

                tamanio = acomodaTamanio(tamanio);

                mv.TDD[3][0] = mv.TDD[1][1];

                if(tamanio == 0){
                    mv.TDD[3][1] = mv.TDD[1][1];
                    mv.registros[3] = -1;
                }else{
                    mv.TDD[3][1] = mv.TDD[1][0] + tamanio;
                    mv.registros[3] = 0x0000FFFF & mv.TDD[1][0];
                }

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño ES

                tamanio = acomodaTamanio(tamanio);

                mv.TDD[4][0] = mv.TDD[3][1];

                if(tamanio == 0){
                    mv.TDD[4][1] = mv.TDD[3][1];
                    mv.registros[3] = -1;
                }else{
                    mv.TDD[4][1] = mv.TDD[3][0] + tamanio;
                    mv.registros[4] = 0x0000FFFF & mv.TDD[4][0];
                }

                printf("MAQUINA VIRTUAL GRUPO G [ %s %02X %04X ]\n",header,version,tamanio);
                printf("\n");

                mv.registros[0] = 0;

                int numInstrucciones = 0;

                while(fread(&mv.memoria[numInstrucciones],sizeof(char),1,archBinario)){ //se lee el archivo binario para cargarlo en la memoria
//                    printf("%02X\n",mv.memoria[numInstrucciones]);
                    numInstrucciones++;
                }

                fclose(archBinario);

                cargaVectorDeFunciones(vecFunciones);

                mv.registros[5]=0; //IP == 0

                while(mv.registros[5] < mv.TDD[0][1]){ // IP menor al DS

                    leePrimerByte(mv.memoria[mv.registros[5]],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);

                    recuperaOperandos(&mv,operandos,mv.registros[5]);
//                    printf("operacion %02x\n",operacion);
//                    printf("tipo op1 %02x\n",operandos[0].tipo);
//                    printf("tipo op2 %02x\n",operandos[0].tipo);
//
//                    printf(" op1 %d\n",operandos[0].registro);
//                    printf(" op2 %d\n",operandos[0].registro);

                    if((operacion >= 0 && operacion < 12) || (operacion >= 0x30 && operacion < 0x3C) || (operacion >= 0xF0))
                        vecFunciones[operacion](&mv, operandos);
                    else{
                        printf("ERROR! LA FUNCION %02X NO EXISTE!... BYE BYE\n",operacion);
                        mv.registros[5] = mv.TDD[1];
                    }

                }

                //disassembler
                //int flagDisassembler = 1;
//                if(argc >= 3){
                    printf("entra al dissa");
                    //if(strcmp(argv[2],"-d") == 0){
                        printf("\n");
                        int i = 0;
                        int ipAssembler = 0;
                        while(ipAssembler < numInstrucciones){ // IP menor al DS

                            leePrimerByte(mv.memoria[ipAssembler],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);

                            vecDisassembler[i].operandos[0].tipo = operandos[0].tipo;
                            vecDisassembler[i].operandos[1].tipo = operandos[1].tipo;

                            vecDisassembler[i].ipInicio = ipAssembler;

                            vecDisassembler[i].codigoOperacion = operacion;

                            recuperaOperandos(&mv,operandos,ipAssembler);

                            vecDisassembler[i].operandos[0] = operandos[0];
                            vecDisassembler[i].operandos[1] = operandos[1];

                            sumaIP(&(ipAssembler),operandos[0].tipo,operandos[1].tipo);

                            vecDisassembler[i].ipFinal = ipAssembler;
                            i++;
                        }

                        cargaVectorDisassembler(imprimeFuncion);
                        vecDisassembler[0].ipInicio = 0;

                        for(int z = 0;z < i;z++){
                            printf("[%04X] ",vecDisassembler[z].ipInicio);
                            for(int ip = vecDisassembler[z].ipInicio; ip < vecDisassembler[z].ipFinal ;ip++){
                               printf("%02X ",mv.memoria[ip] & 0xFF);
                            }
                            printf("\t\t|\t");
                            imprimeFuncion[vecDisassembler[z].codigoOperacion](vecDisassembler[z]);
                            printf("\n");
                        }
                    //}

                //}

                printf("\nPEDRO ARIAS - FACUNDO DELGADO\n");
            }else
                printf("ERROR! LA EXTENSION DEL ARCHIVO NO SE PUEDE LEER... BYE BYE\n");

        }else
            printf("ERROR! EL ARCHIVO BINARIO NO EXISTE... BYE BYE\n");
//    }else
//        printf("ERROR! DEBE PROPORCIONAR DE UN NOMBRE DE ARCHIVO CON EXTENCION .VMX... BYE BYE\n");

    return 0;
}


unsigned short int acomodaTamanio(unsigned short int tamanio){
    unsigned short int aux1 = 0,aux2 = 0;
    aux1 = (tamanio >> 8) & 0x00FF;
    aux2 = (tamanio & 0x00FF) << 8;

    return aux2 | aux1;
}
