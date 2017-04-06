#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>

#include <iostream>
#include <fstream>

#include <vector>
#include <utility>
#include <cstdlib>
#include <iomanip>

//#include <time.h>
//#include <windows.h>

//#pragma OPENCL EXTENSION cl_intel_printf : enable
//#pragma OPENCL EXTENSION cl_amd_printf : enable


#define ARRAY_SIZE 20000
#define ARRAY_SIZE_R 8

const char *getErrorString(cl_int error) {
    switch(error){
        // run-time and JIT compiler errors
        case 0: return "CL_SUCCESS";
        case -1: return "CL_DEVICE_NOT_FOUND";
        case -2: return "CL_DEVICE_NOT_AVAILABLE";
        case -3: return "CL_COMPILER_NOT_AVAILABLE";
        case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case -5: return "CL_OUT_OF_RESOURCES";
        case -6: return "CL_OUT_OF_HOST_MEMORY";
        case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case -8: return "CL_MEM_COPY_OVERLAP";
        case -9: return "CL_IMAGE_FORMAT_MISMATCH";
        case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case -11: return "CL_BUILD_PROGRAM_FAILURE";
        case -12: return "CL_MAP_FAILURE";
        case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case -15: return "CL_COMPILE_PROGRAM_FAILURE";
        case -16: return "CL_LINKER_NOT_AVAILABLE";
        case -17: return "CL_LINK_PROGRAM_FAILURE";
        case -18: return "CL_DEVICE_PARTITION_FAILED";
        case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

        // compile-time errors
        case -30: return "CL_INVALID_VALUE";
        case -31: return "CL_INVALID_DEVICE_TYPE";
        case -32: return "CL_INVALID_PLATFORM";
        case -33: return "CL_INVALID_DEVICE";
        case -34: return "CL_INVALID_CONTEXT";
        case -35: return "CL_INVALID_QUEUE_PROPERTIES";
        case -36: return "CL_INVALID_COMMAND_QUEUE";
        case -37: return "CL_INVALID_HOST_PTR";
        case -38: return "CL_INVALID_MEM_OBJECT";
        case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case -40: return "CL_INVALID_IMAGE_SIZE";
        case -41: return "CL_INVALID_SAMPLER";
        case -42: return "CL_INVALID_BINARY";
        case -43: return "CL_INVALID_BUILD_OPTIONS";
        case -44: return "CL_INVALID_PROGRAM";
        case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
        case -46: return "CL_INVALID_KERNEL_NAME";
        case -47: return "CL_INVALID_KERNEL_DEFINITION";
        case -48: return "CL_INVALID_KERNEL";
        case -49: return "CL_INVALID_ARG_INDEX";
        case -50: return "CL_INVALID_ARG_VALUE";
        case -51: return "CL_INVALID_ARG_SIZE";
        case -52: return "CL_INVALID_KERNEL_ARGS";
        case -53: return "CL_INVALID_WORK_DIMENSION";
        case -54: return "CL_INVALID_WORK_GROUP_SIZE";
        case -55: return "CL_INVALID_WORK_ITEM_SIZE";
        case -56: return "CL_INVALID_GLOBAL_OFFSET";
        case -57: return "CL_INVALID_EVENT_WAIT_LIST";
        case -58: return "CL_INVALID_EVENT";
        case -59: return "CL_INVALID_OPERATION";
        case -60: return "CL_INVALID_GL_OBJECT";
        case -61: return "CL_INVALID_BUFFER_SIZE";
        case -62: return "CL_INVALID_MIP_LEVEL";
        case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
        case -64: return "CL_INVALID_PROPERTY";
        case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
        case -66: return "CL_INVALID_COMPILER_OPTIONS";
        case -67: return "CL_INVALID_LINKER_OPTIONS";
        case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

        // extension errors
        case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
        case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
        case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
        case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
        case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
        case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
        default: return "Unknown OpenCL error";
    }
}


