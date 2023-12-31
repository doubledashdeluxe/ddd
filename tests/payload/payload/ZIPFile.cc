#include <helpers/FakeStorage.hh>
#include <lest.hpp>
#include <payload/ZIPFile.hh>

#include <cstring>
#include <optional>

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("ZIPFile") {
    auto readFile = [&lest_env](ZIPFile &zipFile, const char *path, u8 *expectedFile,
                            size_t expectedFileSize) {
        ZIPFile::CDNode cdNode;
        ZIPFile::LocalNode localNode;
        UniquePtr<u8[]> actualFile(zipFile.readFile(path, nullptr, 0, cdNode, localNode));
        EXPECT(actualFile.get());
        EXPECT(!memcmp(actualFile.get(), expectedFile, expectedFileSize));
        EXPECT(cdNode.uncompressedSize == expectedFileSize);
        EXPECT(localNode.uncompressedSize == expectedFileSize);
        EXPECT(std::string(cdNode.path.get()) == path + 1);
        EXPECT(std::string(localNode.path.get()) == path + 1);
    };

    auto writeFile = [&lest_env](ZIPFile &zipFile, const char *path, const u8 *expectedFile,
                             size_t expectedFileSize) {
        ZIPFile::CDNode cdNode;
        ZIPFile::LocalNode localNode;
        EXPECT(zipFile.writeFile(path, expectedFile, expectedFileSize, nullptr, 0, cdNode,
                localNode));
        EXPECT(cdNode.uncompressedSize == expectedFileSize);
        EXPECT(localNode.uncompressedSize == expectedFileSize);
        EXPECT(std::string(cdNode.path.get()) == path + 1);
        EXPECT(std::string(localNode.path.get()) == path + 1);
    };

    const char *zipPath = "fake:/archive.zip";
    std::array<u8, 0x5> b{0x11, 0x22, 0x33, 0x44, 0x55};
    std::array<u8, 0xb> c{0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00};
    std::array<u8, 0x3> d{0x66, 0x77, 0x88};

    SETUP("0 files") {
        FakeStorage fakeStorage;
        std::array<u8, 0x16> zip{0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT(Storage::WriteFile(zipPath, zip.data(), zip.size(), Storage::Mode::WriteNew));

        bool ok;
        std::optional<ZIPFile> zipFile;
        zipFile.emplace(zipPath, ok);
        EXPECT(ok);

        ZIPFile::CDNode cdNode;
        ZIPFile::LocalNode localNode;

        SECTION("Read a") {
            EXPECT(zipFile->eocd().cdNodeCount == 0);
            UniquePtr<u8[]> actualFile(zipFile->readFile("/a", nullptr, 0, cdNode, localNode));
            EXPECT(!actualFile.get());
        }

        SECTION("Write b") {
            writeFile(*zipFile, "/b", b.data(), b.size());
            EXPECT(zipFile->eocd().cdNodeCount == 1);
            readFile(*zipFile, "/b", b.data(), b.size());
            zipFile.reset();
            zipFile.emplace(zipPath, ok);
            EXPECT(ok);
            EXPECT(zipFile->eocd().cdNodeCount == 1);
            readFile(*zipFile, "/b", b.data(), b.size());
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

        bool ok;
        std::optional<ZIPFile> zipFile;
        zipFile.emplace(zipPath, ok);
        EXPECT(ok);

        ZIPFile::CDNode cdNode;
        ZIPFile::LocalNode localNode;

        SECTION("Read c") {
            EXPECT(zipFile->eocd().cdNodeCount == 1);
            readFile(*zipFile, "/c", c.data(), c.size());
        }

        SECTION("Write b") {
            writeFile(*zipFile, "/b", b.data(), b.size());
            EXPECT(zipFile->eocd().cdNodeCount == 2);
            readFile(*zipFile, "/b", b.data(), b.size());
            readFile(*zipFile, "/c", c.data(), c.size());
            zipFile.reset();
            zipFile.emplace(zipPath, ok);
            EXPECT(ok);
            EXPECT(zipFile->eocd().cdNodeCount == 2);
            readFile(*zipFile, "/b", b.data(), b.size());
            readFile(*zipFile, "/c", c.data(), c.size());
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

        bool ok;
        std::optional<ZIPFile> zipFile;
        zipFile.emplace(zipPath, ok);
        EXPECT(ok);

        ZIPFile::CDNode cdNode;
        ZIPFile::LocalNode localNode;

        SECTION("Read d") {
            EXPECT(zipFile->eocd().cdNodeCount == 1);
            readFile(*zipFile, "/d", d.data(), d.size());
        }

        SECTION("Write b") {
            writeFile(*zipFile, "/b", b.data(), b.size());
            EXPECT(zipFile->eocd().cdNodeCount == 2);
            readFile(*zipFile, "/b", b.data(), b.size());
            readFile(*zipFile, "/d", d.data(), d.size());
            zipFile.reset();
            zipFile.emplace(zipPath, ok);
            EXPECT(ok);
            EXPECT(zipFile->eocd().cdNodeCount == 2);
            readFile(*zipFile, "/b", b.data(), b.size());
            readFile(*zipFile, "/d", d.data(), d.size());
        }
    }
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
