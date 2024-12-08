#pragma once

#include <common/Array.hh>

class OnlineInfo {
public:
    void reset();

    static OnlineInfo &Instance();

    bool m_hasNames;
    bool m_hasIDs;
    Array<Array<u32, 2>, 4> m_characterIDs;
    Array<u32, 4> m_kartIDs;

private:
    OnlineInfo();

    static OnlineInfo s_instance;
};
