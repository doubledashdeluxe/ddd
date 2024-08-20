#include "ChannelBinary.hh"

extern "C" u8 channel_text_start[];
extern "C" u8 channel_text_end[];
extern "C" u8 channel_ctors_start[];
extern "C" u8 channel_ctors_end[];
extern "C" u8 channel_dtors_start[];
extern "C" u8 channel_dtors_end[];
extern "C" u8 channel_rodata_start[];
extern "C" u8 channel_rodata_end[];
extern "C" u8 channel_data_start[];
extern "C" u8 channel_data_end[];
extern "C" u8 channel_bss_start[];
extern "C" u8 channel_bss_end[];

void *ChannelBinary::Start() {
    return channel_text_start;
}

void *ChannelBinary::End() {
    return channel_bss_end;
}

size_t ChannelBinary::Size() {
    return channel_bss_end - channel_text_start;
}

void *ChannelBinary::TextSectionStart() {
    return channel_text_start;
}

void *ChannelBinary::TextSectionEnd() {
    return channel_text_end;
}

size_t ChannelBinary::TextSectionSize() {
    return channel_text_end - channel_text_start;
}

void *ChannelBinary::CtorsSectionStart() {
    return channel_ctors_start;
}

void *ChannelBinary::CtorsSectionEnd() {
    return channel_ctors_end;
}

size_t ChannelBinary::CtorsSectionSize() {
    return channel_ctors_end - channel_ctors_start;
}

void *ChannelBinary::DtorsSectionStart() {
    return channel_dtors_start;
}

void *ChannelBinary::DtorsSectionEnd() {
    return channel_dtors_end;
}

size_t ChannelBinary::DtorsSectionSize() {
    return channel_dtors_end - channel_dtors_start;
}

void *ChannelBinary::RodataSectionStart() {
    return channel_rodata_start;
}

void *ChannelBinary::RodataSectionEnd() {
    return channel_rodata_end;
}

size_t ChannelBinary::RodataSectionSize() {
    return channel_rodata_end - channel_rodata_start;
}

void *ChannelBinary::DataSectionStart() {
    return channel_data_start;
}

void *ChannelBinary::DataSectionEnd() {
    return channel_data_end;
}

size_t ChannelBinary::DataSectionSize() {
    return channel_data_end - channel_data_start;
}

void *ChannelBinary::BssSectionStart() {
    return channel_bss_start;
}

void *ChannelBinary::BssSectionEnd() {
    return channel_bss_end;
}

size_t ChannelBinary::BssSectionSize() {
    return channel_bss_end - channel_bss_start;
}
