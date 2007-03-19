
/*=====================================================================
UNIVERSIDADE FEDERAL DE PELOTAS
BACHARELADO EM CIÊNCIA DA COMPUTAÇÃO
PROJETOS EM COMPUTAÇÃO
Processador de Macros, Integração Macro Montador (Versão preliminar)

Componentes:
Giales Fischer
Pablo Barcellos
Mauro Brito
=====================================================================*/
//main(){}

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define OK 0
#define ERRO 1
#define FALTA_MEMORIA 2
#define ERRO_NA_LEITURA 3


// MAX_ERROS = numero maximo de erros que podem ocorrer no programa:
#define MAX_ERROS 15
#define FALSE 0
#define TRUE 1
#define MAXPOS 8
#define TAM 82
// MAXPOS = tamanho maximo dos parametros

/*======================= Estruturas Usadas =========================*/


/* Linhas em listas encadeadas */
struct tipo_linha
{
  char linha[81];
  struct tipo_linha *proxima_linha;  //proxima linha da definicao
};


/* Pilha de definicoes de macros */
typedef struct definicao_de_macro
{
  char prototipo[81];		//prototipo da macro
  struct tipo_linha *linha;        //primeira linha do encadeamento
  struct definicao_de_macro *proxima_definicao;   //proxima definicao
}Tabela_Definicao_De_Macros;


/* Pilha de chamadas aninhadas */
typedef struct chamadas_aninhadas
{
  struct chamadas_aninhadas *proxima_chamada;  //proxima chamada do aninhamento
  Tabela_Definicao_De_Macros *definicao_atual;         // macro que esta sendo expandida
  struct tipo_linha *linha_anterior;             //guarda a linha onde deve continuar o processamento
}Pilha_Chamadas_Aninhadas;


/* Pilha parametros formais */
typedef struct pilha_parametros_formais
{
    char param[8];  // parametro empilhado
    char nivel_do_parametro;     // nivel de definicao do parametro
    char indice_do_parametro;       // indice do parametro
    struct pilha_parametros_formais *proximo_parametro;  //proximo parametro na pilha
}Pilha_Parametros_Formais;


// Armazena os dados dos parametros reais, para depois trocar com os pares ordenados
typedef struct campo_parametros_reais
{
  char valor[8];              //valor do parametro na chamada
  char posicao[8];            //par #(d,i)
  struct campo_parametros_reais *proximo;  //proximo parametro da pilha
}Campo_Parametros_Reais;


typedef struct lista_parametros_reais
{
  struct campo_parametros_reais *primeiro;   //primeiro parametro da lista
  Campo_Parametros_Reais *ultimo;   //ultimo parametro da lista
  struct lista_parametros_reais *proxima_lista;   //proxima lista de parametros reais
}Lista_Parametros_Reais;

// Ponteiro para o inicio da pilha [de listas] de parametros reais (acima) - (Figura 4.17)
typedef struct pilha_parametros_reais		//mudar nome para prstack
{
  struct lista_parametros_reais  *topo;  //topo da pilha de listas
}Pilha_Parametros_Reais;

// Tipo de dado para armazenar a linha em que ocorreu um erro e o codigo do erro
struct tipo_erro
{
  int cod;
  int linha;
};

/* Armazena os erros encontrados durante o processamento das macros */
struct tabela_erros
{
  struct tipo_erro erros[MAX_ERROS];
  int numero_de_erros; //conta o numero de erros
};


/*======================== Variaveis Globais ========================*/

Pilha_Parametros_Formais *ppf;
Pilha_Parametros_Reais *ppr;

Tabela_Definicao_De_Macros *tab_def_macros;// primeira		

Pilha_Chamadas_Aninhadas *topo_nest;

/*======================= Prototipos das Funcoes ===========================*/

int tamanho_nome_da_macro(char *lin,int *posic);
Tabela_Definicao_De_Macros* pesquisa_tab_def(Tabela_Definicao_De_Macros *first,char line[82],int i);
int inserir_linha(char *line,Tabela_Definicao_De_Macros *first);

int tamanho_parametro(char parm[8]);
int compara_pf_linha(Pilha_Parametros_Formais *ppf,char linha[82],int pos,char *lv,char *indice);
char* substitui_parametro_formal(Pilha_Parametros_Formais *ppf,char linha[82]);

void trocar_par_ordenado_por_parametro_real(char linha[81],Pilha_Parametros_Reais *ppr,char lin_out[81]);
int empilha_parametros(Pilha_Parametros_Formais *p,char *linha,int lv);
void desempilha_parametros(Pilha_Parametros_Formais *p,int lv);

int empilha_lista_parametros_reais(Pilha_Parametros_Reais *ppr,char *linha);
void desempilha_lista_parametros_reais(Pilha_Parametros_Reais *ppr);
int cria_lista_parametros_reais(Lista_Parametros_Reais *lp,char *linha);

void remover_campo_lista_parametros_reais(Lista_Parametros_Reais *lpr);
int alocar_tab_def_macros(Tabela_Definicao_De_Macros *f);
void desalocar_tab_def_macros(Tabela_Definicao_De_Macros *f);

