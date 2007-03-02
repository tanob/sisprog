/**********************************************************************************************
UNIVERSIDADE FEDERAL DE PELOTAS
BACHARELADO EM CIÊNCIA DA COMPUTAÇÃO
PROJETOS EM COMPUTAÇÃO
Processador de Macros (Versão preliminar)

Componentes:
Giales Ficher
Pablo Barcellos
Mauro Brito
***********************************************************************************************/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// MAX_ERROS = numero maximo de erros que podem ocorrer no programa:
#define MAX_ERROS 10
#define FALSE 0
#define TRUE 1
#define MAXPOS 8
// MAXPOS = tamanho maximo dos parametros

/* * * * * * * * * * * * * Estruturas Usadas * * * * * * * * * * * * * * */

//Linhas em listas encadeadas  (Figura 4.6 do livro)
struct tlinha
{
  char linha[81];
  struct tlinha *proxl;  //proxima linha da definicao
};

//pilha de definicoes de macros  (Figura 4.6 do livro)
struct defmacro
{
  char prototipo[81];		//prototipo da macro
  struct tlinha *l;        //primeira linha do encadeamento
  struct defmacro *prox;   //proxima definicao
};

//pilha de chamadas aninhadas
struct pilha_nestcalls
{
  struct pilha_nestcalls *prox_call;  //proxima chamada do aninhamento
  struct defmacro *atual_def;         // macro que esta sendo expandida
  struct tlinha *ant_lin;             //guarda a linha onde deve continuar o processamento
};

// Pilha parametros formais
struct pfstack
{
  char param[8];  // parametro empilhado
  char nivel;     // nivel de definicao do parametro
  char ind;       // indice do parametro
  struct pfstack *prox;  //proximo parametro na pilha
};

// Armazena os dados dos parametros reais, para depois trocar com os pares ordenados
struct dado_listaPR
{
  char valor[8];              //valor do parametro na chamada
  char posicao[8];            //par #(d,i)
  struct dado_listaPR *prox;  //proximo parametro da pilha
};

// MUDAR NOME: PROXLISTA -> PROXPILHA ou PROXPR
struct listaPR
{
  struct dado_listaPR *primeiro;   //primeiro parametro da lista
  struct dado_listaPR *ultimo;   //ultimo parametro da lista
  struct listaPR *proxlista;   //proxima lista de parametros reais
};

// Ponteiro para o inicio da pilha [de listas] de parametros reais (acima) - (Figura 4.17)
struct pprstack		//mudar nome para prstack
{
  struct listaPR  *topo;  //topo da pilha de listas
};

// Tipo de dado para armazenar a linha em que ocorreu um erro e o codigo do erro
struct tipo_erro
{
  int cod;
  int line;
};

// Armazena os erros encontrados durante o processamento das macros
struct tabela_erros
{
  struct tipo_erro erros[MAX_ERROS];
  int n_erros; //conta o numero de erros
};
	 
	 
/* * * * * * * * * * * * * Variaveis Globais * * * * * * * * * * * * * * */

struct pfstack *ppf;
struct pprstack *ppr;		//MUDAR PPRSTACK

struct defmacro *first;		
							//a alocacao de memoria eh feita na funcao macro
							// com o comando alocadef(first), que tb faz a inicializacao

struct pilha_nestcalls *topo_nest;

/* * * * * * * * * * * * Prototipos das Funcoes * * * * * * * * * * * * * */

int tam_operacao(char *lin,int *posic);
struct defmacro* pesquisa_tabdef(struct defmacro *first,char line[82],int i);
int escrevelinha(char *line,struct defmacro *first);
int tamanho_param(char parm[8]);
int compara_pf_linha(struct pfstack *ppf,char linha[82],int pos,char *lv,char *indice);
char* substitui_pf(struct pfstack *ppf,char linha[82]);
void subst_parord_por_PR(char linha[81],struct pprstack *ppr,char lin_out[81]);
int empilha_param(struct pfstack *p,char *linha,int lv);
void desempilha_param(struct pfstack *p,int lv);
int empilha_listas(struct pprstack *ppr,char *linha);
void desempilha_listaPR(struct pprstack *ppr);
int cria_listPR(struct listaPR *lp,char *linha);
void remove_dado_listPR(struct listaPR *lpr);
int alocadef(struct defmacro *f);
void desaloca_def(struct defmacro *f);
int local_insert_erro(struct tabela_erros te,int lin);
void adiciona_erro(struct tabela_erros *te,int codigo,int lin);
int pesquisa_cod_op(char line[82],FILE *arq);
int empilha_nestcall(struct pilha_nestcalls *pn,struct defmacro *df,struct tlinha *pl);
void desempilha_nestcall(struct pilha_nestcalls *pn);
int macros(char *nome_fonte,struct tabela_erros *tab);

/* * * * * * * * * * * * * * Funcoes Usadas * * * * * * * * * * * * * * * */

//Funcao que devolve o tamanho do nome de uma macro
//Entrada: linha atual, posicao onde comeca o nome da macro (usado como saida, recebe valor)

int tam_operacao(char *lin,int *posic)
{
  int i,j,tam;
  tam=0;
  if ((lin[0]!=' ')&&(lin[0]!='\t'))	//tem label
  {
    for(i=0;(lin[i]!=' ')&&(lin[i]!='\t');i++);  //passa pelo label
	 for(i=i;(lin[i]==' ')||(lin[i]=='\t');i++);  //chega ate o codigo de operacao - MACRO
  }
  else
    for(i=0;(lin[i]==' ')||(lin[i]=='\t');i++);   //vai ate o codigo de operacao - MACRO

  *posic=i;	//passa a posicao do primeiro char do codigo de operacao - MACRO

  // procura na coluna de operacoes (lin[j]) por um dos terminadores de linha (entre aspas)
  for(j=i;!strchr("\n \t \x0",lin[j]);j++)
    tam++;

  return tam;		//tam eh o contador do tamanho do nome da macro
}


