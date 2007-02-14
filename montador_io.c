/*
   montador_io.c   
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "montador_io.h"

#define BUF_SIZE 100


void showMessage( char *msg )
{
   clear();
   printf( "\n      %s\n", msg );
   printf( "      %s\n", "Pressione ENTER para continuar" );
   getchar();
   clear();
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


// inclui novo item na etrutura
void new( char *label, char *operation, char *op1, char *op2 )
{
   struct FILE_INFO *new;
   int nop = 0;
   
   if ( op1 )
      nop++;
   if ( op2 )
      nop++;  
   
   new = malloc( sizeof( struct FILE_INFO ) );   
   new->nop = nop;
   new->next = NULL;
   new->prev = NULL;
   strcpy( new->label, label );
   strcpy( new->operation, operation );
   strcpy( new->op1, op1 );
   strcpy( new->op2, op2 );
   
   
   // se esta inserindo o primeiro
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
      new->prev = last;
      last = new;
   }
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
          strcpy( label, "" );
          
          fgets( buffer, sizeof( buffer ), file );
          if ( !feof( file ) )
          {
              //verificando se é label
              if ( strstr( buffer, ":" ) )
              {
                  // printf( "\nTem label" );
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
      printf( "Endereco:%d\tLabel:%s\t\tOperacao:%s\tOp1:%s\tOp2:%s\n", aux->addr, aux->label, aux->operation, aux->op1, aux->op2 );
      aux = aux->next;      
   }
   getchar();
}