int local_insert_erro(struct tabela_erros te,int lin);
void adiciona_erro(struct tabela_erros *te,int codigo,int lin);
int pesquisar_cod_instrucao(char line[82],FILE *arq);

int empilhar_chamadas_aninhadas(Pilha_Chamadas_Aninhadas *pn,Tabela_Definicao_De_Macros *df,struct tipo_linha *pl);
void desempilhar_chamadas_aninhadas(Pilha_Chamadas_Aninhadas *pn);
int processador_de_macros(char *nome_fonte,struct tabela_erros *tab);


/*==================== função tamanho_nome_da_macro ===========================*/

//Funcao que devolve o tamanho do nome de uma macro
//Entrada: linha atual, indice onde comeca o nome da macro (usado como saida, recebe valor)

int tamanho_nome_da_macro(char *lin,int *indice)
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

  *indice=i;	//passa a posicao do primeiro char do codigo de operacao - MACRO

  // procura na coluna de operacoes (lin[j]) por um dos terminadores de linha (entre aspas)
  for(j=i;!strchr("\n \t \x0",lin[j]);j++)
    tam++;

  return tam;		//tam eh o contador do tamanho do nome da macro
}



/*==================== função definicao_de_macro =====================*/

//Funcao que procura uma uma macro na tabela de definicoes, devolvendo um ponteiro
//Entrada: ponteiro para topo da pilha, linha atual, posicao da linha com o primeiro char da macro (i)
//Saida: ponteiro para a definicao da macro

Tabela_Definicao_De_Macros* pesquisa_tab_def(Tabela_Definicao_De_Macros *primeira_def,char line[82],int i)
{
    Tabela_Definicao_De_Macros *def,*aux;
    int tam_op,j,pos;

    char nome_op[8];

    def=NULL;
    aux = primeira_def;
  
    for(j=0;j<=7;j++)
        nome_op[j]=' ';


    while ((aux!=NULL)&&(def==NULL))	//enquanto nao achou
    {
        tam_op = tamanho_nome_da_macro(aux->prototipo, &pos);
        
        //passa para a variavel temporaria o nome da macro, que comeca em pos:
        for(j=0;j<tam_op;j++)
	        nome_op[j]=aux->prototipo[pos+j];
        
        //compara os dois nomes das macros: (usa o valor em bytes de cada uma)
        if (memicmp(&line[i],nome_op,tam_op) == 0)
            def = aux;
	    else
	        aux = aux->proxima_definicao; //vai para a proxima definicao
    }

    return def;
}



/*======================= Função inserir_linha ========================*/

//Funcao que adiciona uma linha na lista encadeada da definicao de uma macro
//Entrada: linha ?? atual ??, ponteiro para a tabela de definicoes
//Saida: status (0 ou 1)

int inserir_linha(char *line,Tabela_Definicao_De_Macros *topo_definicoes)
{
    struct tipo_linha *linha,*linha_aux;
    int ok;

    if (strcmp(topo_definicoes->prototipo,"Temp")==0)	//se a tabela de defs. possui 0 prototipos
        strcpy(topo_definicoes->prototipo,line);	//nao precisa alocar mem para copiar a linha, que eh o prototipo (1¦ linha)
    else
    {
        linha=(struct tipo_linha *)malloc(sizeof(struct tipo_linha)); //aloca memoria para uma linha
        
        if (linha == NULL)  //Nao conseguiu alocar memoria para a linha
         {   //return FALTA_MEMORIA;
         ok=FALSE;
         }  
	     ok=TRUE;

	    if (ok)
	    {
            strcpy(linha->linha,line);	//passa a line para a linha da struct
		    linha->proxima_linha=NULL;   //diz que nao existe proxima linha
            
            if (topo_definicoes->linha==NULL)	 //se a definicao nao tem linhas
                topo_definicoes->linha=linha;	//eh so passar como primeira linha
		    else
		    {
                linha_aux = topo_definicoes->linha;
		        while(linha_aux->proxima_linha!=NULL)
		            linha_aux=linha_aux->proxima_linha;		//busca a ultima linha
		        linha_aux->proxima_linha=linha;			//coloca a nova linha na ultima posicao
		    }
	    }
    }
    
    return ok;
}



/*======================= Função tamanho_parametro =====================*/

//Funcao que conta o numero de chars de um parametro
//Entrada: uma string (o parametro)
//Saida: tamanho do contador

int tamanho_parametro(char parametro[8])
{
    int i=0;
    for(i=0; parametro[i]!=' '; i++);
    
    return i;
}



/*====================== Função compara_pf_linha =====================*/

/*	int compara_pf_linha(Pilha_Parametros_Formais *ppf,char linha[82],int pos,char *lv,char *indice)
	Compara o parâmetro que se encontra na linha na posição `pos` com os parâmetros da pilha de
  parâmetros formais.
	Se encontrado, devolve 1, o nível e o índice do parâmetro através das variáveis `lv` e 
  `indice` respectivamente.
	Variáveis importantes:
	- aux: variável auxiliar para pesquisar na pilha de parâmetros formais;
	- tam: variável que guarda o tamanho do parâmetro;
	- achou: variável booleana de retorno;
*/


