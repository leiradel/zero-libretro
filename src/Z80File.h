#pragma once

#include "Types.h"

#include <vector>

namespace rm {
    class Z80_SNAPSHOT
    {
    public:
        int TYPE;                //0 = 48k, 1 = 128k, 2 = +3, 3 = Pentagon 128k
        byte I;                  //I Register
        int HL_, DE_, BC_, AF_;  //Alternate registers
        int HL, DE, BC, IX, IY;  //16 bit main registers
        byte R;                  //R Register
        int AF, SP;              //AF and SP register
        byte IM;                 //Interupt Mode
        byte BORDER;             //Border colour
        int PC;                  //PC Register
        byte PORT_7FFD;          //Current state of port 7ffd
        byte PORT_FFFD;          //Current state of soundchip AY
        byte PORT_1FFD;          //Last out to port 1ffd (for +3)
        byte AY_REGS[16];          //Contents of AY registers
        bool IFF1;               //Are interrupts enabled?
        bool IFF2;
        bool ISSUE2;             //Issue 2 Keyboard?
        bool AY_FOR_48K;
        int TSTATES;
        byte RAM_BANK[16][8192];       //Contents of the 8192*16 ram banks
    };

    class Z80File
    {
    public:
        static void GetPage(byte const* buffer, int counter, byte* bank, int dataLength) {
            if (dataLength == 0xffff) {
                memcpy(bank, buffer + counter, 16384);
            } else //Compressed data (needs testing!)
            {
                int dataBlockOffset = counter;
                int memStart = 0;
                while ((counter - dataBlockOffset) < dataLength) {
                    byte bite = buffer[counter++];

                    if (bite == 0xED) {
                        int bite2 = buffer[counter];
                        if (bite2 == 0xED) {
                            counter++;
                            int dataSize = buffer[counter++];
                            byte data = buffer[counter++];

                            //compressed data
                            for (int f = 0; f < dataSize; f++) {
                                bank[memStart++] = data;
                            }
                            continue;
                        }
                        bank[memStart++] = bite;
                        continue;
                    } else
                        bank[memStart++] = bite;
                    //   dataCounter = counter - dataBlockOffset;
                }
            }
        }

