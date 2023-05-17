#include <stdio.h>
#include <stdlib.h>
#include "Operaciones.h"
#include <string.h>

void cargaVectorDeFunciones(TOperaciones *);
unsigned short int acomodaTamanio(unsigned short int tamanio);
void cargaMV(TMV *, char * [],int ,int *,char *);
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
            cargaMV(&mv,argv,argc,&numInstrucciones,&version);
            switch (version){
            case 1:
                while(mv.registros[5] < mv.TDD[0][1])
                    ejecutaCicloProcesador(&mv,version);
//                if(argc >= 3){
//                    if(strcmp(argv[2],"-d") == 0){
//                        printf("\n");
//                        dissasembler(&mv,numInstrucciones);
//                    }
//                }
                dissasembler(&mv,numInstrucciones);
                break;
            case 2:
                while(mv.registros[5] < mv.TDD[0][1] || mv.registros[5] < numInstrucciones)
                    ejecutaCicloProcesador(&mv,version);
//                if(argc >= 3){
//                    if(strcmp(argv[argc],"-d") == 0){
//                        printf("\n");
//                        dissasembler(&mv,numInstrucciones);
//                    }
//                }
                dissasembler(&mv,numInstrucciones);
                break;
            }
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

void cargaMV(TMV *mv, char *args[],int argc,int *numInstrucciones,char *version){
    unsigned short int tamanio = 0,tamanioAnt = 0,i;
    char *header = (char * )malloc(6 * sizeof(char));
    FILE *archBinario;
    int cuentaSegmentos = 0;
    int tamanioMemoria = 16384;
    int todoOK = 1;

//    archBinario=fopen(args[1],"rb");
    archBinario=fopen("sample (4).vmx","rb");
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

            if(*version == 2){
                unsigned int tamanioAnt = tamanio;

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamanio del KS

                tamanio = acomodaTamanio(tamanio);

                if(tamanio > 0){ // posicion 2 en la TDDS
                    //printf("%04X\n",tamanioAnt);
                    mv->TDD[2][0] = tamanioAnt;
                    mv->TDD[2][1] = 0;
                    mv->registros[2] = 0x00020000;
                    cuentaSegmentos++;
                    tamanioAnt = tamanio;
                 }else{
                    mv->registros[2] = -1;
                }

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamanio DS

                tamanio = acomodaTamanio(tamanio);

                if(tamanio > 0){
                    mv->TDD[1][0] = mv->TDD[2][0] + tamanioAnt;
                    mv->TDD[1][1] = 0;
                    mv->registros[1] = 0x00010000;
                    cuentaSegmentos++;
                    tamanioAnt = tamanio;
                }else{
                    mv->registros[1] = -1;
                }

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamanio ES

                tamanio = acomodaTamanio(tamanio);

                if(tamanio > 0){
                    mv->TDD[3][0] = mv->TDD[1][0] + tamanioAnt;
                    mv->TDD[3][1] = 0;
                    mv->registros[3] = 0x00030000;
                    cuentaSegmentos++;
                    tamanioAnt = tamanio;
                }else{
                    mv->registros[3] = -1;
                }

                fread(&tamanio,sizeof(unsigned short int),1,archBinario); //Leo el tamanio SS

                tamanio = acomodaTamanio(tamanio);

                if(tamanio > 0){
                    mv->TDD[4][0] = mv->TDD[3][0] + tamanioAnt;
                    mv->TDD[4][1] = 0;
                    mv->registros[4] = 0x00040000;
                    cuentaSegmentos++;
                }else{
                    mv->registros[4] = -1;
                }

//                if(argc >= 3){
//                    if(sscanf(args[3],"m=%d",&tamanioMemoria) || sscanf(args[4],"m=%d",&tamanioMemoria)){ //si existe el parametro m=M
//
//                        if(mv->TDD[cuentaSegmentos][1] < tamanioMemoria){
//                            printf("ERROR! ASIGNA MAL LA MEMORIA PRINCIPAL POR PARAMETRO... BYE BYE\n");
//                            mv->registros[5] = *numInstrucciones;
//                            todoOK = 0;
//                        }
//                    }
//
//                    int len = strlen(args[3]);
//                    if (len >= 4 && strcmp(args[3] + len - 4, ".vmi") == 0) {
//                        strcpy(mv->imagenArchivo,args[3]);
//                    }
//                }
            }else{
                mv->TDD[1][0] = tamanio + 1;
                mv->TDD[1][1] = 16384;

                mv->registros[1] = 0x00010000;
            }

            *numInstrucciones = 0;
            int banderaStop = 0;
            int contador = 0;
            while(fread(&(mv->memoria[contador]),sizeof(char),1,archBinario)){ //se lee el archivo binario para cargarlo en la memoria

                if((mv->memoria[contador] & 0xF0) == 0xF0 && (mv->memoria[contador] & 0x01) == 0x00 && banderaStop == 0){
                    banderaStop = 1;
                    (*numInstrucciones)++;
                }else{
                    if(banderaStop == 1){
                        //printf("bandera stop\n");
                        (mv->TDD[2][1] += 1);
                    }else{
                        (*numInstrucciones)++;
                    }

                }
                contador++;
            }

            fclose(archBinario);

            if(todoOK)
                mv->registros[5]=0; //IP == 0
        }else{
            printf("ERROR! DEBE PROPORCIONAR DE UN NOMBRE DE ARCHIVO CON EXTENCION .VMX... BYE BYE\n");
            mv->registros[5] = mv->TDD[0][1];
        }
    }else{
        printf("EL ARCHIVO BINARIO NO EXISTE\n");
        mv->registros[5] = mv->TDD[0][1];
    }

}

void dissasembler(TMV * mv,int numInstrucciones){
    int i = 0;
    int ipAssembler = 0;
    TOperando operandos[2];
    t_funcionDisassembler imprimeFuncion[0xF2];
    TInstruccionDisassembler vecDisassembler[16384];
    unsigned int operacion;

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