//PODE MUDAR: saida pode ser uma Pilha_Parametros_Formais, que quando da problema, devolve -1 e -1
int compara_pf_linha(Pilha_Parametros_Formais *ppf,char linha[82],int pos,char *lv,char *indice)
{
    Pilha_Parametros_Formais *aux;
    int achou,tam;
    aux=ppf;
    achou=FALSE;

    while((aux!=NULL)&&(!achou))   //pesquisa na pilha de PFs
    {
        tam=tamanho_parametro(aux->param);
	    aux->param[tam]=' ';
	    if ((memcmp(&linha[pos],aux->param,tam)==0)&&(strchr("\t ,\n",linha[pos+tam])))
	    {
            achou=TRUE;		   
	        *lv=aux->nivel_do_parametro;
	        *indice=aux->indice_do_parametro;
        }
	    aux=aux->proximo_parametro;
    }
    return achou;
}



/*======================= Função substitui_parametro_formal ========================*/

/* char*  substitui_parametro_formal(Pilha_Parametros_Formais *ppf,char linha[82])
Função que a partir da linha que entra, compara os parâmetros desta linha com os parâmetros
da pilha de parâmetros formais. Se encontrado, faz a substituição pelo par ordenado #(d,i),
caso contrário, não os altera. A Saída da função será a linha de entrada alterada ou não.
Variáveis importantes:
- continua: verifica se existem mais parâmetros. Se for FALSE sai do laço;
- level: guarda o nível do parâmetro;
- indice: guarda o índice do parâmetro;
- lin_out: variável de retorno. */

char*  substitui_parametro_formal(Pilha_Parametros_Formais *ppf,char linha[82])
{
    int i,ok,j,continua=FALSE;
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



/*===================== Função trocar_par_ordenado_por_parametro_real ==================*/

/*	void trocar_par_ordenado_por_parametro_real(char linha[81],Pilha_Parametros_Reais *ppr,char lin_out[81])
Realiza a substituição do par ordenado pelo parâmetro real. Essa substituição é feita pela análise do par ordenado e a posição do parâmetro na pilha.
Entrada: linha original, pilha de listas de parâmetros reais, linha de retorno.   
Variáveis importantes:
- tam: tamanho do parâmetro;
- lv: nível do parâmetro;
- achou: testa se o par ordenado #(d,i), tem correspondência na atual lista de parâmetros reais. 
*/

void trocar_par_ordenado_por_parametro_real(char linha[81],Pilha_Parametros_Reais *ppr,char lin_out[81])
{
    int i,j,k,l,tam,e,nivel,achou,continua, label=0;
    char val[8],aux;
    Campo_Parametros_Reais *dat;
    Lista_Parametros_Reais *list;
    
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
	        nivel = atoi(&linha[k]);
	        if (nivel > 1)
	        {
                nivel--;
	            itoa(nivel, &aux, 10);
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
		                dat=dat->proximo;
                    }
                }//fim da pesquisa

                if (val[0]!='*')
		        { 
                    tam=tamanho_parametro(val);
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


/*====================== Função empilha_parametros ========================*/

/* int empilha_parametros(Pilha_Parametros_Formais *p,char *linha,int lv)                           */
/* Empilha os parâmetros existentes no protótipo da macro passado pela linha '*linha', colocando-os   */
/* na pilha de parâmetros formais. O parâmetro 'lv' é o contador de nível de definição e 'p' é um     */
/* ponteiro temporário utilizado para alocação. Quando o parâmetro é empilhado, também são incluídos  */
/*  o seu nível e índice.                                                                             */
/* - continua: variável que permite que o laço de busca de operandos continue;                        */
/* - label: variável de controle que indica rotulo como parametro formal;                             */
/* - ok: variável de retorno. Retorna 0 se houve erro na alocação de memória;                         */
/* - idx: conta o índice dos operandos;                                                               */
/* - cont: guarda a posição do próximo operando;                                                      */
/* - aux: guarda o valor do parâmetro.                                                                */

int empilha_parametros(Pilha_Parametros_Formais *p,char *linha,int lv)
{
    int j,i=0,k,continua,ok;
    int idx=0,contador,label=0;
    char aux[8];
    
    for (k=0; k<MAXPOS; k++)
        aux[k]=' ';
    
    contador=0;
    continua=FALSE;
    do
    { 
        for (k=0; k<MAXPOS; k++)
            aux[k]=' ';
        p=(Pilha_Parametros_Formais *)malloc(sizeof(Pilha_Parametros_Formais));
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
	        if (contador==0)
	        {
	            for(i=i;(linha[i]==' ')||(linha[i]=='\t');i++); //percorre os espaços até primeiro char
	            for(i=i;(linha[i]!=' ')&&(linha[i]!='\t');i++);//percorre os chars até primeiro espaço
                for(i=i;(linha[i]==' ')||(linha[i]=='\t');i++);//percorre os espaços até primeiro char(parametro)
            }
	        else
	            i=contador+2; //passa pela virgula
   
            idx++;
		    k=-1;
		    for(j=i; !strchr(", \t \n",linha[j]);j++)  //copia o operando para aux
		    {
                k++;		   
		        aux[k]=linha[j];
            }
		    contador = j;
            }

	        //inicializa  parametro
	        p->param[k]='\0';

            strcpy(p->param,aux); 
	        if (label)
                label=0;
     
            itoa(lv,&p->nivel_do_parametro,10);	 
	        itoa(idx,&p->indice_do_parametro,10);

	        p->proximo_parametro=ppf;
	        if (ppf==NULL)
	            p->proximo_parametro=NULL;
	        ppf=p;  //empilha o parametro na pilha de parametros formais

	        if (strchr("* \t \n",linha[contador]) )
	            continua=FALSE;
            else
	            continua=TRUE;		
        }
    }while(continua);
    
    return ok;
}

