#include <stdio.h>
#include <stdlib.h>
#include "Funciones.h"
#include "Operaciones.h"




void cargaVectorDeFunciones(TOperaciones *);

int main(){
    FILE *archBinario;
    char operacion,version;
    TOperando operandos[2];
    char *header =(char *)malloc(sizeof(char) * 6);
    TMV mv;
    TOperaciones vecFunciones[100];
    short int tamanio;

    archBinario=fopen("fact.vmx","rb");

    fgets(header,6*sizeof(char),archBinario); //Obtengo el header
    printf("%s\n",header);
    fread(&version,sizeof(char),1,archBinario);
    //printf("%s\n",intToHex(version));
    fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño del codigo y asigno al DS
    tamanio = tamanio >> 8;

    mv.TDD[0] = 0;
    mv.TDD[1] = tamanio + 1;

    int numInstrucciones = 0;

    while(fread(&mv.memoria[numInstrucciones],sizeof(char),1,archBinario)){ //se lee el archivo binario para cargarlo en la memoria
        //printf("%s\n",intToHex(mv.memoria[numInstrucciones]));
        numInstrucciones++;

    }

    fclose(archBinario);

    cargaVectorDeFunciones(vecFunciones);
    mv.registros[5]=0; //IP == 0

    leePrimerByte(mv.memoria[mv.registros[5]],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);

    while(mv.registros[5] < mv.TDD[1]){ // IP menor al DS
        recuperaOperandos(&mv,operandos,mv.registros[5]);
        //printf("%s\n",intToHex(mv.registros[5]));

        vecFunciones[operacion](&mv, operandos);
        sumaIP(&(mv.registros[5]),operandos[0].tipo,operandos[1].tipo); //corro el IP a la proxima instruccion
        leePrimerByte(mv.memoria[mv.registros[5]],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);
    }

    //disassebler
    return 0;
}

