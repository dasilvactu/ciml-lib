#define __CL_ENABLE_EXCEPTIONS

#include <omp.h>        //OpenMP

#include <iostream>
#include <iomanip>

#include "genetica.h"

#include "GPTime.h"

#include "oclConfig.h"  //Outras configura��es e bibliotecas

#define TESTA_INDIV 0

int main(int argc, char** argv){

    std::cout << std::setprecision(32) << std::fixed;

    float maxDados, minDados;
    minDados = std::numeric_limits<float>::max();
    maxDados = (-1) * minDados;

    GPTime timeManager(4);
    timeManager.getStartTime(Total_T);

    double tempoAvaliacaoOCL = 0;
    double tempoTotalAvaliacaoOCL = 0;
    double tempoTotalAvaliacao = 0;

    double tempoEvolucaoOCL = 0;
    double tempoTotalEvolucaoOCL = 0;
    double tempoTotalEvolucao = 0;

    int iteracoes = 0;

    Arvore* popAtual;
    popAtual = new Arvore[NUM_INDIV];

    Arvore* popFutura;
    popFutura = new Arvore[NUM_INDIV];

    int* seeds;
    int* seeds2;
    seeds = new int [NUM_INDIV * MAX_NOS];

    ///variaveis lidas de arquivo
    int M, N;
    char** LABELS;
    int* conjuntoOpTerm;
    int* conjuntoOpTerm2;
    float* ctes; //TODO:ainda nao le constantes...
    int NUM_OPBIN, NUM_OPUN, NUM_CTES;

    ///leituras de dados
    float** dadosTreinamento = readTrainingData(&M, &N, &NUM_CTES, &NUM_OPBIN, &NUM_OPUN, &LABELS, &conjuntoOpTerm, &maxDados, &minDados, argv[1]);

    atribuiSemente(SEED);

    ///transposicao de dados
    float* dadosTranspostos;
    dadosTranspostos = new float [M * N];

    unsigned int pos = 0;
    std::cout << "Transpondo dados..." << std::endl;
    for(int j = 0; j < N; ++j ){
        for(int i = 0; i < M; ++i ){
            dadosTranspostos[pos++] = dadosTreinamento[i][j];
        }
    }

    ///vetor de seeds a ser utilizado no c�digo paralelo
    for(int i = 0; i < NUM_INDIV*MAX_NOS; i++){
        seeds[i] = rand();
    }

    imprimeParametros(M, N, NUM_CTES, NUM_OPBIN, NUM_OPUN);
    inicializaPopulacao(popAtual, conjuntoOpTerm, NUM_OPBIN, NUM_OPUN, N, &seeds[0], maxDados, minDados);
    avaliaIndividuos(popAtual, /*dadosTranspostos*/dadosTreinamento, M, N);



        cl_ulong inicio, fim;

        ///Evento para controlar tempo gasto
        cl::Event e_tempo;
        cl::Event eventsEvol;
        std::vector<cl::Event> eventsEvol;
        //std::vector<cl::Event> eventsAval;

        cl_int result; //Variavel para verificar erros
        ///TODO: Colocar conferencias de erros pelo c�digo
        std::vector<cl::Platform> platforms;
        std::vector<cl::Device> devices;

        setupOpenCLOnePlatform(platforms, devices);
        //printPlatformsDevices(platforms, devices);

        ///Estabelecendo o contexto com os devices
        cl::Context contexto(devices, NULL, NULL, NULL, &result);
        if(result != CL_SUCCESS){
            std::cout << "Erro ao criar um contexto OpenCL" << std::endl;
            exit(1);
        }

        ///Criando a fila de comando
        cl_command_queue_properties commandQueueProperties = CL_QUEUE_PROFILING_ENABLE;

        cl::CommandQueue* cmdQueueEvol;
        cl::CommandQueue* cmdQueueAval;

        setupCmdQueuesOnePlatform(cmdQueueAval, cmdQueueEvol, commandQueueProperties, devices, contexto);

        cl::Buffer bufferPopA(contexto, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, NUM_INDIV * sizeof(Arvore));
        cl::Buffer bufferPopF(contexto, CL_MEM_READ_WRITE, NUM_INDIV * sizeof(Arvore));
        cl::Buffer bufferOpTerm(contexto, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, (NUM_OPBIN + NUM_OPUN + NUM_CTES + N - 1) * sizeof(int));
        cl::Buffer bufferSeeds(contexto, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE, NUM_INDIV * MAX_NOS * sizeof(int));
        cl::Buffer dados(contexto, CL_MEM_READ_ONLY, M * N * sizeof(float));

        cmdQueueEvol->enqueueWriteBuffer(bufferSeeds, CL_FALSE, 0, NUM_INDIV * MAX_NOS * sizeof(int), seeds);
        cmdQueueAval->enqueueWriteBuffer(dados, CL_TRUE, 0, M * N * sizeof(float), dadosTranspostos);
        cmdQueueEvol->enqueueWriteBuffer(bufferOpTerm, CL_FALSE, 0, (NUM_OPBIN + NUM_OPUN + NUM_CTES + N - 1) * sizeof(int), conjuntoOpTerm);

        cmdQueueAval->finish();
        cmdQueueEvol->finish();

        size_t globalSize;
        size_t localSize;
        size_t numPoints;
        std::string compileFlags;

        numPoints = (size_t)M;

        size_t maxLocalSize = cmdQueueAval->getInfo<CL_QUEUE_DEVICE>().getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
        setNDRanges(&globalSize, &localSize, &compileFlags, maxLocalSize, numPoints, cmdQueueAval->getInfo<CL_QUEUE_DEVICE>().getInfo<CL_DEVICE_TYPE>());

        std::cout << "Global Size = " <<globalSize << std::endl << "Local size = " << localSize << std::endl << std::endl;

        ///Leitura do arquivo com o programa em C++
        std::ifstream sourceFileName("kernelAvalia.cl");
        std::string sourceFile(std::istreambuf_iterator<char>(sourceFileName),(std::istreambuf_iterator<char>()));
        std::string program_src = setProgramSource(NUM_OPBIN, NUM_OPUN, M, N, (int)localSize, maxDados, minDados) + sourceFile;
        //std::cout << program_src << std::endl;

        ///Criar programa por Source
        cl::Program::Sources source(1, std::make_pair(program_src.c_str(), program_src.length()+1));
        cl::Program programa(contexto, source);

        //compileFlags+=" -cl-opt-disable";
        //std::cout << "Compile Flags = " << compileFlags << std::endl;
        try {
            programa.build(devices, compileFlags.c_str());
        } catch(cl::Error& e){
            std::cerr << getErrorString(e.err()) << std::endl
            << programa.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
            std::cout << e.what() << " : " << e.err() << std::endl;
            exit(1);
        }
        //std::cout << programa.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;

        #if AVALGPU
            cl::Kernel krnlAvalia(programa, "avaliaIndividuos");
        #elif AVALOCL && !AVALOCL_SEQ
            cl::Kernel krnlAvalia(programa, "avaliaIndividuosCPU");
        #else
            cl::Kernel krnlAvalia(programa, "avaliaIndividuosSequencial");
        #endif // AVALGPU

        #if EVOLOCL && !EVOLOCL_SEQ
            cl::Kernel krnlEvolucao(programa, "evolucao");
        #else
            cl::Kernel krnlEvolucao(programa, "evolucaoSequencial");
        #endif // EVOLOCL




    imprimePopulacao(popAtual, LABELS);


    while(criterioDeParada(iteracoes) /*qual o criterio de parada?*/){
       // imprimePopulacao(popAtual, LABELS);
        printf("\n-----------\nGERACAO %d: \n", iteracoes);

        timeManager.getStartTime(Iteracao_T);

        timeManager.getStartTime(Evolucao_T);
        int novosIndividuos = selecionaElite(popAtual, popFutura);



            cmdQueueEvol->enqueueWriteBuffer(bufferPopA, CL_TRUE, 0, NUM_INDIV * sizeof(Arvore), popAtual);
            cmdQueueEvol->enqueueWriteBuffer(bufferPopF, CL_TRUE, 0, NUM_INDIV * sizeof(Arvore), popFutura);
            cmdQueueEvol->finish();

            //cmdQueueAval->enqueueWriteBuffer(bufferPopF, CL_TRUE, 0, NUM_INDIV * sizeof(Arvore), popFutura);
            //cmdQueueAval->finish();

            ///Dispondo argumentos para o kernel + executar
            krnlEvolucao.setArg(0, bufferPopA);
            krnlEvolucao.setArg(1, bufferPopF);
            krnlEvolucao.setArg(2, novosIndividuos);
            krnlEvolucao.setArg(3, bufferOpTerm);
            krnlEvolucao.setArg(4, bufferSeeds);


            ///Dispondo argumentos para o kernel + executar
            krnlAvalia.setArg(0, bufferPopF);
            krnlAvalia.setArg(1, dados);
            krnlAvalia.setArg(2, localSize * sizeof(float), NULL);

        for(unsigned int x = novosIndividuos; x < NUM_INDIV; x += 2){
            cmdQueueEvol->enqueueNDRangeKernel(krnlEvolucao, x - novosIndividuos, cl::NDRange(2), cl::NDRange(1), NULL, &eventsEvol);
            cmdQueueAval->enqueueNDRangeKernel(krnlAvalia, x - novosIndividuos, localSize * 2, cl::NDRange(localSize), eventsEvol, eventsAval[x]);
        }



//            try {
//                result = cmdQueueEvol->enqueueNDRangeKernel(krnlEvolucao, cl::NullRange, cl::NDRange((NUM_INDIV-novosIndividuos)/2), cl::NDRange(1),NULL, &e_tempo);
//            } catch(cl::Error& e){
//                std::cerr << getErrorString(e.err()) << std::endl;
//                exit(1);
//            }

            cmdQueueEvol->finish();

            #if OCL_TIME
            e_tempo.getProfilingInfo(CL_PROFILING_COMMAND_START, &inicio);
            e_tempo.getProfilingInfo(CL_PROFILING_COMMAND_END, &fim);
            tempoEvolucaoOCL = (fim-inicio)/1.0E9;
            tempoTotalEvolucaoOCL += tempoEvolucaoOCL;
            #endif

            ///se est� em um s� dispositivo nao precisa disso
            //cmdQueueEvol->enqueueReadBuffer(bufferPopA, CL_TRUE, 0, NUM_INDIV * sizeof(Arvore), popAtual);
            //#if TWODEVICES || !AVALOCL
            //cmdQueueEvol->enqueueReadBuffer(bufferPopF, CL_TRUE, 0, NUM_INDIV * sizeof(Arvore), popFutura);
            //#endif // TWODEVICES
            //cmdQueueEvol->finish();



            try {
                result = cmdQueueAval->enqueueNDRangeKernel(krnlAvalia, cl::NullRange, cl::NDRange(globalSize), cl::NDRange(localSize), NULL, &e_tempo);
            } catch(cl::Error& e){
                std::cerr << getErrorString(e.err()) << std::endl;
                exit(1);
            }


            cmdQueueAval->finish();

            #if OCL_TIME
            e_tempo.getProfilingInfo(CL_PROFILING_COMMAND_START, &inicio);
            e_tempo.getProfilingInfo(CL_PROFILING_COMMAND_END, &fim);
            tempoAvaliacaoOCL = (fim-inicio)/1.0E9;
            tempoTotalAvaliacaoOCL += tempoAvaliacaoOCL;
            #endif

            cmdQueueAval->enqueueReadBuffer(bufferPopF, CL_TRUE, 0, NUM_INDIV * sizeof(Arvore), popFutura);

            cmdQueueAval->finish();


        //timeManager.getEndTime(Avaliacao_T);

        for(int i = 0; i< NUM_INDIV; i++){
            popAtual[i] = popFutura[i];
        }

        timeManager.getEndTime(Iteracao_T);
        double iterTime = timeManager.getElapsedTime(Iteracao_T);
        double avalTime = timeManager.getElapsedTime(Avaliacao_T);
        double evolTime = timeManager.getElapsedTime(Evolucao_T);

        #if PROFILING_TIME
        #if OCL_TIME
        std::cout << "Tempo evolucao OCL = " << tempoEvolucaoOCL << std::endl;
        std::cout << "Tempo avaliacao OCL = " << tempoAvaliacaoOCL << std::endl;
        #endif
        std::cout << "Evol time = " << evolTime << std::endl;
        std::cout << "Aval time = " << avalTime << std::endl;
        std::cout << "Iter time = " << iterTime << std::endl << std::endl;
        #endif

        #if PROFILING_POP
        //imprimePopulacao(popAtual, LABELS);
        imprimeMelhor(popAtual, LABELS);
        #endif
        iteracoes++;
    }
    timeManager.getEndTime(Total_T);

    std::cout << "\n\nPOPULACAO FINAL" << std::endl;
    imprimePopulacao(popAtual, LABELS);
    std::cout << "\n*";
    imprimeMelhor(popAtual, LABELS);
    std::cout << std::endl << std::endl;

    std::cout << "T_Evol time = " << timeManager.getTotalTime(Evolucao_T) << std::endl;
    std::cout << "T_Aval time = " << timeManager.getTotalTime(Avaliacao_T) << std::endl;
    std::cout << "T_Iter time = " << timeManager.getTotalTime(Iteracao_T) << std::endl;
    std::cout << "Total time  = " << timeManager.getElapsedTime(Total_T) << std::endl;
    std::cout << std::endl;
    #if OCL_TIME
    std::cout << "T_EOcl time = " << tempoTotalEvolucaoOCL << std::endl;
    std::cout << "T_AOcl time = " << tempoTotalAvaliacaoOCL << std::endl << std::endl;
    std::cout << "T_TOcl time = " << tempoTotalEvolucaoOCL+tempoTotalAvaliacaoOCL << std::endl;
    #endif

    std::cout << "FIM" << std::endl;
    std::cout << timeManager.getTotalTime(Evolucao_T) << " ";
    std::cout << timeManager.getTotalTime(Avaliacao_T) << " ";
    std::cout << timeManager.getTotalTime(Iteracao_T) << " ";
    std::cout << timeManager.getElapsedTime(Total_T) << std::endl;
    #if OCL_TIME
    std::cout << "OCLFlag" << std::endl;
    std::cout << tempoTotalEvolucaoOCL << " ";
    std::cout << tempoTotalAvaliacaoOCL << " ";
    std::cout << tempoTotalEvolucaoOCL+tempoTotalAvaliacaoOCL << std::endl;
    #endif
    imprimeMelhor(popAtual, LABELS);
    std::cout << std::endl << std::endl;
    //std::cout << tempoEvolucao << " " << tempoTotalAvaliacao << " " << tempoTotal << std::endl;

    delete [] popAtual;
    delete [] popFutura;
    return 0;
}