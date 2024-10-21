#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
   double sequencial;
   char event_time[50];
   char event_type[50];
   char product_id[50];
   char category_id[50];
   char category_code[50];
   char brand[50];
   float price;
   char user_id[50];
   char user_session[50];
   int ativo; 
} Registro;
typedef struct {
   double sequencial;
   char product_id[50];
   char brand[50];
   float price;
   int ativo;
} Produto;
typedef struct {
   double sequencial;
   char user_id[50];
   char event_type[50];
   char event_time[50];
   int ativo;
} Acesso;
typedef struct {
    double chave; 
    long file_offset;  
} Indice;

void mostrar_produtos(FILE *arquivo_produtos) {
    Produto produto;
    fseek(arquivo_produtos, 0, SEEK_SET);

    printf("Produtos gravados:\n");
    printf("----------------------------------------------------------------------------\n");
    printf("| Sequencial | Brand           | Product ID    | Price |\n");
    printf("----------------------------------------------------------------------------\n");
	int i = 0;
    while (fread(&produto, sizeof(Produto), 1, arquivo_produtos) && i<50) {
        printf("| %.0f | %s | %s | %.2f \n", produto.sequencial, produto.brand, produto.product_id, produto.price);
        i++;
    }
    printf("----------------------------------------------------------------------------\n");
}

void mostrar_acessos(FILE *arquivo_acessos) {
    Acesso acesso;
    fseek(arquivo_acessos, 0, SEEK_SET);
	int i=0;
    printf("Acessos gravados:\n");
    printf("----------------------------------------------------------------------------\n");
    printf("| Sequencial | Event Time           | User ID    | Event Type |\n");
    printf("----------------------------------------------------------------------------\n");

    while (fread(&acesso, sizeof(Acesso), 1, arquivo_acessos) && i<50) {
        printf("| %.0f | %s | %s | %s \n", acesso.sequencial, acesso.event_time, acesso.user_id, acesso.event_type);
        i++;
    }
    printf("----------------------------------------------------------------------------\n");
}

