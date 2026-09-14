#include "SG1000.h"
#include "VDP9918.h"
#include "SN76489.h"
#include "sg1000_config.h"
#include <string.h>
#include "fabgl.h"

extern "C" {
    extern void *arduino_fopen(const char *filename, const char *mode);
    extern int arduino_fclose(void *stream);
    extern size_t arduino_fread(void *ptr, size_t size, size_t nmemb, void *stream);
}

extern fabgl::PS2Controller PS2Controller;

Z80 CPU;
byte CartROM[SG1000_CART_SIZE];
byte WorkRAM[SG1000_RAM_SIZE];
int CartSize = 0;

static byte ControllerState[2] = { 0, 0 };

struct ControllerKey {
    fabgl::VirtualKey key;
    byte controller;
    byte mask;
};

static constexpr ControllerKey ControllerKeys[] = {
    { fabgl::VK_UP,    0, 0x01 }, { fabgl::VK_DOWN,  0, 0x02 },
    { fabgl::VK_LEFT,  0, 0x04 }, { fabgl::VK_RIGHT, 0, 0x08 },
    { fabgl::VK_z,     0, 0x10 }, { fabgl::VK_Z,     0, 0x10 },
    { fabgl::VK_x,     0, 0x20 }, { fabgl::VK_X,     0, 0x20 },
    { fabgl::VK_w,     1, 0x01 }, { fabgl::VK_W,     1, 0x01 },
    { fabgl::VK_s,     1, 0x02 }, { fabgl::VK_S,     1, 0x02 },
    { fabgl::VK_a,     1, 0x04 }, { fabgl::VK_A,     1, 0x04 },
    { fabgl::VK_d,     1, 0x08 }, { fabgl::VK_D,     1, 0x08 },
    { fabgl::VK_f,     1, 0x10 }, { fabgl::VK_F,     1, 0x10 },
    { fabgl::VK_g,     1, 0x20 }, { fabgl::VK_G,     1, 0x20 },
};

static void UpdateControllers(void)
{
    fabgl::Keyboard *keyboard = PS2Controller.keyboard();
    if (!keyboard) return;

    while (keyboard->virtualKeyAvailable()) {
        bool pressed;
        fabgl::VirtualKey key = keyboard->getNextVirtualKey(&pressed);
        for (const ControllerKey &mapping : ControllerKeys) {
            if (mapping.key != key) continue;
            if (pressed)
                ControllerState[mapping.controller] |= mapping.mask;
            else
                ControllerState[mapping.controller] &= ~mapping.mask;
        }
    }
}

extern "C" byte RdZ80(Z80_WORD address)
{
    if (address < SG1000_CART_SIZE) return CartROM[address];
    return WorkRAM[address & (SG1000_RAM_SIZE - 1)];
}

extern "C" void WrZ80(Z80_WORD address, byte value)
{
    if (address >= SG1000_CART_SIZE)
        WorkRAM[address & (SG1000_RAM_SIZE - 1)] = value;
}

static byte ReadControllers(byte port)
{
    if (port == 0xDC)
        return (byte)~((ControllerState[0] & 0x3F) | ((ControllerState[1] & 0x03) << 6));
    return (byte)(~((ControllerState[1] >> 2) & 0x0F) | 0xF0);
}

extern "C" byte InZ80(Z80_WORD port)
{
    const byte ioPort = (byte)port;
    if (ioPort == 0xBE) return RdDataVDP();
    if (ioPort == 0xBF) return RdCtrlVDP();
    if (ioPort == 0xDC || ioPort == 0xDD) return ReadControllers(ioPort);
    return 0xFF;
}

extern "C" void OutZ80(Z80_WORD port, byte value)
{
    const byte ioPort = (byte)port;
    if (ioPort == 0xBE) WrDataVDP(value);
    else if (ioPort == 0xBF) WrCtrlVDP(value);
    else if (ioPort == 0x7E || ioPort == 0x7F) WrPSG(value);
}

extern "C" void PatchZ80(Z80 *registers) { (void)registers; }

extern "C" Z80_WORD LoopZ80(Z80 *registers)
{
    (void)registers;
    yield();
    UpdateControllers();
    VDPEndOfScanline();
    return VDPWantsIRQ() ? INT_IRQ : INT_NONE;
}

int InitMachine(void)
{
    memset(CartROM, 0xFF, sizeof(CartROM));
    memset(WorkRAM, 0x00, sizeof(WorkRAM));
    VDPPlatformInit();
    ResetVDP9918();
    ResetPSG();
    CPU.IPeriod = SG1000_CYCLES_PER_LINE;
    ResetZ80(&CPU);
    return 1;
}

void TrashMachine(void)
{
    VDPPlatformTrash();
}

void ResetSG1000(void)
{
    memset(ControllerState, 0, sizeof(ControllerState));
    ResetVDP9918();
    ResetPSG();
    ResetZ80(&CPU);
}

int LoadCartridge(const char *fileName)
{
    void *file = arduino_fopen(fileName, "rb");
    if (!file) return 0;

    memset(CartROM, 0xFF, sizeof(CartROM));
    CartSize = (int)arduino_fread(CartROM, 1, SG1000_CART_SIZE, file);
    arduino_fclose(file);
    return CartSize > 0;
}

int StartSG1000(void)
{
    ResetSG1000();
    RunZ80(&CPU);
    return 1;
}