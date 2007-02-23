/*
   montador_io.h
*/

#define FIELD_SIZE 10
#define FILE_LEN 100
#define SIZE_MNEM 16

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
   struct SYMBOL_TABLE *next;
};

struct MNEM
{
   int code;
   char name[ 10 ];
};

struct USE_TABLE
{
   char symbol[ FIELD_SIZE ];       
   int addr;
   struct USE_TABLE *next;
};

struct DEFINATION_TABLE
{
   char symbol[ FIELD_SIZE ];       
   int addr;
   int mode;
   struct DEFINATION_TABLE *next;
};

// POG
struct TEMPORARY_TABLE
{
   char symbol[ FIELD_SIZE ];       
   int type;
   struct TEMPORARY_TABLE *next;
};

struct MONTADOR_INFO info;
struct FILE_INFO *first;
struct FILE_INFO *last;

struct SYMBOL_TABLE *st_first;
struct SYMBOL_TABLE *st_last;

struct USE_TABLE *use_first;
struct USE_TABLE *use_last;

struct DEFINATION_TABLE *def_first;
struct DEFINATION_TABLE *def_last;

struct TEMPORARY_TABLE *tmp_first;
struct TEMPORARY_TABLE *tmp_last;


struct MNEM mnem[ SIZE_MNEM ];


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
void st_new( char *symbol, struct FILE_INFO *p_mem, int modo );



