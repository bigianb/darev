#include <cstring>
#include <cmath>
#include "pad.h"
#include "libpad.h"

typedef unsigned char byte;

static char padBuf[2][256] __attribute__((aligned(64)));

int DAT_ram_00325eb8[2];

int DAT_ram_00238890[2][21];

int DAT_ram_00324468 = 0;

int startButtonPressed = 0;

int limitControllerInput = 0;

int acceptPadButton = DA_PAD_CROSS;
int backPadButton = DA_PAD_CIRCLE;

void initPads()
{
    padInit(0);
    int port = 0;
    do {
        padPortOpen(port, 0, padBuf[port]);
        memset((char*)DAT_ram_00238890[port], 0, 21 * 4);
        DAT_ram_00325eb8[port] = 0;
        port += 1;
    } while (port < 2);
}

void scaleAnalogStick(int x, int y, float* fx_out, float* fy_out)
{
    float fx = (float)(x + -0x80) * 0.0078125f;
    float fy = (float)(y + -0x80) * 0.0078125f;
    float fVar1 = sqrt(fx * fx + fy * fy);
    if (0.32f <= fVar1) {
        fVar1 = (1.0f - 0.32f / fVar1) * 1.470588f;
        *fx_out = fx * fVar1;
        *fy_out = fy * fVar1;
    } else {
        *fx_out = 0.0f;
        *fy_out = 0.0f;
    }
}

bool controllerIsAnalogueMode[2] = {false, false};

padButtonStatus localPadButtonStatus[2];

struct PadActuatorStatus
{
    int field0;
    int field1;
    float field2;
    float field3;
    int field4;
    int field5;
};

// 0x451a00
PadActuatorStatus padActuatorStatus[2];

PadBgdaData convertedControllerInput[2];

char padActAlign[6] = {0, 1, -1, -1, -1, -1};

char actuatorDirectBuf[6] = {0, 0, 0, 0, 0, 0};

int lenRecordedInputs = 0;
int DAT_ram_00325c3c = 0;
int DAT_ram_003245f4 = 0;

