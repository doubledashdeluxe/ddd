#pragma once

class MemoryProtection {
public:
    static void Finalize();

private:
    static void FinalizeBATs();
    static void FinalizeMI();

    MemoryProtection();

    static bool s_isFinalized;
};
