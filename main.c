#include <stdio.h>
#include <stdlib.h>
#include "Operaciones.h"
#include <string.h>

void cargaVectorDeFunciones(TOperaciones *);
unsigned short int acomodaTamanio(unsigned short int tamanio);
void cargaMV(TMV *, char * [],unsigned int ,int *,char *);
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
    int entraDissasembler = 0;
    unsigned int tamanioMemoria = 0,aux;
    //if(argc > 1){
            int ipAux = 0;
            int cantDiscos = 0;
            for (int i = 2; i < argc; i++) {
                // Verificar [filename.vmi]
                int len = strlen(argv[i]);
                if (len >= 4 && strcmp(argv[i] + len - 4, ".vmi") == 0) {
                    strcpy(mv.imagenArchivo,argv[i]);
                }
                //Verificar [disc.vdd]
                if (len >= 4 && strcmp(argv[i] + len - 4, ".vdd") == 0) {
                    strcpy(mv.discs[cantDiscos++],argv[i]);
                }

                // Verificar [m=M]
                if (sscanf(argv[i],"m=%d",&aux)) {
                    printf("m=M: %s\n", argv[i]);
                    mv.memorySize = tamanioMemoria = aux;
                }

                // Verificar [-d]
                if (strcmp(argv[i], "-d") == 0) {
                    entraDissasembler = 1;
                }
            }
            strcpy(mv.discs[0],"disk.vdd");
            cargaMV(&mv,argv,tamanioMemoria,&numInstrucciones,&version);
            //dissasembler(&mv,numInstrucciones);
            switch (version){
            case 1:
                while(mv.registros[5] < mv.TDD[0][1]){
                    ipAux = mv.registros[5];
                    ejecutaCicloProcesador(&mv,version,ipAux);
                }
                if(strcmp(argv[argc-1],"-d") == 0){
                        printf("\n");
                        dissasembler(&mv,numInstrucciones);
                }

                break;
            case 2:
                while(mv.registros[5] < mv.TDD[0][1]){
                    ipAux = mv.registros[5];
                    ejecutaCicloProcesador(&mv,version,ipAux);
                }

                if(entraDissasembler){
                    dissasembler(&mv,numInstrucciones);
                }

                break;
            case 3:
                while(mv.registros[5] < mv.TDD[0][1] || mv.registros[5] < numInstrucciones){
                    ipAux = mv.registros[5];
                    ejecutaCicloProcesador(&mv,version,ipAux);
                }

                if(entraDissasembler){
                    dissasembler(&mv,numInstrucciones);
                }

                break;
            }
            dissasembler(&mv,numInstrucciones);

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

