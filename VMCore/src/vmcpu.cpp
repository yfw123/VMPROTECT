#include "../include/vmcpu.hpp"

#ifdef _WIN32_DEV_ENVIRONMENT
    #include <string>
#endif

// #include <bitset>

VMCPU::VMCPU()
{
    AS = (PADDRESS_SPACE) new ADDRESS_SPACE;
    REGS = (PREGISTERSS) new REGISTERSS;

    memset(AS->codeData, 0, sizeof(AS->codeData));

    REGS->PC = 0;
    REGS->SP = sizeof(AS->stack) / sizeof(VDWORD);

    areFramesNeeded = false;
    currentFrameNumber = 0;
    isError = false;

    #ifdef _WIN32_DEV_ENVIRONMENT
        sysBus = new WIN32();
    #else //_LINUX_DEV_ENVIRONMENT
        sysBus = new UNIX();
    #endif
}

VMCPU::~VMCPU()
{
    if(areFramesNeeded)
    {
        std::string fileNameToRemove;
        for (const auto& [key, value] : frameMap) 
        {
            fileNameToRemove = "./.cached." + std::to_string(key) + ".frame";
            int retVal = sysBus->deleteFile(fileNameToRemove);
        }
    }
    delete AS;
    delete REGS;
    delete sysBus;
    dOpcodesFunction.clear();
}

bool VMCPU::loadCode(VBYTE *mcode, int mcsize)
{
    memset(AS->codeData, 0, CODE_DATA_SIZE*sizeof(*(AS->codeData)));
    memset(AS->stack, 0, STACK_SIZE*sizeof(*(AS->stack)));
    memset(AS->dataBuffer, 0, INPUT_BUFFER_SIZE*sizeof(*(AS->dataBuffer)));
    if((unsigned) (mcsize) > (sizeof(AS->codeData) / sizeof(AS->codeData[0]))) 
    {
        std::cout << "[ERROR 101001] TOO BIG A CODE TO EXECUTE!\n";
        return false;
    }
    memcpy(AS->codeData, mcode, mcsize);
    for(int i = 0; i < 8; i++)
    {
        REGS->R[i] = (VDWORD) 0;
    }
    REGS->CF = 0;
    REGS->ZF = 0;
    return true;
}

void VMCPU::run()
{
    bool exit = false;
    VBYTE opcode;

    while(!exit)
    {
        if(areFramesNeeded && (REGS->PC >= frameMap[currentFrameNumber])) REGS->PC = loadFrame(REGS->PC);
        if(isError) return;
        opcode = AS->codeData[REGS->PC++];
        exit = executer(opcode);
    }
    return;
}