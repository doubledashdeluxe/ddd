#pragma once

#include "portable/Array.hh"
#include "portable/Bytes.hh"
#include "portable/Optional.hh"
#include "portable/Ring.hh"
#include "portable/network/UDPSocket.hh"

extern "C" {
#include <stdio.h>
#include <string.h>
}

class DNS {
public:
    enum {
        A = 1,
        SRV = 33,
    };

    typedef Array<char, 80> Name;
    typedef Array<u32, 2> Resolvers;

    struct Target {
        u16 port;
        Name name;
    };

    bool resolveA(const Resolvers &resolvers, const char *name, Optional<u32> &address);
    bool resolveSRV(const Resolvers &resolvers, const char *name, Optional<Target> &target);

    static bool ReadName(const u8 *buffer, u32 size, u32 &offset, Name &name, u32 &nameLength);
    static bool WriteName(u8 *buffer, u32 size, u32 &offset, const Name &name, u32 &nameLength);

protected:
    DNS(UDPSocket &socket);
    ~DNS();

private:
    template <typename D>
    struct Entry {
        s64 expirationTime;
        u16 id;
        Name name;
        Optional<Optional<D>> data;
    };

    typedef Entry<u32> AEntry;
    typedef Entry<Target> SRVEntry;

    virtual s64 secondsToTicks(s64 seconds) = 0;
    virtual s64 getMonotonicTicks() = 0;

    template <typename D>
    bool resolve(const Resolvers &resolvers, const char *name, Optional<D> &data,
            Ring<Entry<D>, 48> &entries, u16 qtype) {
        checkSocket();
        checkResolvers(resolvers);

        s64 now = getMonotonicTicks();
        readResponse(now);

        for (u32 i = 0; i < entries.count(); i++) {
            if (entries[i].expirationTime < now) {
                entries.swapRemoveFront(i);
                continue;
            }

            if (strcmp(entries[i].name.values(), name)) {
                continue;
            }

            if (entries[i].data) {
                data = *entries[i].data;
                return true;
            } else {
                return false;
            }
        }

        if (entries.full()) {
            entries.popFront();
        }
        entries.pushBack();
        Entry<D> *entry = entries.back();
        entry->id = m_id;
        entry->expirationTime = now + secondsToTicks(2);
        u32 nameLength = snprintf(entry->name.values(), entry->name.count(), "%s", name);
        if (nameLength >= entry->name.count()) {
            return false;
        }
        if (writeQuery(entry->name, qtype)) {
            m_id++;
        }
        return false;
    }

    void checkSocket();
    void checkResolvers(const Resolvers &resolvers);
    void resetEntries();
    bool readResponse(s64 now);
    bool hasResolver(const Address &resolver) const;
    bool readResponse(const u8 *buffer, u32 offset, AEntry &entry, u16 rdlength);
    bool readResponse(const u8 *buffer, u32 offset, SRVEntry &entry, u16 rdlength);
    bool writeQuery(const Name &name, u16 qtype);

    template <typename D>
    bool readResponse(s64 now, const u8 *buffer, u32 size, u32 offset, u16 id, u16 ancount,
            Ring<Entry<D>, 48> &entries) {
        Entry<D> *entry = FindQuery(entries, id);
        if (!entry) {
            return false;
        }

        if (ancount == 0) {
            entry->expirationTime = now + secondsToTicks(3600);
            entry->data.emplace();
            return true;
        }

        Name name;
        u32 nameLength;
        if (!ReadName(buffer, size, offset, name, nameLength)) {
            return false;
        }

        if (offset + 0x00a > size) {
            return false;
        }
        u32 ttl = Max<u32>(Bytes::ReadBE<u32>(buffer, offset + 0x004), 5);
        u16 rdlength = Bytes::ReadBE<u16>(buffer, offset + 0x008);
        offset += 0x00a;

        entry->expirationTime = now + secondsToTicks(ttl);
        if (offset + rdlength > size) {
            return false;
        }
        return readResponse(buffer, offset, *entry, rdlength);
    }

    template <typename D>
    static void ResetQueries(Ring<Entry<D>, 48> &entries) {
        for (u32 i = 0; i < entries.count(); i++) {
            if (!entries[i].data) {
                entries.swapRemoveFront(i);
            }
        }
    }

    template <typename D>
    static Entry<D> *FindQuery(Ring<Entry<D>, 48> &entries, u16 id) {
        for (u32 i = 0; i < entries.count(); i++) {
            Entry<D> &entry = entries[i];
            if (!entry.data && entry.id == id) {
                return &entry;
            }
        }
        return nullptr;
    }

    UDPSocket &m_socket;
    Resolvers m_resolvers;
    u16 m_id;
    Ring<AEntry, 48> m_aEntries;
    Ring<SRVEntry, 48> m_srvEntries;
};
