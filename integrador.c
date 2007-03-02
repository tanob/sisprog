/**********************************************************************************************
UNIVERSIDADE FEDERAL DE PELOTAS
BACHARELADO EM CIÊNCIA DA COMPUTAÇÃO
PROJETOS EM COMPUTAÇÃO
Processador de Macros, Integração Macro Montador (Versão preliminar)

Componentes:
Giales Fischer
Pablo Barcellos
Mauro Brito
***********************************************************************************************/



#include "Macros.c"
//#include "montador.c"
#include <stdio.h>
#include <conio.h>

void escreve_erros(struct tabela_erros tab,FILE *listagem);

struct tabela_erros t;
FILE *arq_erros;
FILE *list;

int main(int argc, char **argv)
{
  int status=0, i;
  char nome_f[250], arquivo_listagem[250],nome_monta[14];
  char *nome_erros="ERROS.TAB";

//  clrscr();
 
  if (argc!=2)
  {
    printf("Falta ou excesso de parâmetros.\nSintaxe correta: <nome deste programa> <nome do arquivo fonte>");
    getch();
    exit(1);
  }
  else
    strcpy(nome_f,argv[1]); 

  status = macros(nome_f, &t);  
  if (status)
  {
    printf("Processador de Macros concluido com exito");
    getch();
	 status=1;
  }
 
  else
  {
	 printf("Processador de Macros concluido com erro");
    getch();
    arq_erros=fopen(nome_erros,"r");
    if (arq_erros!=NULL)
    { 
      escreve_erros(t,list);
    }
    fclose(arq_erros);
  }
  //status=1;
  /*if (status)
  {
    status=0;
    strcpy(nome_monta,"MASMAPRG.ASM");
    status=montador(nome_f);
    if (status)
    {
      printf("Montador concluido com exito");
      for(i = 0; i < strlen(nome_f) && nome_f[i] != '.'; i++);
      nome_f[i] = '\0';   
      strcpy(arquivo_listagem, nome_f);     // Gera o nome dos arquivos objeto e de listagem
      strcat(nome_f, ".obj");               //
      strcat(arquivo_listagem, ".lst");       //
      rename("MASMAPRG.OBJ", nome_f);
      rename("MASMAPRG.LST", arquivo_listagem);
    }
  }
  else
    printf("Montador concluido com erro");

  getch();*/
}


void escreve_erros(struct tabela_erros tab,FILE *listagem)
{
  char *teste,*muito_erro,linha[80],msg[80];
  int i,j,k,num,achou;

  muito_erro="  ";

  if(tab.n_erros>=MAX_ERROS)
    muito_erro="EXISTEM MUITOS ERROS\n";
  else
  {
    listagem=fopen("listagem.lst","w");
    if (listagem!=NULL)
    {
	   for(j=0;j<=tab.n_erros;j++)
	   {
	     teste=fgets(linha,80,arq_erros);
	     achou=FALSE;
	     while((teste!=NULL)&&(!achou))
	     {
	       num=(linha[0]-'0')*10 + (linha[1]-'0');
	       if(num==tab.erros[j].cod)
		      achou=TRUE;
		    if(achou)
		    {
		      k=0;
		      for(i=2;linha[i]!='\n';i++)
			   {
			     msg[k]=linha[i];
			     k++;
			   }
		      msg[k]='\n';
		      msg[k+1]=0;
		      fprintf(listagem,"Linha %d: %s",tab.erros[j].line,msg);
		    }
		    teste=fgets(linha,80,arq_erros);
		  } //fim da procura
	     rewind(arq_erros);

      } //fim da varredura na tabela de erros (la‡o for)

    }
  } //fim else

  if (strcmp(muito_erro,"  ")!=0)
    fputs(muito_erro,listagem);

  fclose(listagem);
}