void cargaMV(TMV *mv, char *args[],unsigned int tamanioParam,int *numInstrucciones,char *version){
    unsigned short int tamanio = 0,tamanioAnt = 0,i;
    char *header = (char * )malloc(6 * sizeof(char));
    FILE *archBinario;
    int tamanioTotal = 0;
    int todoOK = 1;
    //archBinario=fopen(args[1],"rb");
    archBinario=fopen("sample (3).vmx","rb");
    if(archBinario){
        fgets(header,6 * sizeof(char),archBinario); //Obtengo el header
        if(strcmp(header,"VMX23") == 0){
            fread(version,sizeof(char),1,archBinario);

            fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamanio del codigo y asigno al CS

            tamanio = acomodaTamanio(tamanio);
            printf("MAQUINA VIRTUAL GRUPO G [ %s %02X %04X ]\n",header,*version,tamanio);
            printf("\n");
            //Carga el header y el nombre del archivo .vmi;
            for(i=0;i<5;i++){
                mv->header[i]=header[i];
            }
            mv->header[6]=(char)tamanio>>8;
            mv->header[7]=(char)tamanio;
            //strcpy(mv->imagenArchivo,args[1]);
            //
            mv->TDD[0][0] = 0x0000;
            mv->TDD[0][1] = tamanio;

            mv->registros[0] = 0;

            if(*version >= 2){
                unsigned int tamanioAnt = tamanio;
                int ultSegmento = 0;
                // Iterar sobre los segmentos

                for (int i = 1; i < 5; i++) {
                    unsigned short int tamanio;

                    fread(&tamanio, sizeof(unsigned short int), 1, archBinario); // Leer el tamaño del segmento

                    tamanio = acomodaTamanio(tamanio);
                    printf("tamanios : %04X\n",tamanio);
                    tamanioTotal += tamanio;

                    if (tamanio > 0) {
                        mv->TDD[ultSegmento + 1][0] = mv->TDD[ultSegmento][0] + tamanioAnt;
                        if(i < 4)
                            mv->TDD[ultSegmento + 1][1] = 0;

                        else{
                            //cuando estoy asignando al SS
                            mv->TDD[ultSegmento + 1][1] = tamanio + mv->TDD[ultSegmento + 1][0];
                            mv->registros[6] = (ultSegmento + 1) << 16;
                            mv->registros[6] += tamanio;
                            //printf("6: %08X\n",mv->registros[6]);
                        }

                        // Asignar el valor del registro correspondiente
                        switch(i){
                        case 1:
                            mv->registros[2] = (ultSegmento + 1) << 16;
                            break;
                        case 2:
                            mv->registros[1] = (ultSegmento + 1) << 16;
                            break;
                        default:
                            mv->registros[i] = (ultSegmento + 1) << 16;
                        }


                        mv->registros[i] &= 0xFFFF0000;
                        ultSegmento++;
                        tamanioAnt = tamanio;

                    } else {
                        mv->registros[i + 1] = -1;
                    }
                    //printf("%d: %08X\n",i,mv->registros[i]);
                }
                tamanioTotal += tamanio;
                mv->usedMemory = tamanioTotal;
                mv->lastValidSegment = ultSegmento;

                printf("used memory: %04X\n",mv->usedMemory);

                if(tamanioParam > 0 && tamanioTotal > tamanioParam){
                    printf("ERROR! LA MEMORIA PROPORCIONADA POR PARAMETRO ES INSUFICIENTE PARA EJECUTAR... BYE BYE\n");
                    exit(0xFF);
                }else{
                    if(tamanioParam > 0)
                        mv->memoria = (char * ) malloc(tamanioParam * sizeof(char));
                    else{
                        mv->memoria = (char * ) malloc(16384 * sizeof(char));
                        mv->memorySize = 16384;
                    }
                }

            }else{
                mv->TDD[1][0] = tamanio + 1;
                mv->TDD[1][1] = 16384;
                mv->memoria = (char * ) malloc(16384 * sizeof(char));
                mv->registros[1] = 0x00010000;
            }

            *numInstrucciones = 0;
            int contador = 0;

            while(fread(&(mv->memoria[contador]),sizeof(char),1,archBinario)){ //se lee el archivo binario para cargarlo en la memoria

                if(contador < mv->TDD[0][1]){
                    (*numInstrucciones)++;
                }else{
                    mv->TDD[1][1] += 1;
                }
                contador++;
            }

            fclose(archBinario);
            mv->registros[5]=0; //IP == 0
        }else{
            printf("ERROR! DEBE PROPORCIONAR DE UN NOMBRE DE ARCHIVO CON EXTENCION .VMX... BYE BYE\n");
            exit(0xFF);
        }
    }else{
        printf("EL ARCHIVO BINARIO NO EXISTE\n");
        exit(0xFF);
    }

}

void dissasembler(TMV * mv,int numInstrucciones){
    int i = 0;
    int ipAssembler = 0;
    TOperando operandos[2];
    t_funcionDisassembler imprimeFuncion[0xF2];
    TInstruccionDisassembler vecDisassembler[16384];
    unsigned int operacion;
    printf("\n==============\n");
    printf("\nDISASSEMBLER :\n");
    printf("\n==============\n");
    while(ipAssembler < numInstrucciones){ // IP menor al DS

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
        operacion = vecDisassembler[z].codigoOperacion;
        if((operacion >= 0 && operacion < 12) || (operacion >= 0x30 && operacion < 0x3F) || (operacion >= 0xF0 && operacion < 0xF2)){
            imprimeFuncion[vecDisassembler[z].codigoOperacion](vecDisassembler[z]);
        }

        printf("\n");
    }

}



