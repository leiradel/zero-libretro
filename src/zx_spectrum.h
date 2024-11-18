#pragma once

#include "AudioDevice.h"
#include "SNAFile.h"
#include "SoundManager.h"
#include "Types.h"
#include "ULA_Plus.h"
#include "Z80.h"

#include <limits.h>
#include <stddef.h>
#include <functional>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace rm {
    //Handy enum for the tape deck
    enum class TapeEventType {
        START_TAPE,
        STOP_TAPE,
        EDGE_LOAD,
        SAVE_TAP,
        CLOSE_TAP,
        FLASH_LOAD,
        NEXT_BLOCK
    };

    /// <summary>
    /// zx_spectrum is the heart of speccy emulation.
    /// It includes core execution, ula, sound, input and interrupt handling
    /// </summary>
    class zx_spectrum
    {
    public:
        std::function<void(int addr, int val)> MemoryWriteEvent;
        std::function<void(int addr, int val)> MemoryReadEvent;
        std::function<void(int addr, int val)> MemoryExecuteEvent;
        std::function<void()> OpcodeExecutedEvent;
        std::function<void(TapeEventType type)> TapeEvent;
        std::function<void(int port, int val, bool write)> PortEvent; //Used to raise generic port event (ex debugging)
        std::function<byte(ushort port)> PortReadEvent;
        std::function<void(ushort port, byte val)> PortWriteEvent;
        std::function<void(int type)> DiskEvent;
        std::function<void()> StateChangeEvent;
        std::function<void()> PopStackEvent;
        std::function<void()> PushStackEvent;
        std::function<void()> FrameEndEvent;
        std::function<void()> FrameStartEvent;
        std::function<void()> RZXPlaybackStartEvent;
        std::function<void()> RZXFrameEndEvent;

        void OnFrameEndEvent()
        {
               FrameEndEvent();
        }

        void OnFrameStartEvent()
        {
            FrameStartEvent();
        }

        void OnRZXPlaybackStartEvent() {
            RZXPlaybackStartEvent();
        }

        void OnMemoryWriteEvent(int addr, int val) {
            MemoryWriteEvent(addr, val);
        }

        void OnMemoryReadEvent(int addr, int val) {
            MemoryReadEvent(addr, val);
        }

        void OnMemoryExecuteEvent(int addr, int val) {
            MemoryExecuteEvent(addr, val);
        }

        void OnTapeEvent(TapeEventType type) {
            TapeEvent(type);
        }

        void OnDiskEvent(int type) {
            DiskEvent(type);
        }

        void OnPortEvent(int port, int val, bool write) {
            PortEvent(port, val, write);
        }

        byte OnPortReadEvent(ushort port) {
            return PortReadEvent(port);
        }

        void OnPortWriteEvent(ushort port, byte val) {
            PortWriteEvent(port, val);
        }

        IntPtr mainHandle;


        Z80 cpu;
        ULA_Plus ula_plus;
        //public Z80_Registers regs;
        SoundManager beeper;
        std::unordered_set<IODevice*> io_devices;
        std::unordered_set<AudioDevice*> audio_devices;
        std::unordered_map<int, SpeccyDevice*> attached_devices;

        int keyLine[8] = { 255, 255, 255, 255, 255, 255, 255, 255 };

        std::vector<int> AttrColors;

        /// <summary>
        /// The regular speccy palette
        /// </summary>
        int NormalColors[16] = {
                                             0x000000,            // Blacks
                                             0x0000C0,            // Red
                                             0xC00000,            // Blue
                                             0xC000C0,            // Magenta
                                             0x00C000,            // Green
                                             0x00C0C0,            // Yellow
                                             0xC0C000,            // Cyan
                                             0xC0C0C0,            // White
                                             0x000000,            // Bright Black
                                             0x0000F0,            // Bright Red
                                             0xF00000,            // Bright Blue
                                             0xF000F0,            // Bright Magenta
                                             0x00F000,            // Bright Green
                                             0x00F0F0,            // Bright Yellow
                                             0xF0F000,            // Bright Cyan
                                             0xF0F0F0             // Bright White
                                    };

        //Misc variables
        int val, addr;
        bool isROMprotected = true;  //not really used ATM
        bool needsPaint = false;     //Raised when the ULA has finished painting the entire screen
        bool CapsLockOn = false;
        int prevT;        //previous cpu t-states
        int inputFrameTime = 0;

        //Sound
        static const short MIN_SOUND_VOL = 0;
        static const short MAX_SOUND_VOL = SHRT_MAX / 2;
        short soundSamples[882 * 2]; //882 samples, 2 channels, 2 bytes per channel (short)
        
        static const bool ENABLE_SOUND = false;
        int averagedSound = 0;
        short soundCounter = 0;
        int lastSoundOut = 0;
        short soundOut = 0;
        int soundTStatesToSample = 79;
        float soundVolume = 0.0f;        //cached reference used when beeper instance is recreated.
        short soundSampleCounter = 0;
        int timeToOutSound = 0;

        //Threading stuff (not used)
        bool doRun = true;           //z80 executes only when true. Mainly for debugging purpose.

        //Important ports
        int lastFEOut = 0;        //The ULA Port
        int last7ffdOut = 0;      //Paging port on 128k/+2/+3/Pentagon
        int last1ffdOut = 0;      //Paging + drive motor port on +3

        //Port 0xfe constants
        static const int BORDER_BIT = 0x07;
        static const int EAR_BIT = 0x10;
        static const int MIC_BIT = 0x08;
        static const int TAPE_BIT = 0x40;

        //Machine properties
        double clockSpeed;        //the CPU clock speed of the machine
        int TstatesPerScanline;   //total # tstates in one scanline
        int ScanLineWidth;        //total # pixels in one scanline
        int CharRows;             //total # chars in one PRINT row
        int CharCols;             //total # chars in one PRINT col
        int ScreenWidth;          //total # pixels in one display row
        int ScreenHeight;         //total # pixels in one display col
        int BorderTopHeight;      //total # pixels in top border
        int BorderBottomHeight;   //total # pixels in bottom border
        int BorderLeftWidth;      //total # pixels of width of left border
        int BorderRightWidth;     //total # pixels of width of right border
        int DisplayStart;         //memory address of display start
        int DisplayLength;        //total # bytes of display memory
        int AttributeStart;       //memory address of attribute start
        int AttributeLength;      //total # bytes of attribute memory

        bool Issue2Keyboard = false; //Only of use for 48k & 16k machines.
        int LateTiming = 0;       //Some machines have late timings. This affects contention and has to be factored in.

        //Utility strings
        static const string ROM_128_BAS;
        static const string ROM_48_BAS;
        static const string ROM_128_SYN;
        static const string ROM_PLUS3_DOS;
        static const string ROM_TR_DOS;

        //The monitor needs to know these states so are public
        string BankInPage3 = "-----";
        string BankInPage2 = "-----";
        string BankInPage1 = "-----";
        string BankInPage0 = ROM_48_BAS;
        bool monitorIsRunning = false;
        
        //Paging
        bool lowROMis48K = true;
        bool trDosPagedIn = false ;//TR DOS is swapped in only if the lower ROM is 48k.
        bool special64KRAM = false;  //for +3
        bool contendedBankPagedIn = false;
        bool showShadowScreen = false;
        bool pagingDisabled = false;    //on 128k, depends on bit 5 of the value output to port (whose 1st and 15th bits are reset)

        //The cpu needs access to this so are public
        int InterruptPeriod;             //Number of t-states to hold down /INT
        int FrameLength;                 //Number of t-states of in 1 frame before interrupt is fired.
        byte FrameCount = 0;            //Used to keep tabs on tape play time out period.
        int flashFrameCount;

        //Contention related stuff
        int contentionStartPeriod;              //t-state at which to start applying contention
        int contentionEndPeriod;                //t-state at which to end applying contention
        std::vector<byte> contentionTable;                  //tstate-memory contention delay mapping

        //Render related stuff
        std::vector<int> ScreenBuffer;                        //buffer for the windows side rasterizer
        std::vector<int> LastScanlineColor;
        short lastScanlineColorCounter;
        std::vector<byte> screen;                           //display memory (16384 for 48k)
        std::vector<short> attr;                           //attribute memory lookup (mapped 1:1 to screen for convenience)
        std::vector<short> tstateToDisp;                   //tstate-display mapping
        std::vector<short> floatingBusTable;               //table that stores tstate to screen/attr addresses values
        int deltaTStates;
        int lastTState;                         //tstate at which last render update took place
        int elapsedTStates;                     //tstates elapsed since last render update
        int ActualULAStart;                     //tstate of top left raster pixel
        int screenByteCtr;                      //offset into display memory based on current tstate
        int ULAByteCtr;                         //offset into current pixel of rasterizer
        int borderColour;                       //Used by the screen update routine to output border colour
        bool flashOn = false;

        //For floating bus implementation
        int lastPixelValue;                     //last 8-bit bitmap read from display memory
        int lastAttrValue;                      //last 8-bit attr val read from attribute memory
        int lastPixelValuePlusOne;              //last 8-bit bitmap read from display memory+1
        int lastAttrValuePlusOne;               //last 8-bit attr val read from attribute memory+1

        //For 4 bright levels ULA artifacting (gamma ramping). DOESN'T WORK!
        //protected bool pixelIsPaper = false;

        //These variables are used to create a screen display box (non border area).
        int TtateAtLeft, TstateWidth, TstateAtTop,
                      TstateHeight, TstateAtRight, TstateAtBottom;

        //16x8k flat RAM bank (because of issues with pointers in c#) + 1 dummy bank
        byte RAMpage[16][8192]; //16 pages of 8192 bytes each

        //8x8k flat ROM bank
        byte ROMpage[8][8192];

        //For writing to ROM space
        byte JunkMemory[2][8192]; 

        //8 "pointers" to the pages
        //NOTE: In the case of +3, Pages 0 and 1 *can* point to a RAMpage. In other cases they point to a
        //ROMpage. To differentiate which is being pointed to, the +3 machine employs the specialMode boolean.
        byte* PageReadPointer[8];
        byte* PageWritePointer[8];

        //Tape edge detection variables
        string tapeFilename = "";
        int tape_detectionCount = 0;
        int tape_PC = 0;
        int tape_PCatLastIn = 0;
        int tape_whichRegToCheck = 0;
        int tape_regValue = 0;
        bool tape_edgeDetectorRan = false;
        int tape_tstatesSinceLastIn = 0;
        int tape_tstatesStep, tape_diff;
        int tape_A, tape_B, tape_C, tape_D, tape_E, tape_H, tape_L;
        bool tape_edgeLoad = false;
        bool tapeBitWasFlipped = false;
        bool tapeBitFlipAck = false;
        bool tape_AutoPlay = false;
        bool tape_AutoStarted = false;
        bool tape_readToPlay = false;
        const int TAPE_TIMEOUT = 100;// 69888 * 10;
        int tape_stopTimeOut = TAPE_TIMEOUT;
        byte tape_FrameCount = 0;
        int tapeTStates = 0;
        uint edgeDuration = 0;
        bool tapeIsPlaying = false;
        int pulseLevel = 0;

        //Tape loading
        int blockCounter = 0;
        bool tapePresent = false;
        bool tape_flashLoad = true;
        bool tapeTrapsDisabled = false;
        int pulseCounter = 0;
        int repeatCount = 0;
        int bitCounter = 0;
        byte bitShifter = 0;
        int dataCounter = 0;
        byte dataByte = 0;
        int currentBit = 0;
        bool isPauseBlockPreproccess = false; //To ensure previous edge is finished correctly
        bool isProcessingPauseBlock = false;  //Signals if the current pause block is currently being serviced.
        uint pauseCounter = 0;

        //AY support
        bool HasAYSound;

        //Handy enum for various keys
        enum class keyCode
        {
            Q, W, E, R, T, Y, U, I, O, P, A, S, D, F, G, H, J, K, L, Z, X, C, V, B, N, M,
            _0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
            SPACE, SHIFT, CTRL, ALT, TAB, CAPS, ENTER, BACK,
            DEL, INS, HOME, END, PGUP, PGDOWN, NUMLOCK,
            ESC, PRINT_SCREEN, SCROLL_LOCK, PAUSE_BREAK,
            TILDE, EXCLAMATION, AT, HASH, DOLLAR, PERCENT, CARAT,
            AMPERSAND, ASTERISK, LBRACKET, RBRACKET, HYPHEN, PLUS, VBAR,
            LCURLY, RCURLY, COLON, DQUOTE, LESS_THAN, GREATER_THAN, QMARK,
            UNDER_SCORE, EQUALS, BSLASH, LSQUARE, RSQUARE, SEMI_COLON,
            APOSTROPHE, COMMA, STOP, FSLASH, LEFT, RIGHT, UP, DOWN,
            F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
            LAST
        };

        enum class JoysticksEmulated
        {
            NONE,
            KEMPSTON,
            SINCLAIR1,
            SINCLAIR2,
            CURSOR,
            LAST
        };

        int joystickType = 0; //A bit field of above joysticks to emulate (usually not more than 2).
        bool UseKempstonPort1F = false; //If 1f, decoding scheme uses top 3 bits (5,6,7) else only the 5th bit is tested (port d4).
        
        ////Each joystickState corresponds to an emulated joystick
        //Bits: 0 = button 3, 1 = button 2, 3 = button 1, 4 = up, 5 = down, 6 = left, 7 = right
        int joystickState[(size_t)JoysticksEmulated::LAST];

        //This holds the key lines used by the speccy for input
        std::vector<bool> keyBuffer;

        //SpecEmu interfacing
        bool externalSingleStep = false;

        //Disk related stuff
        int diskDriveState = 0;

        MachineModel model;
        int emulationSpeed;
        int cpuMultiplier = 1;
        bool isResetOver = false;
        const int MAX_CPU_SPEED = 500;

        //How long should we wait after speccy reset before signalling that it's safe to assume so.
        int resetFrameTarget = 0;
        int resetFrameCounter = 0;

        // Some handy lambda functions...
        int GetDisplacement(byte val) { return (128 ^ val) - 128; }
        //Returns the actual memory address of a page
        int GetPageAddress(int page) { return 8192 * page; }
        //Returns the memory data at a page
        byte* GetPageData(int page) { return RAMpage[page * 2]; }
        //Changes the spectrum palette to the one provided
        void SetPalette(std::vector<int> const& newPalette) { AttrColors = newPalette; }
        // Returns the byte at a given 16 bit address with no contention
        byte PeekByteNoContend(ushort addr) { return PageReadPointer[addr >> 13][addr & 0x1FFF]; }
        // Returns a word at a given 16 bit address with no contention
        ushort PeekWordNoContend(ushort addr) { return (ushort)((PeekByteNoContend(addr) | ((PeekByteNoContend((ushort)(addr + 1)) << 8)))); }
        // Returns a 16 bit value from given address with contention.
        ushort PeekWord(ushort addr) { return (ushort)((PeekByte(addr)) | (PeekByte((ushort)(addr + 1)) << 8)); }

        void Start() {
            doRun = true;
            return;//THREAD
        }

        void Pause() {
            //beeper.Stop();
            return; //THREAD
        }

        void Resume() {
            //beeper.Play();
            return;//THREAD
        }

        void SetEmulationSpeed(int speed) {
            emulationSpeed = speed;
        }

        virtual int GetTotalScreenWidth() {
            return ScreenWidth + BorderLeftWidth + BorderRightWidth;
        }

        virtual int GetTotalScreenHeight() {
            return ScreenHeight + BorderTopHeight + BorderBottomHeight;
        }

        //The display offset of the speccy screen wrt to emulator window in horizontal direction.
        //Useful for "centering" unorthodox screen dimensions like the Pentagon that has different left & right border width.
        virtual int GetOriginOffsetX() {
            return 0;
        }

        //The display offset of the speccy screen wrt to emulator window in vertical direction.
        //Useful for "centering" unorthodox screen dimensions like the Pentagon that has different top & bottom border height.
        virtual int GetOriginOffsetY() {
            return 0;
        }

        virtual void DiskInsert(string filename, byte _unit) {
            diskDriveState |= (1 << _unit);
            OnDiskEvent(diskDriveState);
        }

        virtual void DiskEject(byte _unit) {
            diskDriveState &= ~(1 << _unit);
            OnDiskEvent(diskDriveState);
        }

        bool DiskInserted(byte _unit) {
            if ((diskDriveState & (1 << _unit)) != 0)
                return true;

            return false;
        }

        void SetCPUSpeed(int multiple) {
            cpuMultiplier = multiple;
            beeper.SetVolume(soundVolume);
        }

        void InitCpu() {
            cpu.PeekByte = [this](ushort addr) -> byte { PeekByte(addr); };
            cpu.PokeByte = [this](ushort addr, byte val) -> void { PokeByte(addr, val); };
            cpu.PeekWord = [this](ushort addr) -> ushort { return PeekWord(addr); };
            cpu.PokeWord = [this](ushort addr, ushort val) -> void { PokeWord(addr, val); };
            cpu.In = [this](ushort port) -> byte { return In(port); };
            cpu.Out = [this](ushort port, byte val) -> void { Out(port, val); };
            cpu.Contend = [this](int reg, int times, int count) -> void { Contend(reg, times, count); };
            cpu.InstructionFetchSignal = [this]() -> void {};
            cpu.TapeEdgeDetection = [this]() -> void { OnTapeEdgeDetection(); };
            cpu.TapeEdgeDecA = [this]() -> void { OnTapeEdgeDecA(); };
            cpu.TapeEdgeCpA = [this]() -> void { OnTapeEdgeCpA(); };
        }

        zx_spectrum(IntPtr handle, bool lateTimingModel) {
            mainHandle = handle;
            AttrColors.clear();
            for (size_t i = 0; i < sizeof(NormalColors) / sizeof(NormalColors[0]); i++) { AttrColors.push_back(NormalColors[i]); }
            LateTiming = (lateTimingModel ? 1 : 0);
            tapeBitWasFlipped = false;

            InitCpu();

            //THREAD
            //lock (lockThis)
            {
                beeper.Init(handle, 16, 2, 44100);
                beeper.Play();
            }

           
            //THREAD
            //emulationThread = new System.Threading.Thread(new System.Threading.ThreadStart(Run));
            //emulationThread.Name = @"Emulation Thread";
            //emulationThread.Priority = System.Threading.ThreadPriority.AboveNormal;

            //During warm start, all registers are set to 0xffff
            //http://worldofspectrum.org/forums/showthread.php?t=34574&page=3
        }

        void FlashLoadTape() {
            if (!tape_flashLoad)
                return;

            //if (TapeEvent != null)
            //    OnTapeEvent(new TapeEventArgs(TapeEventType.FLASH_LOAD));
            DoTapeEvent(TapeEventType::FLASH_LOAD);
        }

        void AddDevice(SpeccyDevice* newDevice) {
            RemoveDevice(newDevice->DeviceID());
            attached_devices[(int)newDevice->DeviceID()] = newDevice;
            newDevice->RegisterDevice(this);
        }

        void RemoveDevice(SPECCY_DEVICE deviceId) {
            if (attached_devices.count((int)deviceId) != 0) {
                SpeccyDevice* dev = attached_devices[(int)deviceId];
                dev->UnregisterDevice(this);
            }
        }

        static int RandInt(int min, int max) {
            int const range = max - min;
            int const usable_max = RAND_MAX - (RAND_MAX % range);
            int result;

            do {
                result = rand();
            } while (result >= usable_max);

            return min + (result % range);
        }

        //Resets the speccy
        virtual void Reset(bool hardReset) {
            isResetOver = false;

            DoTapeEvent(TapeEventType::STOP_TAPE);
           
            //All registers are set to 0xffff during a cold boot
            //http://worldofspectrum.org/forums/showthread.php?t=34574&page=3
            if (hardReset)
            {
                cpu.HardReset();
            }
            else {
                cpu.UserReset();
            }
            cpu.is_halted = false;
            tapeBitWasFlipped = false;
            cpu.t_states = 0;
            timeToOutSound = 0;

            ULAByteCtr = 0;
            last1ffdOut = 0;
            last7ffdOut = 0;
            lastFEOut = 0;
            lastTState = 0;
            elapsedTStates = 0;
            flashFrameCount = 0;

            pulseCounter = 0;
            repeatCount = 0;
            bitCounter = 0;
            dataCounter = 0;
            dataByte = 0;
            currentBit = 0;
            isPauseBlockPreproccess = false;

            ula_plus.Reset();

            for (auto& d : io_devices) {
                d->Reset();
            }

            timeToOutSound = 0;
            soundCounter = 0;
            averagedSound = 0;
            flashOn = false;
            lastScanlineColorCounter = 0;

            //We jiggle the wait period after resetting so that FRAMES/RANDOMIZE works randomly enough on the speccy.
            resetFrameTarget = RandInt(40, 90);        
            inputFrameTime = RandInt(0, FrameLength);
        }

        //Updates the tape state
        void UpdateTapeState(int tstates) {
            if (tapeIsPlaying && !tape_edgeDetectorRan) {
                tapeTStates += tstates;
                while (tapeTStates >= edgeDuration) {
                    tapeTStates = (int)(tapeTStates - edgeDuration);
                    DoTapeEvent(TapeEventType::EDGE_LOAD);
                }
            }
            tape_edgeDetectorRan = false;
        }

        // Reset tape state
        void ResetTape() {
            tapeBitFlipAck = false;
            tapeBitWasFlipped = false;
            tapeIsPlaying = false;
            isPauseBlockPreproccess = false;
            isProcessingPauseBlock = false;
            tape_detectionCount = 0;
            tape_diff = 0;
            tape_edgeDetectorRan = false;
            tape_FrameCount = 0;
            tape_PCatLastIn = 0;
            tape_regValue = 0;
            tape_tstatesSinceLastIn = 0;
            tape_tstatesStep = 0;
            tape_whichRegToCheck = 0;
            tape_stopTimeOut = TAPE_TIMEOUT;
            tape_detectionCount = 0;
            tape_AutoStarted = false;
            tape_PC = 0;
            tape_A = tape_B = tape_C = tape_D = tape_E = tape_H = tape_L = 0;
            edgeDuration = 0;
            blockCounter = 0;

            pulseLevel = 0;
            repeatCount = 0;
            bitCounter = 0;
            dataCounter = 0;
            dataByte = 0;
            currentBit = 0;
        }

        //Updates the tape state
        void UpdateTapePlayback() {
            if (!isProcessingPauseBlock) {
                while (tapeTStates >= edgeDuration) {
                    tapeTStates = (int)(tapeTStates - edgeDuration);

                    DoTapeEvent(TapeEventType::EDGE_LOAD);
                }
            }
            else
                FlashLoad();
        }

        //Shutsdown the speccy
        virtual void Shutdown() {
            //THREAD
            //if (!isSuspended)
            //{
            //    doRun = false;
            //    emulationThread.Join();
            //    emulationThread = null;
            // }

            //lock (lockThis) {
                beeper.Shutdown();
                contentionTable.clear();
                floatingBusTable.clear();
                ScreenBuffer.clear();
                screen.clear();
                attr.clear();
                tstateToDisp.clear();
                keyBuffer.clear();
            //}
        }

        void OnTapeEdgeDecA() {
            if (tapeIsPlaying && tape_edgeLoad)
                if (PeekByteNoContend(cpu.regs.PC) == 0x20)
                    if (PeekByteNoContend((ushort)(cpu.regs.PC + 1)) == 0xfd) {
                        if (cpu.regs.A != 0) {
                            int _a = cpu.regs.A;
                            _a--;
                            _a <<= 4;
                            _a += 4 + 7 + 12;
                            tapeTStates += _a;
                            cpu.regs.PC += 2;
                            cpu.regs.A = 0;
                            cpu.regs.F |= 64;
                        }
                    }
        }

        void OnTapeEdgeCpA() {
            if (tape_readToPlay && !tapeTrapsDisabled)
                if(cpu.regs.PC == 0x56b)
                    FlashLoadTape();
        }

        //Re-engineered SpecEmu version. Works a treat!
        void OnTapeEdgeDetection() {
            //Return if not tape is inserted in Tape Deck
            if (!tape_readToPlay || !tape_edgeLoad)
                return;

            if (tapeIsPlaying) {
                if (cpu.regs.PC == tape_PCatLastIn) {
                    if (tape_AutoPlay) {
                        tape_stopTimeOut = TAPE_TIMEOUT;
                        tape_AutoStarted = true;
                    }

                    if (tape_edgeLoad) {
                        if (tapeBitWasFlipped) {
                            tapeBitFlipAck = true;
                            tapeBitWasFlipped = false;
                            return;
                        } else {
                            //bool doLoop = false;
                            switch (tape_whichRegToCheck) {
                                case 1:
                                    tape_regValue = cpu.regs.A;
                                    break;

                                case 2:
                                    tape_regValue = cpu.regs.B;
                                    break;

                                case 3:
                                    tape_regValue = cpu.regs.C;
                                    break;

                                case 4:
                                    tape_regValue = cpu.regs.D;
                                    break;

                                case 5:
                                    tape_regValue = cpu.regs.E;
                                    break;

                                case 6:
                                    tape_regValue = cpu.regs.H;
                                    break;

                                case 7:
                                    tape_regValue = cpu.regs.L;
                                    break;

                                default:
                                    //doLoop = false;
                                    return;
                            }

                            tape_edgeDetectorRan = true;
                            while (!((tape_regValue == 255) || (tape_regValue == 1))) {
                                if (tapeBitFlipAck)
                                    tapeBitWasFlipped = false;

                                tapeTStates += (cpu.t_states - prevT);

                                 if (tapeBitWasFlipped) {
                                    tapeBitFlipAck = true;
                                    return;
                                }

                                if (tapeTStates >= edgeDuration) {
                                    tapeTStates = (int)(tapeTStates - edgeDuration);

                                    DoTapeEvent(TapeEventType::EDGE_LOAD);
                                }

                               
                                tapeTStates += tape_tstatesStep;
                                switch (tape_whichRegToCheck) {
                                    case 1:
                                        cpu.regs.A += (byte)tape_diff;
                                        tape_regValue = cpu.regs.A;
                                        break;

                                    case 2:
                                        cpu.regs.B += (byte)tape_diff;
                                        tape_regValue = cpu.regs.B;
                                        break;

                                    case 3:
                                        cpu.regs.C += (byte)tape_diff;
                                        tape_regValue = cpu.regs.C;
                                        break;

                                    case 4:
                                        cpu.regs.D += (byte)tape_diff;
                                        tape_regValue = cpu.regs.D;
                                        break;

                                    case 5:
                                        cpu.regs.E += (byte)tape_diff;
                                        tape_regValue = cpu.regs.E;
                                        break;

                                    case 6:
                                        cpu.regs.H += (byte)tape_diff;
                                        tape_regValue = cpu.regs.H;
                                        break;

                                    case 7:
                                        cpu.regs.L += (byte)tape_diff;
                                        tape_regValue = cpu.regs.L;
                                        break;

                                    default:
                                        //doLoop = false;
                                        break;
                                }
                            }
                        }
                    }
                    tape_tstatesSinceLastIn = cpu.t_states;
                }
            } else {
                if (FrameCount != tape_FrameCount)
                    tape_detectionCount = 0;

                int elapsedTapeTstates = cpu.t_states - tape_tstatesSinceLastIn;
                if (((elapsedTapeTstates > 0) && (elapsedTapeTstates < 96)) && (cpu.regs.PC == tape_PC)) {
                    tape_tstatesStep = elapsedTapeTstates;
                    //which reg has changes since last IN
                    int numRegsThatHaveChanged = 0;
                    tape_diff = 0;

                    if (tape_A != cpu.regs.A) {
                        tape_regValue = cpu.regs.A;
                        tape_whichRegToCheck = 1;
                        tape_diff = tape_A - cpu.regs.A;
                        numRegsThatHaveChanged++;
                    }
                    if (tape_B != cpu.regs.B) {
                        tape_regValue = cpu.regs.B;
                        tape_whichRegToCheck = 2;
                        tape_diff = tape_B - cpu.regs.B;
                        numRegsThatHaveChanged++;
                    }
                    if (tape_C != cpu.regs.C) {
                        tape_regValue = cpu.regs.C;
                        tape_whichRegToCheck = 3;
                        tape_diff = tape_C - cpu.regs.C;
                        numRegsThatHaveChanged++;
                    }
                    if (tape_D != cpu.regs.D) {
                        tape_regValue = cpu.regs.D;
                        tape_whichRegToCheck = 4;
                        tape_diff = tape_D - cpu.regs.D;
                        numRegsThatHaveChanged++;
                    }
                    if (tape_E != cpu.regs.E) {
                        tape_regValue = cpu.regs.E;
                        tape_whichRegToCheck = 5;
                        tape_diff = tape_E - cpu.regs.E;
                        numRegsThatHaveChanged++;
                    }
                    if (tape_H != cpu.regs.H) {
                        tape_regValue = cpu.regs.H;
                        tape_whichRegToCheck = 6;
                        tape_diff = tape_H - cpu.regs.H;
                        numRegsThatHaveChanged++;
                    }
                    if (tape_L != cpu.regs.L) {
                        tape_regValue = cpu.regs.L;
                        tape_whichRegToCheck = 7;
                        tape_diff = tape_L - cpu.regs.L;
                        numRegsThatHaveChanged++;
                    }

                    tape_A = cpu.regs.A;
                    tape_B = cpu.regs.B;
                    tape_C = cpu.regs.C;
                    tape_D = cpu.regs.D;
                    tape_E = cpu.regs.E;
                    tape_H = cpu.regs.H;
                    tape_L = cpu.regs.L;

                    tape_diff *= -1;
                    if (numRegsThatHaveChanged == 1) //Has only 1 reg changed?
                    {
                        if (tape_diff == 1 || tape_diff == -1)    //Changed only by +1 or -1
                        {
                            tape_detectionCount++;
                            if (tape_detectionCount >= 8) //Is the above true 8 times?
                            {
                                if (tape_AutoPlay) {
                                    tapeIsPlaying = true;
                                    tape_AutoStarted = true;
                                    tape_stopTimeOut = TAPE_TIMEOUT;
                                    //if (TapeEvent != null)
                                    //    OnTapeEvent(new TapeEventArgs(TapeEventType.START_TAPE)); //Yes! Start tape!
                                    DoTapeEvent(TapeEventType::START_TAPE);
                                    tapeTStates = 0;
                                }
                                tape_PCatLastIn = cpu.regs.PC;
                            }
                        }
                    }
                }
                tape_tstatesSinceLastIn = cpu.t_states;
                tape_FrameCount = FrameCount;
                tape_A = cpu.regs.A;
                tape_B = cpu.regs.B;
                tape_C = cpu.regs.C;
                tape_D = cpu.regs.D;
                tape_E = cpu.regs.E;
                tape_H = cpu.regs.H;
                tape_L = cpu.regs.L;
                tape_PC = cpu.regs.PC;
            }
        }

        //The main loop which executes opcodes repeatedly till 1 frame (69888 tstates)
        //has been generated.
        int NO_PAINT_REP = 10;

        void Run() {
            for (int rep = 0; rep < /*(tapeIsPlaying && tape_flashLoad ? NO_PAINT_REP :*/ emulationSpeed; rep++)
            {
                while (doRun)
                {
                    //Raise event for debugger
                    //OpcodeExecutedEvent?.Invoke(this);
                    
                    //lock (lockThis)
                    {
                        //Tape Save trap is active only if lower ROM is 48k
                        if (cpu.regs.PC == 0x04d1 && !tapeTrapsDisabled && lowROMis48K)
                        {
                            OnTapeEvent(TapeEventType::SAVE_TAP);
                            cpu.regs.IX = (ushort)(cpu.regs.IX + cpu.regs.DE);
                            cpu.regs.DE = 0;
                            cpu.regs.PC = 1342;
                            ResetKeyboard();
                        }

                        if (doRun)
                            Process();

                    } //lock

                    if (needsPaint) {

                        if (tapeIsPlaying) {
                            if (tape_AutoPlay && tape_AutoStarted) {
                                if (!(isPauseBlockPreproccess && (edgeDuration > 0) && cpu.and_32_Or_64)) {
                                    if (tape_stopTimeOut <= 0) {
                                        // if (TapeEvent != null)
                                        //     OnTapeEvent(new TapeEventArgs(TapeEventType.STOP_TAPE)); //stop the tape!
                                        DoTapeEvent(TapeEventType::STOP_TAPE);
                                        tape_AutoStarted = false;
                                    }
                                    else
                                        tape_stopTimeOut--;
                                }
                            }
                        }

                        FrameCount++;
                        if (FrameCount >= 50) {
                            FrameCount = 0;
                        }

                        if (!externalSingleStep && emulationSpeed == 1) {
                            while (!beeper.FinishedPlaying() && !tapeIsPlaying)
                                ;//System.Threading.Thread.Sleep(1);
                        }

                        if (emulationSpeed > 1 && rep != emulationSpeed - 1)
                        {
                            needsPaint = false;
                            //System.Threading.Thread.Sleep(1); //TO DO: Remove?
                        }
                    
                        break;
                    }

                    if (externalSingleStep)
                        break;
                } //run loop
            }
        }

        //Sets the sound volume of the beeper/ay
        void SetSoundVolume(float vol) {
            soundVolume = vol;
            beeper.SetVolume(vol);
        }

        //Turns off the sound
        void MuteSound(bool isMute) {
            if (isMute)
                beeper.SetVolume(0.0f);
            else
                beeper.SetVolume(soundVolume);
        }

        //Turns on the sound
        void ResumeSound() {
            beeper.Play();
        }

        //Same as PeekByte, except specifically for opcode fetches in order to
        //trigger Memory Execute in debugger.
        byte GetOpcode(int addr) {
            addr &= 0xffff;
            //Contend(addr, 3, 1);
            if (IsContended(addr)) {
                cpu.t_states += contentionTable[cpu.t_states];
            }

            cpu.t_states += 3;

            int page = (addr) >> 13;
            int offset = (addr) & 0x1FFF;
            byte _b = PageReadPointer[page][offset];

            //This call flags a memory change event for the debugger
            //if (MemoryExecuteEvent != null)
                OnMemoryExecuteEvent(addr, _b);

            return _b;
        }

        //Returns the byte at a given 16 bit address (can be contended)
        byte PeekByte(ushort addr) {
            //Contend(addr, 3, 1);
            if (IsContended(addr)) {
                cpu.t_states += contentionTable[cpu.t_states];
            }

            cpu.t_states += 3;

            int page = (addr) >> 13;
            int offset = (addr) & 0x1FFF;
            byte _b = PageReadPointer[page][offset];

            //This call flags a memory change event for the debugger
            //if (MemoryReadEvent != null)
                OnMemoryReadEvent(addr, _b);

            return _b;
        }

        //Returns the byte at a given 16 bit address (can be contended)
        virtual void PokeByte(ushort addr, byte b) {
            //This call flags a memory change event for the debugger
            //if (MemoryWriteEvent != null)
                OnMemoryWriteEvent(addr, b);

            if (IsContended(addr)) {
                cpu.t_states += contentionTable[cpu.t_states];
            }
            cpu.t_states += 3;
            int page = (addr) >> 13;
            int offset = (addr) & 0x1FFF;

            if (((addr & 49152) == 16384) && (PageReadPointer[page][offset] != b)) {
                UpdateScreenBuffer(cpu.t_states);
            }

            PageWritePointer[page][offset] = b;
        }

        //Pokes a 16 bit value at given address. Contention applies.
        void PokeWord(ushort addr, ushort w) {
            PokeByte(addr, (byte)(w & 0xff));
            PokeByte((ushort)(addr + 1), (byte)(w >> 8));
        }

        //Pokes bytes from an array into a ram bank.
        void PokeRAMPage(int bank, int dataLength, std::vector<byte> const& data) {
            for (int f = 0; f < dataLength; f++) {
                int indx = f / 8192;
                RAMpage[bank * 2 + indx][f % 8192] = data[f];
            }
        }

        //Pokes bytes from an array into contiguous rom banks.
        void PokeROMPages(int bank, int dataLength, std::vector<byte> const& data) {
            for (int f = 0; f < dataLength; f++) {
                int indx = f / 8192;
                ROMpage[bank * 2 + indx][f % 8192] = data[f];
            }
        }

        //Pokes the byte at a given 16 bit address with no contention
        void PokeByteNoContend(int addr, int b) {
            addr &= 0xffff;
            b &= 0xff;

            int page = (addr) >> 13;
            int offset = (addr) & 0x1FFF;

            PageWritePointer[page][offset] = (byte)b;
        }

        //Pokes  byte from an array at a given 16 bit address with no contention
        void PokeBytesNoContend(int addr, int dataOffset, int dataLength, std::vector<byte> const& data) {
            int page, offset;

            for (int f = dataOffset; f < dataOffset + dataLength; f++, addr++) {
                addr &= 0xffff;
                page = (addr) >> 13;
                offset = (addr) & 0x1FFF;
                PageWritePointer[page][offset] = data[f];
            }
        }

        //Returns a value from a port (can be contended)
        virtual byte In(ushort port) {
            //Raise a port I/O event
            //if (PortEvent != null)
                OnPortEvent(port, 0, false);

            return 0;
        }

        //Used purely to raise an event with the debugger for IN with a specific value
        virtual void In(ushort port, byte val) {
            //Raise a port I/O event
            //if (PortEvent != null)
                OnPortEvent(port, val, false);
        }

        //Outputs a value to a port (can be contended)
        //The base call is used only to raise memory events
        virtual void Out(ushort port, byte val) {
            //Raise a port I/O event
            //if (PortEvent != null)
                OnPortEvent(port, val, true);
        }

        virtual bool IsKempstonActive(int port) {
            if (UseKempstonPort1F) {
                if ((port & 0xe0) == 0)
                    return true;
            }
            else if ((port & 0x20) == 0)
                return true;

            return false;
        }
        //Updates the state of the renderer
        virtual void UpdateScreenBuffer(int _tstates) {
            if (_tstates < ActualULAStart) {
                return;
            } else if (_tstates >= FrameLength) {
                _tstates = FrameLength - 1;
                //Since we're updating the entire screen here, we don't need to re-paint
                //it again in the  process loop on framelength overflow.

                needsPaint = true;
            }

            //the additional 1 tstate is required to get correct number of bytes to output in ircontention.sna
            elapsedTStates = (_tstates + 1 - lastTState);

            //It takes 4 tstates to write 1 byte. Or, 2 pixels per t-state.

            int numBytes = (elapsedTStates >> 2) + ((elapsedTStates % 4) > 0 ? 1 : 0);

            int pixelData;
            int pixel2Data = 0xff;
            int attrData;
            int attr2Data;
            int bright;
            int ink;
            int paper;
            int flash;

            for (int i = 0; i < numBytes; i++) {
                if (tstateToDisp[lastTState] > 1) {
                   // for (int p = 0; p < 2; p++) {
                        screenByteCtr = tstateToDisp[lastTState] - 16384; //adjust for actual screen offset
                  
                        pixelData = screen[screenByteCtr];
                        attrData = screen[attr[screenByteCtr] - 16384];

                        lastPixelValue = pixelData;
                        lastAttrValue = attrData;
                       /* if ((I & 0x40) == 0x40) {
                            {
                                if (p == 0) {
                                    screenByteCtr = (screenByteCtr + 16384) | (R & 0xff);
                                    pixel2Data = screenByteCtr & 0xff;
                                }
                                else
                                    screenByteCtr = (screenByteCtr + 16384) | (pixel2Data);
                                pixelData = screen[(screenByteCtr - 1) - 16384];
                                lastAttrValue = screen[attr[(screenByteCtr - 1) - 16384] - 16384];
                            }
                        }
                        */
                        bright = (attrData & 0x40) >> 3;
                        flash = (attrData & 0x80) >> 7;
                        ink = (attrData & 0x07);
                        paper = ((attrData >> 3) & 0x7);
                        int paletteInk = AttrColors[ink + bright];
                        int palettePaper = AttrColors[paper + bright];

                        if (flashOn && (flash != 0)) //swap paper and ink when flash is on
                        {
                            int temp = paletteInk;
                            paletteInk = palettePaper;
                            palettePaper = temp;
                        }

                        if (ula_plus.Enabled && ula_plus.PaletteEnabled) {
                            paletteInk = ula_plus.Palette[(((flash << 1) + (bright >> 3)) << 4) + ink]; //(flash*2 + bright) * 16 + ink
                            palettePaper = ula_plus.Palette[(((flash << 1) + (bright >> 3)) << 4) + paper + 8]; //(flash*2 + bright) * 16 + paper + 8
                        }

                        for (int a = 0; a < 8; ++a) {
                            if ((pixelData & 0x80) != 0) {
                                //PAL interlacing
                                //int pal = paletteInk/2 + (0xffffff - LastScanlineColor[lastScanlineColorCounter]/2);
                                //ScreenBuffer[ULAByteCtr++] = pal;
                                //LastScanlineColor[lastScanlineColorCounter++] = paletteInk;
                                ScreenBuffer[ULAByteCtr++] = paletteInk;
                                lastAttrValue = ink;
                            } else {
                                //PAL interlacing
                                //int pal = palettePaper/2 + (0xffffff - LastScanlineColor[lastScanlineColorCounter]/2);
                                //ScreenBuffer[ULAByteCtr++] = pal;
                                //LastScanlineColor[lastScanlineColorCounter++] = palettePaper;
                                ScreenBuffer[ULAByteCtr++] = palettePaper;
                                lastAttrValue = paper;
                            }
                            pixelData <<= 1;
                        }                 
                    // pixelData = lastPixelValue;
                } else if (tstateToDisp[lastTState] == 1) {
                    int bor;
                    if (ula_plus.Enabled && ula_plus.PaletteEnabled) {
                        bor = ula_plus.Palette[borderColour + 8];
                    } else
                        bor = AttrColors[borderColour];

                    for(int g = 0; g < 8; g++) {
                        //PAL interlacing
                        //int pal = bor/2 + (0xffffff -  LastScanlineColor[lastScanlineColorCounter]/2);
                        //ScreenBuffer[ULAByteCtr++] = pal;
                        //LastScanlineColor[lastScanlineColorCounter++] = bor;                        
                        ScreenBuffer[ULAByteCtr++] = bor;
                    }
                }
                lastTState += 4;

                if(lastScanlineColorCounter >= ScanLineWidth)
                    lastScanlineColorCounter = 0;
            }
        }

        // Wrapper for ULA events
        void UpdateScreenBuffer() {
            UpdateScreenBuffer(cpu.t_states);
        }

        //Loads in the ROM for the machine
        virtual bool LoadROM(string path, string filename) = 0;

        //Updates the state of all inputs from the user
        void UpdateInput() {

            if (keyBuffer[(int)keyCode::SHIFT]) {
                keyLine[0] = keyLine[0] & ~(0x1);
            } else {
                keyLine[0] = keyLine[0] | (0x1);
            }

            if (keyBuffer[(int)keyCode::Z]) {
                keyLine[0] = keyLine[0] & ~(0x02);
            } else {
                keyLine[0] = keyLine[0] | (0x02);
            }

            if (keyBuffer[(int)keyCode::X]) {
                keyLine[0] = keyLine[0] & ~(0x04);
            } else {
                keyLine[0] = keyLine[0] | (0x04);
            }

            if (keyBuffer[(int)keyCode::C]) {
                keyLine[0] = keyLine[0] & ~(0x08);
            } else {
                keyLine[0] = keyLine[0] | (0x08);
            }

            if (keyBuffer[(int)keyCode::V]) {
                keyLine[0] = keyLine[0] & ~(0x10);
            } else {
                keyLine[0] = keyLine[0] | (0x10);
            }

            if (keyBuffer[(int)keyCode::A]) {
                keyLine[1] = keyLine[1] & ~(0x1);
            } else {
                keyLine[1] = keyLine[1] | (0x1);
            }

            if (keyBuffer[(int)keyCode::S]) {
                keyLine[1] = keyLine[1] & ~(0x02);
            } else {
                keyLine[1] = keyLine[1] | (0x02);
            }

            if (keyBuffer[(int)keyCode::D]) {
                keyLine[1] = keyLine[1] & ~(0x04);
            } else {
                keyLine[1] = keyLine[1] | (0x04);
            }

            if (keyBuffer[(int)keyCode::F]) {
                keyLine[1] = keyLine[1] & ~(0x08);
            } else {
                keyLine[1] = keyLine[1] | (0x08);
            }

            if (keyBuffer[(int)keyCode::G]) {
                keyLine[1] = keyLine[1] & ~(0x10);
            } else {
                keyLine[1] = keyLine[1] | (0x10);
            }

            if (keyBuffer[(int)keyCode::Q]) {
                keyLine[2] = keyLine[2] & ~(0x1);
            } else {
                keyLine[2] = keyLine[2] | (0x1);
            }

            if (keyBuffer[(int)keyCode::W]) {
                keyLine[2] = keyLine[2] & ~(0x02);
            } else {
                keyLine[2] = keyLine[2] | (0x02);
            }

            if (keyBuffer[(int)keyCode::E]) {
                keyLine[2] = keyLine[2] & ~(0x04);
            } else {
                keyLine[2] = keyLine[2] | (0x04);
            }

            if (keyBuffer[(int)keyCode::R]) {
                keyLine[2] = keyLine[2] & ~(0x08);
            } else {
                keyLine[2] = keyLine[2] | (0x08);
            }

            if (keyBuffer[(int)keyCode::T]) {
                keyLine[2] = keyLine[2] & ~(0x10);
            } else {
                keyLine[2] = keyLine[2] | (0x10);
            }

            if (keyBuffer[(int)keyCode::_1]) {
                keyLine[3] = keyLine[3] & ~(0x1);
            } else {
                keyLine[3] = keyLine[3] | (0x1);
            }

            if (keyBuffer[(int)keyCode::_2]) {
                keyLine[3] = keyLine[3] & ~(0x02);
            } else {
                keyLine[3] = keyLine[3] | (0x02);
            }

            if (keyBuffer[(int)keyCode::_3]) {
                keyLine[3] = keyLine[3] & ~(0x04);
            } else {
                keyLine[3] = keyLine[3] | (0x04);
            }

            if (keyBuffer[(int)keyCode::_4]) {
                keyLine[3] = keyLine[3] & ~(0x08);
            } else {
                keyLine[3] = keyLine[3] | (0x08);
            }

            if (keyBuffer[(int)keyCode::_5]) {
                keyLine[3] = keyLine[3] & ~(0x10);
            } else {
                keyLine[3] = keyLine[3] | (0x10);
            }

            if (keyBuffer[(int)keyCode::_0]) {
                keyLine[4] = keyLine[4] & ~(0x1);
            } else {
                keyLine[4] = keyLine[4] | (0x1);
            }

            if (keyBuffer[(int)keyCode::_9]) {
                keyLine[4] = keyLine[4] & ~(0x02);
            } else {
                keyLine[4] = keyLine[4] | (0x02);
            }

            if (keyBuffer[(int)keyCode::_8]) {
                keyLine[4] = keyLine[4] & ~(0x04);
            } else {
                keyLine[4] = keyLine[4] | (0x04);
            }

            if (keyBuffer[(int)keyCode::_7]) {
                keyLine[4] = keyLine[4] & ~(0x08);
            } else {
                keyLine[4] = keyLine[4] | (0x08);
            }

            if (keyBuffer[(int)keyCode::_6]) {
                keyLine[4] = keyLine[4] & ~(0x10);
            } else {
                keyLine[4] = keyLine[4] | (0x10);
            }

            if (keyBuffer[(int)keyCode::P]) {
                keyLine[5] = keyLine[5] & ~(0x1);
            } else {
                keyLine[5] = keyLine[5] | (0x1);
            }

            if (keyBuffer[(int)keyCode::O]) {
                keyLine[5] = keyLine[5] & ~(0x02);
            } else {
                keyLine[5] = keyLine[5] | (0x02);
            }

            if (keyBuffer[(int)keyCode::I]) {
                keyLine[5] = keyLine[5] & ~(0x04);
            } else {
                keyLine[5] = keyLine[5] | (0x04);
            }

            if (keyBuffer[(int)keyCode::U]) {
                keyLine[5] = keyLine[5] & ~(0x08);
            } else {
                keyLine[5] = keyLine[5] | (0x08);
            }

            if (keyBuffer[(int)keyCode::Y]) {
                keyLine[5] = keyLine[5] & ~(0x10);
            } else {
                keyLine[5] = keyLine[5] | (0x10);
            }

            if (keyBuffer[(int)keyCode::ENTER]) {
                keyLine[6] = keyLine[6] & ~(0x1);
            } else {
                keyLine[6] = keyLine[6] | (0x1);
            }

            if (keyBuffer[(int)keyCode::L]) {
                keyLine[6] = keyLine[6] & ~(0x02);
            } else {
                keyLine[6] = keyLine[6] | (0x02);
            }

            if (keyBuffer[(int)keyCode::K]) {
                keyLine[6] = keyLine[6] & ~(0x04);
            } else {
                keyLine[6] = keyLine[6] | (0x04);
            }

            if (keyBuffer[(int)keyCode::J]) {
                keyLine[6] = keyLine[6] & ~(0x08);
            } else {
                keyLine[6] = keyLine[6] | (0x08);
            }

            if (keyBuffer[(int)keyCode::H]) {
                keyLine[6] = keyLine[6] & ~(0x10);
            } else {
                keyLine[6] = keyLine[6] | (0x10);
            }

            if (keyBuffer[(int)keyCode::SPACE]) {
                keyLine[7] = keyLine[7] & ~(0x1);
            } else {
                keyLine[7] = keyLine[7] | (0x1);
            }

            if (keyBuffer[(int)keyCode::CTRL]) {
                keyLine[7] = keyLine[7] & ~(0x02);
            } else {
                keyLine[7] = keyLine[7] | (0x02);
            }

            if (keyBuffer[(int)keyCode::M]) {
                keyLine[7] = keyLine[7] & ~(0x04);
            } else {
                keyLine[7] = keyLine[7] | (0x04);
            }

            if (keyBuffer[(int)keyCode::N]) {
                keyLine[7] = keyLine[7] & ~(0x08);
            } else {
                keyLine[7] = keyLine[7] | (0x08);
            }

            if (keyBuffer[(int)keyCode::B]) {
                keyLine[7] = keyLine[7] & ~(0x10);
            } else {
                keyLine[7] = keyLine[7] | (0x010);
            }

            //Check for caps lock key
            if (keyBuffer[(int)keyCode::CAPS]) {
                CapsLockOn = !CapsLockOn;
                keyBuffer[(int)keyCode::CAPS] = false;
            }

            if (CapsLockOn) {
                keyLine[0] = keyLine[0] & ~(0x1);
            }

            //Check if backspace key has been pressed (Caps Shift + 0 equivalent)
            if (keyBuffer[(int)keyCode::BACK]) {
                keyLine[0] = keyLine[0] & ~(0x1);
                keyLine[4] = keyLine[0] & ~(0x1);
            }

            //Check if left cursor key has been pressed (Caps Shift + 5)
            if (keyBuffer[(int)keyCode::LEFT]) {
                keyLine[0] = keyLine[0] & ~(0x1);
                keyLine[3] = keyLine[3] & ~(0x10);
            }

            //Check if right cursor key has been pressed (Caps Shift + 8)
            if (keyBuffer[(int)keyCode::RIGHT]) {
                keyLine[0] = keyLine[0] & ~(0x1);
                keyLine[4] = keyLine[4] & ~(0x04);
            }

            //Check if up cursor key has been pressed (Caps Shift + 7)
            if (keyBuffer[(int)keyCode::UP]) {
                keyLine[0] = keyLine[0] & ~(0x1);
                keyLine[4] = keyLine[4] & ~(0x08);
            }

            //Check if down cursor key has been pressed (Caps Shift + 6)
            if (keyBuffer[(int)keyCode::DOWN]) {
                keyLine[0] = keyLine[0] & ~(0x1);
                keyLine[4] = keyLine[4] & ~(0x10);
            }
        }

        //Resets the state of all the keys
        void ResetKeyboard() {
            for (int f = 0; f < keyBuffer.size(); f++)
                keyBuffer[f] = false;

            for (int f = 0; f < 8; f++)
                keyLine[f] = 255;
        }

        //Updates audio state, called from Process()
        void UpdateAudio(int dt) {
            for (auto& ad : audio_devices) {
                ad->Update(dt);
            }
            averagedSound += soundOut;
            soundCounter++;
        }

        // The HALT behavior is incorrect in most emulators where PC is decremented so
        // that HALT is executed again. The correct behaviour is:
        // "When HALT is low PC has already been incremented and the opcode fetched is for the instruction after HALT.
        // The halt state stops this instruction from being executed and PC from incrementing so this opcode is read
        // again and again until an exit condition occurs. When an interrupt occurs during the halt state PC is pushed
        // unchanged onto the stack as it is already the correct return address."
        // When loading or saving snapshots we need to account for the old behaviour and increment PC so that we emulate
        // correctly post-load or during save.
        virtual void CorrectPCForHalt() {
            if (cpu.is_halted) {
                cpu.regs.PC++;
            }
        }

        virtual void SaveSNA(SNA_SNAPSHOT* snapshot) {
            if (model == MachineModel::_48k || model == MachineModel::_NTSC48k)
                snapshot->TYPE = 0;
            else
                snapshot->TYPE = 1;
                
            snapshot->I = (byte)cpu.regs.I;
            snapshot->H_ = cpu.regs.HL_ >> 8;
            snapshot->L_ = cpu.regs.HL_ & 255;
            snapshot->D_ = cpu.regs.DE_ >> 8;
            snapshot->E_ = cpu.regs.DE_ & 255;
            snapshot->B_ = cpu.regs.BC_ >> 8;
            snapshot->C_ = cpu.regs.BC_ & 255;
            snapshot->A_ = cpu.regs.AF_ >> 8;
            snapshot->F_ = cpu.regs.AF_ & 255;

            snapshot->H = cpu.regs.H;
            snapshot->L = cpu.regs.L;
            snapshot->D = cpu.regs.D;
            snapshot->E = cpu.regs.E;
            snapshot->B = cpu.regs.B;
            snapshot->C = cpu.regs.C;
            snapshot->IYH = cpu.regs.IYH;
            snapshot->IYL = cpu.regs.IYL;
            snapshot->IXH = cpu.regs.IXH;
            snapshot->IXL = cpu.regs.IXL;

            snapshot->IFF2 = cpu.iff_1 ? 1 << 2 : 0;
            snapshot->R = cpu.regs.R;
            snapshot->A = cpu.regs.A;
            snapshot->F = cpu.regs.F;
        
            snapshot->IM = cpu.interrupt_mode;
            snapshot->BORDER = borderColour;

            if (model == MachineModel::_48k || model == MachineModel::_NTSC48k) {
                ushort snap_pc = cpu.regs.PC;
                if (cpu.is_halted) {
                    snap_pc--;
                }
                cpu.PushStack(snap_pc);
                snapshot->SPH = cpu.regs.SP >> 8;
                snapshot->SPL = cpu.regs.SP & 255;

                int screenAddr = DisplayStart;

                for (int f = 0; f < 49152; ++f)
                    snapshot->RAM[f] = PeekByteNoContend((ushort)(screenAddr +f));

                cpu.PopStack(); //Ignore the PC that will be popped.
            }
            else {
                snapshot->SPH = cpu.regs.SP >> 8;
                snapshot->SPL = cpu.regs.SP & 255;

                if (cpu.is_halted) {
                    snapshot->PCH = (cpu.regs.PC - 1) >> 8;
                    snapshot->PCL = (cpu.regs.PC - 1) & 255;
                }
                else {
                    snapshot->PCH = cpu.regs.PC >> 8;
                    snapshot->PCL = cpu.regs.PC & 255;
                }

                snapshot->PORT_7FFD = last7ffdOut;
                snapshot->TR_DOS = trDosPagedIn ? 1 : 0;
                
                memcpy(snapshot->RAM_BANK[0], RAMpage[(int)RAM_BANK::FIVE_LOW], 8192);
                memcpy(snapshot->RAM_BANK[1], RAMpage[(int)RAM_BANK::FIVE_HIGH], 8192);

                memcpy(snapshot->RAM_BANK[2], RAMpage[(int)RAM_BANK::TWO_LOW], 8192);
                memcpy(snapshot->RAM_BANK[3], RAMpage[(int)RAM_BANK::TWO_HIGH], 8192);

                int BankInPage4 = snapshot->PORT_7FFD & 0x07;

                memcpy(snapshot->RAM_BANK[4], RAMpage[BankInPage4 * 2], 8192);
                memcpy(snapshot->RAM_BANK[5], RAMpage[BankInPage4 * 2 + 1], 8192);

                int t = 3;
                for (int f = 0; f < 8; f++) {
                    if (f == 5 || f == 2 || f == BankInPage4)
                        continue;

                    memcpy(snapshot->RAM_BANK[t * 2], RAMpage[f * 2], 8192);
                    memcpy(snapshot->RAM_BANK[t * 2 + 1], RAMpage[f * 2 + 1], 8192);
                    t++;
                }
            }
        }
        
        //Sets the speccy state to that of the SNA file
        virtual void UseSNA(SNA_SNAPSHOT const* sna) {
            cpu.regs.I = sna->I;
            cpu.regs.HL_ = (ushort)sna->H_ << 8 | sna->L_;
            cpu.regs.DE_ = (ushort)sna->D_ << 8 | sna->E_;
            cpu.regs.BC_ = (ushort)sna->B_ << 8 | sna->C_;
            cpu.regs.AF_ = (ushort)sna->A_ << 8 | sna->F_;

            cpu.regs.HL = (ushort)sna->H << 8 | sna->L;
            cpu.regs.DE = (ushort)sna->D << 8 | sna->E;
            cpu.regs.BC = (ushort)sna->B << 8 | sna->C;
            cpu.regs.IY = (ushort)sna->IYH << 8 | sna->IYL;
            cpu.regs.IX = (ushort)sna->IXH << 8 | sna->IXL;

            cpu.iff_1 = ((sna->IFF2 & 0x04) != 0);

            if (cpu.iff_1)
                cpu.interrupt_count = 1;        //force ignore re-triggered interrupts

            cpu.regs.R = sna->R;
            cpu.regs.R_ = (byte)(cpu.regs.R & 0x80);
            cpu.regs.AF = (ushort)sna->A << 8 | sna->F;
            cpu.regs.SP = (ushort)sna->SPH << 8 | sna->SPL;
            cpu.interrupt_mode = sna->IM;
            borderColour = sna->BORDER;
        }

        //Sets the speccy state to that of the SNA file
        public virtual void UseSZX(SZXFile szx) {
            cpu.regs.I = szx.z80Regs.I;
            cpu.regs.HL_ = szx.z80Regs.HL1;
            cpu.regs.DE_ = szx.z80Regs.DE1;
            cpu.regs.BC_ = szx.z80Regs.BC1;
            cpu.regs.AF_ = szx.z80Regs.AF1;
            cpu.regs.HL = szx.z80Regs.HL;
            cpu.regs.DE = szx.z80Regs.DE;
            cpu.regs.BC = szx.z80Regs.BC;
            cpu.regs.IY = szx.z80Regs.IY;
            cpu.regs.IX = szx.z80Regs.IX;
            cpu.iff_1 = (szx.z80Regs.IFF1 != 0);
            cpu.regs.R = szx.z80Regs.R;
            cpu.regs.R_ = (byte)(cpu.regs.R & 0x80);
            cpu.regs.AF = szx.z80Regs.AF;
            cpu.regs.SP = szx.z80Regs.SP;
            cpu.interrupt_mode = szx.z80Regs.IM;
            cpu.regs.PC = szx.z80Regs.PC;
            cpu.interrupt_count = (byte)((szx.z80Regs.Flags & SZXFile.ZXSTZF_EILAST) != 0 ? 2 : 0);
            cpu.is_halted = (szx.z80Regs.Flags & SZXFile.ZXSTZF_HALTED) != 0;

            CorrectPCForHalt();

            Issue2Keyboard = (szx.keyboard.Flags & SZXFile.ZXSTKF_ISSUE2) != 0;
            
            if (szx.paletteLoaded)
            {
                if (ula_plus == null) {
                    ula_plus = new ULA_Plus();
                    AddDevice(ula_plus);
                }
                ula_plus.PaletteEnabled = szx.palette.flags > 0 ? true : false;
                ula_plus.PaletteGroup = szx.palette.currentRegister;

                for (int f = 0; f < 64 ; f++)
                {
                    byte val = szx.palette.paletteRegs[f];

                    //3 bits to 8 bits to be stored as hmlhmlml for each color

                    //First get B
                    int bh = (val & 0x2) >> 1;
                    int bl = val & 0x1;
                    int bm = bl;
                    int B = (bh << 7) | (bm << 6) | (bl << 5) | (bh << 4) | (bm << 3) | (bl << 2) | (bm << 1) | bl;

                    //R
                    int rl = (val & 0x4) >> 2;
                    int rm = (val & 0x8) >> 3;
                    int rh = (val & 0x10) >> 4;

                    int R = (rh << 7) | (rm << 6) | (rl << 5) | (rh << 4) | (rm << 3) | (rl << 2) | (rm << 1) | rl;

                    //G
                    int gl = (val & 0x20) >> 5;
                    int gm = (val & 0x40) >> 6;
                    int gh = (val & 0x80) >> 7;

                    int G = (gh << 7) | (gm << 6) | (gl << 5) | (gh << 4) | (gm << 3) | (gl << 2) | (gm << 1) | gl;

                    ula_plus.Palette[f] = (R << 16) | (G << 8) | B;
                }
            }

            if (szx.header.MinorVersion > 3)
                cpu.regs.MemPtr = szx.z80Regs.MemPtr;
            else
                cpu.regs.MemPtr = (ushort)(szx.z80Regs.MemPtr & 0xff);

            for (int f = 0; f < 16; f++) {
                Array.Copy(szx.RAM_BANK[f], 0, RAMpage[f], 0, 8192);
            }
        }

        //Sets the speccy state to that of the Z80 file
        public virtual void UseZ80(Z80_SNAPSHOT z80) 
        {
            cpu.regs.I = z80.I;
            cpu.regs.HL_ = (ushort)z80.HL_;
            cpu.regs.DE_ = (ushort)z80.DE_;
            cpu.regs.BC_ = (ushort)z80.BC_;
            cpu.regs.AF_ = (ushort)z80.AF_;

            cpu.regs.HL = (ushort)z80.HL;
            cpu.regs.DE = (ushort)z80.DE;
            cpu.regs.BC = (ushort)z80.BC;
            cpu.regs.IY = (ushort)z80.IY;
            cpu.regs.IX = (ushort)z80.IX;

            cpu.iff_1 = z80.IFF1;
            cpu.regs.R = z80.R;
            cpu.regs.R_ = (byte)(cpu.regs.R & 0x80);
            cpu.regs.AF = (ushort)z80.AF;
            cpu.regs.SP = (ushort)z80.SP;
            cpu.interrupt_mode = z80.IM;
            cpu.regs.PC = (ushort)z80.PC;
            cpu.t_states = z80.TSTATES % FrameLength;
            borderColour = z80.BORDER;
            Issue2Keyboard = z80.ISSUE2;
        }

        private uint GetUIntFromString(string data) {
            byte[] carray = System.Text.ASCIIEncoding.UTF8.GetBytes(data);
            uint val = BitConverter.ToUInt32(carray, 0);
            return val;
        }

        //Saves machine state to a SZX file
        public virtual void SaveSZX(String filename) {
            CreateSZX().SaveSZX(filename);
        }

        private SZXFile CreateSZX() {
            SZXFile szx = new SZXFile();
            szx.header = new SZXFile.ZXST_Header();
            szx.creator = new SZXFile.ZXST_Creator();
            szx.z80Regs = new SZXFile.ZXST_Z80Regs();
            szx.specRegs = new SZXFile.ZXST_SpecRegs();
            szx.keyboard = new SZXFile.ZXST_Keyboard();

            for (int f = 0; f < 16; f++)
                szx.RAM_BANK[f] = new byte[8192];
            szx.header.MachineId = (byte)model;
            szx.header.Magic = GetUIntFromString("ZXST");
            szx.header.MajorVersion = 1;
            szx.header.MinorVersion = 4;
            szx.header.Flags |= (byte)LateTiming;
            szx.creator.CreatorName = "Zero Spectrum Emulator by Arjun ".ToCharArray();
            szx.creator.MajorVersion = SZXFile.SZX_VERSION_SUPPORTED_MAJOR;
            szx.creator.MinorVersion = SZXFile.SZX_VERSION_SUPPORTED_MINOR;
            if (Issue2Keyboard)
                szx.keyboard.Flags |= SZXFile.ZXSTKF_ISSUE2;
            szx.keyboard.KeyboardJoystick |= 8;
            szx.z80Regs.AF = (ushort)cpu.regs.AF;
            szx.z80Regs.AF1 = (ushort)cpu.regs.AF_;
            szx.z80Regs.BC = (ushort)cpu.regs.BC;
            szx.z80Regs.BC1 = (ushort)cpu.regs.BC_;
            szx.z80Regs.MemPtr = (ushort)cpu.regs.MemPtr;
            szx.z80Regs.CyclesStart = (uint)cpu.t_states;
            szx.z80Regs.DE = (ushort)cpu.regs.DE;
            szx.z80Regs.DE1 = (ushort)cpu.regs.DE_;
           
            szx.z80Regs.HL = (ushort)cpu.regs.HL;
            szx.z80Regs.HL1 = (ushort)cpu.regs.HL_;
            szx.z80Regs.I = (byte)cpu.regs.I;
            szx.z80Regs.IFF1 = (byte)(cpu.iff_1 ? 1 : 0);
            szx.z80Regs.IFF2 = (byte)(cpu.iff_2 ? 1 : 0);
            szx.z80Regs.IM = (byte)cpu.interrupt_mode;
            szx.z80Regs.IX = (ushort)cpu.regs.IX;
            szx.z80Regs.IY = (ushort)cpu.regs.IY;
            szx.z80Regs.PC = (ushort)cpu.regs.PC;
            szx.z80Regs.R = (byte)cpu.regs.R;
            szx.z80Regs.SP = (ushort)cpu.regs.SP;
            szx.specRegs.Border = (byte)borderColour;
            szx.specRegs.Fe = (byte)lastFEOut;
            szx.specRegs.pagePort = (byte)last1ffdOut;
            szx.specRegs.x7ffd = (byte)last7ffdOut;

            if (cpu.interrupt_count != 0)
                szx.z80Regs.Flags |= SZXFile.ZXSTZF_EILAST;
            else if (cpu.is_halted) {
                szx.z80Regs.Flags |= SZXFile.ZXSTZF_HALTED;
                szx.z80Regs.PC--;
            }

            foreach (var ad in audio_devices) {
                if (ad is AY_8192) {
                    AY_8192 ay_device = (AY_8192)(ad);
                    szx.ayState = new SZXFile.ZXST_AYState();
                    szx.ayState.cFlags = 0;
                    szx.ayState.currentRegister = (byte)ay_device.SelectedRegister;
                    szx.ayState.chRegs = ay_device.GetRegisters();
                }
            }

            for (int f = 0; f < 16; f++) {
                Array.Copy(RAMpage[f], 0, szx.RAM_BANK[f], 0, 8192);
            }

            if (tapeFilename != "") {
                szx.InsertTape = true;
                szx.externalTapeFile = tapeFilename;
            }
            if (ula_plus.Enabled) {
                szx.palette = new SZXFile.ZXST_PaletteBlock();
                szx.palette.paletteRegs = new byte[64];
                szx.paletteLoaded = true;
                szx.palette.flags = (byte)(ula_plus.PaletteEnabled ? 1 : 0);
                szx.palette.currentRegister = (byte)ula_plus.PaletteGroup;
                for (int f = 0; f < 64; f++) {
                    int rgb = ula_plus.Palette[f];
                    int bbyte = (rgb & 0xff);
                    int gbyte = (rgb >> 8) & 0xff;
                    int rbyte = (rgb >> 16) & 0xff;
                    int bl = bbyte & 0x1;
                    int bm = bl;
                    int bh = (bbyte >> 4) & 0x1;
                    int gl = (gbyte & 0x1);
                    int gm = (gbyte >> 1) & 0x1;
                    int gh = (gbyte >> 4) & 0x1;
                    int rl = (rbyte & 0x1);
                    int rm = (rbyte >> 1) & 0x1;
                    int rh = (rbyte >> 4) & 0x1;
                    byte val = (byte)(((gh << 7) | (gm << 6) | (gl << 5)) | ((rh << 4) | (rm << 3) | (rl << 2)) | ((bh << 1) | bl));
                    szx.palette.paletteRegs[f] = val;
                }
            }

            return szx;
        }

        //Enable/disable stereo sound for AY playback
        public void SetStereoSound(int val) {
            foreach(var ad in audio_devices) {
                if (val == 0)
                    ad.EnableStereoSound(false);
                else {
                    ad.EnableStereoSound(true);
                    if (val == 1)
                        ad.SetChannelsACB(true);
                    else
                        ad.SetChannelsACB(false);
                }
            }
           
        }

        //Enables/Disables AY sound
        public virtual void EnableAY(bool val) {
            if (model == MachineModel._48k) {
                HasAYSound = val;
                if (val) {
                    AY_8192 ay_device = new AY_8192();
                    AddDevice(ay_device);
                }
                else {
                    RemoveDevice(SPECCY_DEVICE.AY_3_8912);
                }
            }
            else {
                HasAYSound = true;
                AY_8192 ay_device = new AY_8192();
                AddDevice(ay_device);
            }
        }

        //Sets up the contention table for the machine
        public abstract void BuildContentionTable();

        //Builds the tstate to attribute map used for floating bus
        public void BuildAttributeMap() {
            int start = DisplayStart;

            for (int f = 0; f < DisplayLength; f++, start++) {
                int addrH = start >> 8; //div by 256
                int addrL = start % 256;

                int pixelY = (addrH & 0x07);
                pixelY |= (addrL & (0xE0)) >> 2;
                pixelY |= (addrH & (0x18)) << 3;

                int attrIndex_Y = AttributeStart + ((pixelY >> 3) << 5);// pixel/8 * 32

                addrL = start % 256;
                int pixelX = addrL & (0x1F);

                attr[f] = (short)(attrIndex_Y + pixelX);
            }
        }

        //Resets the render state everytime an interrupt is generated
        public void ULAUpdateStart() {
            ULAByteCtr = 0;
            lastScanlineColorCounter = 0;
            screenByteCtr = DisplayStart;
            lastTState = ActualULAStart;
            needsPaint = true;
        }

        //Returns true if the given address should be contended, false otherwise
        virtual bool IsContended(int addr) = 0;

        //Contends the machine for a given address (_addr)
        public void Contend(int _addr) {
            if (model != MachineModel._plus3 && IsContended(_addr)) {
                cpu.t_states += contentionTable[cpu.t_states];
            }
        }

        //Contends the machine for a given address (_addr) for n tstates (_time) for x times (_count)
        public void Contend(int _addr, int _time, int _count) {
            if (model != MachineModel._plus3 && IsContended(_addr)) {
                for (int f = 0; f < _count; f++) {
                    cpu.t_states += contentionTable[cpu.t_states] + _time;
                }
            } else
                cpu.t_states += _count * _time;
        }

        //IO Contention:
        // Contention| LowBitReset| Result
        //-----------------------------------------
        // No        | No         | N:4
        // No        | Yes        | N:1 C:3
        // Yes       | Yes        | C:1 C:3
        // Yes       | No         | C:1 C:1 C:1 C:1

        //Should never be called on +3
        public void ContendPortEarly(int _addr) {
            if (IsContended(_addr)) {
                cpu.t_states += contentionTable[cpu.t_states];
            }
            cpu.t_states++;
        }

        //Should never be called on +3
        public void ContendPortLate(int _addr) {
            bool lowBitReset = (_addr & 0x01) == 0;

            if (lowBitReset) {
                cpu.t_states += contentionTable[cpu.t_states];
                cpu.t_states += 2;
            }
            else if (IsContended(_addr)) {
                cpu.t_states += contentionTable[cpu.t_states]; cpu.t_states++;
                cpu.t_states += contentionTable[cpu.t_states]; cpu.t_states++;
                cpu.t_states += contentionTable[cpu.t_states];
            }
            else {
                cpu.t_states += 2;
            }
        }

        public void ForceContention(int _addr) {
            if (IsContended(_addr)) {
                cpu.t_states += contentionTable[cpu.t_states]; cpu.t_states++;
                cpu.t_states += contentionTable[cpu.t_states]; cpu.t_states++;
                cpu.t_states += contentionTable[cpu.t_states]; cpu.t_states++;
            }
            else {
                cpu.t_states += 3;
            }
        }

        //Returns true if the last submitted buffer has finished playing
        //public bool AudioDone()
        //{
        //    return beeper.FinishedPlaying();
        //}

        //Loads the ULAPlus palette
        public bool LoadULAPlusPalette(string filename) {
            using (System.IO.FileStream fs = new System.IO.FileStream(filename, System.IO.FileMode.Open)) {
                using (System.IO.BinaryReader r = new System.IO.BinaryReader(fs)) {
                    int bytesToRead = (int)fs.Length;

                    if (bytesToRead > 63)
                        return false; //not a 64 byte palette file

                    byte[] buffer = new byte[bytesToRead];
                    int bytesRead = r.Read(buffer, 0, bytesToRead);

                    if (bytesRead == 0)
                        return false; //something bad happened!

                    for (int f = 0; f < 64; ++f)
                        ula_plus.Palette[f] = buffer[f];

                    cpu.regs.PC = PeekWordNoContend(cpu.regs.SP);
                    cpu.regs.SP += 2;
                    return true;
                }
            }
        }

        public int GetNumRZXFramesPlayed() {
            if (isPlayingRZX)
                return rzx.NumFramesPlayed;

            return 0;
        }

        public void NextRZXFrame() {
            cpu.t_states = 0;
            isPlayingRZX = rzx.NextPlaybackFrame();
        }

        public void EndRZXFrame() {
            if (cpu.iff_1) {
                Interrupt();
            }

            if (cpu.t_states >= FrameLength) {
                int deltaSoundT = FrameLength - cpu.t_states;
                timeToOutSound += deltaSoundT;
                cpu.t_states = (ushort)FrameLength; //generate interrupt
            }

            if (!needsPaint)
                UpdateScreenBuffer(FrameLength - 1);

            flashFrameCount++;

            if (flashFrameCount > 15) {
                flashOn = !flashOn;
                flashFrameCount = 0;
            }

            ULAUpdateStart();
            NextRZXFrame();
        }

        private void PlayAudio() {
            averagedSound /= soundCounter;

            while (timeToOutSound >= soundTStatesToSample) {
                int sumChannel1Output = 0;
                int sumChannel2Output = 0;

                foreach (var ad in audio_devices) {
                    ad.EndSampleFrame();

                    sumChannel1Output += ad.SoundChannel1;
                    sumChannel2Output += ad.SoundChannel2;
                    ad.ResetSamples();
                }
                soundSamples[soundSampleCounter++] = (short)(sumChannel1Output + averagedSound);
                soundSamples[soundSampleCounter++] = (short)(sumChannel2Output + averagedSound);

                if (soundSampleCounter >= soundSamples.Length) {
                    byte[] sndbuf = beeper.LockBuffer();
                    if (sndbuf != null) {
                        System.Buffer.BlockCopy(soundSamples, 0, sndbuf, 0, sndbuf.Length);
                        beeper.UnlockBuffer(sndbuf);
                    }
                    soundSampleCounter = 0;// (short)(soundSampleCounter - (soundSamples.Length));
                }
                timeToOutSound -= soundTStatesToSample;
            }
            averagedSound = 0;
            soundCounter = 0;
        }

        public void ProcessRZX() {
            prevT = cpu.t_states;
            cpu.Step();
            deltaTStates = cpu.t_states - prevT;

            //// Change CPU speed///////////////////////
            if (emulationSpeed > 9) {
                deltaTStates /= emulationSpeed;
                if (deltaTStates < 1)
                    deltaTStates = 0;// (tapeIsPlaying ? 0 : 1); //tape loading likes 0, sound emulation likes 1. WTF?

                cpu.t_states = prevT + deltaTStates;
                if (tapeIsPlaying)
                    soundTStatesToSample = 79;
            }
            /////////////////////////////////////////////////
            timeToOutSound += deltaTStates;
            UpdateAudio(deltaTStates);

            //There is no tape playback in RZX

            averagedSound += soundOut;
            soundCounter++;
            //Update sound every 79 tstates
            if (timeToOutSound >= soundTStatesToSample) {
                PlayAudio();
            }

            if (cpu.t_states >= FrameLength) {
                int deltaSoundT = FrameLength - cpu.t_states;
                timeToOutSound += deltaSoundT;
                cpu.t_states -= FrameLength;
            }

            if (rzx.fetchCount >= rzx.frame.instructionCount) {
                RZXFrameEventArgs e = new RZXFrameEventArgs(rzx.NumFramesPlayed, rzx.fetchCount, rzx.frame.instructionCount, rzx.frame.inputCount, rzx.inputCount);
                if (RZXFrameEndEvent != null) {
                    RZXFrameEndEvent(this, e);
                }
                OnFrameEndEvent();
                EndRZXFrame();
                if (!doRun) {
                    return;
                }
            }
        }

        //The heart of the speccy. Executes opcodes till 69888 tstates (1 frame) have passed
        public void Process() {
            //Handle re-triggered interrupts!
            bool ran_interrupt = false;
            if (cpu.iff_1  && cpu.t_states < InterruptPeriod) {

                if (cpu.interrupt_count == 0) {
                    if (cpu.parityBitNeedsReset) {
                        cpu.SetParity(false);
                    }
                    if (isRecordingRZX) {
#if NEW_RZX_METHODS
                        rzx.UpdateRecording(cpu.t_states);
#else
                    //rzx.RecordFrame(rzxInputs);
#endif
                    }

                    StateChangeEvent?.Invoke(this, new StateChangeEventArgs(SPECCY_EVENT.RE_INTERRUPT));

                    Interrupt();
                    ran_interrupt = true;
                    cpu.parityBitNeedsReset = false;
                }
            }

            if (cpu.interrupt_count > 0)
                cpu.interrupt_count--;

            //Check if TR DOS needs to be swapped for Pentagon 128k.
            //TR DOS is swapped in when PC >= 15616 and swapped out when PC > 16383.
            if (model == MachineModel._pentagon) {
                if (trDosPagedIn) {
                    if (cpu.regs.PC > 0x3FFF) {
                        if ((last7ffdOut & 0x10) != 0) {
                            //48k basic
                            PageReadPointer[0] = ROMpage[2];
                            PageReadPointer[1] = ROMpage[3];
                            PageWritePointer[0] = JunkMemory[0];
                            PageWritePointer[1] = JunkMemory[1];
                            BankInPage0 = ROM_48_BAS;
                            lowROMis48K = true;
                        } else {
                            //128k basic
                            PageReadPointer[0] = ROMpage[0];
                            PageReadPointer[1] = ROMpage[1];
                            PageWritePointer[0] = JunkMemory[0];
                            PageWritePointer[1] = JunkMemory[1];
                            BankInPage0 = ROM_128_BAS;
                            lowROMis48K = false;
                        }
                        trDosPagedIn = false;
                    }
                }
                else if (lowROMis48K) {
                     if ((cpu.regs.PC >> 8) == 0x3d) {
                        PageReadPointer[0] = ROMpage[4];
                        PageReadPointer[1] = ROMpage[5];
                        PageWritePointer[0] = JunkMemory[0];
                        PageWritePointer[1] = JunkMemory[1];
                        trDosPagedIn = true;
                        BankInPage0 = ROM_TR_DOS;
                    }
                } 
            }

            if (!ran_interrupt) {
                prevT = cpu.t_states;

                cpu.Step();

                deltaTStates = cpu.t_states - prevT;

                //// Change CPU speed///////////////////////
                if (cpuMultiplier > 1 && !tapeIsPlaying) {
                    deltaTStates /= cpuMultiplier;
                    if (deltaTStates < 1)
                        deltaTStates = (tapeIsPlaying ? 0 : 1); //tape loading likes 0, sound emulation likes 1. WTF?

                    cpu.t_states = prevT + deltaTStates;
                }
                /////////////////////////////////////////////////
                timeToOutSound += deltaTStates;
            }
            
            //UpdateTape
            if (tapeIsPlaying && !tape_edgeDetectorRan) {
                if (!ran_interrupt)
                    tapeTStates += deltaTStates;

                UpdateTapePlayback();
            }

            tape_edgeDetectorRan = false;

            //Update Sound
            if (!externalSingleStep) {
                UpdateAudio(deltaTStates);

                averagedSound += soundOut;
                soundCounter++;
                //Update sound every 79 tstates
                if (timeToOutSound >= soundTStatesToSample) {
                    PlayAudio();
                }
            }

            //Randomize when the keyboard state is updated as in a real speccy (kinda).
            if (cpu.t_states >= inputFrameTime) {
                UpdateInput();
                inputFrameTime = rnd_generator.Next(FrameLength);
            }

            //End of frame?
            if (cpu.t_states >= FrameLength) {
                //If machine has already repainted the entire screen,
                //somewhere midway through execution, we can skip this.
                if (!needsPaint)
                    UpdateScreenBuffer(FrameLength);

                OnFrameEndEvent();

                cpu.t_states -= FrameLength;

                flashFrameCount++;

                if (flashFrameCount > 15) {
                    flashOn = !flashOn;
                    flashFrameCount = 0;
                }

                ULAUpdateStart();

                if (isRecordingRZX) {
                    /*  rzxFrame = new RZXFile.RZX_Frame();
                      rzxFrame.inputCount = (ushort)rzxInputs.Count;
                      rzxFrame.instructionCount = (ushort)rzxFetchCount;
                      rzxFrame.inputs = rzxInputs.ToArray();
                      rzx.frames.Add(rzxFrame);
                      rzxFetchCount = 0;
                      rzxInputCount = 0;*/
#if NEW_RZX_METHODS
                    rzx.UpdateRecording(cpu.t_states);
#else
                    //rzx.RecordFrame(rzxInputs);
#endif
                    //rzxInputs = new System.Collections.Generic.List<byte>();
                }
            }
        }

        //Processes an interrupt
        public void Interrupt() {
            if (cpu.interrupt_mode < 2) //IM0 = IM1 for our purpose
            {
                //When interrupts are enabled we can be sure that the reset sequence is over.
                //However, it actually takes a few more frames before the speccy reaches the copyright message,
                //so we have to wait a bit.
                if (!isResetOver)
                {
                    resetFrameCounter++;
                    if (resetFrameCounter > resetFrameTarget)
                    {
                        isResetOver = true;
                        resetFrameCounter = 0;
                    }
                }
            }
            int oldT = cpu.t_states;
            cpu.Interrupt();
            deltaTStates = cpu.t_states - oldT;
            timeToOutSound += deltaTStates; 
            //UpdateAudio(deltaT);
        }

        public void StopTape(bool cancelCallback = false) {
            tapeIsPlaying = false;
            //tape_readToPlay = false;
            //if (pulseLevel != 0)
            //    FlipTapeBit();
            if (TapeEvent != null && !cancelCallback)
                OnTapeEvent(new TapeEventArgs(TapeEventType.STOP_TAPE)); //stop the tape!
        }

        private void FlipTapeBit() {
            pulseLevel = 1 - pulseLevel;

            tapeBitWasFlipped = true;
            tapeBitFlipAck = false;

            if (pulseLevel == 0) {
                soundOut = 0;
            } else
                soundOut = short.MinValue >> 1; //half
        }

        public void NextPZXBlock() {
            while (true) {
                blockCounter++;
                if (blockCounter >= PZXFile.blocks.Count) {
                    blockCounter--;
                    //tape_readToPlay = false;
                    StopTape();
                    return;
                }

                currentBlock = PZXFile.blocks[blockCounter];
               
                if (currentBlock is PZXFile.PULS_Block) {
                    //Initialise for PULS loading
                    pulseCounter = -1;
                    repeatCount = -1;

                    //Pulse is low by default for PULS blocks
                    if (pulseLevel != 0)
                        FlipTapeBit();
                   
                    //Process pulse if there is one
                    if (!NextPULS()) {
                        continue; //No? Next block please!
                    } else
                        break;
                } else if (currentBlock is PZXFile.DATA_Block) {
                    pulseCounter = -1;
                    bitCounter = -1;
                    dataCounter = -1;
                    bitShifter = 0;
                    
                    if (pulseLevel != (((PZXFile.DATA_Block)currentBlock).initialPulseLevel))
                        FlipTapeBit();

                    if (!NextDataBit()) {
                        continue;
                    } else
                        break;
                } else if (currentBlock is PZXFile.PAUS_Block) {
                    //Would have been nice to skip PAUS blocks when doing fast loading
                    //but some loaders like Auf Wiedersehen Monty (Kixx) rely on the pause
                    //length to do processing during loading. In this case, fill in the
                    //loading screen.

                    //Ensure previous edge is finished correctly by flipping the edge one last time
                    //edgeDuration = (35000 * 2);
                    //isPauseBlockPreproccess = true;
                    PZXFile.PAUS_Block block = (PZXFile.PAUS_Block)currentBlock;

                    if (block.initialPulseLevel != pulseLevel)
                        FlipTapeBit();
                   
                    edgeDuration = (block.duration);

                    int diff = (int)edgeDuration - tapeTStates;
                    if (diff > 0) {
                        edgeDuration = (uint)diff;
                        tapeTStates = 0;
                        isPauseBlockPreproccess = true;
                        break;
                    } else {
                        tapeTStates = -diff;
                    }
                    continue;
                } else if ((currentBlock is PZXFile.STOP_Block)) {
                    StopTape();
                   // if (ziggyWin.zx.keyBuffer[(int)ZeroWin.Form1.keyCode::ALT])
                   //     ziggyWin.saveSnapshotMenuItem_Click(this, null);
                    break;
                }
            }
            if (TapeEvent != null)
                OnTapeEvent(new TapeEventArgs(TapeEventType.NEXT_BLOCK));
            //dataGridView1.Rows[blockCounter - 1].Selected = true;
           // dataGridView1.CurrentCell = dataGridView1.Rows[blockCounter - 1].Cells[0];
        }

        public bool NextPULS() {

            PZXFile.PULS_Block block = (PZXFile.PULS_Block)currentBlock;

            while (pulseCounter < block.pulse.Count - 1) {
                pulseCounter++; //b'cos pulseCounter is -1 when it reaches here initially
                repeatCount = block.pulse[pulseCounter].count;
                if ((block.pulse[pulseCounter].duration == 0) && repeatCount > 1) {
                    if ((repeatCount & 0x01) != 0) 
                         FlipTapeBit();                        
                    continue; //next pulse
                }
                edgeDuration = block.pulse[pulseCounter].duration;

                if (edgeDuration > 0) {
                    int diff = (int)edgeDuration - tapeTStates;
                    if (diff > 0) {
                        edgeDuration = (uint)diff;
                        tapeTStates = 0;
                        return true;
                    } else
                        tapeTStates = -diff;
                }

                FlipTapeBit(); 
                repeatCount--;
                if (repeatCount <= 0)
                    continue;
                
                return true;
            }
            
            //All pulses done!
            return false;
        }

        public bool NextDataBit() {
            PZXFile.DATA_Block block = (PZXFile.DATA_Block)currentBlock;
           
            //Bits left for processing?
            while (bitCounter < block.count - 1) {
                bitCounter++;
                if (bitShifter == 0) {
                    bitShifter = 0x80;
                    //All 8 bits done so get next byte
                    dataCounter++;
                    if (dataCounter < block.data.Count) {
                        dataByte = block.data[dataCounter];
                    }
                }
                currentBit = ((dataByte & bitShifter) == 0 ? 0 : 1);
                bitShifter >>= 1;
                pulseCounter = 0;
                int numPulses = 0;

                if (currentBit == 0) {
                    edgeDuration = (block.s0[0]);
                    numPulses = block.p0;
                }
                else {
                    edgeDuration = (block.s1[0]);
                    numPulses = block.p1;
                }

                if (numPulses == 0)
                    continue;

                if (edgeDuration > 0) {
                    int diff = (int)edgeDuration - tapeTStates;
                    if(diff > 0) {
                        edgeDuration = (uint)diff;
                        tapeTStates = 0;
                        return true;
                    }
                    else
                        tapeTStates = -diff;
                }

                FlipTapeBit();
            }

            //All bits done. Now do the tail pulse to finish off things
            if (block.tail > 0) {
                currentBit = -1;
                edgeDuration = (block.tail);

                if (edgeDuration > 0) {
                    int diff = (int)edgeDuration - tapeTStates;
                    if (diff > 0) {
                        edgeDuration = (uint)diff;
                        tapeTStates = 0;
                        return true;
                    } else
                        tapeTStates = -diff;
                }
                FlipTapeBit();
            } else {
                //HACK: Sometimes a tape might have its last tail pulse missing.
                //In case it's the last block in the tape, it's best to flip the tape bit
                //a last time to ensure that the process is terminated properly.
                if (blockCounter == PZXFile.blocks.Count - 1) {
                    currentBit = -1;
                    edgeDuration = (3500 * 2);
                    return true;
                }
            }

            return false;
        }

        private void FlashLoad()
        {
            if (blockCounter < 0)
                blockCounter = 0;

            PZXFile.Block currBlock = PZXFile.blocks[blockCounter];


            /*if (currBlock is PZXFile.PAUS_Block) {
                if (!isProcessingPauseBlock) {
                    isProcessingPauseBlock = true;
                    edgeDuration = ((PZXFile.PAUS_Block)currBlock).duration;
                    pauseCounter = edgeDuration;
                    //tapeTStates = 0;
                    return;
                }
                else {
                    pauseCounter -= (uint)tapeTStates;
                    if (pauseCounter > 0)
                        return;
                    else {
                        pauseCounter = 0;
                        isProcessingPauseBlock = false;
                    }
                }
            }*/

            if (!(currBlock is PZXFile.PULS_Block))
                blockCounter++;

            if (blockCounter >= PZXFile.tapeBlockInfo.Count)
            {
                blockCounter--;
                //tape_readToPlay = false;
                StopTape();
                return;
            }

            if (!PZXFile.tapeBlockInfo[blockCounter].IsStandardBlock)
            {
                if (!(currBlock is PZXFile.PULS_Block))
                    blockCounter--;
                return;
            }

            PZXFile.DATA_Block dataBlock = (PZXFile.DATA_Block)PZXFile.blocks[blockCounter + 1];
            edgeDuration = (1000);
            //if (pulseLevel != dataBlock.initialPulseLevel)
            //    FlipTapeBit();
            cpu.regs.H = 0;
            int byteCounter = dataBlock.data.Count;
            int dataIndex = 0;
            bool loadStageFlagByte = true;
            while (true)
            {
                if (byteCounter == 0)
                {
                    cpu.regs.A = (byte)(cpu.regs.C & 32);
                    cpu.regs.B = 0;
                    cpu.regs.F = 0x50; //01010000b
                    break;
                }
                byteCounter--;
                cpu.regs.L = dataBlock.data[dataIndex++];
                cpu.regs.H ^= cpu.regs.L;
                if (cpu.regs.DE == 0)
                {
                    cpu.regs.A = cpu.regs.H;
                    cpu.Cp_R(1);
                    break;
                }
                if (loadStageFlagByte)
                {
                    loadStageFlagByte = false;
                    cpu.regs.A = (byte)(cpu.regs.AF_ >> 8);
                    cpu.Xor_R(cpu.regs.L);
                    if ((cpu.regs.F & 0x040) == 0)
                        break;
                }
                else
                {
                    PokeByteNoContend(cpu.regs.IX++, cpu.regs.L);
                    cpu.regs.DE--;
                }
            }

            //Simulate RET
            cpu.regs.PC = cpu.PopStack();
            cpu.regs.MemPtr = cpu.regs.PC;

            blockCounter++;
            if (blockCounter >= PZXFile.blocks.Count)
            {
                blockCounter--;
                //tape_readToPlay = false;
                StopTape();
                return;
            }
        }

        private void DoTapeEvent(TapeEventArgs e) {
            if (tapeBitFlipAck)
                tapeBitWasFlipped = false;

            if (e.EventType == TapeEventType.EDGE_LOAD) {
                FlipTapeBit();

#region PULS

                if (currentBlock is PZXFile.PULS_Block) {
                    PZXFile.PULS_Block block = (PZXFile.PULS_Block)currentBlock;
                    repeatCount--;
                    //progressBar1.Value += progressStep;
                    //Need to repeat?
                   // if (repeatCount < block.pulse[pulseCounter].count) {
                     if (repeatCount > 0) {
                       edgeDuration = (block.pulse[pulseCounter].duration);
                        int diff = (int)edgeDuration - tapeTStates;
                        if (diff > 0) {
                            edgeDuration = (uint)diff;
                            tapeTStates = 0;
                        }
                    } else {
                        
                        if (!NextPULS()) //All pulses done for the block?
                        {
                            NextPZXBlock();
                            return;
                        }
                    }
                    return;
                }
#endregion PULS

#region DATA
                else if (currentBlock is PZXFile.DATA_Block) {
                    PZXFile.DATA_Block block = (PZXFile.DATA_Block)currentBlock;

                    //Are we done with pulses for a certain sequence?
                    if (currentBit == 0) {
                        pulseCounter++;
                        if (pulseCounter < block.p0) {
                            edgeDuration = (block.s0[pulseCounter]);
                        } else {
                            //All pulses done for this bit so fetch next bit
                            if (!NextDataBit()) {
                                NextPZXBlock();
                                return;
                            }
                        }
                    } else if (currentBit == 1) {
                        pulseCounter++;
                        if (pulseCounter < block.p1) {
                            edgeDuration = (block.s1[pulseCounter]);
                        } else {
                            //All pulses done for this bit so fetch next bit
                            if (!NextDataBit()) {
                                NextPZXBlock();
                                return;
                            }
                        }
                    } else //we were doing the tail!
                    {
                        NextPZXBlock();
                        return;
                    }
                    return;
                }

#endregion DATA

#region PAUS
                else if (currentBlock is PZXFile.PAUS_Block) {
                    isPauseBlockPreproccess = false;
                    NextPZXBlock();
                    return;
                }
/*
 else if (currentBlock is PZXFile.PAUS_Block) {
                    if (isPauseBlockPreproccess) {
                        PZXFile.PAUS_Block block = (PZXFile.PAUS_Block)currentBlock;
                        isPauseBlockPreproccess = false;
                        edgeDuration = (block.duration);
                        if (pulse != block.initialPulseLevel)
                            FlipTapeBit();
                    } else {
                        NextPZXBlock();
                    }
                    return;
                }
                */
#endregion PAUS
            } else if (e.EventType == TapeEventType.STOP_TAPE) //stop
            {
                StopTape();
                blockCounter--;

            } else if (e.EventType == TapeEventType.START_TAPE) {
                if (TapeEvent != null)
                    OnTapeEvent(new TapeEventArgs(TapeEventType.START_TAPE));

                NextPZXBlock();
               
            } else if (e.EventType == TapeEventType.FLASH_LOAD) {
                FlashLoad();
            }
        }
    };
}