        static bool LoadZ80(std::vector<byte> const& buffer, Z80_SNAPSHOT* snapshot) {
            if (buffer.size() == 0)
                return false; //something bad happened!

            snapshot->AF = (ushort)buffer[0] << 8;
            snapshot->AF |= buffer[1];
            snapshot->BC = (buffer[2] | ((ushort)buffer[3] << 8));
            snapshot->HL = (buffer[4] | ((ushort)buffer[5] << 8));
            snapshot->PC = (buffer[6] | ((ushort)buffer[7] << 8));
            snapshot->SP = (buffer[8] | ((ushort)buffer[9] << 8));
            snapshot->I = buffer[10];
            snapshot->R = buffer[11];

            byte byte12 = buffer[12];
            if (byte12 == 255)
                byte12 = 1;

            snapshot->R |= (byte)((byte12 & 0x01) << 7);
            snapshot->BORDER = (byte)((byte12 >> 1) & 0x07);
            bool isCompressed = ((byte12 & 0x20) != 0);

            snapshot->DE = (buffer[13] | (buffer[14] << 8));
            snapshot->BC_ = (buffer[15] | (buffer[16] << 8));
            snapshot->DE_ = (buffer[17] | (buffer[18] << 8));
            snapshot->HL_ = (buffer[19] | (buffer[20] << 8));
            snapshot->AF_ = ((buffer[21] << 8) | buffer[22]);

            snapshot->IY = (buffer[23] | (buffer[24] << 8));
            snapshot->IX = (buffer[25] | (buffer[26] << 8));

            snapshot->IFF1 = (buffer[27] != 0);
            snapshot->IFF2 = (buffer[28] != 0);

            byte byte29 = buffer[29];

            snapshot->IM = (byte)(byte29 & 0x3);
            snapshot->ISSUE2 = ((byte29 & 0x08) != 0);

            //Version 2 or 3
            if (snapshot->PC == 0) {
                int headerLength = buffer[30];
                snapshot->PC = (buffer[32] | ((ushort)buffer[33] << 8));
                switch (buffer[34]) {
                    case 0:
                    case 1:
                    snapshot->TYPE = 0;
                    break;

                    case 3:
                    if (headerLength == 23)
                        snapshot->TYPE = 1;
                    else
                        snapshot->TYPE = 0;
                    break;

                    case 4:
                    case 5:
                    case 6:
                    snapshot->TYPE = 1;
                    break;

                    case 7:
                    case 8:
                    snapshot->TYPE = 2;
                    break;

                    case 9:
                    snapshot->TYPE = 3;
                    break;
                }
                int counter = 32 + headerLength;

                //128K or Pentagon?
                // if ((snapshot.TYPE == 1) || (snapshot.TYPE == 3))
                {
                    snapshot->PORT_7FFD = buffer[35];
                    snapshot->AY_FOR_48K = (buffer[37] & 0x4) != 0;
                    snapshot->PORT_FFFD = buffer[38];
                    for (int f = 0; f < 16; f++)
                        snapshot->AY_REGS[f] = buffer[39 + f];
                }

                snapshot->TSTATES = 0;
                if (headerLength != 23) {
                    snapshot->TSTATES = (buffer[55] | (buffer[56] << 8)) * buffer[57];
                    if (headerLength == 55)
                        snapshot->PORT_1FFD = buffer[86];
                }

                byte _bank[16384];

                //Load rest of the data
                while (counter < buffer.size()) {
                    //Get length of data block
                    int dataLength = buffer[counter] | ((ushort)buffer[counter + 1] << 8);
                    counter += 2;
                    if (counter >= buffer.size()) break;  //Some 128K .z80 files have a trailing zero or two (DamienG)
                    int page = buffer[counter++];

                    //copies page data to temporary RAM array
                    GetPage(buffer.data(), counter, _bank, dataLength);
                    counter += (dataLength == 0xffff ? 16384 : dataLength);

                    switch (page) {
                        //Ignore any ROM pages.
                        //For 128k we can deduce from 0x7ffd, which ROM to use.
                        case 0:
                        case 1:
                        case 2:
                        break;

                        case 3:
                        memcpy(snapshot->RAM_BANK[0], _bank, 8192);
                        memcpy(snapshot->RAM_BANK[1], _bank + 8192, 8192);
                        break;

                        case 4:
                        if (snapshot->TYPE > 0) {
                            memcpy(snapshot->RAM_BANK[2], _bank, 8192);
                            memcpy(snapshot->RAM_BANK[3], _bank + 8192, 8192);
                        }
                        else //48k
                        {
                            memcpy(snapshot->RAM_BANK[4], _bank, 8192);
                            memcpy(snapshot->RAM_BANK[5], _bank + 8192, 8192);
                        }
                        break;

                        case 5:
                        if (snapshot->TYPE > 0) {
                            memcpy(snapshot->RAM_BANK[4], _bank, 8192);
                            memcpy(snapshot->RAM_BANK[5], _bank + 8192, 8192);
                        }
                        else //48k
                        {
                            memcpy(snapshot->RAM_BANK[0], _bank, 8192);
                            memcpy(snapshot->RAM_BANK[1], _bank + 8192, 8192);
                        }
                        break;

                        case 6:
                        memcpy(snapshot->RAM_BANK[6], _bank, 8192);
                        memcpy(snapshot->RAM_BANK[7], _bank + 8192, 8192);
                        break;

                        case 7:
                        memcpy(snapshot->RAM_BANK[8], _bank, 8192);
                        memcpy(snapshot->RAM_BANK[9], _bank + 8192, 8192);
                        break;

                        case 8: //for both 48k and 128k
                        memcpy(snapshot->RAM_BANK[10], _bank, 8192);
                        memcpy(snapshot->RAM_BANK[11], _bank + 8192, 8192);
                        break;

                        case 9:
                        memcpy(snapshot->RAM_BANK[12], _bank, 8192);
                        memcpy(snapshot->RAM_BANK[13], _bank + 8192, 8192);
                        break;

                        case 10:
                        memcpy(snapshot->RAM_BANK[14], _bank, 8192);
                        memcpy(snapshot->RAM_BANK[15], _bank + 8192, 8192);
                        break;

                        default:
                        break;
                    }
                }
            }
            else //Version 1
            {
                snapshot->TYPE = 0;
                //int screenAddr = GetPageAddress(10);
                byte RAM_48K[49152];

                if (!isCompressed) {
                    //copy ram bank 5
                    memcpy(RAM_48K, buffer.data() + 30, 49152);
                }
                else {
                    bool done = false;
                    int byteCounter = 30;
                    int memCounter = 0;

                    while (!done) {
                        byte bite = buffer[byteCounter++];
                        if (bite == 0) {
                            //check if this is the end marker
                            byte bite2 = buffer[byteCounter];
                            if (bite2 == 0xED) {
                                byte bite3 = buffer[byteCounter + 1];
                                if (bite3 == 0xED) {
                                    byte bite4 = buffer[byteCounter + 2];
                                    if (bite4 == 0) {
                                        done = true;
                                        break;
                                    }
                                }
                            }
                            RAM_48K[memCounter++] = bite;
                        }
                        else
                            if (bite == 0xED) {
                            byte bite2 = buffer[byteCounter];
                            if (bite2 == 0xED) {
                                byteCounter++;
                                int dataLength = buffer[byteCounter++];
                                byte data = buffer[byteCounter++];

                                //compressed data
                                for (int f = 0; f < dataLength; f++) {
                                    RAM_48K[memCounter++] = data;
                                }
                                continue;
                            }
                            RAM_48K[memCounter++] = bite;
                            continue;
                        }
                        else
                            RAM_48K[memCounter++] = bite;
                    } //while
                } //compressed

                //whew! Ok, now copy to appropriate pages for 48k. Namely 5, 2, 0
                memcpy(snapshot->RAM_BANK[10], RAM_48K, 8192);
                memcpy(snapshot->RAM_BANK[11], RAM_48K + 8192, 8192);
                memcpy(snapshot->RAM_BANK[4], RAM_48K + 8192 * 2, 8192);
                memcpy(snapshot->RAM_BANK[5], RAM_48K + 8192 * 3, 8192);
                memcpy(snapshot->RAM_BANK[0], RAM_48K + 8192 * 4, 8192);
                memcpy(snapshot->RAM_BANK[1], RAM_48K + 8192 * 5, 8192);
            }
            return snapshot;
        }
    };
}
