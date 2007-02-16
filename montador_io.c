/*
   montador_io.c   
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "montador_io.h"

#define BUF_SIZE 100


void createMinem()
{
   minem[ 0 ].code = 2;
   strcpy( minem[ 0 ].name, "ADD" );
   
   minem[ 1 ].code = 0;
   strcpy( minem[ 1 ].name, "BR" );

   minem[ 2 ].code = 5;
   strcpy( minem[ 2 ].name, "BRNEG" );

   minem[ 3 ].code = 1;
   strcpy( minem[ 3 ].name, "BRPOS" );
   
   minem[ 4 ].code = 4;
   strcpy( minem[ 4 ].name, "BRZERO" );
   
   minem[ 5 ].code = 15;
   strcpy( minem[ 5 ].name, "CALL" );
   
   minem[ 6 ].code = 13;
   strcpy( minem[ 6 ].name, "COPY" );
   
    minem[ 7 ].code = 10;
   strcpy( minem[ 7 ].name, "DIVIDE" );

   minem[ 8 ].code = 3;
   strcpy( minem[ 8 ].name, "LOAD" );

   minem[ 9 ].code = 14;
   strcpy( minem[ 9 ].name, "MULT" );

   minem[ 10 ].code = 12;
   strcpy( minem[ 10 ].name, "READ" );

   minem[ 11 ].code = 16;
   strcpy( minem[ 11 ].name, "RET" );

   minem[ 12 ].code = 11;
   strcpy( minem[ 12 ].name, "STOP" );
   
   minem[ 13 ].code = 7;
   strcpy( minem[ 13 ].name, "STORE" );

   minem[ 14 ].code = 6;
   strcpy( minem[ 14 ].name, "SUB" );
   
   minem[ 15 ].code = 8;
   strcpy( minem[ 15 ].name, "WRITE" );   
   
   minem[ 16 ].code = -1;
   strcpy( minem[ 16 ].name, "SPACE" );   

   minem[ 17 ].code = -1;
   strcpy( minem[ 17 ].name, "CONST" );   
   
   minem[ 18 ].code = -1;
   strcpy( minem[ 18 ].name, "INTDEF" );   

   minem[ 19 ].code = -1;
   strcpy( minem[ 19 ].name, "EXTDEF" );   
}


void callInputError( short int error )
{
      printf( "\n\n" );
      
      if ( error == 0 )
      {
         printf( "Sintaxe de entrada invalida\n\n" );
         printf( "OPCOES\n" );
         printf( "-i Especifica arquivo de entrada para o montador\n" );
         printf( "-o Especifica arquivo de saida\n\n" );      
         printf( "FORMA DE USAR\n" );
         printf( "montador -i ARQUIVO_DE_ENTRADA -o ARQUIVO_DE_SAIDA \n\n" );
         exit( 0 );
       }
       else if ( error == 1 )
       {
         printf( "ERRO\nO arquivo %s(arquivo de entrada) nao foi encontrado...\n\n", info.input ); 
         exit( 0 );
       }
       else if ( error == 2 )
       {
         printf( "ATENCAO\nO arquivo %s(arquivo de saida) ja existe...\n\n", info.output ); 
         exit( 0 );
       }

}

// pega informacoes dos parametros passados e coloca na estrutura
int getParamInfo( int argc, char *argv[] )
{
   FILE *file;
   short int index;
   
   // numero incorreto de parametros, retorna problemas...
   if ( argc < 5 )
   {
      callInputError( 0 );
   }
   
   // sem problemas, vamos selecionar as coisas...
   else
   {
      for ( index = 0; index < argc; index += 1 )
      {
         // achando arquivo de entrada
         if ( !strcmp( "-i", argv[ index ] ) )
         {
            if ( argv[ index + 1 ] )
               strcpy( info.input, argv[ index + 1 ] );               
         }

         // achando arquivo de saida
         if ( !strcmp( "-o", argv[ index ] ) )
         {
            if ( argv[ index + 1 ] )
               strcpy( info.output, argv[ index + 1 ] );
         }
         
         info.size_param = argc;
      }
   }
   
   // se entrada do usuario esta correta
  if ( !strcmp( "-i" , info.input ) || !strcmp( "-i" , info.output ) || !strcmp( "-o" , info.input ) || !strcmp( "-o" , info.output ) )
  {
      callInputError( 0 );
  }
  
   // passando parametros errados
  if ( ( strcmp( argv[ 1 ], "-o" ) && strcmp( argv[ 2 ], "-o" ) && strcmp( argv[ 3 ], "-o" ) && strcmp( argv[ 4 ], "-o" )  )   )
  {
      callInputError( 0 );
  }
  
   // passando parametros errados
  if ( ( strcmp( argv[ 1 ], "-i" ) && strcmp( argv[ 2 ], "-i" ) && strcmp( argv[ 3 ], "-i" ) && strcmp( argv[ 4 ], "-i" )  )   )
  {
      callInputError( 0 );
  }
  
  // arquivo de entrada nao existente
  if ( !( file = fopen( info.input, "r" ) )  )   
  {
      callInputError( 1 );
  }
  
  // arquivo de saida existe
  if ( ( file = fopen( info.output, "r" ) )  )   
  {
      callInputError( 2 );
  }

   return 1;
}


// mostra as informacoes da estrutura
void showParamInfo()
{
   if ( strlen( info.input ) > 0  &&  strlen( info.output  ) > 0 )
   {
      printf( "Arquivo de entrada: %s\n", info.input );
      printf( "Arquivo de saida...: %s\n", info.output );
   }
   getchar();
}



// inclui novo item na etrutura e insere itens na tabela de simbolos
void new( char *label, char *operation, char *op1, char *op2 )
{
   struct FILE_INFO *new;   
   int nop = 0;
   int int_op1;
   int int_op2;
   
   //printf( "\nLABEL:%s\n", label );
   //printf( "OPERATION:%s\n", operation );
   //printf( "Op1:%s\n", op1 );
   //printf( "Op2:%s\n\n", op2 );
   
   /*if ( !is( label ) )
      label[ 0 ] = 0;
      
   if ( !is( op1 ) )
      op1[ 0 ] = 0;

   if ( !is( op2 ) )
      op2[ 0 ] = 0;
   */
      
   if ( strlen( op1 ) > 0 )   nop += 1;
   if ( strlen( op2 ) > 0 )   nop += 1;
   
   new = malloc( sizeof( struct FILE_INFO ) );
   strcpy( new->label, label );
   strcpy( new->operation, operation );
   strcpy( new->op1, op1 );
   strcpy( new->op2, op2 );   
   new->next = NULL;
   new->prev = NULL;
   new->nop = nop;
   
   // caso haja linha em branco
   if ( strlen( operation ) > 0 )
   {
      // primeiro item   
      if ( first == NULL )
      {
         new->addr = 0;
         first = new;
         last = new;   
      }
      else
      {
         new->addr = last->addr + last->nop + 1;
         last->next = new;
         last = new;
      }
   }   
   
   int_op1 = atoi( op1 );
   int_op2 = atoi( op2 );
   
   // se forem simbolos insere na tabela de simbolos
   // simbolo em op1   
   
   if ( strlen( label ) > 0 )
   {
      printf( "Definindo endereco para: %s\n", label );
      
   }
   
   if ( strlen( op1 ) > 0 && !int_op1 )
   {
      printf( "Simbolo encontrado: %s\n", op1 );
   }
   
   if ( strlen( op2 ) > 0 && !int_op2 )
   {
      printf( "Simbolo encontrado: %s\n", op2 );
   }

   
   getchar();
   
}

