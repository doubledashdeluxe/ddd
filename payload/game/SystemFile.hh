#pragma once

#include "game/SaveFile.hh"

#include <payload/Replace.hh>

class SystemFile : public SaveFile {
public:
    void REPLACED(store)();
    REPLACE void store() override;
};
