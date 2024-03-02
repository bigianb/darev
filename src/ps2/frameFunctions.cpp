#include "frameFunctions.h"

const int maxFrameFunctions = 16;

FrameFunctionDef frameFunctions[maxFrameFunctions];
int numRegisteredFrameFunctions = 0;

int DAT_ram_00325c94;

void registerFrameFunction(void (*function)(void), int priority, char* name)
{
    // Check if this function is already registered.
    for (int i = 0; i < numRegisteredFrameFunctions; ++i) {
        if (frameFunctions[i + 1].func == function) {
            return;
        }
    }

    int srcIdx = numRegisteredFrameFunctions;
    if (numRegisteredFrameFunctions > 0) {
        /*
           This is a prority queue where a lower number has higher priority,
           so we start at the back and walk back until we find an entry who's priority value
           is lower or equal (so equal or higher priority) to the new function.
           We insert the new entry there and shift everything else back.
        */
        int tgtIdx = numRegisteredFrameFunctions;
        srcIdx = tgtIdx - 1;
        
        while (srcIdx >= 0 && priority < frameFunctions[srcIdx].priority) {
            // New function is a higher priority than the one at srcIdx
            frameFunctions[tgtIdx--] = frameFunctions[srcIdx--];
        }
        // The while loop will have decremented this one too many times.
        ++srcIdx;
    }

    frameFunctions[srcIdx].func = function;
    frameFunctions[srcIdx].priority = priority;
    frameFunctions[srcIdx].name = name;
    if (DAT_ram_00325c94 >= (srcIdx-1)) {
        ++DAT_ram_00325c94;
    }

    ++numRegisteredFrameFunctions;
    return;
}
