#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define tamanho_cromossomo 500
#define qtde_variaveis 3
// ===============================================================================================
//estruturas

typedef struct Item{
    float variaveis[qtde_variaveis]; //peso, utilidade, preco
} Item;

typedef struct Individuo{
    int cromossomo[tamanho_cromossomo];
    float variaveis[qtde_variaveis]; // peso, utilidade, preco
    int grau_de_dominancia;
} Individuo;

// ===============================================================================================
//funções genéricas

int gerarAleatorio(int min, int max){
    int num = (rand()% (max-min)) + min;
    return num;
}

int lerArquivo(Item *lista_de_itens){
    FILE *arq;
    char buffer[255];
    char *token;

    int indice;

    arq = fopen("ProblemaDaMochila.csv", "r");
    if( arq == NULL ) {
        printf("Arquivo nao encontrado!\n");
        return -1;
    }    

    fgets(buffer, 255, arq);

    while (fgets(buffer, 255, arq)){
        token = strtok(buffer, ";");

        indice = atof(token);
        indice--;
        token = strtok(NULL, ";");
        for (int i = 0; i < qtde_variaveis; i++){
            lista_de_itens[indice].variaveis[i] = atof(token);
            token = strtok(NULL, ";");
        }
    }    
    fclose(arq);
}

void trocarIndividuos(Individuo *x1, Individuo *x2){
    Individuo temp = *x1;
    *x1 = *x2;
    *x2 = temp;
}

void trocarFloats(float *x1, float *x2){
    float temp = *x1;
    *x1 = *x2;
    *x2 = temp;
}

int buscarIndividuoMenorDominancia(Individuo *populacao, int posicao, int tamanho_populacao){
    int posicao_melhor_individuo = posicao;
    for (int i = posicao + 1; i < tamanho_populacao; i++){
        if (populacao[i].grau_de_dominancia < populacao[posicao_melhor_individuo].grau_de_dominancia){
            posicao_melhor_individuo = i;
        }
    }
    return posicao_melhor_individuo;
}

int buscarIndividuoMenorVariavel(Individuo *populacao, int posicao, int final_da_frente, int variavel){
    int posicao_melhor_individuo = posicao;
    for (int i = posicao + 1; i < final_da_frente; i++){
        if (populacao[i].variaveis[variavel] < populacao[posicao_melhor_individuo].variaveis[variavel]){
            posicao_melhor_individuo = i;
        }
    }
    return posicao_melhor_individuo;
}

int buscarIndividuoMaiorVariavel(Individuo *populacao, int posicao, int final_da_frente, int variavel){
    int posicao_melhor_individuo = posicao;
    for (int i = posicao + 1; i < final_da_frente; i++){
        if (populacao[i].variaveis[variavel] > populacao[posicao_melhor_individuo].variaveis[variavel]){
            posicao_melhor_individuo = i;
        }
    }
    return posicao_melhor_individuo;
}

// selection sort é mais modular agora, ponteiro para funcao como argumento para escolher por qual atributo será ordenado a população
// argumentos inicio e fim decidem a parte do vetor a ser ordenado, são usados para calculo de distancia
void selectionSort(Individuo *populacao, int inicio, int tamanho_populacao, int (*tipoDeOrdenacao) (Individuo*, int, int)){
    int posicao_melhor_individuo;
    for (int i = inicio; i < tamanho_populacao; i++){
        posicao_melhor_individuo = tipoDeOrdenacao(populacao, i, tamanho_populacao);
        trocarIndividuos(&(populacao[posicao_melhor_individuo]), &(populacao[i]));
    }
}

int buscarMaiorNumeroVetorFloat(int inicio, int tamanho_populacao, float *vetor){
    int posicao_melhor_individuo = inicio;
    for (int i = inicio; i < tamanho_populacao; i++){
        if (vetor[i] > vetor[posicao_melhor_individuo]){
            posicao_melhor_individuo = i;
        }   
    }
    return posicao_melhor_individuo;
}

