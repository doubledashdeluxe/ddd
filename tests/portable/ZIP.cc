#include <native/FakeZIP.hh>
#include <snitch/snitch_all.hpp>

#include <optional>
#include <string>
#include <vector>

TEST_CASE("ZIPFile") {
    auto readFile = [&](ZIP &zip, const char *path, const std::vector<u8> &expectedFile) {
        ZIP::Reader reader(zip, path);
        CHECK(reader.ok());
        CHECK(reader.cdNode()->uncompressedSize == expectedFile.size());
        CHECK(std::string(reader.cdNode()->path.values()) == path + 1);
        CHECK(reader.localNode()->uncompressedSize == expectedFile.size());
        CHECK(std::string(reader.localNode()->path.values()) == path + 1);
        CHECK(*reader.size() == expectedFile.size());
        std::vector<u8> actualFile;
        for (u32 offset = 0; offset < *reader.size();) {
            const u8 *chunk;
            u32 chunkSize;
            CHECK(reader.read(chunk, chunkSize));
            actualFile.insert(actualFile.end(), chunk, chunk + chunkSize);
            offset += chunkSize;
        }
        CHECK(actualFile == expectedFile);
    };

    auto writeFile = [&](ZIP &zip, const char *path, const std::vector<u8> &expectedFile) {
        ZIP::Writer writer(zip, path, expectedFile.size());
        CHECK(writer.ok());
        CHECK(writer.write(expectedFile.data(), expectedFile.size()));
        CHECK(writer.cdNode()->uncompressedSize == expectedFile.size());
        CHECK(std::string(writer.cdNode()->path.values()) == path + 1);
        CHECK(writer.localNode()->uncompressedSize == expectedFile.size());
        CHECK(std::string(writer.localNode()->path.values()) == path + 1);
    };

    std::vector<u8> b{0x11, 0x22, 0x33, 0x44, 0x55};
    std::vector<u8> c{0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00};
    std::vector<u8> d{0x66, 0x77, 0x88};

    SECTION("0 files") {
        std::vector<u8> data{0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        std::optional<FakeZIP> zip;
        zip.emplace(data);
        CHECK(zip->ok());

        SECTION("Read a") {
            CHECK(zip->eocd()->cdNodeCount == 0);
            ZIP::Reader reader(*zip, "/a");
            CHECK_FALSE(reader.ok());
            CHECK_FALSE(reader.cdNode());
            CHECK_FALSE(reader.localNode());
            CHECK_FALSE(reader.size());
            const u8 *buffer;
            u32 size;
            CHECK_FALSE(reader.read(buffer, size));
        }

        SECTION("Write b") {
            writeFile(*zip, "/b", b);
            CHECK(zip->eocd()->cdNodeCount == 1);
            readFile(*zip, "/b", b);
            zip.reset();
            zip.emplace(data);
            CHECK(zip->ok());
            CHECK(zip->eocd()->cdNodeCount == 1);
            readFile(*zip, "/b", b);
        }
    }

    SECTION("1 compressed file") {
        std::vector<u8> data{0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x58, 0xba,
                0x99, 0x57, 0x5b, 0xa8, 0x0c, 0x60, 0x06, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x63, 0x63, 0x10, 0x64, 0x80, 0x41, 0x00, 0x50, 0x4b, 0x01,
                0x02, 0x1e, 0x03, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x58, 0xba, 0x99, 0x57, 0x5b,
                0xa8, 0x0c, 0x60, 0x06, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x81, 0x00, 0x00, 0x00,
                0x00, 0x63, 0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
                0x2f, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};

        std::optional<FakeZIP> zip;
        zip.emplace(data);
        CHECK(zip->ok());

        SECTION("Read c") {
            CHECK(zip->eocd()->cdNodeCount == 1);
            readFile(*zip, "/c", c);
        }

        SECTION("Write b") {
            writeFile(*zip, "/b", b);
            CHECK(zip->eocd()->cdNodeCount == 2);
            readFile(*zip, "/b", b);
            readFile(*zip, "/c", c);
            zip.reset();
            zip.emplace(data);
            CHECK(zip->ok());
            CHECK(zip->eocd()->cdNodeCount == 2);
            readFile(*zip, "/b", b);
            readFile(*zip, "/c", c);
        }
    }

    SECTION("1 uncompressed file") {
        std::vector<u8> data{0x50, 0x4b, 0x03, 0x04, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0xbc,
                0x99, 0x57, 0xa3, 0x8a, 0x06, 0x30, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x64, 0x66, 0x77, 0x88, 0x50, 0x4b, 0x01, 0x02, 0x1e, 0x03,
                0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0xbc, 0x99, 0x57, 0xa3, 0x8a, 0x06, 0x30,
                0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xa4, 0x81, 0x00, 0x00, 0x00, 0x00, 0x64, 0x50,
                0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x2f, 0x00, 0x00,
                0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00};

        std::optional<FakeZIP> zip;
        zip.emplace(data);
        CHECK(zip->ok());

        SECTION("Read d") {
            CHECK(zip->eocd()->cdNodeCount == 1);
            readFile(*zip, "/d", d);
        }

        SECTION("Write b") {
            writeFile(*zip, "/b", b);
            CHECK(zip->eocd()->cdNodeCount == 2);
            readFile(*zip, "/b", b);
            readFile(*zip, "/d", d);
            zip.reset();
            zip.emplace(data);
            CHECK(zip->ok());
            CHECK(zip->eocd()->cdNodeCount == 2);
            readFile(*zip, "/b", b);
            readFile(*zip, "/d", d);
        }
    }
}
