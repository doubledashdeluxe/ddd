#include "MotorManager.hh"

MotorManager *MotorManager::Instance() {
    return s_instance;
}
