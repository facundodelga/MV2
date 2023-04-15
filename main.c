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

    archBinario=fopen("fact.vmx","rb");

    fgets(header,6*sizeof(char),archBinario); //Obtengo el header
    printf("%s\n",header);
    fread(&version,sizeof(char),1,archBinario);
    printf("%s\n",intToHex(version));
    fread(&mv.TDD[0],sizeof(unsigned short int),1,archBinario); //Leo el tamaño del codigo y asigno al DS
    mv.TDD[1] = mv.TDD[0] + 1;
    printf("%d\n",mv.TDD[0]);
    printf("%d\n",mv.TDD[1]);
    int numInstrucciones = 0;

    while(fread(&mv.memoria[numInstrucciones],sizeof(char),1,archBinario)) //se lee el archivo binario para cargarlo en la memoria
        numInstrucciones++;

    fclose(archBinario);

    cargaVectorDeFunciones(vecFunciones);
    mv.registros[5]=0; //IP == 0

    leePrimerByte(mv.memoria[mv.registros[5]],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);

    while(mv.registros[5] < mv.TDD[1]){ // IP menor al DS
        recuperaOperandos(&mv,operandos,mv.registros[5]);
        sumaIP(&(mv.registros[5]),operandos[0].tipo,operandos[1].tipo); //corro el IP a la proxima instruccion
        vecFunciones[operacion](&mv, operandos);
        leePrimerByte(mv.memoria[mv.registros[5]],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);
    }

    //disassebler
    return 0;
}