/*======================= Função desempilha_parametros ====================*/

/* void desempilha_parametros(Pilha_Parametros_Formais *p,int lv)*/
/* Tem a tarefa de desempilhar os parâmetros formais da pilha(pilha_parametros_formais) que estão no nível de definição 'lv'. O ponteiro 'p' é usado para alocação.*/
/* - sai: variável que sai do laço assim que não existir mais parâmetros do nível atual para desempilhar.*/

void desempilha_parametros(Pilha_Parametros_Formais *p,int lv)
{
    int sai;
    
    sai=FALSE;
    while ((ppf!=NULL)&&(!sai))
    {
        p=ppf;
	    if (itoa(lv,&p->nivel_do_parametro,10) == NULL)
	    {
	        ppf=ppf->proximo_parametro;
		    free(p);
		    p=NULL;
	    }
	    else
	        sai=TRUE;
    }
}

/*===================== Função empilha_lista_parametros_reais ========================*/

/* int empilha_lista_parametros_reais(Pilha_Parametros_Reais *ppr,char *linha)
/* Empilha 'ppr' uma lista de parâmetros reais a partir da chamada da macro encontrada na linha '*linha'.
/* Variáveis importantes:
/* - lpr: Usada para alocação da lista;
/* - resp: variável de retorno da subrotina cria_lista_parametros_reais  (0 ou 1);
/* - ok: variável de retorno. Retorna 0 se ocorreu erro na alocação de memória.*/

int empilha_lista_parametros_reais(Pilha_Parametros_Reais *ppr,char *linha)
{
    Lista_Parametros_Reais *lpr;
    int resp,ok; 
    
    lpr=(Lista_Parametros_Reais *)malloc(sizeof(Lista_Parametros_Reais));
    if (lpr==NULL)
        ok=0;
    else
    {   
        lpr->primeiro=NULL;		
	    lpr->ultimo=NULL;		
	    resp=cria_lista_parametros_reais(lpr,linha);    
	    if (resp==0)
	        ok=0;
        else 
	    {      
	        lpr->proxima_lista=ppr->topo;
	        if (ppr->topo==NULL)
	            lpr->proxima_lista=NULL;
	            
            ppr->topo=lpr;
	        ok=1;		
        }
    }	
    
    return ok;
}



/*===================== Função desempilha_lista_parametros_reais ======================*/

/* void desempilha_lista_parametros_reais(Pilha_Parametros_Reais *ppr)         */
/* Desempilha a lista de parâmetros reais do nível de expansão atual.  */
/* Utiliza a variável 'lpr' para a remoção.                            */

void desempilha_lista_parametros_reais(Pilha_Parametros_Reais *ppr)
{
    Lista_Parametros_Reais *lpr;
    if (ppr->topo!=NULL)
    {
        lpr=ppr->topo;
	    ppr->topo=ppr->topo->proxima_lista;
	    
        while(lpr->primeiro!=NULL)
	        remover_campo_lista_parametros_reais(lpr);
    
        free(lpr);
    }
}



/*===================== Função cria_lista_parametros_reais ==========================*/

/* int cria_lista_parametros_reais(Lista_Parametros_Reais *lp,char *linha)
/* Cria a lista de parâmetros reais a partir da chamada da macro encontrada na linha 'linha'. O parâmetro é empilhado com seu valor e sua posição indicada pelo par ordenado #(d,i).
/* Variáveis importantes:
/* - continua: avisa que a busca por operandos deve continuar;
/* - ok: variável de retorno. Retorna 0 se ocorreu erro de alocação de memória;
/* - cont: posição do próximo operando;
/* - idx: índice do parâmetro;
/* - val: guarda o valor do parâmetro;
/* - aux: guarda a posição do parâmetro;
/* - p: usada para alocação;                                            */

