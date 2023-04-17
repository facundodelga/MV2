#include <stdio.h>
#include <stdlib.h>
#include "Funciones.h"
#include "Operaciones.h"
#include <string.h>

void cargaVectorDeFunciones(TOperaciones *);

int main(int argc,char *argv[]){
    FILE *archBinario;
    char operacion,version;
    TOperando operandos[2];
    char *header =(char *)malloc(sizeof(char) * 6);
    TMV mv;
    TOperaciones vecFunciones[242];
    short int tamanio;
    //if(argc > 0){
        //archBinario=fopen(argv[1],"rb");
        archBinario=fopen("fact.vmx","rb");
        if(archBinario){
            fgets(header,6 * sizeof(char),archBinario); //Obtengo el header
            //printf("%s\n",header);
            if(strcmp(header,"VMX23") == 0){
                printf("%s\n",header);
                fread(&version,sizeof(char),1,archBinario);
                //printf("%s\n",intToHex(version));
                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño del codigo y asigno al DS
                tamanio = tamanio >> 8;

                mv.TDD[0] = 0;
                mv.TDD[1] = tamanio + 1;
                mv.registros[1] = mv.TDD[1];

                int numInstrucciones = 0;

                while(fread(&mv.memoria[numInstrucciones],sizeof(char),1,archBinario)){ //se lee el archivo binario para cargarlo en la memoria
                    //printf("%s\n",intToHex(mv.memoria[numInstrucciones]));
                    numInstrucciones++;

                }

                fclose(archBinario);

                cargaVectorDeFunciones(vecFunciones);
                mv.registros[5]=0; //IP == 0

                while(mv.registros[5] < mv.TDD[1]){ // IP menor al DS
                    leePrimerByte(mv.memoria[mv.registros[5]],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);
                    recuperaOperandos(&mv,operandos,mv.registros[5]);
                    if((operacion >= 0 && operacion < 12) || (operacion >= 0x30 && operacion < 0x3C) || (operacion >= 0xF0))
                        vecFunciones[operacion](&mv, operandos);
                    else{
                        printf("ERROR! LA FUNCION %s NO EXISTE!... BYE BYE\n",intToHex2B(operacion));
                        mv.registros[5] = mv.TDD[1];
                    }
                }
            }else
                printf("ERROR! LA EXTENSION DEL ARCHIVO NO SE PUEDE LEER... BYE BYE\n");

        }else
            printf("ERROR! EL ARCHIVO BINARIO NO EXISTE... BYE BYE\n");
//    }else
//        printf("ERROR! DEBE PROPORCIONAR DE UN NOMBRE DE ARCHIVO CON EXTENCION .VMX... BYE BYE\n");

    //disassebler
    return 0;
}

