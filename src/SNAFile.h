#pragma once

#include "Types.h"

#include <vector>

namespace rm {
    struct SNA_SNAPSHOT
    {
        byte TYPE;                      //0 = 48k, 1 = 128;
        byte I;                  //I Register
        byte L_, H_, E_, D_, C_, B_, F_, A_; //Alternate registers
        byte L, H, E, D, C, B, IYL, IYH, IXL, IXH; //16 bit main registers
        byte IFF2;               //Interrupt enabled? (bit 2 on/off)
        byte R;                  //R Register
        byte F, A, SPL, SPH; //AF and SP register
        byte IM;                 //Interupt Mode
        byte BORDER;             //Border colour

        union {
            // 48K
            byte RAM[49152];              //Contents of the RAM

            // 128
            byte PCL, PCH;                                  //PC Register
            byte PORT_7FFD;                          //Current state of port 7ffd
            byte TR_DOS;                             //Is TR DOS ROM paged in?
            byte RAM_BANK[16][8192];        //Contents of the 8192*16 ram banks
        };
    };

    class SNAFile
    {
    public:
        //Will return a filled snapshot structure from buffer
        static bool LoadSNA(std::vector<byte> const& buffer, SNA_SNAPSHOT* snapshot) {
            if (buffer.size() == 0)
                return false; //something bad happened!

            if (buffer.size() == 49179) {
                snapshot->TYPE = 0;
            }
            else if (buffer.size() == 131103 || buffer.size() == 147487) {
                snapshot->TYPE = 1;
            }
            else
                return false;

            snapshot->I = buffer[0];
            snapshot->H_ = buffer[2];
            snapshot->L_ = buffer[1];
            snapshot->D_ = buffer[4];
            snapshot->E_ = buffer[3];
            snapshot->B_ = buffer[6];
            snapshot->C_ = buffer[5];
            snapshot->A_ = buffer[8];
            snapshot->F_ = buffer[7];

            snapshot->H = buffer[10];
            snapshot->L = buffer[9];
            snapshot->D = buffer[12];
            snapshot->E = buffer[11];
            snapshot->B = buffer[14];
            snapshot->C = buffer[13];
            snapshot->IYH = buffer[16];
            snapshot->IYL = buffer[15];
            snapshot->IXH = buffer[18];
            snapshot->IXL = buffer[17];

            snapshot->IFF2 = buffer[19];
            snapshot->R = buffer[20];
            snapshot->A = buffer[22];
            snapshot->F = buffer[21];
            snapshot->SPH = buffer[24];
            snapshot->SPL = buffer[23];
            snapshot->IM = buffer[25];
            snapshot->BORDER = (byte)(buffer[26] & 0x07);

            //48k snapshot
            if (snapshot->TYPE == 0) {
                memcpy(snapshot->RAM, buffer.data() + 27, 49152);
            }
            else {
                //128k snapshot
                //Copy ram bank 5
                memcpy(snapshot->RAM_BANK[10], buffer.data() + 27, 8192);
                memcpy(snapshot->RAM_BANK[11], buffer.data() + 27 + 8192, 8192);

                //Copy ram bank 2
                memcpy(snapshot->RAM_BANK[4], buffer.data() + 27 + 16384, 8192);
                memcpy(snapshot->RAM_BANK[5], buffer.data() + 27 + 16384 + 8192, 8192);

                snapshot->PORT_7FFD = buffer[49181]; //we'll load this in earlier 'cos we need it now!

                int BankInPage4 = snapshot->PORT_7FFD & 0x07;

                //Copy currently paged in bank (actually we don't care here 'cos we're simply filling in all the b(l)anks)
                memcpy(snapshot->RAM_BANK[BankInPage4 * 2], buffer.data() + 27 + 16384 + 16384, 8192);
                memcpy(snapshot->RAM_BANK[BankInPage4 * 2 + 1], buffer.data() + 27 + 16384 + 16384 + 8192, 8192);

                snapshot->PCH = buffer[49180];
                snapshot->PCL = buffer[49179];

                snapshot->TR_DOS = buffer[49182];

                int t = 0;
                for (int f = 0; f < 8; f++) {
                    if (f == 5 || f == 2 || f == BankInPage4)
                        continue;

                    memcpy(snapshot->RAM_BANK[f * 2], buffer.data() + 49183 + 16384 * t, 8192);
                    memcpy(snapshot->RAM_BANK[f * 2 + 1], buffer.data() + 49183 + 16384 * t + 8192, 8192);
                    t++;
                }
            }
            return snapshot;
        }
    };
}