// carrega conteudo do arquivo para a memoria
void loadFile2Memory( )
{
   FILE *file;
   char buffer[ BUF_SIZE ];
   char label[ 10 ];
   char operation[ 10 ];
   char op1[ 10 ];
   char op2[ 10 ];
   
   
   if (  ( file = fopen( info.input, "r" ) )  == NULL  )
   {
      printf( "\nErro na abertura de arquivo..." );
      exit( 1 );
    }
    else
    {
       while ( !feof( file ) )
       {
          label[ 0 ] = 0;
          operation[ 0 ] = 0;
          op1[ 0 ] = 0;
          op2[ 0 ] = 0;
          
          fgets( buffer, sizeof( buffer ), file );
          if ( !feof( file ) )
          {
              //verificando se é label
              if ( strstr( buffer, ":" ) )
              {        
                  sscanf( buffer, "%s %s %s %s", label, operation, op1, op2 );
                  label[ strlen( label ) - 1 ] = '\0';
              }
              else
              {
                  //printf( "\nNao tem label" );
                  sscanf( buffer, "%s %s %s", operation, op1, op2 );
              }
              //void new( int line, int addr, char *label, char *operation, char *op1, char *op2 )              
              new(  label, operation, op1, op2  );  
          }
       }
    }
}

// mostra estrutura da memoria soh para testes
void showMem()
{
   struct FILE_INFO *aux = first;   
   printf( "\n\n" );
   while ( aux != NULL )
   {      
      printf( "LABEL = %s\t OPERATION = %s\tOP1 = %s\tOP2 = %s\n\n", aux->label, aux->operation, aux->op1, aux->op2 );
      aux = aux->next;      
   }
   getchar();
}