//Funcao que procura uma uma macro na tabela de definicoes, devolvendo um ponteiro
//Entrada: ponteiro para topo da pilha, linha atual, posicao da linha com o primeiro char da macro (i)
//Saida: ponteiro para a definicao da macro

struct defmacro* pesquisa_tabdef(struct defmacro *first,char line[82],int i)
{
//declaracoes
  struct defmacro *df,*aux;
  int tam_op,j,pos;
  char nome_op[8];

//inicializacoes
  df=NULL;
  aux=first;
  for(j=0;j<=7;j++)
    nome_op[j]=' ';

//funcao:
  while ((aux!=NULL)&&(df==NULL))	//enquanto nao achou
  {
    tam_op=tam_operacao(aux->prototipo,&pos);

    //passa para a variavel temporaria o nome da macro, que comeca em pos:
	 for(j=0;j<tam_op;j++)
	   nome_op[j]=aux->prototipo[pos+j];

	 //compara os dois nomes das macros: (usa o valor em bytes de cada uma)
	 if (memicmp(&line[i],nome_op,tam_op) == 0)
	   df=aux;
	 else
	   aux=aux->prox; //vai para a proxima definicao
  }
  return df;
}


//Funcao que adiciona uma linha na lista encadeada da definicao de uma macro
//Entrada: linha ?? atual ??, ponteiro para a tabela de definicoes
//Saida: status (0 ou 1)

//MUDAR lin -> linhasenc ou linhas
int escrevelinha(char *line,struct defmacro *first)
{
  struct tlinha *lin,*laux;
  int ok;

  if (strcmp(first->prototipo,"Temp")==0)	//se a tabela de defs. possui 0 prototipos
    strcpy(first->prototipo,line);	//nao precisa alocar mem para copiar a linha, que eh o prototipo (1¦ linha)
  else
  {
    lin=(struct tlinha *)malloc(sizeof(struct tlinha)); //aloca memoria para uma linha

	 if (lin==NULL)  //Nao conseguiu alocar memoria para a linha
	   ok=FALSE;
	 else            //Conseguiu
	   ok=TRUE;

	 if (ok)
	 {
	   strcpy(lin->linha,line);	//passa a line para a linha da struct
		lin->proxl=NULL;			//diz que nao existe proxima linha

		if (first->l==NULL)			//se a definicao nao tem linhas
		  first->l=lin;			//eh so passar como primeira linha
		else
		{
		  laux=first->l;
		  while(laux->proxl!=NULL)
		    laux=laux->proxl;		//busca a ultima linha
		  laux->proxl=lin;			//coloca a nova linha na ultima posicao
		}
	 }
  }
  return ok;
}

//Funcao que conta o numero de chars de um parametro
//Entrada: uma string (o parametro)
//Saida: tamanho do contador

int tamanho_param(char parm[8])
{
  int i=0;
  for(i=0;parm[i]!=' ';i++);
  return i;
}

/*	int compara_pf_linha(struct pfstack *ppf,char linha[82],int pos,char *lv,char *indice)
	Compara o parâmetro que se encontra na linha na posição `pos` com os parâmetros da pilha de
  parâmetros formais.
	Se encontrado, devolve 1, o nível e o índice do parâmetro através das variáveis `lv` e 
  `indice` respectivamente.
	Variáveis importantes:
	- aux: variável auxiliar para pesquisar na pilha de parâmetros formais;
	- tam: variável que guarda o tamanho do parâmetro;
	- achou: variável booleana de retorno;
*/


//PODE MUDAR: saida pode ser uma struct pfstack, que quando da problema, devolve -1 e -1
int compara_pf_linha(struct pfstack *ppf,char linha[82],int pos,char *lv,char *indice)
{
  struct pfstack *aux;
  int achou,tam;
  aux=ppf;
  achou=FALSE;

  while((aux!=NULL)&&(!achou))   //pesquisa na pilha de PFs
  {
    tam=tamanho_param(aux->param);
	 aux->param[tam]=' ';
	 if ((memcmp(&linha[pos],aux->param,tam)==0)&&(strchr("\t ,\n",linha[pos+tam])))
	 {
	   achou=TRUE;		   
	   *lv=aux->nivel;
	   *indice=aux->ind;
	 }
	 aux=aux->prox;
  }
  return achou;
}

/* char*  substitui_pf(struct pfstack *ppf,char linha[82])
Função que a partir da linha que entra, compara os parâmetros desta linha com os parâmetros
da pilha de parâmetros formais. Se encontrado, faz a substituição pelo par ordenado #(d,i),
caso contrário, não os altera. A Saída da função será a linha de entrada alterada ou não.
Variáveis importantes:
- continua: verifica se existem mais parâmetros. Se for FALSE sai do laço;
- level: guarda o nível do parâmetro;
- indice: guarda o índice do parâmetro;
- lin_out: variável de retorno.
*/