int pesquisa_binaria (FILE *arquivo_indice, double chave, Indice *indice_encontrado){
	fseek(arquivo_indice, 0, SEEK_END);
    long tamanho_indice = ftell(arquivo_indice) / sizeof(Indice); 
    long inicio = 0;
    long fim = tamanho_indice - 1;

    while (inicio <= fim) {
        long meio = (inicio + fim) / 2;
        fseek(arquivo_indice, meio * sizeof(Indice), SEEK_SET);
        Indice indice_atual;
        fread(&indice_atual, sizeof(Indice), 1, arquivo_indice);

        if (indice_atual.chave == chave) {
            *indice_encontrado = indice_atual; 
            return 1;
        } else if (indice_atual.chave < chave) {
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
    }
    return 0; 
}

Acesso buscar_acesso (FILE *arquivo_dados, FILE *arquivo_indice, double chave) {
    Indice indice_encontrado;

    if (pesquisa_binaria(arquivo_indice, chave, &indice_encontrado)) {
        fseek(arquivo_dados, indice_encontrado.file_offset, SEEK_SET); 
        Acesso registro;
        fread(&registro, sizeof(Acesso), 1, arquivo_dados); 
        
        printf("Registro encontrado: %.0f | %s | %s | %s \n", registro.sequencial, registro.event_time, registro.user_id, registro.event_type); 
        return registro;
    }

    Acesso registro_vazio = {-1, "", "",""};
    printf("Registro com chave %.0f nao encontrado.\n", chave);
    return registro_vazio;
}

Produto buscar_produto (FILE *arquivo_dados, FILE *arquivo_indice, double chave) {
    Indice indice_encontrado;

    if (pesquisa_binaria(arquivo_indice, chave, &indice_encontrado)) {
        fseek(arquivo_dados, indice_encontrado.file_offset, SEEK_SET); 
        Produto registro;
        fread(&registro, sizeof(Produto), 1, arquivo_dados); 
               
        printf("Registro encontrado: %.0f | %s | %s | %.2f \n", registro.sequencial, registro.brand, registro.product_id, registro.price); // Exibe o registro encontrado
        return registro;
    }

    Produto registro_vazio = {-1, "", "", 0};
    printf("Registro com chave %.0f nao encontrado.\n", chave);
    return registro_vazio;
}
void inserir_novo_acesso(FILE *arquivo_dados, FILE *arquivo_indice, Registro novo_registro) {
    Acesso acesso;
    fseek(arquivo_dados, 0, SEEK_SET);
    
    while (fread(&acesso, sizeof(Acesso), 1, arquivo_dados)) {
    	if (acesso.ativo==0){
    		strcpy(acesso.event_time, novo_registro.event_time);
    		strcpy(acesso.event_type, novo_registro.event_type);
    		strcpy(acesso.user_id, novo_registro.user_id);
    		acesso.ativo=1;
    		fseek(arquivo_dados, -sizeof(Acesso), SEEK_CUR);
        	fwrite(&acesso, sizeof(Acesso), 1, arquivo_dados);
    		return;
		}
    }
    
    long offset = ftell(arquivo_dados);
    
    acesso.sequencial = offset / sizeof(Acesso) + 1;  
    acesso.ativo = 1;  
    strcpy(acesso.event_time, novo_registro.event_time);
    strcpy(acesso.event_type, novo_registro.event_type);
    strcpy(acesso.user_id, novo_registro.user_id);

    fwrite(&acesso, sizeof(Acesso), 1, arquivo_dados);

    Indice novo_indice;
    novo_indice.chave = acesso.sequencial;  
    novo_indice.file_offset = offset;  
    fwrite(&novo_indice, sizeof(Indice), 1, arquivo_indice);
}

void inserir_novo_produto(FILE *arquivo_dados, FILE *arquivo_indice, Registro novo_registro) {
    Produto produto;
    fseek(arquivo_dados, 0, SEEK_SET);
    
    while (fread(&produto, sizeof(Produto), 1, arquivo_dados)) {
    	if (produto.ativo==0){
    		strcpy(produto.brand, novo_registro.brand);
    		strcpy(produto.product_id, novo_registro.product_id);
    		produto.price=novo_registro.price;
    		produto.ativo=1;
    		fseek(arquivo_dados, -sizeof(Produto), SEEK_CUR);
        	fwrite(&produto, sizeof(Produto), 1, arquivo_dados);
    		return;
		}
    }
    
    long offset = ftell(arquivo_dados);
    
    produto.sequencial = offset / sizeof(Produto) + 1;  
    produto.ativo = 1;  // Marca como ativo
    strcpy(produto.brand, novo_registro.brand);
    strcpy(produto.product_id, novo_registro.product_id);
    produto.price=novo_registro.price;

    fwrite(&produto, sizeof(Produto), 1, arquivo_dados);

    Indice novo_indice;
    novo_indice.chave = produto.sequencial;  
    novo_indice.file_offset = offset;  
    fwrite(&novo_indice, sizeof(Indice), 1, arquivo_indice);
}


void remover_acesso (FILE *arquivo_dados, FILE *arquivo_indice, double chave) {
    Indice indice_encontrado;

    if (pesquisa_binaria(arquivo_indice, chave, &indice_encontrado)) {
        fseek(arquivo_dados, indice_encontrado.file_offset, SEEK_SET);
        Acesso registro;
        fread(&registro, sizeof(Acesso), 1, arquivo_dados);

        registro.ativo = 0;
        strcpy(registro.event_time, "--------");
        strcpy(registro.event_type, "--------");
        strcpy(registro.user_id, "--------");

        fseek(arquivo_dados, indice_encontrado.file_offset, SEEK_SET);
        fwrite(&registro, sizeof(Acesso), 1, arquivo_dados);

        printf("Registro com chave %.0f removido.\n", chave);
    } else {
        printf("Registro com chave %.0f nao encontrado.\n", chave);
    }
}

void remover_produto(FILE *arquivo_dados, FILE *arquivo_indice, double chave) {
    Indice indice_encontrado;

    if (pesquisa_binaria(arquivo_indice, chave, &indice_encontrado)) {
        fseek(arquivo_dados, indice_encontrado.file_offset, SEEK_SET);
        Produto registro;
        fread(&registro, sizeof(Produto), 1, arquivo_dados);

        registro.ativo = 0;
        strcpy(registro.brand, "--------");
        strcpy(registro.product_id, "--------");
        registro.price=0;
        
        fseek(arquivo_dados, indice_encontrado.file_offset, SEEK_SET);
        fwrite(&registro, sizeof(Produto), 1, arquivo_dados);

        printf("Registro com chave %.0f removido.\n", chave);
    } else {
        printf("Registro com chave %.0f nao encontrado.\n", chave);
    }
}

int ler_e_inserir_dados(const char *arquivo_csv, FILE *arquivo_produtos, FILE *arquivo_acessos, FILE *arquivo_indice_produtos, FILE *arquivo_indice_acessos) {
    FILE *arquivo_csv_f = fopen(arquivo_csv, "r");
    if (arquivo_csv_f == NULL) {
        printf("Erro ao abrir o arquivo CSV.\n");
        return 0;
    }

    char linha[1024];
	Registro registro_lido;
	registro_lido.sequencial = 1;
    
    if (fgets(linha, sizeof(linha), arquivo_csv_f) == NULL) {
        printf("Erro ao ler o cabecalho ou o arquivo esta vazio.\n");
        fclose(arquivo_csv_f);
        return EXIT_FAILURE;
    }
    int i = 0;
    Produto produto_lido;
    Acesso acesso_lido;
    Indice indice;
    indice.chave=1;
    while (fgets(linha, sizeof(linha), arquivo_csv_f) != NULL && i<50) {

        linha[strcspn(linha, "\n")] = '\0';

        char brand[50] = "N/A";
		char category_code[50] = "N/A"; 
        sscanf(linha, "%49[^,],%49[^,],%49[^,],%49[^,],%49[^,],%49[^,],%f,%49[^,],%49[^,]", registro_lido.event_time, registro_lido.event_type, registro_lido.product_id, registro_lido.category_id, category_code, brand, &registro_lido.price, registro_lido.user_id, registro_lido.user_session);	
		
		strcpy(produto_lido.brand, brand);
		produto_lido.price=registro_lido.price;
		strcpy(produto_lido.product_id, registro_lido.product_id);
		produto_lido.sequencial=registro_lido.sequencial;
		produto_lido.ativo=1;
		
        long offset_produto = ftell(arquivo_produtos);  
        fwrite(&produto_lido, sizeof(Produto), 1, arquivo_produtos);

        indice.chave = registro_lido.sequencial;   
        indice.file_offset = offset_produto;        
        fwrite(&indice, sizeof(Indice), 1, arquivo_indice_produtos);
		
		strcpy(acesso_lido.event_time, registro_lido.event_time);
		strcpy(acesso_lido.event_type, registro_lido.event_type);
		acesso_lido.sequencial=registro_lido.sequencial;
		strcpy(acesso_lido.user_id, registro_lido.user_id);
		acesso_lido.ativo=1;
		
		// Grava o registro no arquivo de acessos
        long offset_acesso = ftell(arquivo_acessos); 
        fwrite(&acesso_lido, sizeof(Acesso), 1, arquivo_acessos);

        indice.chave = registro_lido.sequencial;  
        indice.file_offset = offset_acesso;        
        fwrite(&indice, sizeof(Indice), 1, arquivo_indice_acessos);
        
    	registro_lido.sequencial++;
    	i++;
	}
    fclose(arquivo_csv_f);
    return 1;
}
void encontrar_marca_mais_frequente(FILE *arquivo_produtos){
    Produto produto;
    int contador[100] = {0}; 
    char marcas[100][50];     
    int total_marcas = 0;
    fseek(arquivo_produtos, 0, SEEK_SET);
    
    while(fread(&produto, sizeof(Produto), 1, arquivo_produtos)){
        if (produto.ativo) {
            int encontrado = 0;
            for(int i = 0; i < total_marcas; i++){
                if(strcmp(marcas[i], produto.brand) == 0 && strcmp("N/A", produto.brand) != 0){
                    contador[i]++;
                    encontrado = 1;
                    break;
                }
            }
            if(!encontrado && total_marcas < 100){ //100 marcas
                strcpy(marcas[total_marcas], produto.brand);
                contador[total_marcas]= 1;
                total_marcas++;
            }
        }
    }
    int max_ocorrencias = 0;
    char marca_mais_frequente[50] = "";

    for (int i = 0; i < total_marcas; i++) {
        if (contador[i] > max_ocorrencias) {
            max_ocorrencias = contador[i];
            strcpy(marca_mais_frequente, marcas[i]);
        }
    }
    if (max_ocorrencias > 0) {
        printf("Marca com mais produtos: %s (Total: %d)\n", marca_mais_frequente, max_ocorrencias);
    } else {
        printf("Nenhuma marca encontrada.\n");
    }
}

void encontrar_produto_maior_valor(FILE *arquivo_produtos){
    Produto produto;
    float max_preco = -1.0;
    Produto produto_maximo;
    int encontrado = 0;
    fseek(arquivo_produtos, 0, SEEK_SET);
    
    while(fread(&produto, sizeof(Produto), 1, arquivo_produtos)){
        if(produto.ativo && produto.price > max_preco){
            max_preco =produto.price;
            produto_maximo = produto;
            encontrado = 1;
        }
    }

    if(encontrado){
        printf("Produto com maior valor: %s | valor: %.2f | Sequencial: %.0f\n", produto_maximo.product_id, produto_maximo.price, produto_maximo.sequencial);
    }else{
        printf("Nenhum produto encontrado.\n");
    }
}

int main() {
    FILE *arquivo_produtos, *arquivo_acessos;
    FILE *arquivo_indice_produtos, *arquivo_indice_acessos;

    arquivo_produtos = fopen("produtos.dat", "wb+");
    arquivo_indice_produtos = fopen("indice_produtos.dat", "wb+");
    arquivo_acessos = fopen("acessos.dat", "wb+");
    arquivo_indice_acessos = fopen("indice_acessos.dat", "wb+");


    if (arquivo_produtos == NULL || arquivo_acessos == NULL || arquivo_indice_produtos == NULL || arquivo_indice_acessos == NULL) {
        printf("Erro ao abrir arquivos.\n");
        return 1;
    }

    if (ler_e_inserir_dados("products.csv", arquivo_produtos, arquivo_acessos, arquivo_indice_produtos, arquivo_indice_acessos) == 0){
    	return 1;
	};
	

    mostrar_produtos(arquivo_produtos);
	mostrar_acessos(arquivo_acessos);
    //buscar_acesso(arquivo_acessos, arquivo_indice_acessos, 1);
    //buscar_produto(arquivo_produtos, arquivo_indice_produtos, 2);
    
    encontrar_marca_mais_frequente(arquivo_produtos);

	Registro novo_registro = {
    0,  // sequencial (pode ser 0, pois será gerado)
    "2024-10-19 12:00:00",  //event_time
    "view",  //event_type
    "123456",  //product_id
    "cat01",  //category_id
    "catCode",  //category_code
    "BrandX",  //brand
    99.99,  //price
    "user01",  //user_id
    "session01",  //user_session
    1  //ativo
	};

    //buscar_acesso(arquivo_acessos, arquivo_indice_acessos, 50); 
    buscar_produto(arquivo_produtos, arquivo_indice_produtos, 25); 

    //inserir_novo_acesso(arquivo_acessos, arquivo_indice_acessos, novo_registro);
    inserir_novo_produto(arquivo_produtos, arquivo_indice_produtos, novo_registro);

    //remover_acesso(arquivo_acessos, arquivo_indice_acessos, 50);
    remover_produto(arquivo_produtos, arquivo_indice_produtos, 25);
	
	//buscar_acesso(arquivo_acessos, arquivo_indice_acessos, 50); 
    buscar_produto(arquivo_produtos, arquivo_indice_produtos, 25); 

    encontrar_produto_maior_valor(arquivo_produtos);
	
    fclose(arquivo_produtos);
    fclose(arquivo_indice_produtos);
    fclose(arquivo_acessos);
    fclose(arquivo_indice_acessos);

    return 0;
}
