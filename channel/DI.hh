#pragma once

#include <portable/Types.hh>

class DI {
public:
    // Reads the disc ID for use with DiscID and initializes the drive
    // Returns true if success, false if error
    static bool ReadDiscID();

    // Reads from disc into the destination buffer
    // Returns true if success, false if error
    static bool Read(void *dst, u32 size, u32 offset);

    // Checks if the disc is inserted into the console
    // Prints an error if the drive is not initialized
    // Returns true if ok, false if not inserted or initialized
    static bool IsInserted();

    // Checks if the drive is initialized
    // Returns true if initialized, false if not
    static bool IsInitialized();

    // Resets and deinitializes the drive
    static void Reset();

private:
    // Clears all interrupts in disr
    // This should be called first if applicable, as it fully overwrites the disr
    static void ClearInterrupts();

    // Makes a command for dicmdbuf0
    static void MakeCommand(u8 command, u8 sub0, u16 sub1);

    // Sends the command to the drive
    // The dma arg is false for immediate mode, true for DMA mode
    static void SendCommand(bool dma);

    // Waits for a command to finish
    static void WaitForCommand();

    // This should only be called after WaitForCommand finishes
    // Returns whether or not a device error occured during command execution
    static bool IsDeviceError();

    DI();
};
