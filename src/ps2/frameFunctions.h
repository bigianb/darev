#pragma once

class FrameFunctionDef
{
public:
    void (*func)(void);
    int priority;
    const char* name;
};

extern FrameFunctionDef frameFunctions[];
extern int numRegisteredFrameFunctions;
extern int prevExecutedFrameFunctionIdx;

void registerFrameFunction(void (*function)(void), int priority, const char* name);
