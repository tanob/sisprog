/*
   montador_io.c   
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "montador_io.h"

#define BUF_SIZE 100


void createmnem()
{
   mnem[ 0 ].code = 2;
   strcpy( mnem[ 0 ].name, "ADD" );
   
   mnem[ 1 ].code = 0;
   strcpy( mnem[ 1 ].name, "BR" );

   mnem[ 2 ].code = 5;
   strcpy( mnem[ 2 ].name, "BRNEG" );

   mnem[ 3 ].code = 1;
   strcpy( mnem[ 3 ].name, "BRPOS" );
   
   mnem[ 4 ].code = 4;
   strcpy( mnem[ 4 ].name, "BRZERO" );
   
   mnem[ 5 ].code = 15;
   strcpy( mnem[ 5 ].name, "CALL" );
   
   mnem[ 6 ].code = 13;
   strcpy( mnem[ 6 ].name, "COPY" );
   
   mnem[ 7 ].code = 10;
   strcpy( mnem[ 7 ].name, "DIVIDE" );

   mnem[ 8 ].code = 3;
   strcpy( mnem[ 8 ].name, "LOAD" );

   mnem[ 9 ].code = 14;
   strcpy( mnem[ 9 ].name, "MULT" );

   mnem[ 10 ].code = 12;
   strcpy( mnem[ 10 ].name, "READ" );

   mnem[ 11 ].code = 16;
   strcpy( mnem[ 11 ].name, "RET" );

   mnem[ 12 ].code = 11;
   strcpy( mnem[ 12 ].name, "STOP" );
   
   mnem[ 13 ].code = 7;
   strcpy( mnem[ 13 ].name, "STORE" );

   mnem[ 14 ].code = 6;
   strcpy( mnem[ 14 ].name, "SUB" );
   
   mnem[ 15 ].code = 8;
   strcpy( mnem[ 15 ].name, "WRITE" );   
   
   //mnem[ 16 ].code = -1;
   //strcpy( mnem[ 16 ].name, "SPACE" );   

   //mnem[ 17 ].code = -1;
   //strcpy( mnem[ 17 ].name, "CONST" );   
   
   //mnem[ 18 ].code = -1;
   //strcpy( mnem[ 18 ].name, "INTDEF" );   

   //mnem[ 19 ].code = -1;
   //strcpy( mnem[ 19 ].name, "EXTDEF" );   
}

// retorna o codigo do mnemonico se existir ou -1 se nao existe 
int is_mnem( char *mnem_param )
{
   int index;
   for ( index = 0; index < SIZE_MNEM; index++ )
   {
      if ( !strcmp( mnem_param, mnem[ index ].name ) )
      {
        return mnem[ index ].code;
      }
   }
   return -1;
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




// funcao para inserir simbolos temporario
void insert_into_temp_table( char *symbol, int type )
{
    struct TEMPORARY_TABLE *new;
    new = malloc( sizeof( struct TEMPORARY_TABLE ) );
    strcpy( new->symbol, symbol );
    new->type = type;
    new->next = NULL;

    // primeiros
    if ( tmp_first == NULL )    
    {
       tmp_first = malloc( sizeof( struct TEMPORARY_TABLE ) );
       tmp_first = new;
       tmp_last = new;
    }
    
    // outros
    else
    {
       tmp_last->next = new;
       tmp_last = new; 
    }
    
}

// retorna true se jah existir na tabela de definicoes, logo, nao precisa inserir
int is_in_definition_table( char *symbol )
{
    struct DEFINITION_TABLE *aux = def_first;
    for ( ; aux != NULL; aux = aux->next )
    {
        if ( !strcmp( symbol, aux->symbol ) )
        {
            return 1;
        }
    }     
    return 0;
}

// soh insere se ainda nao existir referencia
void insert_into_definition_table( char *symbol, int addr, int mode )
{
    struct DEFINITION_TABLE *new;
    new = malloc( sizeof( struct DEFINITION_TABLE ) );
    new->addr = addr;
    strcpy( new->symbol, symbol );
    new->mode = mode;
    
    if ( !is_in_definition_table( symbol ) )
    {
       if ( def_first == NULL )
       {
         def_first = malloc( sizeof( struct DEFINITION_TABLE ) );
         def_first = new;
         def_last = new;
       }
       else
       {
         def_last->next = new;
         def_last = new;
       }
    }
}

void insert_into_use_table( char *symbol, int addr )
{
    struct USE_TABLE *new;
    new = malloc( sizeof( struct USE_TABLE ) );
    new->addr = addr;    
    strcpy( new->symbol, symbol );
    
    if ( use_first == NULL )
    {
      use_first = malloc( sizeof( struct USE_TABLE ) );
      use_first = new;
      use_last = new;
    }
    else
    {
      use_last->next = new;
      use_last = new;
    }  
}


// retorna { 0, 1, -1 } = { INTUSE, INTDEF, NAO ESTA NA TABELA }
int is_in_temp_table( char *symbol )
{
   struct TEMPORARY_TABLE *aux = tmp_first;   
   for ( ; aux != NULL; aux = aux->next )
   {
      if ( !strcmp( symbol, aux->symbol ) )
      {
        return aux->type;
      }
   }
   return -1;
}

// inclui simbolo na tabela de simbolos, na tabela de definicoes ou na tabela de usos
// dependendo de que tipo de declaracao esta sendo feita
int new( char *label, char *operation, char *op1, char *op2 )
{
   struct FILE_INFO *new;   
   int nop = 0;
   int int_op1;
   int int_op2;
   int flag = 0;
    
   
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
   
   
   // se for definicao global ou de uso interno de simbolo global coloca na tabela temporaria   
   // usando simbolo externo
   if ( !strcmp( operation, "INTUSE" ) )
   {
      if ( strlen( label ) > 0 && strlen( op1 ) == 0 )
      {
        insert_into_temp_table( label, 0 );
        flag = 1;
      }
      else
      {
        // erro na declaracao de uso
        printf( "\nERRO\n" );
      }
   }

   // definindo simbolo global
   if ( !strcmp( operation, "INTDEF" ) )
   {
      if ( strlen( label ) == 0 && strlen( op1 ) > 0 )
      {
        insert_into_temp_table( op1, 1 );
        flag = 1;      
      }
      else
      {
        // erro na definicao de simbolo global
        printf( "\nERRO\n" );
      }
   }
   

    // soh insere simbolo novo se for operation != de INTDEF e !+ de INTUSE


   if ( !strcmp( operation, "INTDEF" ) )
   {
      printf( "\n%s - INTDEF\n", op1 );  
      return 0; 
   }
   else if ( !strcmp( operation, "INTUSE" ) )
   {
      printf( "\n%s - INTUSE\n", label );  
      return 0;    
   }
   else
   {
      // caso haja linha em branco
      if ( strlen( operation ) > 0 )
      {      
         // senao estiver veruficando declaracao de simbolos externos ou outra operacao que nao existe, nao insere
        if ( !flag && is_mnem( operation ) != -1 )
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
      }
    }   
   
   int_op1 = atoi( op1 );
   int_op2 = atoi( op2 );

   
   // se forem simbolos insere na tabela de simbolos
   // simbolo em op1   
   
   if ( strlen( label ) > 0 )
   {
      if ( is_in_temp_table( label ) == 1 )
      {
         if ( is_in_definition_table( label ) )
         {
            // erro de definicao multi definida
         }
         else
         {
            printf( "Definindo endereco para: %s e inserido na tabela de definicoes", label );      
            insert_into_definition_table( label, new->addr, 1 );
         }        
      }
      else
      {
        printf( "Definindo endereco para: %s", label );
      }
      printf( "\n" );
   }
   
   if ( strlen( op1 ) > 0 && !int_op1 )
   {
      printf( "Simbolo encontrado: %s", op1 );      

      // se econtrou simbolo externo insere na tabela de simbolos
      if ( is_in_temp_table( op1 ) == 0 )
      {
        printf( " - inserindo na tabela de usos" );
        insert_into_use_table( op1, new->addr + 1 );
      }
      printf( "\n" );

   }
   
   if ( strlen( op2 ) > 0 && !int_op2 )
   {
      printf( "Simbolo encontrado: %s", op2 );

      // se econtrou simbolo externo insere na tabela de simbolos
      if ( is_in_temp_table( op2 ) == 0 )
      {
        printf( " - inserindo na tabela de usos" );
        insert_into_use_table( op1, new->addr + 2 );
      }

      printf( "\n" );
   }
   getchar();  
   return 1; 
}

// carrega conteudo do arquivo para a memoria
// e ja monta algumas tabelas

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


// mostra dados que estao dentro da tabela de definicao
void show_definition_table()
{
    struct DEFINITION_TABLE *aux;
    // se tem intens
    for ( aux = def_first; aux != NULL; aux = aux->next )
    {
        printf( "\nSimbolo: %s\tEndereco:%i\tModo:%i", aux->symbol, aux->addr, aux->mode );
    }
}

// mostra dados que estao dentro da tabela de definicao
void show_use_table()
{
    struct USE_TABLE *aux;
    // se tem intens
    for ( aux = use_first; aux != NULL; aux = aux->next )
    {
        printf( "\nSimbolo: %s\tEndereco:%i", aux->symbol, aux->addr );
    }
}

// adiciona erro a lista de erros
void add_erro( char *erro )
{
    struct ERRO *novo = malloc( sizeof( struct ERRO ) ); 
    strcpy( novo->erro, erro );
    novo->next = NULL;
    
    if ( erro_first == NULL )    
    {
        erro_first = malloc( sizeof( struct ERRO ) ); 
        erro_first = novo;
        erro_last = novo;
    }
    else
    {
        erro_last->next = novo;
        erro_last = novo;
    }
}

// percorre toda uma determinada lista
// parametros: { 0, 1, 2 } = { symbol_table, definition_table, use_table }
void walking_into( int table, void *action )
{
    struct SYMBOL_TABLE *aux0 = st_first;
    struct DEFINITION_TABLE *aux1 = def_first;
    struct USE_TABLE *aux2 = use_first;
          
    if ( table == 0 )
    {
        printf("\n0\n");
        for ( ; aux0 != NULL; aux0 = aux0->next )
            printf( "SIMBOLO:%s\t ENDERECO:%d\n", aux0->symbol, aux0->addr );
    }
    
    if ( table == 1 )
    {
        printf("\n1\n");
        for ( ; aux1 != NULL; aux1 = aux1->next )
            printf( "SIMBOLO:%s\t ENDERECO:%d\n", aux1->symbol, aux1->addr );
    }
    
    if ( table == 2 )
    {
        printf("\n2\n");
        for ( ; aux2 != NULL; aux2 = aux2->next )
            printf( "SIMBOLO:%s\t ENDERECO:%d\n", aux2->symbol, aux2->addr );
    }


}
