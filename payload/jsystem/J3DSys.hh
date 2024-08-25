#pragma once

class J3DSys {
public:
    void drawInit();

    static J3DSys &Instance();

private:
    static J3DSys s_instance;
};
