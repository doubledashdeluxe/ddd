#include <lest.hpp>
#include <payload/SZSReader.hh>

#include <vector>

class SZSLoader : public SZSReader {
public:
    SZSLoader(const std::vector<u8> *compressed) : m_compressed(compressed) {}

    ~SZSLoader() = default;

    bool read() {
        return SZSReader::read();
    }

    u32 uncompressedSize() const {
        return m_uncompressedSize;
    }

    const std::vector<u8> &uncompressed() const {
        return m_uncompressed;
    }

private:
    bool setSize(u32 size) override {
        m_uncompressedSize = size;
        return true;
    }

    bool read(const u8 *&buffer, u32 &size) override {
        if (!m_compressed) {
            return false;
        }
        buffer = m_compressed->data();
        size = m_compressed->size();
        m_compressed = nullptr;
        return true;
    }

    bool write(const u8 *buffer, u32 size) override {
        m_uncompressed.insert(m_uncompressed.end(), buffer, buffer + size);
        return true;
    }

    const std::vector<u8> *m_compressed;
    u32 m_uncompressedSize;
    std::vector<u8> m_uncompressed;
};

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("SZS") {
    SETUP("Incomplete header") {
        std::vector<u8> compressed{0x59, 0x61, 0x7a, 0x30, 0x00};

        SECTION("read") {
            SZSLoader loader(&compressed);
            EXPECT_NOT(loader.read());
        }
    }

    SETUP("Wrong magic") {
        std::vector<u8> compressed{0x59, 0x61, 0x79, 0x30, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x11, 0x70, 0x01};

        SECTION("read") {
            SZSLoader loader(&compressed);
            EXPECT_NOT(loader.read());
        }
    }

    SETUP("Correct file") {
        std::vector<u8> compressed{0x59, 0x61, 0x7a, 0x30, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x11, 0x70, 0x01};

        SECTION("read") {
            SZSLoader loader(&compressed);
            EXPECT(loader.read());
            EXPECT(loader.uncompressedSize() == 0xb);
            for (u32 i = 0; i < loader.uncompressedSize(); i++) {
                EXPECT(loader.uncompressed()[i] == 0x11 * (i % 0x2));
            }
        }
    }
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
