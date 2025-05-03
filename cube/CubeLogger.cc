#include "CubeLogger.hh"

#include "cube/Arena.hh"

void CubeLogger::Init() {
    s_instance = new (MEM1Arena::Instance(), -0x4) CubeLogger;
}

CubeLogger::CubeLogger() {}

CubeLogger *CubeLogger::s_instance;
