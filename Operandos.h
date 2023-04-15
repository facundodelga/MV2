
//typedef struct {
//    char *memoria;
//    int TDD[8];
//    int registros[16]; // 16 registros de 4 bytes
//}TMV;
//
//typedef struct{
//    char tipo;
//    char registro; //eax ds
//    char segmentoReg; //AH AL o cantidad de bytes que vamos a leer
//    int desplazamiento; //desplazamiento de memoria y tipo inmediato
//}TOperando;
//
//int getOp(TMV *,TOperando );
//void setOp(TMV *,TOperando ,int );
//int getReg(TMV *,TOperando );
//int getMem(TMV *,TOperando );
//void recuperaOperandos(TMV *,TOperando *,int ); //mv, vector de operandos, ip
