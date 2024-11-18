#pragma once

#include "Types.h"

#include <zlib.h>
#include <assert.h>
#include <vector>

namespace rm {
    bool decompressData(byte const* compressedData, size_t compressedSize, byte* decompressedData, size_t decompressedSize);

    //Supports SZX 1.4 specification
    class SZXFile
    {
    public:
        enum class ZXTYPE
        {
            ZXSTMID_16K = 0,
            ZXSTMID_48K,
            ZXSTMID_128K,
            ZXSTMID_PLUS2,
            ZXSTMID_PLUS2A,
            ZXSTMID_PLUS3,
            ZXSTMID_PLUS3E,
            ZXSTMID_PENTAGON128,
            ZXSTMID_TC2048,
            ZXSTMID_TC2068,
            ZXSTMID_SCORPION,
            ZXSTMID_SE,
            ZXSTMID_TS2068,
            ZXSTMID_PENTAGON512,
            ZXSTMID_PENTAGON1024,
            ZXSTMID_128KE
        };

        static const int ZXSTZF_EILAST = 1;
        static const int ZXSTZF_HALTED = 2;
        static const int ZXSTRF_COMPRESSED = 1;
        static const int ZXSTKF_ISSUE2 = 1;
        static const int ZXSTMF_ALTERNATETIMINGS = 1;

        static const int SZX_VERSION_SUPPORTED_MAJOR = 1;
        static const int SZX_VERSION_SUPPORTED_MINOR = 4;

        struct ZXST_Header
        {
            byte Magic[4];
            byte MajorVersion;
            byte MinorVersion;
            byte MachineId;
            byte Flags;
        };

        struct ZXST_Creator
        {
            byte CreatorName[32];

            byte MajorVersion[2];
            byte MinorVersion[2];

            byte Data[1];
        };

        // Block Header. Each real block starts
        // with this header.
        struct ZXST_Block
        {
            byte Id[4];
            byte Size[4];
        };

        struct ZXST_Z80Regs
        {
            byte F, A, C, B, E, D, L, H;
            byte F1, A1, C1, B1, E1, D1, L1, H1;
            byte IXL, IXH, IYL, IYH, SPL, SPH, PCL, PCH;
            byte I;
            byte R;
            byte IFF1, IFF2;
            byte IM;
            byte CyclesStart[4];
            byte HoldIntReqCycles;
            byte Flags;
            byte MemPtrL, MemPtrH;
            //public byte BitReg;
            //public byte Reserved;
        };

        struct ZXST_SpecRegs
        {
            byte Border;
            byte x7ffd;
            byte pagePort; //either 0x1ffd (+2A/+3) or 0xeff7 (Pentagon 1024)
            byte Fe;

            byte Reserved[4];
        };

        struct ZXST_RAMPage
        {
            byte wFlags[2];
            byte chPageNo;
        };

        struct ZXST_AYState
        {
            byte cFlags;
            byte currentRegister;

            byte chRegs[16];
        };

        struct ZXST_Keyboard
        {
            byte Flags[4];
            byte KeyboardJoystick;
        };

        struct ZXST_Tape
        {
            byte currentBlockNo[2];
            byte flags[2];
            byte uncompressedSize[4];
            byte compressedSize[4];

            byte fileExtension[16];
        };

        struct ZXST_Plus3Disk
        {
            byte numDrives;
            byte motorOn;
        };

        struct ZXST_DiskFile
        {
            byte flags[2];
            byte driveNum;
            byte uncompressedSize[4];
        };

        struct ZXST_PaletteBlock
        {
            byte flags;
            byte currentRegister;

            byte paletteRegs[64];
        };

        byte RAM_BANK[16][8192];       //Contents of the 8192*16 ram banks

        ZXST_Header header;
        ZXST_Creator creator;
        ZXST_Z80Regs z80Regs;
        ZXST_SpecRegs specRegs;
        ZXST_Keyboard keyboard;
        ZXST_AYState ayState;
        ZXST_Tape tape;
        ZXST_Plus3Disk plus3Disk;
        std::vector<ZXST_DiskFile> plus3DiskFile;
        ZXST_PaletteBlock palette;

