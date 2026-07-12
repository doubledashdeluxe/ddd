#include "ChannelLauncher.hh"

#include "cube/ES.hh"

#include <portable/Bytes.hh>
#include <portable/Log.hh>

extern "C" {
#include <string.h>
}

void ChannelLauncher::Launch() {
    INFO("Launching the channel...");
    ES es;
    if (!es.ok()) {
        return;
    }
    Array<u8, 0xd8> ticketView(0x00);
    Bytes::WriteBE<u64>(ticketView.values(), 0x10, 0x0001000844444443); // Title ID
    Bytes::WriteBE<u16>(ticketView.values(), 0x18, 0xffff);             // Access mask
    memset(ticketView.values() + 0x56, 0xff, 0x200 / 8);                // Content access mask
    if (!es.launchTitle(0x0001000844444443, ticketView)) {
        return;
    }
    while (true) {}
}
