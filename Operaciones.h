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
char* intToHex2B(int n);
char* decToOctal(int num);
int hextoint(char* hex);
int octtoint(char* oct);

//definicion protos funcion para el sistema

void readSys(TMV *mv,TSistema aux);
void writeSys(TMV *mv,TSistema aux);

//definicion de tipo funcion para los vectores de funciones
typedef void (*TOperaciones)(TMV *,TOperando *);
typedef void (*t_funcionSys)(TMV *,TSistema);

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

