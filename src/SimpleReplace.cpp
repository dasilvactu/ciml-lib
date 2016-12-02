#include "SimpleReplace.h"

SimpleReplace::SimpleReplace(){

}
void SimpleReplace::Replace(Subject **pop){

            int elitismo = conf->elitism*conf->popSize;
            sort(pop + elitismo, pop + conf->popSize * 2, SortMyPop);

            int indice;

            for(int i = elitismo; i < conf->popSize; i++){
                indice = conf->popSize + i - elitismo;
                if(pop[i]->fitness < pop[indice]->fitness)
                    swap(pop[i], pop[indice]);
            }
            sort(pop, pop + conf->popSize * 2, SortMyPop);
}

SimpleReplace::~SimpleReplace(){

}
