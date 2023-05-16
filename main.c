#include <stdio.h>
#include <stdlib.h>
#include "Operaciones.h"
#include <string.h>

void cargaVectorDeFunciones(TOperaciones *);
unsigned short int acomodaTamanio(unsigned short int tamanio);
void cargaMV(TMV *, char * [],int *,char *);
void dissasembler(TMV *, int );

//    printf("operacion %02x\n",operacion);
//    printf("tipo op1 %02x\n",operandos[0].tipo);
//    printf("tipo op2 %02x\n",operandos[0].tipo);
//
//    printf(" op1 %d\n",operandos[0].registro);
//    printf(" op2 %d\n",operandos[0].registro);


int main(int argc,char *argv[]){
    char version;
    int numInstrucciones;
    TMV mv;

    //if(argc > 1){
            cargaMV(&mv,argv,&numInstrucciones,&version);
            switch (version){
            case 1:
                while(mv.registros[5] < mv.TDD[0][1])
                    ejecutaCicloProcesador(&mv,version);
                break;
            case 2:
                while(mv.registros[5] < mv.TDD[2][0] || mv.registros[5] < numInstrucciones)
                    ejecutaCicloProcesador(&mv,version);
                break;
            }
        //disassembler
        //if(argc >= 3){
            //if(strcmp(argv[2],"-d") == 0){
                printf("\n");
                dissasembler(&mv,numInstrucciones);
            //}
        //}
        printf("\nPEDRO ARIAS - FACUNDO DELGADO\n");
    return 0;
}


unsigned short int acomodaTamanio(unsigned short int tamanio){
    unsigned short int aux1 = 0,aux2 = 0;
    aux1 = (tamanio >> 8) & 0x00FF;
    aux2 = (tamanio & 0x00FF) << 8;

    return aux2 | aux1;
}

void cargaMV(TMV *mv, char *args[],int *numInstrucciones,char *version){
    unsigned short int tamanio = 0,tamanioAnt = 0;
    char *header = (char * )malloc(6 * sizeof(char));
    FILE *archBinario;
    int cuentaSegmentos = 0;
    int tamanioMemoria = 16384;
    int todoOK = 1;

    //archBinario=fopen(archivo,"rb");
    archBinario=fopen("sample (4).vmx","rb");
    if(archBinario){
        fgets(header,6 * sizeof(char),archBinario); //Obtengo el header

        if(strcmp(header,"VMX23") == 0){
            fread(version,sizeof(char),1,archBinario);

            fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño del codigo y asigno al CS

            tamanio = acomodaTamanio(tamanio);
            printf("MAQUINA VIRTUAL GRUPO G [ %s %02X %04X ]\n",header,*version,tamanio);
            printf("\n");
            mv->TDD[0][0] = 0x0000;
            mv->TDD[0][1] = tamanio;

            mv->registros[0] = 0;

            mv->TDD[1][0] = tamanio + 1;
            mv->TDD[1][1] = 16384;

            mv->registros[1] = 0x00000000 | mv->TDD[1][0];

            if(*version == 2){
                int tamanioAnt = tamanio;

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño del KS

                tamanio = acomodaTamanio(tamanio);

                if(tamanio > 0){
                    mv->TDD[2][0] = tamanioAnt;
                    mv->TDD[2][1] = 0;
                    mv->registros[2] = mv->TDD[2][0];
                    cuentaSegmentos++;
                    tamanioAnt = tamanio;
                 }else{
                    mv->registros[2] = -1;
                }

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño DS

                tamanio = acomodaTamanio(tamanio);

                if(tamanio > 0){
                    mv->TDD[1][0] = mv->TDD[2][0] + tamanioAnt;
                    mv->TDD[1][1] = 0;
                    mv->registros[1] = mv->TDD[1][0];
                    cuentaSegmentos++;
                    tamanioAnt = tamanio;
                }else{
                    mv->registros[1] = -1;
                }

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño ES

                tamanio = acomodaTamanio(tamanio);

                if(tamanio > 0){
                    mv->TDD[3][0] = mv->TDD[1][0] + tamanioAnt;
                    mv->TDD[3][1] = 0;
                    mv->registros[3] = mv->TDD[3][0];
                    cuentaSegmentos++;
                    tamanioAnt = tamanio;
                }else{
                    mv->registros[3] = -1;
                }

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamaño SS

                tamanio = acomodaTamanio(tamanio);

                if(tamanio > 0){
                    mv->TDD[4][0] = mv->TDD[3][0] + tamanioAnt;
                    mv->TDD[4][1] = 0;
                    mv->registros[4] = mv->TDD[4][1];
                    cuentaSegmentos++;
                }else{
                    mv->registros[4] = -1;
                }

                if(args[3] != NULL){ //si existe el parametro m=M
                    sscanf(args[3],"m=%d",&tamanioMemoria);
                    if(mv->TDD[cuentaSegmentos][1] < tamanioMemoria){
                        printf("ERROR! ASIGNA MAL LA MEMORIA PRINCIPAL POR PARAMETRO... BYE BYE\n");
                        mv->registros[5] = *numInstrucciones;
                        todoOK = 0;
                    }
                }
            }

            *numInstrucciones = 0;

            while(fread(&(mv->memoria[*numInstrucciones]),sizeof(char),1,archBinario)){ //se lee el archivo binario para cargarlo en la memoria
//                    printf("%02X\n",mv->memoria[numInstrucciones]);
                (*numInstrucciones)++;
            }

            fclose(archBinario);

            if(todoOK)
                mv->registros[5]=0; //IP == 0
        }else{
            printf("ERROR! DEBE PROPORCIONAR DE UN NOMBRE DE ARCHIVO CON EXTENCION .VMX... BYE BYE\n");
            mv->registros[5] = *numInstrucciones;
        }
    }else{
        printf("EL ARCHIVO BINARIO NO EXISTE\n");
        mv->registros[5] = *numInstrucciones;
    }

}

void dissasembler(TMV * mv,int numInstrucciones){
    int i = 0;
    int ipAssembler = 0;
    TOperando operandos[2];
    t_funcionDisassembler imprimeFuncion[242];
    TInstruccionDisassembler vecDisassembler[16384];
    unsigned int operacion;

    while(ipAssembler < mv->TDD[1][0]){ // IP menor al DS

        leePrimerByte(mv->memoria[ipAssembler],&(operandos[0].tipo),&(operandos[1].tipo),&operacion);

        vecDisassembler[i].operandos[0].tipo = operandos[0].tipo;
        vecDisassembler[i].operandos[1].tipo = operandos[1].tipo;

        vecDisassembler[i].ipInicio = ipAssembler;

        vecDisassembler[i].codigoOperacion = operacion;

        recuperaOperandos(mv,operandos,ipAssembler);

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
           printf("%02X ",mv->memoria[ip] & 0xFF);
        }
        printf("\t\t|\t");
        imprimeFuncion[vecDisassembler[z].codigoOperacion](vecDisassembler[z]);
        printf("\n");
    }

}