char*  substitui_pf(struct pfstack *ppf,char linha[82])
{
  int i,ok,j,continua=FALSE;
  int lv, ind;
  char level=' ',indice=' ';
  static char lin_out[82];  
  j=0;
  for (i=0; i<82; i++)
    lin_out[i]=' ';
  lin_out[0]='\0';  
  i=0;
  if ((linha[0]!=' ')&&(linha[0]!='\n')&&(linha[0]!='\t')) //se tem algo escrito no label
  {
    ok=compara_pf_linha(ppf,linha,0,&level,&indice);	  
    if (ok)
    //substitui o param da linha pelo par ordenado da pilha(sem espaco)
    {
      lin_out[0]='#';
	   lin_out[1]='(';
	   lin_out[2]=level;
	   lin_out[3]=',';
	   lin_out[4]=indice;
	   lin_out[5]=')';   
      lin_out[6]=' ';   
      lin_out[7]=' ';   
      j=8;

      for(i=0;(linha[i]!=' ')&&(linha[i]!='\t');i++);
      for(i=i;((linha[i]==' ')||(linha[i]=='\t'));i++);
    }
    else   //nao encontrou param na pilha-> copia igual
	 {      
	   for(i=0;((linha[i]!=' ')&&(linha[i]!='\n')&&(linha[i]!='\t'));i++)
		{
	      lin_out[j]=linha[i];
		  j++;
		}
	 }	  
 }
 else //não tem label - conferir se não gera erro na saída
 {   
   for(i=0;((linha[i]==' ')||(linha[i]=='\t'));i++)
   {
      lin_out[j]=linha[i];   
	  j++;
	}
 } 
 for(i=i;((linha[i]!=' ')&&(linha[i]!='\t'));i++) //copia cod op
 {                   
   lin_out[j]=linha[i];
   j++;
 }
 if (linha[i]!='\n')
 {
   do  //inicio do laco que busca os operandos
   {
      continua=FALSE;
      j++;
	  if (linha[i-1]==',')
	  {
	    lin_out[j-1]=linha[i-1];
	    j--;
	  }
	  for(i=i;((linha[i]==' ')||(linha[i]=='\t'));i++); //rastreia operando    
      ok=compara_pf_linha(ppf,linha,i,&level,&indice);
      if (ok)  //encontrou parametro na pilha de PFs
	  {
        j++;
		lin_out[j++]='#';
		lin_out[j++]='(';
		lin_out[j++]=level;
		lin_out[j++]=',';
		lin_out[j++]=indice;
		lin_out[j++]=')';
		for(i=i;((linha[i]!='\n')&&(linha[i]!=','));i++);  //passa pelo operando
      }
	  else   //nao encontrou param na pilha-> copia igual
	  {
        j++;
	 	for(i=i;((linha[i]!='\n')&&(linha[i]!='\t')&&(linha[i]!=','));i++)
		{
		  lin_out[j]=linha[i];
		  j++;
        }
      }
      if(linha[i]==',')//||(linha[i]==' '))	//teste para continuar
      {
        continua=TRUE;
		i++;
	  }
	  else
	    continua=FALSE;
   }while(continua);
 }
 else
   lin_out[j]='\n';   //terminou o texto de saida

 for(j=j;(j<=MAXPOS)&&(linha[i]!='\n');j++)
 {
   lin_out[j]=linha[i];
   i++;
 }
 if (linha[i]=='\n')
   lin_out[j]='\n';
 return lin_out;
}
	
/*	void subst_parord_por_PR(char linha[81],struct pprstack *ppr,char lin_out[81])
Realiza a substituição do par ordenado pelo parâmetro real. Essa substituição é feita pela análise do par ordenado e a posição do parâmetro na pilha.
Entrada: linha original, pilha de listas de parâmetros reais, linha de retorno.   
Variáveis importantes:
- tam: tamanho do parâmetro;
- lv: nível do parâmetro;
- achou: testa se o par ordenado #(d,i), tem correspondência na atual lista de parâmetros reais. 
*/

