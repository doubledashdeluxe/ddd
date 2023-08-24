#include "JUTException.hh"

JUTExceptionHandler JUTException::setPreUserCallback(JUTExceptionHandler /* exceptionHandler */) {
    return REPLACED(setPreUserCallback)(nullptr);
}
