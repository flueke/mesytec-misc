
// CLI tool for decoding data from mesytec VME modules.
// Author: Florian Lüke <f.lueke@mesytec.com>
// License: BSL-1.0
//
// - Reads lines containing a single data word from stdin until EOF.
// - Decodes each data word and prints out relevant information.
// - Based on 'Decode the data structure' from the ANN001_using_mesytec_VME_modules document
//   (https://www.mesytec.com/products/appnotes/AN001_using_mesytec_VME_modules.pdf)
// - Might need to be updated for newer modules/streaming mode/sampling mode, etc.
//
// Example:
// echo "0x40010c07 0x10100868 0x1000036e 0x103002aa 0x10110868 0x10010417 0x10310317 0xc18d01bd" | ./decode_mesytec_vme_data
//   0x40010c07 module_header, module_id=0x01, module_setting=0x3, data_length=7 words
//   0x10100868 data_word, channel_address=16, mdpp_flags=0x0
//   0x1000036e data_word, channel_address= 0, mdpp_flags=0x0
//   0x103002aa data_word, channel_address=48, mdpp_flags=0x0
//   0x10110868 data_word, channel_address=17, mdpp_flags=0x0
//   0x10010417 data_word, channel_address= 1, mdpp_flags=0x0
//   0x10310317 data_word, channel_address=49, mdpp_flags=0x0
//   0xc18d01bd end_of_event, low_stamp=26018237
//
// Alternatively the tool can be started and data words can be pasted into the terminal.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

struct DecodedVMEDataWord
{
    std::uint32_t word;

    bool header_found: 1;
    bool data_found: 1;
    bool extended_ts_found: 1;
    bool fill_word_found: 1;
    bool eoe_found: 1;

    // header
    std::uint16_t data_length;
    std::uint8_t module_setting;
    std::uint8_t  module_id;

    // data word
    std::uint8_t channel_address;
    std::uint8_t mdpp_flags;

    // extended timestamp
    std::uint16_t high_stamp;

    // EndOfEvent
    std::uint32_t low_stamp;
};

DecodedVMEDataWord decode_mesytec_vme_word(std::uint32_t word)
{
    DecodedVMEDataWord result = {};
    result.word = word;

    result.header_found = ((word & 0xC0000000) == 0x40000000);
    result.data_found = (((word & 0xF0000000) == 0x10000000) // MDPP
                         || ((word & 0xFF800000) == 0x04000000)); // MxDC
    result.extended_ts_found = ((word & 0xFF800000)==0x04800000);
    result.fill_word_found = (word == 0x00000000);
    result.eoe_found = ((word & 0xC0000000) == 0xC0000000);

    if (result.header_found)
    {
        result.data_length = (word & 0x000003FF);
        result.module_id = (word & 0x00FF0000) / 0x10000;
        result.module_setting = (word & 0x0000FC00) / 0x400;
    }

    if (result.data_found)
    {
        result.channel_address = (word & 0x003F0000) / 0x0010000;
        result.mdpp_flags = (word & 0x0FC00000) / 0x0040000;
    }

    if (result.extended_ts_found)
        result.high_stamp = (word & 0x0000FFFF);

    if (result.eoe_found)
        result.low_stamp = (word & 0x3FFFFFFF);

    return result;
}

template<typename Out>
void print_decoded_vme_word(Out &out, const DecodedVMEDataWord &dw)
{
    fprintf(out, "0x%08x ", dw.word);


    if (dw.header_found)
    {
        fprintf(out, "module_header, module_id=0x%02x, module_setting=0x%x, data_length=%d words",
               dw.module_id, dw.module_setting, dw.data_length);
    }

    else if (dw.data_found)
        fprintf(out, "data_word, channel_address=%2d, mdpp_flags=0x%x",
               dw.channel_address, dw.mdpp_flags);

    else if (dw.extended_ts_found)
        fprintf(out, "extended_ts, high_stamp=%d", dw.high_stamp);

    else if (dw.eoe_found)
        fprintf(out, "end_of_event, low_stamp=%d", dw.low_stamp);

    else if (dw.fill_word_found)
        fprintf(out, "fill_word");

    fprintf(out, "\n");
}

int main()
{
    // Remove all default settings for std::cin => number prefixes are interpreted properly
    std::cin.unsetf(std::ios::dec);
    std::cin.unsetf(std::ios::hex);
    std::cin.unsetf(std::ios::oct);

    std::uint32_t data_word;

    while (std::cin >> data_word)
    {
        auto decoded = decode_mesytec_vme_word(data_word);
        print_decoded_vme_word(stdout, decoded);
    }
}