void subst_parord_por_PR(char linha[81],struct pprstack *ppr,char lin_out[81])
{
  int i,j,k,l,tam,e,lv,achou,continua, label=0;
  char val[8],aux;
  struct dado_listaPR *dat;
  struct listaPR *list;
  for(i=0;i<=80;i++)
	 lin_out[i]=' ';
  for(i=0;i<=MAXPOS;i++)
	 val[i]=' ';
  i=0;
  j=0;
  do   //inicio do laco
  {
    label=0;
    for(i=i;(linha[i]!='#')&&(linha[i]!='\n');i++)   //rastreia #
	{
      lin_out[j]=linha[i];
	  j++;
	}
	
	if(i==0)
	  label=1;
	
    if (linha[i]!='\n')
	{
	  k=i+2;
	  lv=atoi(&linha[k]);
	  if (lv>1)
	  {
        lv--;
	    itoa(lv,&aux,10);
        linha[k]=aux;
	    for(k=i;((linha[k]!=')')&&(k<80));k++)
		{                                            
		  lin_out[j]=linha[k];
		  j++;
        }        
		lin_out[j++]=')';
      }
	  else
	  {        
	    //pesquisa na lista de parametros reais
	    tam=0;
	    achou=FALSE;
	 	val[0]='*';  // flag de vazio
		for(k=i;(linha[k]!=')');k++)
		  tam++;
        tam++;
		list=ppr->topo;
		if (list!=NULL)
		{
          dat=list->primeiro;
		  while((dat!=NULL)&&(!achou))
		  {
		    if (memicmp(&linha[i],dat->posicao,tam)==0)
		    {
              achou=TRUE;
		      for(k=0;dat->valor[k]!=' ';k++) // copia valor para val
  		        val[k]=dat->valor[k];              
            }
		    dat=dat->prox;
	      }
	    }//fim da pesquisa

        if (val[0]!='*')
		{ 
          tam=tamanho_param(val);
          l=j;
		  k=0;
		  for(j=l;j<l+tam;j++)
		  {
		    lin_out[j]=val[k];
		    k++;
          }
          for(e=0;e<=MAXPOS;e++)
	        val[e]=' ';
        }
		else
		  lin_out[j]=' ';
      }

	  for(i=i;linha[i]!=')';i++);
	  if ((linha[i+1]==',')||(label))//(linha[i+1]==' '))&&(!strchr("\n *  ",linha[i+2])))
	  {
        continua=TRUE;
	    i++;
      }
	  else
	    continua=FALSE;
    }
    else
	  continua=FALSE;
  }while(continua);

  if (linha[i]=='\n')
  {
    j++;
	lin_out[j]='\n';
	lin_out[j+1]='\0';
  }
  else
  {
    for(i=i+1;(linha[i]!='\n')&&(linha[i]!='*');i++)
	{
	  j++;
	  lin_out[j]=' ';
    }

    j++;
	if (linha[i]=='\n')
	{
      lin_out[j]=linha[i];
	  lin_out[j+1]='\0';
    }
    else
	  if (linha[i]=='*')   //teste de comentario
	  {
        for(i=i;(i<=MAXPOS)&&(linha[i]!='\n');i++)
        {
		  lin_out[j]=linha[i];
          j++;
	    }
		j++;
		if (linha[i]=='\n')
		{  
          lin_out[j]='\n';
		  lin_out[j+1]='\0';
        }
		else
		{
          lin_out[MAXPOS]='\n';
		  lin_out[MAXPOS+1]='\0';
        }

      } //fim do teste de comentario
	  else
	  { //i posicao maxima
	    lin_out[j]='\n';
	    lin_out[j+1]='\0';
      }
  }
}

/*
int empilha_param(struct pfstack *p,char *linha,int lv)
Empilha os parâmetros existentes no protótipo da macro passado pela linha '*linha', colocando-os na pilha de parâmetros formais. O parâmetro 'lv' é o contador de nível de definição e 'p' é um ponteiro temporário utilizado para alocação. Quando o parâmetro é empilhado, também são incluídos o seu nível e índice.
- continua: variável que permite que o laço de busca de operandos continue;
- label: variável de controle que indica rotulo como parametro formal;
- ok: variável de retorno. Retorna 0 se houve erro na alocação de memória;
- idx: conta o índice dos operandos;
- cont: guarda a posição do próximo operando;
- aux: guarda o valor do parâmetro.
*/

int empilha_param(struct pfstack *p,char *linha,int lv)
{
  int j,i=0,k,continua,ok;
  int idx=0,cont,label=0;
  char aux[8];
  for (k=0; k<MAXPOS; k++)
    aux[k]=' ';
  cont=0;
  continua=FALSE;
  do
  { 
    for (k=0; k<MAXPOS; k++)
    aux[k]=' ';
	 p=(struct pfstack *)malloc(sizeof(struct pfstack));
	 if (p==NULL)
     {
       printf("Nao foi possivel alocar parametro");
       system("pause");	 
	   ok=0;
     }	   
	 else
    {
	   ok=1;
	   if (((linha[0]!=' ')&&(linha[0]!='\t'))&&(!continua))
      {
	     for(j=0;(linha[j]!=' ')&&(linha[j]!='\t');j++)                         
		    aux[j]=linha[j];  
		  i=j;
		  continua=TRUE;
	   }
	   else
	   {
	     if (cont==0)
	     {
	       for(i=i;(linha[i]==' ')||(linha[i]=='\t');i++); //percorre os espaços até primeiro char
			 for(i=i;(linha[i]!=' ')&&(linha[i]!='\t');i++);//percorre os chars até primeiro espaço
			 for(i=i;(linha[i]==' ')||(linha[i]=='\t');i++);//percorre os espaços até primeiro char(parametro)
		  }
	     else
	       i=cont+2; //passa pela virgula
   
		  idx++;
		  k=-1;
		  for(j=i; !strchr(", \t \n",linha[j]);j++)  //copia o operando para aux
		  {
		    k++;		   
		    aux[k]=linha[j];
	     }
		  cont=j;
      }

	   //inicializa  parametro
	   p->param[k]='\0';

      strcpy(p->param,aux); 
	   if (label)
        label=0;
     itoa(lv,&p->nivel,10);	 
	  itoa(idx,&p->ind,10);

	  p->prox=ppf;
	  if (ppf==NULL)
	    p->prox=NULL;
	  ppf=p;  //empilha o parametro na pilha de parametros formais

	  if (strchr("* \t \n",linha[cont]) )
	    continua=FALSE;
	  else
	    continua=TRUE;		
    }
  }while(continua);
  return ok;
}

/*
void desempilha_param(struct pfstack *p,int lv)
Tem a tarefa de desempilhar os parâmetros formais da pilha(pfstack) que estão no nível de definição 'lv'. O ponteiro 'p' é usado para alocação.
- sai: variável que sai do laço assim que não existir mais parâmetros do nível atual para desempilhar.
*/
void desempilha_param(struct pfstack *p,int lv)
{
  int sai;
  sai=FALSE;
  while ((ppf!=NULL)&&(!sai))
  {
    p=ppf;
	 if (itoa(lv,&p->nivel,10) == NULL)
	 {
	   ppf=ppf->prox;
		free(p);
		p=NULL;
	 }
	 else
	   sai=TRUE;
  }
}