int main(){

    std::cout << std::setprecision(10) << std::fixed;
    cl_int result; //Variavel para verificar erros

    float somaTotal = 0;
    float *X = new float[ARRAY_SIZE];
    //float *Y = new float[ARRAY_SIZE_R];

    ///Inicializa X com 0
    for(int i = 0; i < ARRAY_SIZE; i++){
        X[i] = 0;
    }
    ///Medir tempo dentro do host
    //LARGE_INTEGER start, eNd, frequency;
    //QueryPerformanceFrequency(&frequency);

    //QueryPerformanceCounter(&start);
    for(int i = 0; i < ARRAY_SIZE; i++){
        somaTotal+=X[i];
    }
    //QueryPerformanceCounter(&eNd);

    std::cout << "Soma Inicial = " << somaTotal << std::endl;
//    std::cout << "Tempo soma sequencial = " << (double)(eNd.QuadPart - start.QuadPart)/frequency.QuadPart  << std::endl << std::endl;

    ///TODO: Colocar conferencias de erros pelo c�digo
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;

    ///Encontrando as plataformas disponiveis
    result = cl::Platform::get(&platforms);
    if(result != CL_SUCCESS){
        std::cout << "Erro ao encontrar plataformas." << std::endl;
        exit(1);
    }

    ///Encontrando os devices disponiveis na plataforma 0
    platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);
    if(result != CL_SUCCESS){
        std::cout << "Erro ao encontrar devices." << std::endl;
        exit(1);
    }


    std::cout << "Available Platforms: \n";
    for(cl_uint i = 0; i < platforms.size(); ++i) {
        std::cout <<"\t[" << i << "]"<<platforms[i].getInfo<CL_PLATFORM_NAME>() << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Available Devices for Platform " << platforms[0].getInfo<CL_PLATFORM_NAME>()<< ":\n";
    for(cl_uint i = 0; i < devices.size(); ++i) {
        std::cout<<"\t[" << i << "]"<<devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
    }
    std::cout << std::endl;

    ///Estabelecer os seguintes valores (aqui pode ser diferente para cada dispositivo)
    cl_uint compute_units;
    size_t global_work_size = 1024;
    size_t local_work_size = 256;
    size_t num_groups;

    num_groups = global_work_size / local_work_size;
    float* Y = new float[num_groups];


    ///Estabelecendo o contexto com os devices
    cl::Context contexto(devices, NULL, NULL, NULL, &result);
    if(result != CL_SUCCESS){
        std::cout << "Erro ao criar um contexto OpenCL" << std::endl;
        exit(1);
    }

    ///Criando a fila de comando para o device 0
    cl_command_queue_properties commandQueueProperties = CL_QUEUE_PROFILING_ENABLE;
    ///AQUI
    cl::CommandQueue cmdQueueCPU(contexto, devices[2], commandQueueProperties, &result);
    cl::CommandQueue cmdQueueGPU(contexto, devices[0], commandQueueProperties, &result);
    if(result != CL_SUCCESS){
        std::cout << "Erro ao criar a Command Queue" << std::endl;
        exit(1);
    }
    ///Evento para controlar tempo gasto
    cl::Event e_tempo1, e_tempo2;

    ///Criar Objetos de Buffer
    cl::Buffer bufferX(contexto, CL_MEM_READ_WRITE| CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), X);
    cl::Buffer bufferY(contexto, CL_MEM_WRITE_ONLY| CL_MEM_USE_HOST_PTR, num_groups * sizeof(float), Y);

    ///Tranfere os dados para o buffer de memoria
    //cmdQueueCPU.enqueueWriteBuffer(bufferX, CL_TRUE, 0, ARRAY_SIZE*sizeof(float), X);

    ///Leitura do arquivo com o programa em C++
	std::ifstream sourceFileName("estudo.cl");
	std::string sourceFile(std::istreambuf_iterator<char>(sourceFileName),(std::istreambuf_iterator<char>()));

    ///Criar programa por Source
    //cl::Program::Sources fonte(1, std::make_pair(kernel_str, std::strlen(kernel_str)));
    cl::Program::Sources source(1, std::make_pair(sourceFile.c_str(), sourceFile.length()+1));
    cl::Program programa(contexto, source);

    try {
        programa.build(devices);
    } catch(cl::Error& e){
        std::cerr << getErrorString(e.err()) << std::endl;
        exit(1);
    }

    int teste = ARRAY_SIZE;
    ///kernel para inicializar o vetor X (0, 1, 2, ... , ARRAY_SIZE-1)
    cl::Kernel krnlInicializa(programa, "inicializaCPU");
    krnlInicializa.setArg(0, bufferX);
    krnlInicializa.setArg(1, sizeof(int), &teste);
    std::cout << "Inicializando Array..." << std::endl;
    ///EXECUTA NA CPU
    try {
        result = cmdQueueCPU.enqueueNDRangeKernel(krnlInicializa, cl::NullRange, cl::NDRange(ARRAY_SIZE), cl::NDRange(1), NULL, &e_tempo1);
    } catch(cl::Error& e){
        std::cerr << getErrorString(e.err()) << std::endl;
        exit(1);
    }
    cmdQueueCPU.finish();

    std::cout << "Somatorio Estagio 1..." << std::endl;
    ///KERNEL para somat�rio (estagio 1)
    cl::Kernel krnl(programa, "somatorioSequencialEsperto");

    ///Dispondo argumentos para o kernel + executar
    krnl.setArg(0, bufferX);
    krnl.setArg(1, local_work_size*sizeof(int), 0);
    krnl.setArg(2, sizeof(int), &teste);
    krnl.setArg(3, bufferY);


    ///EXECUTA NA GPU
    try {
        result = cmdQueueGPU.enqueueNDRangeKernel(krnl, cl::NullRange, cl::NDRange(global_work_size), cl::NDRange(local_work_size), NULL, &e_tempo1);
    } catch(cl::Error& e){
        std::cerr << getErrorString(e.err()) << std::endl;
        exit(1);
    }
    ///Sincroniza��o
    cmdQueueGPU.finish();

    float *Z = new float[1];
    cl::Buffer bufferZ(contexto, CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR, sizeof(float),Z);

