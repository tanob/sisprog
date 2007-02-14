// insere uma nova linha na estrutura que mantem o arquivo na memoria
void newLine( int line, int addr, char *operation, char *operand1, char*operand2 )
{
   struct FILE_INFO *aux = file_info;
   struct FILE_INFO *new;
      
   new = malloc( sizeof( struct FILE_INFO ) );
   if ( new )
   {
      showMessage( "Erro ao alocar memoria" );
      exit( 1 );
   }
   
   new->line = line;
   new->addr = addr;   
   strcpy( new->operation, operation );
   strcpy( new->operand1, operand1 );
   strcpy( new->operand2, operand2 );
   new->next = NULL;
   new->prev = NULL;
   
   // se eh o primeiro item cria a lista
   if ( !last )
   {
      aux = new;      
      last = aux;
   }
   // se jah existe itens insere no final
   else
   {
      last->next = new;
      new->prev = last;
      last = new;     
   }   
}