/*
int empilha_listas(struct pprstack *ppr,char *linha)
Empilha 'ppr' uma lista de parâmetros reais a partir da chamada da macro encontrada na linha '*linha'.
Variáveis importantes:
- lpr: Usada para alocação da lista;
- resp: variável de retorno da subrotina cria_listPR  (0 ou 1);
- ok: variável de retorno. Retorna 0 se ocorreu erro na alocação de memória.
*/
int empilha_listas(struct pprstack *ppr,char *linha)
{
  struct listaPR *lpr;
  int resp,ok; 
  lpr=(struct listaPR *)malloc(sizeof(struct listaPR));
  if (lpr==NULL)
    ok=0;
  else
  {   
    lpr->primeiro=NULL;		
	lpr->ultimo=NULL;		
	resp=cria_listPR(lpr,linha);    
	if (resp==0)
	  ok=0;
	else
	{      
	  lpr->proxlista=ppr->topo;
	  if (ppr->topo==NULL)
	    lpr->proxlista=NULL;
	  ppr->topo=lpr;
	  ok=1;		
    }
  }	
  return ok;
}

/* void desempilha_listaPR(struct pprstack *ppr)
Desempilha a lista de parâmetros reais do nível de expansão atual.
Utiliza a variável 'lpr' para a remoção.
*/
void desempilha_listaPR(struct pprstack *ppr)
{
  struct listaPR *lpr;
  if (ppr->topo!=NULL)
  {
    lpr=ppr->topo;
	ppr->topo=ppr->topo->proxlista;
	while(lpr->primeiro!=NULL)
	  remove_dado_listPR(lpr);
    free(lpr);
  }
}

/* int cria_listPR(struct listaPR *lp,char *linha)
Cria a lista de parâmetros reais a partir da chamada da macro encontrada na linha 'linha'. O parâmetro é empilhado com seu valor e sua posição indicada pelo par ordenado #(d,i).
Variáveis importantes:
- continua: avisa que a busca por operandos deve continuar;
- ok: variável de retorno. Retorna 0 se ocorreu erro de alocação de memória;
- cont: posição do próximo operando;
- idx: índice do parâmetro;
- val: guarda o valor do parâmetro;
- aux: guarda a posição do parâmetro;
- p: usada para alocação;
*/

int cria_listPR(struct listaPR *lp,char *linha)
{ 
  int j,i,k,continua,ok;
  int idx=0,cont;
  char aux[8],val[8];
  struct dado_listaPR *p;
  cont=0;
  i=0;
  j=0;
  continua=FALSE;
  printf("linhalista=%s",linha);
  system("pause");  
  do
  {
    for(k=0;k<=MAXPOS;k++)
	{
	  aux[k]=' ';
	  val[k]=' ';
    }	  	  	  
	p=(struct dado_listaPR *)malloc(sizeof(struct dado_listaPR));
	if (p==NULL)
	  ok=0;
    else
	{ 
      
      p->valor[0]='\0';
      p->posicao[0]='\0';
      ok=1;
	  if ((linha[0]!=' ')&&(linha[0]!='\t')&&(!continua))
      {      
        for(j=0;((linha[j]!=' ')&&(linha[j]!='\t'));j++)
		  val[j]=linha[j];
        continua=TRUE;			  
		i=j;
      }
      else
	  {        
	    if (cont==0)
        {       
	      for(i=i;linha[i]==' ';i++);                           //rastreia opcode
	      for(i=i+1;((linha[i]!=' ')&&(linha[i]!='\t'));i++);   //rastreia espaco
	      for(i=i;((linha[i]==' ')||(linha[i]=='\t'));i++);     //rastreia operando
	    }
		else
		  i=cont;
		  
        idx++;
        k=0;
		if(linha[j+1]==',')
		  val[0]='\0';
	    else
        {
         if (linha[j]==',')
         { 
           if(linha[j+1]==' ')  
             for(i=i;(linha[i]!=' '); i++);
           i++;
         }    
		 for(j=i;((linha[j]!='\n')&&(linha[j]!=',')&&(linha[j]!='\t'));j++)
	     {
		   val[k]=linha[j];
		   k++;
         }
         cont=j;
        }    
      }     
      
      aux[0]='#';
	  aux[1]='(';
	  aux[2]='1';
	  aux[3]=',';
	  itoa(idx,&aux[4],10);
	  aux[5]=')';

	  strcpy(p->posicao,aux);
	  strcpy(p->valor,val);
	  p->prox=NULL;
	  if (lp->ultimo!=NULL)
	    lp->ultimo->prox=p;
	  lp->ultimo=p;
	  if (lp->primeiro==NULL)
	    lp->primeiro=p; 

	  if ((linha[cont]==' ')||(linha[cont]=='\n'))
	    continua=FALSE;
	  else
	    continua=TRUE;
    }
  }while(continua);
  return ok; 
}

/* void remove_dado_listPR(struct listaPR *lpr)
Remove um registro (parâmetro) da lista de parâmetros reais passado como 'lpr'.A variável p: ponteiro serve como auxiliar na remoção.
*/
void remove_dado_listPR(struct listaPR *lpr)
{
  struct dado_listaPR *p;
	if (lpr->primeiro!=NULL)
	 {
		p=lpr->primeiro;
		lpr->primeiro=p->prox;
		free(p);
		p=NULL;
		if (lpr->primeiro==NULL)
		  lpr->ultimo=NULL;
	 }
}