int buscarMenorNumeroVetorFloat(int inicio, int tamanho_populacao, float *vetor){
    int posicao_melhor_individuo = inicio;
    for (int i = inicio; i < tamanho_populacao; i++){
        if (vetor[i] < vetor[posicao_melhor_individuo]){
            posicao_melhor_individuo = i;
        }   
    }
    return posicao_melhor_individuo;
}

//ordenacao usando valores do vetor float para ordenar a populacao
void ordenarComBaseNoVetorFloat(Individuo *populacao, int inicio, int tamanho_populacao, float *vetor, int (*tipoDeOrdenacao) (int, int, float*)){
    int posicao_melhor_individuo, posicao_vetor;
    float temp;
    for (int i = inicio; i < tamanho_populacao; i++){
        posicao_vetor = tipoDeOrdenacao(i - inicio, tamanho_populacao - inicio, vetor);
        posicao_melhor_individuo = inicio + posicao_vetor;
        //ordena vetor indivíduos
        trocarIndividuos(&(populacao[posicao_melhor_individuo]), &(populacao[i]));
        //ordenar vetor float
        trocarFloats(&(vetor[i-inicio]), &(vetor[posicao_melhor_individuo - inicio]));
    }
}

// ordena a populacao e o vetor float de acordo com os indices da populacao
void ordenarVetorFloatJuntoComPopulacao(Individuo *populacao, int inicio, int tamanho_populacao, float *vetor, int variavel, int (*tipoDeOrdenacao) (Individuo*, int, int, int)){
    int posicao_melhor_individuo;
    float temp;
    for (int i = inicio; i < tamanho_populacao; i++){
        posicao_melhor_individuo = tipoDeOrdenacao(populacao, i, tamanho_populacao, variavel);
        //ordena vetor indivíduos
        trocarIndividuos(&(populacao[posicao_melhor_individuo]), &(populacao[i]));
        //ordenar vetor float
        trocarFloats(&(vetor[i-inicio]), &(vetor[posicao_melhor_individuo - inicio]));
    }
}

// ===============================================================================================
//funções envolvendo o indivíduo

void printarIndividuo(Individuo individuo){
    printf("%.2f, %.0f, %.2f, %d\n", individuo.variaveis[0], individuo.variaveis[1], individuo.variaveis[2], individuo.grau_de_dominancia);
}

void gerarIndividuoAleatorio(Individuo *individuo, Item *lista_de_itens){
    int aleatorio;
    individuo->variaveis[0] = 0; individuo->variaveis[1] = 0; individuo->variaveis[2] = 0;

    for (int i = 0; i < tamanho_cromossomo; i++){
        aleatorio = gerarAleatorio(5, 30);
        if (gerarAleatorio(0, 1000) < aleatorio){
            individuo->cromossomo[i] = 1;
            for (int i = 0; i < qtde_variaveis; i++){
                individuo->variaveis[i] += lista_de_itens[i].variaveis[i];
            }
        } else {
            individuo->cromossomo[i] = 0;
        }
    }
}

void calcularFuncoes(Individuo *individuo, Item *lista_de_itens){

    individuo->variaveis[2] = 0; individuo->variaveis[1] = 0; individuo->variaveis[0] = 0;

    for (int i = 0; i < tamanho_cromossomo; i++){
        if (individuo->cromossomo[i]){
            for (int i = 0; i < qtde_variaveis; i++){
                individuo->variaveis[i] += lista_de_itens[i].variaveis[i];
            }
        }
    }
}

void mutacao(Individuo *filho, int taxa_mutacao){
    for (int i = 0; i < tamanho_cromossomo; i++){
        if(gerarAleatorio(0,1000) < taxa_mutacao){
            if (filho->cromossomo[i]){
                filho->cromossomo[i] = 0;
            } else {
                filho->cromossomo[i] = 1;
            }
        }
    }
}

// ===============================================================================================
// funções envolvendo população