        std::vector<byte> embeddedTapeData;
        string externalTapeFile;

        byte numDrivesPresent = 0;
        bool InsertTape = false;
        std::vector<bool> InsertDisk;
        std::vector<string> externalDisk;
        bool paletteLoaded = false;

        string GetID(byte* id) {
            byte bytes[4];
            memcpy(bytes, id, 4);
            return string((char*)bytes, 4);
        }

        uint GetUIntFromString(string data) {
            uint val;
            memcpy(&val, data.c_str(), 4);
            return val;
        }

#define UINT(b) ((uint)(b[0]) << 24 | (uint)(b[1]) << 16 | (uint)(b[2]) << 8 | (b[3]))

#define UNUINT(b, v) do { \
        (b)[0] = (v) & 255; \
        (b)[1] = ((v) >> 8) & 255; \
        (b)[2] = ((v) >> 16) & 255; \
        (b)[3] = ((v) >> 24) & 255; \
    } while (0)

        bool LoadSZX(std::vector<byte> const& buffer) {
            if (buffer.size() == 0)
                return false; //something bad happened!

            //Read in the szx header to begin proceedings
            memcpy(&header, buffer.data(), sizeof(header));

            if (header.MajorVersion != 1) {
                return false;
            }

            int bufferCounter = sizeof(header);

            while (bufferCounter < buffer.size()) {
                //Read the block info
                ZXST_Block block;
                memcpy(&block, buffer.data() + bufferCounter, sizeof(block));

                bufferCounter += sizeof(block);
                string blockID = GetID(block.Id);

                switch (UINT(block.Id)) {
                    case UINT("SPCR"):
                    //Read the ZXST_SpecRegs structure
                    memcpy(&specRegs, buffer.data() + bufferCounter, sizeof(specRegs));
                    break;

                    case UINT("Z80R"):
                    //Read the ZXST_SpecRegs structure
                    memcpy(&z80Regs, buffer.data() + bufferCounter, sizeof(z80Regs));
                    break;

                    case UINT("KEYB"):
                    //Read the ZXST_SpecRegs structure
                    memcpy(&keyboard, buffer.data() + bufferCounter, sizeof(keyboard));
                    break;

                    case UINT("AY\0\0"):
                    memcpy(&ayState, buffer.data() + bufferCounter, sizeof(ayState));
                    break;

                    case UINT("+3\0\0"):
                    memcpy(&plus3Disk, buffer.data() + bufferCounter, sizeof(plus3Disk));

                    numDrivesPresent = plus3Disk.numDrives;
                    plus3DiskFile.reserve(plus3Disk.numDrives);
                    externalDisk.resize(plus3Disk.numDrives);
                    InsertDisk.resize(plus3Disk.numDrives);
                    break;

                    case UINT("DSK\0"): {
                    ZXST_DiskFile df;
                    memcpy(&df, buffer.data() + bufferCounter, sizeof(df));
                    plus3DiskFile.emplace_back(df);
                    InsertDisk[df.driveNum] = true;

                    int offset2 = bufferCounter + sizeof(df);
                    std::vector<byte> file;
                    file.resize(UINT(df.uncompressedSize) + 1);
                    memcpy(file.data(), buffer.data() + offset2, UINT(df.uncompressedSize));
                    externalDisk[df.driveNum] = std::string((char*)file.data());
                    break;
                    }

                    case UINT("TAPE"):
                    memcpy(&tape, buffer.data() + bufferCounter, sizeof(tape));
                    InsertTape = true;

                    //Embedded tape file
                    if ((tape.flags[0] & 1) != 0) {
                        int offset = bufferCounter + sizeof(tape);
                        //Compressed?
                        if ((tape.flags[0] & 2) != 0) {
                            embeddedTapeData.resize(UINT(tape.uncompressedSize));
                            decompressData(buffer.data() + bufferCounter, UINT(tape.compressedSize), embeddedTapeData.data(), UINT(tape.uncompressedSize));
                        }
                        else {
                            embeddedTapeData.resize(UINT(tape.compressedSize));
                            memcpy(embeddedTapeData.data(), buffer.data() + bufferCounter, UINT(tape.compressedSize));
                        }
                    }
                    else //external tape file
                    {
                        int offset = bufferCounter + sizeof(tape);
                        externalTapeFile = string((char*)buffer.data() + offset, UINT(tape.compressedSize) - 1);
                    }
                    break;

                    case UINT("RAMP"): {
                    //Read the ZXST_SpecRegs structure
                    ZXST_RAMPage ramPages;
                    memcpy(&ramPages, buffer.data() + bufferCounter, sizeof(ramPages));

                    if (ramPages.wFlags[0] == ZXSTRF_COMPRESSED) {
                        int offset = bufferCounter + sizeof(ramPages);
                        int compressedSize = ((int)block.Size - (sizeof(ramPages)));//  - Marshal.SizeOf(block) - 1 ));
                        std::vector<byte> pageData;
                        pageData.resize(16384);
                        decompressData(buffer.data() + offset, compressedSize, pageData.data(), 16384);

                        memcpy(RAM_BANK[ramPages.chPageNo * 2], pageData.data(), 8192);
                        memcpy(RAM_BANK[ramPages.chPageNo * 2 + 1], pageData.data() + 8192, 8192);
                    }
                    else {
                        int bufferOffset = bufferCounter + sizeof(ramPages);
                        {
                            memcpy(RAM_BANK[ramPages.chPageNo * 2], buffer.data() + bufferOffset, 8192);
                            memcpy(RAM_BANK[ramPages.chPageNo * 2 + 1], buffer.data() + bufferOffset + 8192, 8192);
                        }
                    }
                    break;
                    }

                    case UINT("PLTT"):
                    memcpy(&palette, buffer.data() + bufferCounter, sizeof(palette));
                    paletteLoaded = true;
                    break;

                    default: //unrecognised block, so skip to next
                    break;
                }

                bufferCounter += UINT(block.Size); //Move to next block
            }
            return true;
        }