/* int alocadef(struct defmacro *f)
Função que serve para alocar uma nova definição de macro na tabela de definições de macro.
O ponteiro 'f' é utilizado para fazer a alocação.
Retorna 1 se alocou e 0 se não alocou.
*/
int alocadef(struct defmacro *f)
{
  f=(struct defmacro *)malloc(sizeof(struct defmacro));
  if (f==NULL)
    return 0;
  else
  {
    strcpy(f->prototipo,"Temp");
	 f->prox=first;
	 f->l=NULL;
	 if (first==NULL)
	   f->prox=NULL;
	 first=f;
	 return 1;
  }
}

/* -	void desaloca_def(struct defmacro *f)
Desaloca toda a tabela de definições de macros. O ponteiro 'f' é utilizado como auxiliar para a realização da desalocação.
lin é um ponteiro auxiliar para remoção das linhas da definição da macro.
*/
void desaloca_def(struct defmacro *f)
{
  struct tlinha *lin;
  while(first!=NULL)
  {
    f=first;
	 while(f->l!=NULL)
	 {
	  lin=first->l;
	  first->l = first->l->proxl;
	  free(lin);
	  lin=NULL;
	 }
	 first=first->prox;
	 free(f);
	 f=NULL;
  }
}

/* int local_insert_erro(struct tabela_erros te,int lin)
Função que a partir da tabela de erros que entra como 'te' e o número da linha 'lin', devolve o local na tabela onde o próximo registro deve ser inserido.
Variáveis importantes:
- pos: variável de retorno;
- teste: testa se a posição de inserção já foi encontrada.
*/
int local_insert_erro(struct tabela_erros te,int lin)
{
  int i,pos,teste;
  if (te.n_erros==0)
  pos=0;
  else
  {
    pos=te.n_erros;
	 teste=FALSE;
	 i=0;
	 do
	 {
	   if (lin<=te.erros[i].line)
	   {
	     pos=i;
		  teste=TRUE;
		}
		i++;
	}while((i<MAX_ERROS)&&(!teste));
  }
  return pos;
}

/* void adiciona_erro(struct tabela_erros *te,int codigo,int lin)
Insere na tabela de erros 'te' um registro contendo o código do erro 'codigo' (previamente informado em uma tabela de erros) e a linha 'lin' onde foi encontrado esse erro.
*/
void adiciona_erro(struct tabela_erros *te,int codigo,int lin)
{
  int i1,j1;
  // i1,j1: contadores

  if (te->n_erros>=MAX_ERROS)
  {
    te->n_erros++;
  }
  else
  {
    i1=local_insert_erro(*te,lin);
	 if (i1>=te->n_erros)
	 {
	   te->erros[i1].cod=codigo;
	   te->erros[i1].line=lin;
	 }
	 else
	 {
	   for(j1=te->n_erros;j1>i1;j1--)
		{
		  te->erros[j1].line=te->erros[j1-1].line;
		  te->erros[j1].cod=te->erros[j1-1].cod;
		}
	   te->erros[i1].line=lin;
	   te->erros[i1].cod=codigo;
	 }
	 te->n_erros++;
  }
}

/* int pesquisa_cod_op(char line[82],FILE *arq)
Compara as operações existentes no arquivo 'cod_op.tab' com o campo de operação da linha 'line' devolvendo 1 se caso encontrada a operação.
Variáveis importantes:
- achou: variável de retorno (0 negativo ou 1 positivo);
- tam: tamanho da operação;
- pos: posição na linha onde está o campo de operação;
- simbolo: variável que guarda a operação do arquivo;
- teste: variável que testa se foi possível ler a linha do arquivo.
*/

int pesquisa_cod_op(char line[82],FILE *arq)
{
  int achou,tam,i,pos;
  char simbolo[8],*teste;
  for(i=0;i<=7;i++)
    simbolo[i]=' ';
  achou=FALSE;
  tam=tam_operacao(line,&pos);
  teste=fgets(simbolo,8,arq);
  while((teste!=NULL)&&(!achou))
  {
    if(memicmp(&line[pos],simbolo,tam)==0)
	   achou=TRUE;
	 teste=fgets(simbolo,8,arq);
  }
  return achou;
}


/* int empilha_nestcall(struct pilha_nestcalls *pn,struct defmacro *df,struct tlinha *pl)
Função que coloca na pilha de chamadas aninhadas um registro contendo a atual definição expandida 'df' e a linha 'pl' onde o processamento deve continuar após a expansão desta macro. O ponteiro 'pn' e um ponteiro auxiliar para alocação.
*/

int empilha_nestcall(struct pilha_nestcalls *pn,struct defmacro *df,struct tlinha *pl)
{
  pn=(struct pilha_nestcalls *)malloc(sizeof(struct pilha_nestcalls));
  if (pn==NULL)  
    return 0;  
  else
  {
    pn->atual_def=df;
    pn->ant_lin=pl;
    if (topo_nest==NULL)
	  pn->prox_call=NULL;
    else
	  pn->prox_call=topo_nest;
    topo_nest=pn;
    return 1;
  }
}

/* void desempilha_nestcall(struct pilha_nestcalls *pn)
Ao contrário da função anterior, esse procedimento desempilha um registro da pilha de chamadas aninhadas. O ponteiro 'pn' é utilizado para auxiliar na  remoção.
*/ 
void desempilha_nestcall(struct pilha_nestcalls *pn)
{
  if (topo_nest!=NULL)
  {
    pn=topo_nest;
    topo_nest=topo_nest->prox_call;
	  free(pn);
    pn=NULL;
  }
}

