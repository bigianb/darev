#pragma once

class FrameFunctionDef
{
public:
    void (*func)(void);
    int priority;
    char* name;
};

extern FrameFunctionDef frameFunctions[];
extern int numRegisteredFrameFunctions;
extern int prevExecutedFrameFunctionIdx;