void gerarPopulacaoAleatoria(Individuo *populacao,int inicio, int final, Item *lista_de_itens){
    for (int i = inicio; i < final; i++){
        gerarIndividuoAleatorio(&populacao[i], lista_de_itens);
    }
}

int verificarVariaveisIguais(Individuo individuo1, Individuo individuo2){
    for (int i = 0; i < qtde_variaveis; i++){
        if (individuo1.variaveis[i] != individuo2.variaveis[i]){
            return 0;
        }
    }
    return 1;
}

int compararMinOrMax(Individuo individuo1, Individuo individuo2, int min_or_max, int variavel){

    if (min_or_max){
        if (individuo1.variaveis[variavel] > individuo2.variaveis[variavel]){
            return 1;
        } else if (individuo2.variaveis[variavel] > individuo1.variaveis[variavel]){
            return -1;
        }
    } else {
        if (individuo1.variaveis[variavel] < individuo2.variaveis[variavel]){
            return 1;
        } else if (individuo2.variaveis[variavel] < individuo1.variaveis[variavel]){
            return -1;
        }
    }
    return 0;
}

int verificarDominanciaEntreIndividuos(Individuo individuo1, Individuo individuo2, int *min_or_max){
    int i = 0;
    int dominante;

    do{
        dominante = compararMinOrMax(individuo1, individuo2, min_or_max[i], i);
        i++;
    } while (!dominante); 

    int valor;

    for (i; i < qtde_variaveis; i++){
        valor = compararMinOrMax(individuo1, individuo2, min_or_max[i], i);
        if (valor != 0 && valor != dominante){
            return 0;
        }
    }
    return dominante;
}

void calcularGrauDeDominanciaDosIndividuos(Individuo *populacao, int tamanho_populacao, int *min_or_max){
    //zerar os graus de dominancia para realizar o novo calculo
    for (int i = 0; i < tamanho_populacao; i++){
        populacao[i].grau_de_dominancia = 0;
    }

    int dominancia;

    for (int i = 0; i < tamanho_populacao; i++){
        if (populacao[i].variaveis[0] < 30){
            for (int j = i + 1; j < tamanho_populacao; j++){
                if (populacao[j].variaveis[0] < 30){
                    dominancia = verificarDominanciaEntreIndividuos(populacao[i], populacao[j], min_or_max);
                    if (dominancia == 1){
                        populacao[j].grau_de_dominancia++;
                    } else if (dominancia == -1){
                        populacao[i].grau_de_dominancia++;
                    }
                } else {
                    populacao[j].grau_de_dominancia = tamanho_populacao;
                }
            }
        } else {
            populacao[i].grau_de_dominancia = tamanho_populacao;
        }
    }
    selectionSort(populacao, 0, tamanho_populacao, buscarIndividuoMenorDominancia);
}

Individuo *alocarPopulacao(int tamanho){
    Individuo *populacao = (Individuo*)malloc(tamanho*sizeof(Individuo)); 
    return populacao;
}

void mostrarPopulacao(Individuo *populacao, int tamanho_populacao){
    printf("Utilidade Preco Peso Dominancia\n");
    for (int i = 0; i < tamanho_populacao; i++){
        printarIndividuo(populacao[i]);
    }
}

int verificarIgualdadeGenetica(Individuo individuo1,Individuo individuo2){
    for (int i = 0; i < tamanho_cromossomo; i++){
        if (individuo1.cromossomo[i] != individuo2.cromossomo[i]){
            return 0;
        }
    }
    return 1;
}

void zerarIndividuosRepetidos(Individuo *populacao, int tamanho_populacao){
    for (int i = 0; i < tamanho_populacao; i++){
        if (populacao[i].variaveis[0] < 30){
            for (int j = i + 1; j < tamanho_populacao; j++){
                if (verificarVariaveisIguais(populacao[i], populacao[j])){
                    if (verificarIgualdadeGenetica(populacao[i], populacao[j]))
                        populacao[j].variaveis[0] = 31;
                }                                  
            }
        }
    }
}