/* int macros(char *nome_fonte, struct tabela_erros *tab)
Função principal do pré-processador de macros, chamada pelo integrador.c. Possui como 
parâmetros de entrada o nome do arquivo de entrada 'nome_fonte' e a tabela de 
erros 'tab'
Sucintamente ele opera da seguinte maneira:
 - Le cada linha do código fonte;
 - Se houver um campo de operação, analisa se ele é um delimitador de definição de
   macro 'MACRO', se é um delimitador de fechamento de definição de macro 'MEND',
   um protótipo de macro, uma chamada de macro ou outra operação qualquer.
   Se caso não houver este campo, simplesmente copia a linha sem mudanças para o arquivo
   de saída;
 - Para cada uma das possibilidades citadas acima, faz o processamento apropriado;
 - No final do processamento, a função retorna 1 se operação foi bem sucedida e o
   resultado do processamento fica registrado no arquivo de saida MASMAPRG.ASM.
   
Variáveis importantes:
- fonte: arquivo de entrada com nome 'nome_fonte';
- dest: arquivo de saída com nome MASMAPRG.ASM;
- arq: arquivo que contém a tabela de operações, com nome 'cod_op.tab';
- c_linha: conta linhas no texto de entrada;
- c_linha2: conta linhas de uma definição de macro (usado quando e>0);
- d: contador de nível de definição;
- e: contador de nível de expansão;
- achou: indica se o opcode foi encontrado na tabela de instruções;
- ldef: guarda número da linha onde foi encontrada a instrução "MEND";
- l_write: avisa se a linha alterada no modo de expansão deve ser escrita;
- ok: variável de retorno;
- linha: contém a linha do arquivo fonte;
- l_nova: linha resultante do processamento de uma linha do código fonte;
ou de uma linha da definição da macro;
- teste: testa se foi possível ler a linha do código fonte;
- def: guarda a definição da macro pesquisada na tabela de definições;
- pfaux,lin,pnc: ponteiros auxiliares para alocação e remoção na pilha de parâmetros formais,
na tabela de definições e na pilha de chamadas aninhadas respectivamente;
*/
int macros(char *nome_fonte, struct tabela_erros *tab)
{	 
  FILE *fonte;
  FILE *dest;
  FILE *arq;

  int c_linha, c_linha2, d, e, i, j, achou, ldef, l_write, other, prototype,
    call, macro, mend, fim, ok;
  char linha[82], l_nova[82], *teste;
  struct defmacro *def;
  struct pfstack *pfaux;
  struct tlinha *lin;
  struct pilha_nestcalls *pnc;
      
//INICIALIZACOES
//*VARIAVEIS  LOCAIS
  for(i=0; i<=MAXPOS+1; i++)
  {
    linha[i] = ' ';
    l_nova[i] = ' ';
  //teste[i] = ' ';
  }
  l_write = TRUE;
  fim = 0;
  c_linha = 0;
  d = 0;
  e = 0;
  ldef = -1;  
//*VARIAVEIS GLOBAIS

//ponteiro para inicio da tabela de definicoes de macros
  first = NULL;
// ppf = pilha de parametros formais
  ppf = NULL;
// ppr = pilha de parametros reais
  ppr=(struct pprstack *)malloc(sizeof(struct pprstack));
  ppr->topo = NULL;
// topo da pilha de nestcalls
  topo_nest = NULL;
  tab->n_erros = 0;
  for(i=0; i<MAX_ERROS; i++)
    tab->erros[i].line = -1;

//Inicio da funcao

  if((fonte = fopen(nome_fonte, "r")) != NULL)    
    if((dest = fopen("MASMAPRG.ASM","w")) != NULL)
    {
      for (i=0; i<82; i++)
        linha[i]=' ';
      teste = fgets(linha,82,fonte);   // se fgets == NULL => nao conseguiu ler a linha
      c_linha++;      
	   if(teste != NULL)
        if((linha[80] != ' '))
        {
          adiciona_erro(tab, 2, c_linha); //ERRO: LINHA MUITO LONGA
	       ok = 0;	       
        }         
        arq = fopen("cod_op.tab", "r"); //ABRE ARQUIVO COM CODIGOS DE OPERAÇÃO      
	     if(arq != NULL)
        {        
          ok = 1;         // VARIAVEL DE RETORNO
          while(fim == 0)
          {
            printf("linha=%s",linha);
            system("pause");
            prototype = FALSE;
            call = FALSE;
            macro = FALSE;
            mend = FALSE;
            other = FALSE;
            fim = FALSE;
          
            if ((linha[0] != ' ')&&(linha[0] != '\t')) //se tem algo antes do opcode
            {          
              for (i=0; !strchr("\t * \n", linha[i]); i++);  //procura espaço
              if ((linha[i] == '\n')||(linha[i] == '*'))
                other = TRUE;  
              else
                for (i=i; ((linha[i] == ' ')||(linha[i] == '\t'))&&(i <= MAXPOS); i++);  //procura proximo char
            }
            else //se não tem algo antes do opcode
              for (i=0; ((linha[i] == ' ')||(linha[i] == '\t'))&&(i <= MAXPOS); i++);
         	  //encontra primeiro caracter da linha
              //comeca a pesquisa para identificar o codigo de operacao
           
            if ((i != MAXPOS+1)&&(linha[i] != '\n')&&(linha[i] != '*'))
            {
              achou = pesquisa_cod_op(linha, arq);
              rewind(arq); //volta ao inicio da tabela de codigo de operação
              if(achou)   //achou codigo de operação na tabela
                if (c_linha!=ldef+1)
                  other = TRUE;
                else
                {
                  adiciona_erro(tab,1,c_linha);
                  ok = 0;                
                  prototype = TRUE;
                }
                else //não é codigo de operação
                if ((memicmp(&linha[i], "MACRO",5) == 0)&&(strchr("* \t \n",linha[i+5])))   //Definicao de Macro
                {
                  if (c_linha == ldef+1)
                  {
                    adiciona_erro(tab, 1, c_linha); ///REDEFINICAO DE OPERACAO
                    ok = 0;
                    prototype = TRUE;
                  }
                  else
                    macro=TRUE;
                }
                else
                  if ((memicmp(&linha[i], "MEND",4) == 0)&&(strchr("* \t \n",linha[i+4])))    //FIM de definicao de Macro
					   {
                    if (c_linha==ldef+1)
                    {
                      adiciona_erro(tab, 1, c_linha); //ERRO:REDEFINICAO DE OPERACAO
                      ok = 0;
                      prototype = TRUE;
                    }
                    else
                      mend = TRUE;
                  }
                  else
                  if((c_linha == ldef+1)||(c_linha2 == ldef+1))
                    prototype = TRUE;
                  else
                  {
                    def = pesquisa_tabdef(first, linha, i);
                    if (def != NULL)
                    {
                      printf("def=%s",def->prototipo);
                      call = TRUE;
                      if(d == 0)
                      {
                        if(e > 0)  //chamada aninhada
                          empilha_nestcall(pnc, def, lin);  //guarda proxima linha
                        else
                          empilha_nestcall(pnc,def,NULL);
                        lin=NULL;
                        c_linha2=0;
                      }
                    }
                    else
                    {
                      if(d == 0)
                        adiciona_erro(tab, 3, c_linha);  //ERRO: CHAMADA DE MACRO NAO DEFINIDA
                      other = TRUE;
                    }
                  }
            }
            else
              other = TRUE;
              
            //comeca o processamento propriamente dito
		      if (macro)    //Definicao de Macro
		      {
              d++;
              if(e > 0)
                ldef = c_linha2;
              else
                ldef = c_linha;
              if(d == 1)
                alocadef(first);
              else
                escrevelinha(linha, first);
            }
            else
              if(prototype)   //Prototipo de Macro
              {              
                if(e == 0)
                  empilha_param(ppf, linha, d);               
                if (d > 0)
                  escrevelinha(linha, first);
              }
	           else
                if(call)
                {
                  if(d == 0)
                  {
                    e++;
                    empilha_listas(ppr, linha);
                  }
                  if ((d > 0)&&(e == 0))
                    strcpy(l_nova,substitui_pf(ppf, linha));
                  if (d > 0)
                    escrevelinha(l_nova, first);
                }
                else
                  if(mend)    //Fim de Macro
                  {
                    if(d == 0)
                    {
                      desempilha_listaPR(ppr);
                      e--;
                      lin = topo_nest->ant_lin;
                      desempilha_nestcall(pnc);
                    }
                    else
                    {
                      if(e == 0)
                        desempilha_param(pfaux, d);
                      d--;
                      escrevelinha(linha, first);
                    }
                  }
				  else 
                    if(other)
                    { 
                      if((e == 0)&&(d > 0))
                        strcpy(l_nova,substitui_pf(ppf, linha));
                      if(d == 0)
                        fputs (linha, dest);  //escreve na saida a linha atual
                      else
                       if(l_write)
                         escrevelinha(linha, first);
                       else
                         escrevelinha(l_nova, first);
                    }
            //se opcode = END e não há nada depois  
            if((memicmp(&linha[i], "END", 3) == 0)&&(strchr("\t \n",linha[i+3])))
            {
              if(d > 0)
                adiciona_erro(tab, 4, c_linha); //ERRO: MEND esperado
              fim = 1;
            }
            l_write = FALSE;   //avisa se a proxima linha deve ser escrita
            if(e > 0)  //MODO DE EXPANSAO
            {
              if(lin == NULL)
                lin = topo_nest->atual_def->l; //aponta para a primeira linha da def atual            }
              else
                lin = lin->proxl;
              //printf("%s",lin);
              //getch();
              c_linha2++;
              for(j=0; j<=80; j++)
                linha[j]=lin->linha[j];
              subst_parord_por_PR(linha, ppr, l_nova);
              for(j=0; j<=80; j++)
                linha[j]=l_nova[j];
              if (d > 0)
                l_write = TRUE;
            }
            else
            { 
              for (i=0; i<82; i++)
                linha[i]=' ';
              teste = fgets(linha, 82, fonte);  // fgets == NULL => nao conseguiu ler a linha
              c_linha++;
              if(teste != NULL)
              {
                if(linha[80] != ' ')
                {
                  adiciona_erro(tab, 2, c_linha);
                  ok = 0;                
                }
              }
              else
                fim = 1;
            }
          } //fim while
        }
	     else
		    printf("ERRO NA ABERTURA DA TABELA DE OPERACOES");
    }
    else
      printf("ERRO NA ABERTURA DO ARQUIVO DE SAIDA\n");
  else
    printf("ERRO NA ABERTURA DO ARQUIVO FONTE\n");
  
  desaloca_def(def);
  fclose(fonte);
  fclose(dest);
  fclose(arq);
  return ok;
}
