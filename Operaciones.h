typedef struct {
    char memoria[16384];
    int TDD[8];
    int registros[16]; // 16 registros de 4 bytes
}TMV;

typedef struct{
    char tipo;
    char registro; //eax ds
    char segmentoReg; //AH AL o cantidad de bytes que vamos a leer
    int desplazamiento; //desplazamiento de memoria y tipo inmediato
}TOperando;

typedef struct {
    unsigned int posicion;
    char cantidad;
    char tamanio;
    char formato;
}TSistema;

int getOp(TMV *,TOperando );
void setOp(TMV *,TOperando ,int );
int getReg(TMV *,TOperando );
int getMem(TMV *,TOperando );
void recuperaOperandos(TMV *,TOperando *,int ); //mv, vector de operandos, ip
void sumaIP(int *ip,char operando1,char operando2);

//definicion protos funcion para el sistema

void readSys(TMV *mv,TSistema aux);
void writeSys(TMV *mv,TSistema aux);

//definicion de tipo funcion para los vectores de funciones
typedef void (*TOperaciones)(TMV *,TOperando *);
typedef void (*t_funcionSys)(TMV *,TSistema);

void cargaVectorDeFunciones(TOperaciones *v);

void MOV(TMV *mv, TOperando *op);
void ADD(TMV *mv, TOperando *op);
void SUB(TMV *mv, TOperando *op);
void MUL(TMV *mv, TOperando *op);
void DIV(TMV *mv, TOperando *op);
void SWAP(TMV *mv, TOperando *op);
void CMP(TMV *mv, TOperando *op);
void SHL(TMV *mv, TOperando *op);
void SHR(TMV *mv, TOperando *op);
void AND(TMV *mv, TOperando *op);
void OR(TMV *mv, TOperando *op);
void XOR(TMV *mv, TOperando *op);
void SYS(TMV *mv, TOperando *op);
void JMP(TMV *mv, TOperando *op);
void JZ(TMV *mv, TOperando *op);
void JP(TMV *mv, TOperando *op);
void JN(TMV *mv, TOperando *op);
void JNZ(TMV *mv, TOperando *op);
void JNP(TMV *mv, TOperando *op);
void JNN(TMV *mv, TOperando *op);
void LDL(TMV *mv, TOperando *op);
void LDH(TMV *mv, TOperando *op);
void RND(TMV *mv, TOperando *op);
void NOT(TMV *mv, TOperando *op);
void STOP(TMV *mv, TOperando *op);
void setCC(TMV *mv,int numero);



//DISASSEMBLER

typedef struct{
    unsigned int ipInicio;
    unsigned int ipFinal;
    unsigned int codigoOperacion;
    TOperando operandos[2];
}TInstruccionDisassembler;

typedef void (*t_funcionDisassembler)(TInstruccionDisassembler );

void cargaVectorDisassembler(t_funcionDisassembler *v);

void imprimeMOV(TInstruccionDisassembler disInstruccion);
void imprimeADD(TInstruccionDisassembler disInstruccion);
void imprimeSUB(TInstruccionDisassembler disInstruccion);
void imprimeSWAP(TInstruccionDisassembler disInstruccion);
void imprimeMUL(TInstruccionDisassembler disInstruccion);
void imprimeDIV(TInstruccionDisassembler disInstruccion);
void imprimeCMP(TInstruccionDisassembler disInstruccion);
void imprimeSHL(TInstruccionDisassembler disInstruccion);
void imprimeSHR(TInstruccionDisassembler disInstruccion);
void imprimeAND(TInstruccionDisassembler disInstruccion);
void imprimeOR(TInstruccionDisassembler disInstruccion);
void imprimeXOR(TInstruccionDisassembler disInstruccion);
void imprimeSYS(TInstruccionDisassembler disInstruccion);
void imprimeJMP(TInstruccionDisassembler disInstruccion);
void imprimeJZ(TInstruccionDisassembler disInstruccion);
void imprimeJP(TInstruccionDisassembler disInstruccion);
void imprimeJN(TInstruccionDisassembler disInstruccion);
void imprimeJNZ(TInstruccionDisassembler disInstruccion);
void imprimeJNP(TInstruccionDisassembler disInstruccion);
void imprimeJNN(TInstruccionDisassembler disInstruccion);
void imprimeLDL(TInstruccionDisassembler disInstruccion);
void imprimeLDH(TInstruccionDisassembler disInstruccion);
void imprimeRND(TInstruccionDisassembler disInstruccion);
void imprimeNOT(TInstruccionDisassembler disInstruccion);
void imprimeSTOP(TInstruccionDisassembler disInstruccion);

void obtieneTAG(char reg,char segmento,char nombre[]);
void imprimeOperando(TOperando op);