int cria_lista_parametros_reais(Lista_Parametros_Reais *lp,char *linha)
{ 
    int j=0,i=0,k=0,continua,ok=1;
    int indice=0,cont=0;
    char aux[8],val[8];
    Campo_Parametros_Reais *p;

    continua=FALSE;
    
    printf("linhalista=%s",linha); // <------------- AQUI <----------
    system("pause");
    do
    {
        for(k=0;k<=MAXPOS-1;k++)
	    {
	        aux[k]=' ';
	        val[k]=' ';
        }	  	  	  
	    p=(Campo_Parametros_Reais *)malloc(sizeof(Campo_Parametros_Reais));
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
		  
                indice++;
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
                    cont=j;//7?
                }    
            }     
      
            aux[0]='#';
	        aux[1]='(';
	        aux[2]='1';
	        aux[3]=',';
	        itoa(indice,&aux[4],10);
	        aux[5]=')';

	        strcpy(p->posicao,aux);
	        strcpy(p->valor,val);
	        p->proximo=NULL;
	        
            if (lp->ultimo!=NULL)
	            lp->ultimo->proximo=p;
	        
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



/*===================== Função remover_campo_lista_parametros_reais ====================*/

/* void remover_campo_lista_parametros_reais(Lista_Parametros_Reais *lpr)        */
/* Remove um registro (parâmetro) da lista de parâmetros reais passado*/
/* como 'lpr'.A variável p: ponteiro serve como auxiliar na remoção.  */

void remover_campo_lista_parametros_reais(Lista_Parametros_Reais *lpr)
{
    Campo_Parametros_Reais *p;
	
    if (lpr->primeiro!=NULL)
    {
		p=lpr->primeiro;
		lpr->primeiro=p->proximo;
		free(p);
		p=NULL;
		
		if (lpr->primeiro==NULL)
            lpr->ultimo=NULL;
    }
}



/*========================= Função alocar_tab_def_macros ==========================*/

/* int alocar_tab_def_macros(Tabela_Definicao_De_Macros *f)
/* Função que serve para alocar uma nova definição de macro na tabela
/* de definições de macro.
/* O ponteiro 'f' é utilizado para fazer a alocação.
/* Retorna 1 se alocou e 0 se não alocou.                             */

int alocar_tab_def_macros(Tabela_Definicao_De_Macros *f)
{
    f=(Tabela_Definicao_De_Macros *)malloc(sizeof(Tabela_Definicao_De_Macros));
    if (f==NULL)
        return 0;
    else
    {
        strcpy(f->prototipo,"Temp");
	    f->proxima_definicao = tab_def_macros;
	    f->linha=NULL;
	    
        if (tab_def_macros == NULL)
	        f->proxima_definicao=NULL;
	    
        tab_def_macros=f;
	    
        return 1;
    }
}



/*======================= Função desalocar_tab_def_macros =======================*/

/* -void desalocar_tab_def_macros(Tabela_Definicao_De_Macros *f)
/* Desaloca toda a tabela de definições de macros. O ponteiro 'f' é 
/* utilizado como auxiliar para a realização da desalocação.
/* lin é um ponteiro auxiliar para remoção das linhas da 
/* definição da macro.                                               */

void desalocar_tab_def_macros(Tabela_Definicao_De_Macros *f)
{
    struct tipo_linha *lin;
    
    while(tab_def_macros != NULL)
    {
        f = tab_def_macros;
	    while(f->linha!=NULL)
	    {
	        lin=tab_def_macros->linha;
	        tab_def_macros->linha = tab_def_macros->linha->proxima_linha;
	        free(lin);
	        lin=NULL;
	    }
	    tab_def_macros=tab_def_macros->proxima_definicao;
	    free(f);
	    f=NULL;
    }
}



/*==================== Função local_insert_erro ======================*/

/* int local_insert_erro(struct tabela_erros te,int lin)
/* Função que a partir da tabela de erros que entra como 'te' e o 
/* número da linha 'lin', devolve o local na tabela onde o 
/* próximo registro deve ser inserido.
/* Variáveis importantes:
/* - pos: variável de retorno;
/* - teste: testa se a posição de inserção já foi encontrada.*/

int local_insert_erro(struct tabela_erros te,int linha)
{
    int i,pos,teste;
    
    if (te.numero_de_erros==0)
        pos=0;
    else
    {
        pos=te.numero_de_erros;
	    teste=FALSE;
	    i=0;
	    do
	    {
	        if (linha<=te.erros[i].linha)
	        {
	            pos=i;
		        teste=TRUE;
		    }
		    i++;
	    }while((i<MAX_ERROS)&&(!teste));
    }
    
    return pos;
}



/*======================= Função adiciona_erro =======================*/

/* void adiciona_erro(struct tabela_erros *te,int codigo,int lin)
/* Insere na tabela de erros 'te' um registro contendo o código do erro
/* 'codigo' (previamente informado em uma tabela de erros) e a linha 
/* 'lin' onde foi encontrado esse erro.*/

void adiciona_erro(struct tabela_erros *te,int codigo,int linha)
{
    int i1,j1;
    // i1,j1: contadores

    if (te->numero_de_erros >= MAX_ERROS)
    {
        te->numero_de_erros++;
    }
    else
    {
        i1=local_insert_erro(*te,linha);
	    if (i1>=te->numero_de_erros)
	    {
	        te->erros[i1].cod=codigo;
	        te->erros[i1].linha=linha;
	    }
	    else
	    {
	        for(j1=te->numero_de_erros; j1>i1; j1--)
		    {
                te->erros[j1].linha=te->erros[j1-1].linha;
		        te->erros[j1].cod=te->erros[j1-1].cod;
		    }
	        te->erros[i1].linha=linha;
	        te->erros[i1].cod=codigo;
        }
	    te->numero_de_erros++;
    }
}



