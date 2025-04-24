#pragma once

#include <portable/ZIP.hh>

#include <vector>

class FakeZIP final : public ZIP {
public:
    FakeZIP(std::vector<u8> &data);
    ~FakeZIP();

private:
    bool read(void *dst, u32 size, u32 offset) override;
    bool write(const void *src, u32 size, u32 offset) override;
    bool truncate(u64 size) override;
    bool size(u64 &size) override;
    u32 getDOSTime() override;

    std::vector<u8> &m_data;
};