//    std::cout << "Second Call: " << std::endl;
//    std::cout << "  Global size = " << local_work_size << std::endl;
//    std::cout << "  Local size = " << local_work_size << std::endl;
//    std::cout << "  Work groups = " << 1 << std::endl;
    std::cout << "Somatorio Estagio 2..." << std::endl;
    ///KERNEL para somat�rio (estagio 2)
    cl::Kernel krnl2(programa, "somatorioSequencialEsperto");
    krnl2.setArg(0, bufferY);
    krnl2.setArg(1, local_work_size*sizeof(int), 0);
    teste = num_groups;
    krnl2.setArg(2, sizeof(int), &teste);
    krnl2.setArg(3, bufferZ);
    ///EXECUTA na GPU
    try {
        result = cmdQueueGPU.enqueueNDRangeKernel(krnl2, cl::NullRange, cl::NDRange(local_work_size), cl::NDRange(local_work_size), NULL, &e_tempo2);
    } catch(cl::Error& e){
        std::cerr << getErrorString(e.err()) << std::endl;
        exit(1);
    }

    cmdQueueGPU.finish();
    ///Lendo a mem�ria
    //(CL_TRUE) deixa as fun��es s�ncronas: esperam o t�rmino de tudo que est� antes para ocorrerem
//    cmdQueue.enqueueReadBuffer(bufferX, CL_FALSE, 0, ARRAY_SIZE * sizeof(float), X);
//    cmdQueue.enqueueReadBuffer(bufferY, CL_FALSE, 0, num_groups * sizeof(float), Y);
//    cmdQueue.enqueueReadBuffer(bufferZ, CL_FALSE, 0, sizeof(float), Z);

    X = (float*)cmdQueueCPU.enqueueMapBuffer(bufferX,CL_FALSE, CL_MAP_READ, 0, ARRAY_SIZE * sizeof(float));
    Y = (float*)cmdQueueCPU.enqueueMapBuffer(bufferY,CL_FALSE, CL_MAP_READ, 0, num_groups * sizeof(float));
    Z = (float*)cmdQueueCPU.enqueueMapBuffer(bufferZ,CL_FALSE, CL_MAP_READ, 0, sizeof(float));
    cmdQueueCPU.finish();

    float soma = 0;
    float soma2 = 0;
    //cmdQueue.enqueueReadBuffer(bufferY, CL_FALSE, 0, num_groups * sizeof(float), Y);
    for(int i=0; i< ARRAY_SIZE; i++){
        soma2+=X[i];
    }
    //std::cout << "Soma2 = " << soma2 << std::endl << std::endl;
    //QueryPerformanceCounter(&start);
    for(int i = 0; i < num_groups; i++){
        soma+=Y[i];
    }

    //QueryPerformanceCounter(&eNd);

    std::cout << std::endl;
    std::cout << "Soma = " << soma << std::endl;
    //std::cout << "Tempo soma sequencial = " << (double)(eNd.QuadPart - start.QuadPart)/frequency.QuadPart  << std::endl;

    //cmdQueueCPU.enqueueReadBuffer(bufferX, CL_FALSE, 0, ARRAY_SIZE * sizeof(float), X);
    //cmdQueue.enqueueReadBuffer(bufferY, CL_FALSE, 0, num_groups * sizeof(float), Y);
    //cmdQueue.enqueueReadBuffer(bufferZ, CL_FALSE, 0, sizeof(float), Z);


    cl_ulong inicio1, fim1, inicio2, fim2;
    double tempoExecucao1, tempoExecucao2;

    e_tempo1.getProfilingInfo(CL_PROFILING_COMMAND_START, &inicio1);
    e_tempo1.getProfilingInfo(CL_PROFILING_COMMAND_END, &fim1);
    tempoExecucao1 = (fim1-inicio1)/1.0E9;
    std::cout << "tempo de Execucao 1 = " << tempoExecucao1 << std::endl;

    e_tempo2.getProfilingInfo(CL_PROFILING_COMMAND_START, &inicio2);
    e_tempo2.getProfilingInfo(CL_PROFILING_COMMAND_END, &fim2);
    tempoExecucao2 = (fim2-inicio2)/1.0E9;
    std::cout << "tempo de Execucao 2 = " << tempoExecucao2 << std::endl;


    float somaFinal = 0;
    for(int i = 0; i < ARRAY_SIZE; i++) somaFinal+=X[i];
    std::cout << std::endl;
    std::cout << "Y = ";
    for(int i = 0; i < num_groups; i++) std::cout << (float)Y[i] << " ";
    std::cout << std::endl;

    std::cout << "Z = "<< Z[0] << std::endl;
    if(somaFinal == Z[0]){
        std::cout << "Valor Correto!" << std::endl;
    } else {
        std::cout << "Valor Errado!" << std::endl;
    }

    delete [] X, Y, Z;
    return 0;
}