// ===============================================================================================
// Funções de seleção

int selecaoPorTorneio(Individuo *pais, int tamanho_pais){
    int pai1 = gerarAleatorio(0, tamanho_pais);
    int pai2 = gerarAleatorio(0, tamanho_pais);

    if (pais[pai1].grau_de_dominancia < pais[pai2].grau_de_dominancia){
        return pai1;
    }
    return pai2;
}

// ===============================================================================================
//funções de cruzamento

void crossoverUniforme(Individuo pai1, Individuo pai2, Individuo *filho1, Individuo *filho2){
    for (int i = 0; i < tamanho_cromossomo; i++){
        if (gerarAleatorio(0,2)){
            filho1->cromossomo[i] = pai1.cromossomo[i];
            filho2->cromossomo[i] = pai2.cromossomo[i];
        } else {
            filho1->cromossomo[i] = pai2.cromossomo[i];
            filho2->cromossomo[i] = pai1.cromossomo[i];
        }
    }
}

// ===============================================================================================
// funções de tipo de população

void estrategiaPhiLambda(Individuo *populacao, int tamanho_populacao, int *min_or_max){

    int tamanho_pais_fihos = 2 * tamanho_populacao;
    
    int j;

    zerarIndividuosRepetidos(populacao, tamanho_pais_fihos); // repetidos serão dominados por todos indivíduos
    calcularGrauDeDominanciaDosIndividuos(populacao, tamanho_pais_fihos, min_or_max); // calcula dominancia de todos

    // CALCULOS DE DISTANCIA DA ULTIMA FRENTE DE DOMINANCIA

    int inicio_ultima_frente = tamanho_populacao - 1, final_ultima_frente = tamanho_populacao - 1;

    while (populacao[--inicio_ultima_frente].grau_de_dominancia == populacao[tamanho_populacao-1].grau_de_dominancia);
    inicio_ultima_frente++; //while para descobrir o indice onde começa a ultima frente

    while (populacao[++final_ultima_frente].grau_de_dominancia == populacao[tamanho_populacao-1].grau_de_dominancia);
    //while para descobrir o indice onde termina a ultima frente

    float distancias_individuos[final_ultima_frente - inicio_ultima_frente];//vetor de distância alocado de acordo com o tamanho da frente

    for (int i = 0; i < final_ultima_frente - inicio_ultima_frente; i++){
        distancias_individuos[i] = 0;
    }
    
    for (int i = 0; i < qtde_variaveis; i++){
        ordenarVetorFloatJuntoComPopulacao(populacao, inicio_ultima_frente, final_ultima_frente, distancias_individuos, i, buscarIndividuoMenorVariavel);
        j = 0;
        float distancia_maxima = (populacao[final_ultima_frente-1].variaveis[i] - populacao[inicio_ultima_frente].variaveis[i]);
        
        distancias_individuos[j++] = populacao[inicio_ultima_frente + 1].variaveis[i]/distancia_maxima;
        for (int i = inicio_ultima_frente + 1; i < final_ultima_frente - 1; i++){
            distancias_individuos[j++] = (populacao[i + 1].variaveis[i] - populacao[i - 1].variaveis[i])/distancia_maxima;
        }
        distancias_individuos[j] = populacao[inicio_ultima_frente - 1].variaveis[i]/distancia_maxima;
    }
    
    // ordena por distancias para os indivíduos com maior ficar acima
    ordenarComBaseNoVetorFloat(populacao, inicio_ultima_frente, final_ultima_frente, distancias_individuos, buscarMaiorNumeroVetorFloat);
}