void readInput()
{
    int readDataLengths[2];

    int port = 0;
    do {
        int lenRead = padRead(port, 0, &localPadButtonStatus[port]);
        readDataLengths[port] = lenRead;
        if (lenRead < 1) {
            int failCounter = convertedControllerInput[port].failedReadCountdown - 1;
            if (failCounter < 0) {
                failCounter = 0;
            }
            convertedControllerInput[port].failedReadCountdown = failCounter;
        } else {
            int mode = localPadButtonStatus[port].mode;
            convertedControllerInput[port].failedReadCountdown = 0x19;
            if ((mode & 0xf0) == 0x70) {
                // DualShock 2
                if (!controllerIsAnalogueMode[port]) {
                    // This will happen on every call which is probably not what is required.
                    if ((mode != 0x79) && padInfoPressMode(port, 0)) {
                        padEnterPressMode(port, 0);
                    }
                } else {
                    padSetActAlign(port, 0, padActAlign);
                    // Actuators set, switch state so pressure set will be performed next
                    controllerIsAnalogueMode[port] = false;
                }
            } else {
                // Enable analogue mode (switch to dualshock 2)
                padSetMainMode(port, 0, 1, 3);
                controllerIsAnalogueMode[port] = true;
            }
        }
        port += 1;
    } while (port < 2);

    startButtonPressed = 0;
    /*
        if (lenRecordedInputs != 0) {
            if (DAT_ram_00325c3c == 0) {
                port = 0;
                do {
                    if (readDataLengths[port] > 0 && ((localPadButtonStatus[port].btns & PAD_START) == 0)) {
                        startButtonPressed = 1;
                    }
                    port += 1;
                } while (port < 2);

                port = 0;
                do {
                    piVar17 = &convertedControllerInput[port].RLUD_buttons;
                    uVar16 = *piVar17;
                    *piVar17 = (int)ptrRecordedInputs->buttonStickData[port].buttons;

                    scaleAnalogStick(ptrRecordedInputs->buttonStickData[port].stick_l_x,
                                     ptrRecordedInputs->buttonStickData[port].stick_l_y,
                                     &convertedControllerInput[port].l_stick_x,
                                     &convertedControllerInput[port].l_stick_y);
                    scaleAnalogStick(ptrRecordedInputs->buttonStickData[port].stick_r_x,
                                     ptrRecordedInputs->buttonStickData[port].stick_r_y,
                                     &convertedControllerInput[port].r_stick_x,
                                     &convertedControllerInput[port].r_stick_y);

                    fVar26 = convertedControllerInput[port].r_stick_x;
                    if (0.8 < fVar26) {
                        *piVar17 = *piVar17 | 0x800000;
                    }
                    if (fVar26 < -0.8) {
                        *piVar17 = *piVar17 | 0x400000;
                    }
                    fVar26 = convertedControllerInput[port].r_stick_y;
                    if (0.8 < fVar26) {
                        *piVar17 = *piVar17 | 0x200000;
                    }
                    if (fVar26 < -0.8) {
                        *piVar17 = *piVar17 | 0x100000;
                    }
                    uVar16 = (*piVar17 ^ uVar16) & *piVar17;

                    convertedControllerInput[port].updatedButtons = uVar16;
                    convertedControllerInput[port].updatedButtons2 = uVar16;
                    port += 1;
                } while (port < 2);

                curRandomNum = ptrRecordedInputs->randomSeed;
                ptrRecordedInputs += 1;
                lenRecordedInputs -= 1;
            }
        }
    */
    if (lenRecordedInputs == 0) {
        port = 0;
        do {
            int originalButtonState = convertedControllerInput[port].RLUD_buttons;

            // Deal with rumble
            PadActuatorStatus* piVar14 = &padActuatorStatus[port];
            actuatorDirectBuf[0] = (piVar14->field0 != 0) ? 1 : 0;
            if (actuatorDirectBuf[0]) {
                piVar14->field0 -= 1;
            }
            if (piVar14->field2 > 0.0f) {
                piVar14->field2 -= piVar14->field3;
                if (piVar14->field2 < 0.0f) {
                    piVar14->field2 = 0.0f;
                }
            }
            if (piVar14->field1 == 0) {
                actuatorDirectBuf[1] = 0;
            } else {
                piVar14->field1 -= 1;
                actuatorDirectBuf[1] = (byte)(piVar14->field2 * 255.0f);
            }

            if (actuatorDirectBuf[0] != piVar14->field4 || (actuatorDirectBuf[1] != piVar14->field5)) {
                // rumble settings have changed
                padSetActDirect(port, 0, actuatorDirectBuf);
            }

            piVar14->field4 = actuatorDirectBuf[0];
            piVar14->field5 = actuatorDirectBuf[1];

            convertedControllerInput[port].l_stick_x = 0;
            convertedControllerInput[port].l_stick_y = 0;
            convertedControllerInput[port].r_stick_x = 0;
            convertedControllerInput[port].r_stick_y = 0;
            convertedControllerInput[port].RLUD_buttons = 0;

            if (readDataLengths[port] > 0) {
                // convert so 1 is pressed
                unsigned short buttons = localPadButtonStatus[port].btns ^ 0xffff;
                // internal representation is big endian
                convertedControllerInput[port].RLUD_buttons = ((buttons & 0xFF) << 8) | ((buttons >> 8) & 0xFF);

                if (DAT_ram_003245f4 != 0) {
                    DAT_ram_003245f4 -= 1;
                    // mask out start button
                    convertedControllerInput[port].RLUD_buttons &= 0xf7ff;
                }

                if ((localPadButtonStatus[port].mode & 0xf0) == 0x70) {
                    if (localPadButtonStatus[port].mode == 0x79) {
                        convertedControllerInput[port].pad_right = localPadButtonStatus[port].right_p / 255.0f;
                        convertedControllerInput[port].pad_left = localPadButtonStatus[port].left_p / 255.0f;
                        convertedControllerInput[port].pad_up = localPadButtonStatus[port].up_p / 255.0f;
                        convertedControllerInput[port].pad_down = localPadButtonStatus[port].down_p / 255.0f;

                        convertedControllerInput[port].pad_circle = localPadButtonStatus[port].circle_p / 255.0f;
                        convertedControllerInput[port].pad_square = localPadButtonStatus[port].square_p / 255.0f;
                        convertedControllerInput[port].pad_triangle = localPadButtonStatus[port].triangle_p / 255.0f;
                        convertedControllerInput[port].pad_cross = localPadButtonStatus[port].cross_p / 255.0f;

                        convertedControllerInput[port].pad_l1 = localPadButtonStatus[port].l1_p / 255.0f;
                        convertedControllerInput[port].pad_r1 = localPadButtonStatus[port].r1_p / 255.0f;
                        convertedControllerInput[port].pad_l2 = localPadButtonStatus[port].l2_p / 255.0f;
                        convertedControllerInput[port].pad_r2 = localPadButtonStatus[port].r2_p / 255.0f;
                    } else {
                        int daButtons = convertedControllerInput[port].RLUD_buttons;
                        convertedControllerInput[port].pad_right = ((daButtons & DA_PAD_RIGHT) == DA_PAD_RIGHT) ? 1.0f : 0.0f;
                        convertedControllerInput[port].pad_left = ((daButtons & DA_PAD_LEFT) == DA_PAD_LEFT) ? 1.0f : 0.0f;
                        convertedControllerInput[port].pad_up = ((daButtons & DA_PAD_UP) == DA_PAD_UP) ? 1.0f : 0.0f;
                        convertedControllerInput[port].pad_down = ((daButtons & DA_PAD_DOWN) == DA_PAD_DOWN) ? 1.0f : 0.0f;

                        convertedControllerInput[port].pad_circle = ((daButtons & DA_PAD_CIRCLE) == DA_PAD_CIRCLE) ? 1.0f : 0.0f;
                        convertedControllerInput[port].pad_square = ((daButtons & DA_PAD_SQUARE) == DA_PAD_SQUARE) ? 1.0f : 0.0f;
                        convertedControllerInput[port].pad_triangle = ((daButtons & DA_PAD_TRIANGLE) == DA_PAD_TRIANGLE) ? 1.0f : 0.0f;
                        convertedControllerInput[port].pad_cross = ((daButtons & DA_PAD_CROSS) == DA_PAD_CROSS) ? 1.0f : 0.0f;

                        convertedControllerInput[port].pad_l1 = ((daButtons & DA_PAD_L1) == DA_PAD_L1) ? 1.0f : 0.0f;
                        convertedControllerInput[port].pad_r1 = ((daButtons & DA_PAD_R1) == DA_PAD_R1) ? 1.0f : 0.0f;
                        convertedControllerInput[port].pad_l2 = ((daButtons & DA_PAD_L2) == DA_PAD_L2) ? 1.0f : 0.0f;
                        convertedControllerInput[port].pad_r2 = ((daButtons & DA_PAD_R2) == DA_PAD_R2) ? 1.0f : 0.0f;
                    }

                    // l/r transposed
                    scaleAnalogStick(localPadButtonStatus[port].rjoy_h, localPadButtonStatus[port].rjoy_v,
                                     &convertedControllerInput[port].l_stick_x, &convertedControllerInput[port].l_stick_y);
                    scaleAnalogStick(localPadButtonStatus[port].ljoy_h, localPadButtonStatus[port].ljoy_v,
                                     &convertedControllerInput[port].r_stick_x, &convertedControllerInput[port].r_stick_y);

                    if (0.8 < convertedControllerInput[port].r_stick_x) {
                        convertedControllerInput[port].RLUD_buttons |= DA_RSTICK_RIGHT;
                    } else if (convertedControllerInput[port].r_stick_x < -0.8) {
                        convertedControllerInput[port].RLUD_buttons |= DA_RSTICK_LEFT;
                    }
                    if (0.8 < convertedControllerInput[port].r_stick_y) {
                        convertedControllerInput[port].RLUD_buttons |= DA_RSTICK_DOWN;
                    } else if (convertedControllerInput[port].r_stick_y < -0.8) {
                        convertedControllerInput[port].RLUD_buttons |= DA_RSTICK_UP;
                    }
                }

                int buttonsNow = convertedControllerInput[port].RLUD_buttons;
                int pressedButtons = (buttonsNow ^ originalButtonState) & buttonsNow;
                convertedControllerInput[port].updatedButtons = pressedButtons;
                convertedControllerInput[port].updatedButtons2 = pressedButtons;
            }
            port += 1;
        } while (port < 2);
    }
    
    port = 0;  
    do {
        if (convertedControllerInput[port].RLUD_buttons == 0) {
            DAT_ram_00325eb8[port] = 0;
        } else {
            if (DAT_ram_00324468 == 0) {
                DAT_ram_00325eb8[port] += 1;
            } else {
                DAT_ram_00325eb8[port] += 2;
            }

            if ((DAT_ram_00325eb8[port] > 0x14) && ((DAT_ram_00325eb8[port] & 3U) == 0)) {
                convertedControllerInput[port].updatedButtons2 = convertedControllerInput[port].RLUD_buttons;
            }
        }
        if (limitControllerInput != 0) {
            convertedControllerInput[port].r_stick_y = 0.0f;
            convertedControllerInput[port].r_stick_x = 0.0f;
            convertedControllerInput[port].l_stick_y = 0.0f;
            convertedControllerInput[port].l_stick_x = 0.0f;

            int uVar16 = acceptPadButton | backPadButton;
            convertedControllerInput[port].RLUD_buttons &= uVar16;
            convertedControllerInput[port].updatedButtons &= uVar16;
            convertedControllerInput[port].updatedButtons2 &= uVar16;

            convertedControllerInput[port].pad_right = 0.0f;
            convertedControllerInput[port].pad_left = 0.0f;
            convertedControllerInput[port].pad_up = 0.0f;
            convertedControllerInput[port].pad_down = 0.0f;
            convertedControllerInput[port].pad_l1 = 0.0f;
            convertedControllerInput[port].pad_l2 = 0.0f;
            convertedControllerInput[port].pad_circle = 0.0f;
            convertedControllerInput[port].pad_square = 0.0f;
            convertedControllerInput[port].pad_triangle = 0.0f;
            convertedControllerInput[port].pad_cross = 0.0f;
            convertedControllerInput[port].pad_r1 = 0.0f;
            convertedControllerInput[port].pad_r2 = 0.0f;
        }
        port += 1;
    } while (port < 2);
    limitControllerInput = 0;
}
