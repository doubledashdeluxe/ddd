#include <portable/SZSReader.hh>

#include <vector>

class FakeSZSReader : public SZSReader {
public:
    FakeSZSReader(const std::vector<u8> *compressed);
    ~FakeSZSReader();
    bool read();
    u32 uncompressedSize() const;
    const std::vector<u8> &uncompressed() const;

private:
    bool setSize(u32 size) override;
    bool read(const u8 *&buffer, u32 &size) override;
    bool write(const u8 *buffer, u32 size) override;

    const std::vector<u8> *m_compressed;
    u32 m_uncompressedSize;
    std::vector<u8> m_uncompressed;
};