// verifica se ja ha simbolos na tabela de simbolos
// retorno ( 0, 1, 2 ) == ( nao tem, ja tem mas sem endereco, ja tem e com endereco definido )
struct SYMBOL_TABLE *getSymbol( char *symbol )
{
   struct SYMBOL_TABLE *aux = st_first;
   for ( ; aux; aux = aux->next )
   {
      if ( !strcmp( symbol, aux->symbol ) )
         return aux;
   }
   return NULL;
}

// cria item na tabela de simbolos
void new_st( char *symbol, int addr )
{
   struct SYMBOL_TABLE *new;
   new = malloc( sizeof( struct SYMBOL_TABLE ) );
   strcpy( new->symbol, symbol );
   new->addr = addr;
   new->next = NULL;
      
   // nao existe itens, cria!
   if ( st_first == NULL )   
   {
      st_first = new;
      st_last = new;
   }
   else
   {st_last->next = new;
      st_last = new;      
   }
}

// novo item na tabela de simbolos
// symbol == simbolo
// p_mem == ponteiro para memoria que guarda item
// modo == ( 0, 1 ) = ( definindo endereco, simbolo encontrado ) 
void st_new( char *symbol, struct FILE_INFO *p_mem, int modo )
{
   struct SYMBOL_TABLE *exist = getSymbol( symbol );

   // definindo endereco
   if ( modo == 0 )     
   {
      // existe e com endereco definido
      if ( exist != NULL && exist->addr != -1 )
      {
         // tratar erro de simbolo com multiplas definicoes
      }
      
      // existe mas sem endereco definido
      else if ( exist != NULL && exist->addr == -1 )
      {
         exist->addr = p_mem->addr;
      }
      
   }
   
   // se esta apenas fazendo referencia a um simbolo insere ele com endereco negativo( -1 )
   else if ( modo == 1 )
   {
      if ( exist == NULL )
      {  
         new_st( symbol, -1 );
      }
   }
}

void testandoValores()
{
   struct FILE_INFO *aux = first;
   int index;
   for ( index = 0; aux ; aux = aux->next, index++ )
   {
      if ( aux->nop == 2 )
         printf( "Indice:%d\t  Endereco:%d\t Operacao:%s\t Operando1:%s\t Operando2:%s\n", index, aux->addr, aux->operation, aux->op1, aux->op2 );
      if ( aux->nop == 1 )
         printf( "Indice:%d\t  Endereco:%d\t Operacao:%s\t Operando1:%s\n", index, aux->addr, aux->operation, aux->op1 );
      if ( aux->nop == 0 )
         printf( "Indice:%d\t  Endereco:%d\t Operacao:%s\n", index, aux->addr, aux->operation );
   }
   printf("\n\n");
}
