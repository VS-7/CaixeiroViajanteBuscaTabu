#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <climits>

/*
    Criado por @Vitor Sérgio - Disciplina: Pesquisa Operacional
*/
using namespace std;


int** lerMatrizDistancias(const char* nomeArquivo, int& numLocalidades) {
    ifstream file(nomeArquivo);
    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << nomeArquivo << endl;
        exit(EXIT_FAILURE);
    }

    file >> numLocalidades;
    int** matrizDistancias = new int*[numLocalidades];
    for (int i = 0; i < numLocalidades; ++i) {
        matrizDistancias[i] = new int[numLocalidades];
        for (int j = 0; j < numLocalidades; ++j) {
            file >> matrizDistancias[i][j];
        }
    }

    file.close();
    return matrizDistancias;
}

// Função para calcular a distância total de um caminho
int calcularDistancia(int* caminho, int** matrizDistancias, int numLocalidades) {
    int distancia = 0;
    for (int i = 0; i < numLocalidades - 1; ++i) {
        distancia += matrizDistancias[caminho[i]][caminho[i + 1]];
    }
    distancia += matrizDistancias[caminho[numLocalidades - 1]][caminho[0]];  // Retorno à fazenda inicial
    return distancia;
}

// Função para gerar uma solução inicial aleatória
int* gerarSolucaoInicial(int numLocalidades) {
    int* solucao = new int[numLocalidades + 1]; // Adicione 1 para o ponto de retorno ao início

    // Inicializa a solução de forma ordenada de 0 a numLocalidades-1
    for (int i = 0; i < numLocalidades; ++i) { 
        solucao[i] = i;
    }

    // Embaralha a solução, exceto o primeiro e último nó
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = numLocalidades - 2; i > 1; --i) { 
        int j = 1 + rand() % i; 
        swap(solucao[i], solucao[j]);
    }

    // Adiciona o ponto 0 no final da solução
    solucao[numLocalidades] = 0;

    return solucao;
}

// Função para trocar dois nós na solução
void trocarDoisNos(int* solucao, int capacidadeCaminhao, int** matrizDistancias, int numLocalidades) {
    int* novaSolucao = new int[numLocalidades + 1];
    copy(solucao, solucao + numLocalidades + 1, novaSolucao);

    int idx1, idx2;
    do {
        idx1 = 1 + rand() % (numLocalidades - 2); 
        idx2 = 1 + rand() % (numLocalidades - 2); 
    } while (idx1 == idx2);

    int capacidadeAtual = 0;
    for (int i = idx1; i <= idx2; ++i) {
        capacidadeAtual += matrizDistancias[novaSolucao[i - 1]][novaSolucao[i]];
    }

    while (capacidadeAtual > capacidadeCaminhao) {
        idx1 = 1 + rand() % (numLocalidades - 2); 
        idx2 = 1 + rand() % (numLocalidades - 2); 

        capacidadeAtual = 0;
        for (int i = idx1; i <= idx2; ++i) {
            capacidadeAtual += matrizDistancias[novaSolucao[i - 1]][novaSolucao[i]];
        }
    }

    swap(novaSolucao[idx1], novaSolucao[idx2]);

    copy(novaSolucao, novaSolucao + numLocalidades + 1, solucao);
    delete[] novaSolucao;
}

// Função para aceitar ou rejeitar um movimento baseado na lista tabu
bool aceitarMovimento(pair<int, int> movimento, pair<int, int>* tabuList, int tabuSize) {
    for (int i = 0; i < tabuSize; ++i) {
        if (movimento == tabuList[i]) {
            return false;
        }
    }
    return true;
}

// Função para a Busca Tabu
void buscaTabu(int** matrizDistancias, int capacidadeCaminhao, int numIteracoes, int numLocalidades) {
    int* melhorSolucao = new int[numLocalidades + 1];
    int melhorDistancia = INT_MAX;

    int* solucaoAtual = gerarSolucaoInicial(numLocalidades);
    copy(solucaoAtual, solucaoAtual + numLocalidades + 1, melhorSolucao);

    pair<int, int>* tabuList = new pair<int, int>[5];  // Lista tabu para evitar movimentos repetidos

    for (int iteracao = 0; iteracao < numIteracoes; ++iteracao) {
        // Movimento: Troca de dois nós, considerando a capacidade do caminhão
        trocarDoisNos(solucaoAtual, capacidadeCaminhao, matrizDistancias, numLocalidades);

        // Aceitar soluções melhores que a melhor encontrada
        int distanciaNovaSolucao = calcularDistancia(solucaoAtual, matrizDistancias, numLocalidades);
        if (distanciaNovaSolucao < melhorDistancia) {
            copy(solucaoAtual, solucaoAtual + numLocalidades + 1, melhorSolucao);
            melhorDistancia = distanciaNovaSolucao;
        }

        // Aceitar ou rejeitar o movimento baseado na lista tabu
        pair<int, int> movimento = {solucaoAtual[1], solucaoAtual[2]};
        if (aceitarMovimento(movimento, tabuList, 5)) {
            copy(solucaoAtual, solucaoAtual + numLocalidades + 1, melhorSolucao);
        }

        // Adicionar o movimento à lista tabu
        tabuList[iteracao % 5] = movimento;
    }

    // Imprimir resultados
    cout << "\nMelhor solução encontrada: [ ";
    for (int i = 0; i <= numLocalidades; ++i) {
        cout << melhorSolucao[i] << " ";
    }
    cout << "]" << endl;

    cout << "Distância total: " << melhorDistancia << " km" << endl;
    
    // Liberar memória
    delete[] melhorSolucao;
    delete[] solucaoAtual;
    delete[] tabuList;
}

int main() {
    int** matrizDistancias;
    int numLocalidades;
    matrizDistancias = lerMatrizDistancias("grafo.txt", numLocalidades);

    int capacidadeCaminhao = 150;
    int numIteracoes = 1000;

    int* solucaoInicial = gerarSolucaoInicial(numLocalidades);
    cout << "Solução inicial: [ ";
    for (int i = 0; i <= numLocalidades; ++i) {
        cout << solucaoInicial[i] << " ";
    }
    cout << "]" << endl;
    cout << "Distância inicial: " << calcularDistancia(solucaoInicial, matrizDistancias, numLocalidades) << " km" << endl;

    buscaTabu(matrizDistancias, capacidadeCaminhao, numIteracoes, numLocalidades);
    
    // Liberar memória
    for (int i = 0; i < numLocalidades; ++i) {
        delete[] matrizDistancias[i];
    }
    delete[] matrizDistancias;
    delete[] solucaoInicial;

    return 0;
}
