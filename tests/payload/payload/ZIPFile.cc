#include <helpers/FakeStorage.hh>
#include <lest.hpp>
#include <payload/ZIPFile.hh>

#include <array>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("ZIPFile") {
    auto readFile = [&lest_env](ZIPFile &zipFile, const char *path,
                            const std::vector<u8> &expectedFile) {
        ZIPFile::Reader reader(zipFile, path);
        EXPECT(reader.ok());
        EXPECT(reader.cdNode()->uncompressedSize == expectedFile.size());
        EXPECT(std::string(reader.cdNode()->path.values()) == path + 1);
        EXPECT(reader.localNode()->uncompressedSize == expectedFile.size());
        EXPECT(std::string(reader.localNode()->path.values()) == path + 1);
        EXPECT(*reader.size() == expectedFile.size());
        std::vector<u8> actualFile;
        for (u32 offset = 0; offset < *reader.size();) {
            const u8 *chunk;
            u32 chunkSize;
            EXPECT(reader.read(chunk, chunkSize));
            actualFile.insert(actualFile.end(), chunk, chunk + chunkSize);
            offset += chunkSize;
        }
        EXPECT(actualFile == expectedFile);
    };

    auto writeFile = [&lest_env](ZIPFile &zipFile, const char *path,
                             const std::vector<u8> &expectedFile) {
        ZIPFile::Writer writer(zipFile, path, expectedFile.size());
        EXPECT(writer.ok());
        EXPECT(writer.write(expectedFile.data(), expectedFile.size()));
        EXPECT(writer.cdNode()->uncompressedSize == expectedFile.size());
        EXPECT(std::string(writer.cdNode()->path.values()) == path + 1);
        EXPECT(writer.localNode()->uncompressedSize == expectedFile.size());
        EXPECT(std::string(writer.localNode()->path.values()) == path + 1);
    };

    const char *zipPath = "fake:/archive.zip";
    std::vector<u8> b{0x11, 0x22, 0x33, 0x44, 0x55};
    std::vector<u8> c{0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00};
    std::vector<u8> d{0x66, 0x77, 0x88};

    SETUP("0 files") {
        FakeStorage fakeStorage;
        std::array<u8, 0x16> zip{0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT(Storage::WriteFile(zipPath, zip.data(), zip.size(), Storage::Mode::WriteNew));

        std::optional<ZIPFile> zipFile;
        zipFile.emplace(zipPath);
        EXPECT(zipFile->ok());

        SECTION("Read a") {
            EXPECT(zipFile->eocd()->cdNodeCount == 0);
            ZIPFile::Reader reader(*zipFile, "/a");
            EXPECT_NOT(reader.ok());
            EXPECT_NOT(reader.cdNode());
            EXPECT_NOT(reader.localNode());
            EXPECT_NOT(reader.size());
            const u8 *buffer;
            u32 size;
            EXPECT_NOT(reader.read(buffer, size));
        }

        SECTION("Write b") {
            writeFile(*zipFile, "/b", b);
            EXPECT(zipFile->eocd()->cdNodeCount == 1);
            readFile(*zipFile, "/b", b);
            zipFile.reset();
            zipFile.emplace(zipPath);
            EXPECT(zipFile->ok());
            EXPECT(zipFile->eocd()->cdNodeCount == 1);
            readFile(*zipFile, "/b", b);
        }
    }

    SETUP("1 compressed file") {
        FakeStorage fakeStorage;
        std::array<u8, 0x6a> zip{0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x58,
                0xba, 0x99, 0x57, 0x5b, 0xa8, 0x0c, 0x60, 0x06, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00,
                0x00, 0x01, 0x00, 0x00, 0x00, 0x63, 0x63, 0x10, 0x64, 0x80, 0x41, 0x00, 0x50, 0x4b,
                0x01, 0x02, 0x1e, 0x03, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x58, 0xba, 0x99, 0x57,
                0x5b, 0xa8, 0x0c, 0x60, 0x06, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x01, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x81, 0x00, 0x00,
                0x00, 0x00, 0x63, 0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
                0x00, 0x2f, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT(Storage::WriteFile(zipPath, zip.data(), zip.size(), Storage::Mode::WriteNew));

        std::optional<ZIPFile> zipFile;
        zipFile.emplace(zipPath);
        EXPECT(zipFile->ok());

        SECTION("Read c") {
            EXPECT(zipFile->eocd()->cdNodeCount == 1);
            readFile(*zipFile, "/c", c);
        }

        SECTION("Write b") {
            writeFile(*zipFile, "/b", b);
            EXPECT(zipFile->eocd()->cdNodeCount == 2);
            readFile(*zipFile, "/b", b);
            readFile(*zipFile, "/c", c);
            zipFile.reset();
            zipFile.emplace(zipPath);
            EXPECT(zipFile->ok());
            EXPECT(zipFile->eocd()->cdNodeCount == 2);
            readFile(*zipFile, "/b", b);
            readFile(*zipFile, "/c", c);
        }
    }

    SETUP("1 uncompressed file") {
        FakeStorage fakeStorage;
        std::array<u8, 0x67> zip{0x50, 0x4b, 0x03, 0x04, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e,
                0xbc, 0x99, 0x57, 0xa3, 0x8a, 0x06, 0x30, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
                0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x66, 0x77, 0x88, 0x50, 0x4b, 0x01, 0x02, 0x1e,
                0x03, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0xbc, 0x99, 0x57, 0xa3, 0x8a, 0x06,
                0x30, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xa4, 0x81, 0x00, 0x00, 0x00, 0x00, 0x64,
                0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x2f, 0x00,
                0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT(Storage::WriteFile(zipPath, zip.data(), zip.size(), Storage::Mode::WriteNew));

        std::optional<ZIPFile> zipFile;
        zipFile.emplace(zipPath);
        EXPECT(zipFile->ok());

        SECTION("Read d") {
            EXPECT(zipFile->eocd()->cdNodeCount == 1);
            readFile(*zipFile, "/d", d);
        }

        SECTION("Write b") {
            writeFile(*zipFile, "/b", b);
            EXPECT(zipFile->eocd()->cdNodeCount == 2);
            readFile(*zipFile, "/b", b);
            readFile(*zipFile, "/d", d);
            zipFile.reset();
            zipFile.emplace(zipPath);
            EXPECT(zipFile->ok());
            EXPECT(zipFile->eocd()->cdNodeCount == 2);
            readFile(*zipFile, "/b", b);
            readFile(*zipFile, "/d", d);
        }
    }
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