/*=================== Função int pesquisar_cod_instrucao =====================*/

/* int pesquisar_cod_instrucao(char line[82],FILE *arq)
/*Compara as operações existentes no arquivo 'cod_op.tab' com o campo 
/* de operação da linha 'line' devolvendo 1 se caso encontrada a 
/* operação.
/* Variáveis importantes:
/* - achou: variável de retorno (0 negativo ou 1 positivo);
/* - tam: tamanho da operação;
/* - pos: posição na linha onde está o campo de operação;
/* - simbolo: variável que guarda a operação do arquivo;
/* - teste: variável que testa se foi possível ler a linha do arquivo.*/

int pesquisar_cod_instrucao(char line[82],FILE *arq)
{
    int achou,tam,i,pos;
    char simbolo[8],*teste;
    
    for(i=0;i<=7;i++)
        simbolo[i]=' ';
    achou=FALSE;
    tam=tamanho_nome_da_macro(line,&pos);
    teste=fgets(simbolo,8,arq);
    while((teste!=NULL)&&(!achou))
    { 
        if(memicmp(&line[pos],simbolo,tam)==0)
	        achou=TRUE;
	    teste=fgets(simbolo,8,arq);
    }

    return achou;
}



/*==================== Função empilhar_chamadas_aninhadas =======================*/

/* int empilhar_chamadas_aninhadas(Pilha_Chamadas_Aninhadas *pn,Tabela_Definicao_De_Macros *df,struct tipo_linha *pl)
/* Função que coloca na pilha de chamadas aninhadas um registro 
/* contendo a atual definição expandida 'df' e a linha 'pl' onde o 
/* processamento deve continuar após a expansão desta macro. 
/* O ponteiro 'pn' e um ponteiro auxiliar para alocação.*/

int empilhar_chamadas_aninhadas(Pilha_Chamadas_Aninhadas *chamada_aninhada,Tabela_Definicao_De_Macros *df,struct tipo_linha *linha)
{
    chamada_aninhada=(Pilha_Chamadas_Aninhadas *)malloc(sizeof(Pilha_Chamadas_Aninhadas));
    if (chamada_aninhada==NULL)  
        return 0;  
    else
    {
        chamada_aninhada->definicao_atual = df;
        chamada_aninhada->linha_anterior = linha;
        if (topo_nest == NULL)
	        chamada_aninhada->proxima_chamada = NULL;
        else
	        chamada_aninhada->proxima_chamada = topo_nest;
        topo_nest = chamada_aninhada;

        return 1;
    }
}



/*=================== Função desempilhar_chamadas_aninhadas =====================*/

/* void desempilhar_chamadas_aninhadas(Pilha_Chamadas_Aninhadas *pn)
/* Ao contrário da função anterior, esse procedimento desempilha 
/* um registro da pilha de chamadas aninhadas. O ponteiro 'pn' é 
/* utilizado para auxiliar na  remoção.                               */ 

void desempilhar_chamadas_aninhadas(Pilha_Chamadas_Aninhadas *chamada_aninhada)
{
    if (topo_nest!=NULL)
    {
        chamada_aninhada=topo_nest;
        topo_nest=topo_nest->proxima_chamada;
	    free(chamada_aninhada);
        chamada_aninhada=NULL;
    }
}



/*==================== Função processador_de_macros ==================*/

/* int macros(char *nome_fonte, struct tabela_erros *tab)
/* Função principal do pré-processador de macros, chamada pelo 
/* integrador.c. Possui como parâmetros de entrada o nome do arquivo de
/* entrada 'nome_fonte' e a tabela de erros 'tab'                    
/* Sucintamente ele opera da seguinte maneira:
/* - Le cada linha do código fonte;
/* - Se houver um campo de operação, analisa se ele é um delimitador 
/* de definição de macro 'MACRO', se é um delimitador de fechamento de
/* definição de macro 'MEND', um protótipo de macro, uma chamada de 
/* macro ou outra operação qualquer.
/* Se caso não houver este campo, simplesmente copia a linha sem 
/*mudanças para o arquivo de saída;
/* - Para cada uma das possibilidades citadas acima, faz o 
/* processamento apropriado;
/* - No final do processamento, a função retorna 1 se operação foi 
/* bem sucedida e o resultado do processamento fica registrado no 
/* arquivo de saida MASMAPRG.ASM.                                     

/*Variáveis importantes:
/*- fonte: arquivo de entrada com nome 'nome_fonte';
/*- dest: arquivo de saída com nome MASMAPRG.ASM;
/*- arq: arquivo que contém a tabela de operações, com nome 'cod_op.tab';
/*- c_linha: conta linhas no texto de entrada;
/*- c_linha2: conta linhas de uma definição de macro (usado quando e>0);
/*- d: contador de nível de definição;
/*- e: contador de nível de expansão;
/*- achou: indica se o opcode foi encontrado na tabela de instruções;
/*- ldef: guarda número da linha onde foi encontrada a instrução "MEND";
/*- l_write: avisa se a linha alterada no modo de expansão deve ser escrita;
/*- ok: variável de retorno;
/*- linha: contém a linha do arquivo fonte;
/*- l_nova: linha resultante do processamento de uma linha do código fonte;
/*ou de uma linha da definição da macro;
/*- teste: testa se foi possível ler a linha do código fonte;
/*- def: guarda a definição da macro pesquisada na tabela de definições;
/*- pfaux,lin,pnc: ponteiros auxiliares para alocação e remoção na 
/* pilha de parâmetros formais, na tabela de definições e na pilha 
/* de chamadas aninhadas respectivamente;                              */

