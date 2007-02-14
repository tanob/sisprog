/*
   montador_io.h
*/

#define FIELD_SIZE 5
#define FILE_LEN 100

struct MONTADOR_INFO
{
   char input[ FILE_LEN ];
   char output[ FILE_LEN ];   
   short int size_param;
};

struct FILE_INFO
{   
   // endereco
   int addr;
   // numero de operandos
   int nop;   
   
   char label[ FIELD_SIZE ];
   char operation[ FIELD_SIZE ];
   char op1[ FIELD_SIZE ];
   char op2[ FIELD_SIZE ];
   struct FILE_INFO *next;
   struct FILE_INFO *prev;   
};

struct SYMBOL_TABLE
{
   char symbol[ FIELD_SIZE ];
   int addr;
};

struct MONTADOR_INFO info;
struct FILE_INFO *first;
struct FILE_INFO *last;

struct SYMBOL_TABLE *st_first;
struct SYMBOL_TABLE *st_last;


// limpa tela generica
#ifndef NIX
   #define clear(); system("cls");
#else
   #define clear(); system("clear");
#endif

void trace( char *warning );
void callInputError( short int error );
int getParamInfo( int argc, char *argv[] );
void showParamInfo();
void loadFile2Memory();
void showMem();
void testandoValores();