int metodoDeBorda(Individuo* populacao, int tamanho_populacao, int *min_or_max_borda){
    int fim_da_frente = 0; // fim da frente de menor dominancia
    while (populacao[++fim_da_frente].grau_de_dominancia == populacao[0].grau_de_dominancia); // busca onde termina a frente de menor dominancia

    float notas_individuos[fim_da_frente];

    for (int i = 0; i < fim_da_frente; i++){
        notas_individuos[i] = 0;
    }

    for (int i = 0; i < qtde_variaveis; i++){
        if (min_or_max_borda){
            ordenarVetorFloatJuntoComPopulacao(populacao, 0, fim_da_frente, notas_individuos, i, buscarIndividuoMaiorVariavel);
        } else {
            ordenarVetorFloatJuntoComPopulacao(populacao, 0, fim_da_frente, notas_individuos, i, buscarIndividuoMenorVariavel);
        }
        for (int j = 1; j <= fim_da_frente; j++){
            notas_individuos[j] += j;
        }
    }
    
    int indice_melhor_individuo = buscarMenorNumeroVetorFloat(0, fim_da_frente, notas_individuos);

    return indice_melhor_individuo;
}

int main(){
    srand(time(NULL)); 

    Item lista_de_itens[tamanho_cromossomo];
    lerArquivo(lista_de_itens);

    int qtde_geracoes;

    printf("\nQuantidade de geracoes desejada: ");
    scanf("%d", &qtde_geracoes);

    Individuo *populacao;

    int min_or_max[3];
    min_or_max[0] = 1; min_or_max[1] = 1; min_or_max[2] = 0;

    int min_or_max_borda[3];
    min_or_max_borda[0] = 0; min_or_max_borda[1] = 1; min_or_max_borda[2] = 0;

    //gerando a primeira população de pais aleatória
    int tamanho_populacao = 1000;
    populacao = alocarPopulacao(2 * tamanho_populacao);
    gerarPopulacaoAleatoria(populacao, 0, tamanho_populacao, lista_de_itens);
    calcularGrauDeDominanciaDosIndividuos(populacao, tamanho_populacao, min_or_max);
    //mostrarPopulacao(populacao, tamanho_populacao);

    int pai1, pai2, melhor_individuo = 0;

    //0.5%, 10%, 80%
    int taxa_de_mutacao = 5, probabilidade_mutacao = 100, taxa_de_cruzamento = 800;

    int vetorGeracoes[qtde_geracoes];

    for (int i = 0; i < qtde_geracoes; i++){

        for (int j = tamanho_populacao; j < ((tamanho_populacao*2)-1); j+=2){
            
            pai1 = selecaoPorTorneio(populacao, tamanho_populacao); //selecao
            pai2 = selecaoPorTorneio(populacao, tamanho_populacao); //selecao
            
            if (gerarAleatorio(0,1000) < taxa_de_cruzamento){           //cruzamento
                crossoverUniforme(populacao[pai1], populacao[pai2], &(populacao[j]), &(populacao[j+1]));
                
                if(gerarAleatorio(0,1000) < probabilidade_mutacao){ //mutação
                    mutacao(&(populacao[j]), taxa_de_mutacao);
                    mutacao(&(populacao[j+1]), taxa_de_mutacao);
                }

            } else { //caso não ocorra filhos se repetem como pais
                populacao[j] = populacao[pai1];
                populacao[j+1] = populacao[pai2];
                
                if(gerarAleatorio(0,1000) < (probabilidade_mutacao * 2)){ //mutação
                    mutacao(&(populacao[j]), (taxa_de_mutacao * 2));
                    mutacao(&(populacao[j+1]), (taxa_de_mutacao * 2));
                }
            } 
            //calculo do fitness dos novos filhos
            calcularFuncoes(&populacao[j], lista_de_itens);
            calcularFuncoes(&populacao[j+1], lista_de_itens);
        } 

        estrategiaPhiLambda(populacao, tamanho_populacao, min_or_max);

        printf("%d ", i+1);
        
        melhor_individuo = metodoDeBorda(populacao, tamanho_populacao, min_or_max_borda);

        printarIndividuo(populacao[melhor_individuo]);
        
    }
    mostrarPopulacao(populacao, tamanho_populacao);

    return 0;
}