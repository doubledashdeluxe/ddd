#pragma once

#include "loader/Apploader.hh"

class Loader {
public:
    static Apploader::GameEntryFunc Run();

private:
    typedef void (*PayloadEntryFunc)();

    Loader();
};