int processador_de_macros(char *nome_fonte, struct tabela_erros *tab)
{	 
    FILE *arquivo_fonte;
    FILE *arquivo_de_saida;
    FILE *tabela_de_operacoes;

    int contador_de_linhas, c_linha2, nivel_definicao, nivel_expansao, i, j, achou, linha_MEND, l_write, other, prototype,
    call, macro, mend, fim, ok;
    char linha[TAM], l_nova[TAM], *teste;
    Tabela_Definicao_De_Macros *def;
    Pilha_Parametros_Formais *pfaux;
    struct tipo_linha *lin;
    Pilha_Chamadas_Aninhadas *chamada_aninhada;

    if(!(teste=(char*)malloc(sizeof(char)*TAM)))
        return FALTA_MEMORIA;





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
  contador_de_linhas = 0;
  nivel_definicao = 0;
  nivel_expansao = 0;
  linha_MEND = -1; //ldef
//*VARIAVEIS GLOBAIS

//ponteiro para inicio da tabela de definicoes de macros
  tab_def_macros = NULL;
// ppf = pilha de parametros formais
  ppf = NULL;
// ppr = pilha de parametros reais
  ppr=(Pilha_Parametros_Reais *)malloc(sizeof(Pilha_Parametros_Reais));
  ppr->topo = NULL;
// topo da pilha de nestcalls
  topo_nest = NULL;
  tab->numero_de_erros = 0;
  for(i=0; i<MAX_ERROS; i++)
    tab->erros[i].linha = -1;

//Inicio da funcao

    if((arquivo_fonte = fopen(nome_fonte, "r")) == NULL)
    {
        printf("ERRO NA ABERTURA DO ARQUIVO FONTE\n");
        return ERRO_NA_LEITURA;//"ERRO NA ABERTURA DO ARQUIVO FONTE
    }

    if((arquivo_de_saida = fopen("MASMAPRG.ASM","w")) == NULL)
    {
        printf("ERRO NA ABERTURA DO ARQUIVO DE SAIDA\n");
        return ERRO_NA_LEITURA;//"ERRO NA ABERTURA DO ARQUIVO DE SAIDA\n"
    }

    for (i=0; i<82; i++)
       linha[i]=' ';
    teste = fgets(linha,82,arquivo_fonte);   // se fgets == NULL => nao conseguiu ler a linha
    contador_de_linhas++;

    if(teste == NULL)
    puts("Teste == NULL");
    if((linha[80] != ' '))
    {
        adiciona_erro(tab, 2, contador_de_linhas); //ERRO: LINHA MUITO LONGA
        ok = 0;	       
    }         

    if((tabela_de_operacoes = fopen("cod_instrucao.txt", "r"))==NULL){
                printf("ERRO NA ABERTURA DA TABELA DE OPERACOES\n");
	            return ERRO_NA_LEITURA;//ERRO NA ABERTURA DA TABELA DE OPERACOES
    }

    ok = 1;         // VARIAVEL DE RETORNO
    while(fim == 0)
    {
        printf("%s",linha);
        system("pause>nul");
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
              achou = pesquisar_cod_instrucao(linha, tabela_de_operacoes);
              rewind(tabela_de_operacoes); //volta ao inicio da tabela de codigo de operação

              if(achou)   //achou codigo de operação na tabela
              {
                  if (contador_de_linhas != linha_MEND+1)
                      other = TRUE;
                  else
                  {
                      adiciona_erro(tab,1,contador_de_linhas);
                      ok = 0;
                      prototype = TRUE;
                  }
              }
              else //não é codigo de operação
                  if ((memicmp(&linha[i], "MACRO",5) == 0)&&(strchr("* \t \n",linha[i+5])))   //Definicao de Macro
                  {
                      if (contador_de_linhas == linha_MEND+1)
                      {
                          adiciona_erro(tab, 1, contador_de_linhas); ///REDEFINICAO DE OPERACAO
                          ok = 0;
                          prototype = TRUE;
                      }
                      else
                          macro=TRUE;
                  }
                  else
                  if ((memicmp(&linha[i], "MEND",4) == 0)&&(strchr("* \t \n",linha[i+4])))    //FIM de definicao de Macro
                  {
                    if (contador_de_linhas==linha_MEND+1)
                    {
                      adiciona_erro(tab, 1, contador_de_linhas); //ERRO:REDEFINICAO DE OPERACAO
                      ok = 0;
                      prototype = TRUE;
                    }
                    else
                      mend = TRUE;
                  }
                  else
                  if((contador_de_linhas == linha_MEND+1)||(c_linha2 == linha_MEND+1))
                    prototype = TRUE;
                  else
                  {
                    def = pesquisa_tab_def(tab_def_macros, linha, i);
                    if (def != NULL)
                    {
                      printf("def=%s",def->prototipo);
                      call = TRUE;
                      if(nivel_definicao == 0)
                      {
                        if(nivel_expansao > 0)  //chamada aninhada
                          empilhar_chamadas_aninhadas(chamada_aninhada, def, lin);  //guarda proxima linha
                        else
                          empilhar_chamadas_aninhadas(chamada_aninhada,def,NULL);
                        lin=NULL;
                        c_linha2=0;
                      }
                    }
                    else
                    {
                      if(nivel_definicao == 0)
                        adiciona_erro(tab, 3, contador_de_linhas);  //ERRO: CHAMADA DE MACRO NAO DEFINIDA
                      other = TRUE;
                    }
                  }
            }
            else
              other = TRUE;

            //comeca o processamento propriamente dito
		      if (macro)    //Definicao de Macro
		      {
              nivel_definicao++;
              if(nivel_expansao > 0)
                linha_MEND = c_linha2;
              else
                linha_MEND = contador_de_linhas;
              if(nivel_definicao == 1)
                alocar_tab_def_macros(tab_def_macros);
              else
                inserir_linha(linha, tab_def_macros);
            }
            else
              if(prototype)   //Prototipo de Macro
              {
                if(nivel_expansao == 0)
                  empilha_parametros(ppf, linha, nivel_definicao);               
                if (nivel_definicao > 0)
                  inserir_linha(linha, tab_def_macros);
              }
	           else
                if(call)
                {
                  if(nivel_definicao == 0)
                  {
                    nivel_expansao++;
                    empilha_lista_parametros_reais(ppr, linha);
                  }
                  if ((nivel_definicao > 0)&&(nivel_expansao == 0))
                    strcpy(l_nova,substitui_parametro_formal(ppf, linha));
                  if (nivel_definicao > 0)
                    inserir_linha(l_nova, tab_def_macros);
                }
                else
                  if(mend)    //Fim de Macro
                  {
                    if(nivel_definicao == 0)
                    {
                      desempilha_lista_parametros_reais(ppr);
                      nivel_expansao--;
                      lin = topo_nest->linha_anterior;
                      desempilhar_chamadas_aninhadas(chamada_aninhada);
                    }
                    else
                    {
                      if(nivel_expansao == 0)
                        desempilha_parametros(pfaux, nivel_definicao);
                      nivel_definicao--;
                      inserir_linha(linha, tab_def_macros);
                    }
                  }
				  else
                    if(other)
                    {
                      if((nivel_expansao == 0)&&(nivel_definicao > 0))
                        strcpy(l_nova,substitui_parametro_formal(ppf, linha));
                      if(nivel_definicao == 0)
                        fputs (linha, arquivo_de_saida);  //escreve na saida a linha atual
                      else
                       if(l_write)
                         inserir_linha(linha, tab_def_macros);
                       else
                         inserir_linha(l_nova, tab_def_macros);
                    }
            //se opcode = END e não há nada depois  
            if((memicmp(&linha[i], "END", 3) == 0)&&(strchr("\t \n",linha[i+3])))
            {
              if(nivel_definicao > 0)
                adiciona_erro(tab, 4, contador_de_linhas); //ERRO: MEND esperado
              fim = 1;
            }
            l_write = FALSE;   //avisa se a proxima linha deve ser escrita
            if(nivel_expansao > 0)  //MODO DE EXPANSAO
            {
              if(lin == NULL)
                lin = topo_nest->definicao_atual->linha; //aponta para a primeira linha da def atual            }
              else
                lin = lin->proxima_linha;
              //printf("%s",lin);
              //getch();
              c_linha2++;
              for(j=0; j<=80; j++)
                linha[j]=lin->linha[j];
              trocar_par_ordenado_por_parametro_real(linha, ppr, l_nova);
              for(j=0; j<=80; j++)
                linha[j]=l_nova[j];
              if (nivel_definicao > 0)
                l_write = TRUE;
            }
            else
            {
              for (i=0; i<82; i++)
                linha[i]=' ';
              teste = fgets(linha, 82, arquivo_fonte);  // fgets == NULL => nao conseguiu ler a linha
              contador_de_linhas++;
              if(teste != NULL)
              {
                if(linha[80] != ' ')
                {
                  adiciona_erro(tab, 2, contador_de_linhas);
                  ok = 0;                
                }
              }
              else
                fim = 1;
            }
          } //fim while


    desalocar_tab_def_macros(def);
    fclose(arquivo_fonte);
    fclose(arquivo_de_saida);
    fclose(tabela_de_operacoes);

    return 100;
}