        template<typename T>
        void write(std::vector<byte>* dest, T const& source) {
            dest->resize(dest->size() + sizeof(T));
            memcpy(dest->data() + dest->size() - sizeof(T), &T, sizeof(T));
        }

        void GetSZXData(std::vector<byte>* r) {
            //using (MemoryStream ms = new MemoryStream(1000)) {
                //using (BinaryWriter r = new BinaryWriter(ms)) {
                    r->clear();
                    ZXST_Block block;

                    write(r, header);

                    UNUINT(block.Id, UINT("CRTR"));
                    UNUINT(block.Size, (uint)sizeof(creator));
                    write(r, block);
                    write(r, creator);

                    UNUINT(block.Id, UINT("Z80R"));
                    UNUINT(block.Size, (uint)sizeof(z80Regs));
                    write(r, block);
                    write(r, z80Regs);

                    UNUINT(block.Id, UINT("SPCR"));
                    UNUINT(block.Size, (uint)sizeof(specRegs));
                    write(r, block);
                    write(r, specRegs);

                    UNUINT(block.Id, UINT("KEYB"));
                    UNUINT(block.Size, (uint)sizeof(keyboard));
                    write(r, block);
                    write(r, keyboard);

                    if (paletteLoaded) {
                        UNUINT(block.Id, UINT("PLTT"));
                        UNUINT(block.Size, (uint)sizeof(palette));
                        write(r, block);
                        write(r, palette);
                    }

                    if (header.MachineId > (byte)ZXTYPE::ZXSTMID_48K) {
                        UNUINT(block.Id, UINT("AY\0\0"));
                        UNUINT(block.Size, (uint)sizeof(ayState));
                        write(r, block);
                        write(r, ayState);
                        byte ram[16384];
                        for (int f = 0; f < 8; f++) {
                            ZXST_RAMPage ramPage;
                            ramPage.chPageNo = (byte)f;
                            ramPage.wFlags[0] = 0; //not compressed
                            ramPage.wFlags[1] = 0; //not compressed
                            UNUINT(block.Id, UINT("RAMP"));
                            UNUINT(block.Size, (uint)(sizeof(ramPage) + 16384));
                            write(r, block);
                            write(r, ramPage);
                            for (int g = 0; g < 8192; g++) {
                                ram[g] = (byte)(RAM_BANK[f * 2][g] & 0xff);
                                ram[g + 8192] = (byte)(RAM_BANK[f * 2 + 1][g] & 0xff);
                            }
                            write(r, ram);
                        }
                    } else //48k
                    {
                        byte ram[16384];
                        //page 0
                        ZXST_RAMPage ramPage;
                        ramPage.chPageNo = 0;
                        ramPage.wFlags[0] = 0; //not compressed
                        ramPage.wFlags[1] = 0; //not compressed
                        UNUINT(block.Id, UINT("RAMP"));
                        UNUINT(block.Size, (uint)(sizeof(ramPage) + 16384));
                        write(r, block);
                        write(r, ramPage);
                        for (int g = 0; g < 8192; g++) {
                            //me am angry.. poda thendi... saree vangi tharamattaaai?? poda! nonsense! style moonji..madiyan changu..malayalam ariyatha
                            //Lol! That's my wife cursing me for spending my time on this crap instead of her. Such a sweetie pie!
                            ram[g] = (byte)(RAM_BANK[0][g] & 0xff);
                            ram[g + 8192] = (byte)(RAM_BANK[1][g] & 0xff);
                        }
                        write(r, ram);

                        //page 2
                        ramPage.chPageNo = 2;
                        ramPage.wFlags[0] = 0; //not compressed
                        ramPage.wFlags[1] = 0; //not compressed
                        //ramPage.chData = new byte[16384];
                        // Array.Copy(RAM_BANK[2 * 2], 0, ramPage.chData, 0, 8192);
                        //Array.Copy(RAM_BANK[2 * 2 + 1], 0, ramPage.chData, 8192, 8192);
                        UNUINT(block.Id, UINT("RAMP"));
                        UNUINT(block.Size, (uint)(sizeof(ramPage) + 16384));
                        write(r, block);
                        write(r, ramPage);
                        for (int g = 0; g < 8192; g++) {
                            ram[g] = (byte)(RAM_BANK[ramPage.chPageNo * 2][g] & 0xff);
                            ram[g + 8192] = (byte)(RAM_BANK[ramPage.chPageNo * 2 + 1][g] & 0xff);
                        }
                        write(r, ram);

                        //page 5
                        ramPage.chPageNo = 5;
                        ramPage.wFlags[0] = 0; //not compressed
                        ramPage.wFlags[1] = 0; //not compressed
                        UNUINT(block.Id, UINT("RAMP"));
                        UNUINT(block.Size, (uint)(sizeof(ramPage) + 16384));
                        write(r, block);
                        write(r, ramPage);
                        for (int g = 0; g < 8192; g++) {
                            ram[g] = (byte)(RAM_BANK[ramPage.chPageNo * 2][g] & 0xff);
                            ram[g + 8192] = (byte)(RAM_BANK[ramPage.chPageNo * 2 + 1][g] & 0xff);
                        }
                        write(r, ram);
                    }

                    if (InsertTape) {
                        UNUINT(block.Id, UINT("TAPE"));

                        char const* ext = strrchr(externalTapeFile.c_str(), '.');
                        if (ext == nullptr) ext = "";
                        int len = strlen(ext);
                        memset(tape.fileExtension, 0, sizeof(tape.fileExtension));
                        for (int f = 1; f < len; f++)
                            tape.fileExtension[f - 1] = ext[f];

                        tape.flags[0] = 0;
                        tape.flags[1] = 0;
                        UNUINT(tape.currentBlockNo, 0);
                        UNUINT(tape.compressedSize, externalTapeFile.size());
                        UNUINT(tape.uncompressedSize, externalTapeFile.size());
                        UNUINT(block.Size, (uint)sizeof(tape) + (uint)tape.uncompressedSize);
                        write(r, block);
                        write(r, tape);
                     
                        r->insert(r->end(), externalTapeFile.begin(), externalTapeFile.end());
                    }
                //}

                //szxData = ms.ToArray();
            //}
            //return szxData;
        }

    };
}
