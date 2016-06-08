
#include "No.h"
#include <../include/Individuo.h>
#include <iostream>
#include <cstddef>
#include <vector>
No::No(int a, int j, int k)
{
    linha_cache = -1;
    this->tipo = a;
    linha = j;
    coluna = k;
    switch(tipo){
        case -1: //entrada
            num_entradas = 0;
            this->custo = 0;
            break;

        case 0: //and
            num_entradas = 2;
            this->custo = 0;
            //entradas = new No *[num_entradas];
            break;
        case 1: //or
            num_entradas = 2;
            this->custo = 0;
            //entradas = new No *[num_entradas];
            break;
        case 2: //nand
            num_entradas = 2;
            this->custo = 0;
            //entradas = new No *[num_entradas];
            break;
        case 3: //nor
            num_entradas = 2;
            this->custo = 0;
            break;
        case 4: //not
            num_entradas = 2;
            this->custo = 0;
            break;
        case 5:
            num_entradas = 2;
            this->custo = 0;
            break;
        case 6:
            num_entradas = 2;
            this->custo = 0;
            break;
        default:
            throw "tipo não permitido";
            break;
    }

    //isLigante = false;
    //entradas = ind_entradas;
    //ctor
}

No::No(No *copia, Individuo *ind) : No(copia->tipo, copia->linha, copia->coluna){



    isLigante = copia->isLigante;
    //pontuacao = 0;
    for(int i=0;i<num_entradas;i++){
        if(copia->entradas[i].coluna == -1){
            entradas.push_back(*ind->entradas[copia->entradas[i].linha]);
        } else {
            entradas.push_back(*ind->matrizNo[copia->entradas[i].linha][copia->entradas[i].coluna]);
        }

    }
}

No::~No()
{
    //dtor

}

bool No::getSaida(int j, bool **tabela_entrada){
    isLigante = true;
    bool entrada0, entrada1;
    if(linha_cache == j)
        return saida_cache;
    linha_cache = j;
    if(tipo != -1){
        entrada0 = entradas[0].getSaida(j, tabela_entrada);
        entrada1 = entradas[1].getSaida(j, tabela_entrada);
        cout << "Entrada 0: Linha " << entradas[0].linha << " Coluna " << entradas[0].coluna << endl;
        cout << "Entrada 1: Linha " << entradas[1].linha << " Coluna " << entradas[1].coluna << endl;
        cout << "linha: "<< j<< " Tipo: " << this->tipo <<" Entrada 0: " << entrada0 << " Entrada 1: " << entrada1 << endl;
    }

    switch(tipo){
        case 0: //and

            saida_cache = entrada0 and entrada1;
            break;
        case 1: //or
            saida_cache =  entrada0 or entrada1;
            break;
        case 2: //n-and
            saida_cache =  !(entrada0 and entrada1);
            break;
        case 3: //nor
            saida_cache =  !(entrada0 or entrada1);
            break;
        case 4: //not
            saida_cache =  !(entrada0);
            break;
        case 5: //ID
            saida_cache =  (entrada0);
            break;
        case 6: //xor

            saida_cache =  (entrada0 and !entrada1)or(!entrada0 and entrada1);
            break;
        case -1:
            saida_cache =  tabela_entrada[j][this->linha];
            break;
        default:
            return 0;
            break;
    }
    return saida_cache;
}

void No::mudaTipo(int tipo){
    this->tipo = tipo;
    switch(tipo){
        case 4:
            num_entradas = 2;
            break;
            default:
            num_entradas = 2;
            break;
    }
}



