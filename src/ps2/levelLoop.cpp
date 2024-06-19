#include "levelLoop.h"

#include <kernel.h>
#include <timer.h>

#include "display.h"
#include "dlist.h"
#include "draw.h"
#include "filesys.h"
#include "frameFunctions.h"
#include "gsAllocator.h"
#include "lump.h"
#include "pad.h"
#include "state.h"
#include "text.h"

void runLevelLoop()
{
    int displayEnableCountdown = 0;

    sceneFrameNum = 0;
    curRandomNum = 0;

    //if (DAT_ram_00325c4c == 0) {
    curRandomNum = cpu_ticks();
    // }

    //DAT_ram_002b1fe8 = 0;
    curLevelName[0] = '\0';
    /*
    FUN_ram_001830b8();
    FUN_ram_00183388();
*/
    // Queue RunWater if not already there
    /*
    iVar8 = prevExecutedFrameFunctionIdx;
    uVar21 = (ulong)numRegisteredFrameFunctions;
    uVar22 = 0x14c5b8;
    lVar15 = 0;
    if (0 < (long)uVar21) {
        do {
            iVar23 = (int)lVar15;
            lVar15 = (long)(iVar23 + 1);
            uVar16 = uVar21;
            if (frameFunctions[iVar23 + 1].func == runWaterFunc) {
                goto LAB_ram_0020294c;
            }
        } while (lVar15 < (long)uVar21);
    }
    uVar16 = uVar21;
    if ((0 < (long)uVar21) && (0 < frameFunctions[numRegisteredFrameFunctions].priority)) {
        in_t0 = 0xc;
        iVar23 = numRegisteredFrameFunctions;
        do {
            iVar13 = iVar23 + -1;
            iVar7 = iVar23 * 0xc;
            uVar16 = (ulong)iVar13;
            uVar25 = iVar7 + 0x6440dbU & 7;
            uVar2 = (uint)(frameFunctions + iVar23) & 7;
            uVar17 = (*(long*)((iVar7 + 0x6440dbU) - uVar25) << (7 - uVar25) * 8 |
                      uVar16 & 0xffffffffffffffffU >> (uVar25 + 1) * 8) &
                         -1L << (8 - uVar2) * 8 |
                     *(ulong*)((int)(frameFunctions + iVar23) - uVar2) >> uVar2 * 8;
            pcVar10 = frameFunctions[iVar23].name;
            uVar25 = iVar7 + 0x6440e7U & 7;
            puVar3 = (ulong*)((iVar7 + 0x6440e7U) - uVar25);
            *puVar3 = *puVar3 & -1L << (uVar25 + 1) * 8 | uVar17 >> (7 - uVar25) * 8;
            uVar25 = iVar7 + 0x6440e0U & 7;
            puVar3 = (ulong*)((iVar7 + 0x6440e0U) - uVar25);
            *puVar3 = uVar17 << uVar25 * 8 | *puVar3 & 0xffffffffffffffffU >> (8 - uVar25) * 8;
            frameFunctions[iVar23 + 1].name = pcVar10;
            if ((long)uVar16 < 1) {
                break;
            }
            iVar23 = iVar13;
        } while (0 < frameFunctions[iVar13].priority);
    }


    in_a3 = (long)iVar8;
    iVar23 = (int)uVar16;
    frameFunctions[iVar23 + 1].func = runWaterFunc;
    frameFunctions[iVar23 + 1].name = "runwater";
    frameFunctions[iVar23 + 1].priority = 0;
    if ((long)uVar16 <= in_a3) {
        prevExecutedFrameFunctionIdx = iVar8 + 1;
        in_a3 = (long)prevExecutedFrameFunctionIdx;
    }
    numRegisteredFrameFunctions += 1;

LAB_ram_0020294c:

*/
    getLmp("fx.lmp");
    getLmp("hud.lmp");
    getLmp("mapicons.lmp");
    getLmp("icons.lmp");
    getLmp("mouth.lmp");

    /*

        switch (selectedLanguageId) {
        case 0:
            pLoadingVifData = (undefined*)findLmpEntry("loading.lmp", "loading_us.vif");
            pcVar10 = "loading.lmp";
            entryName = "loading_us.tex";
            break;
        case 1:
            pLoadingVifData = (undefined*)findLmpEntry("load_sp.lmp", "loading_sp.vif");
            pcVar10 = "load_sp.lmp";
            entryName = "loading_sp.tex";
            break;
        case 2:
            pLoadingVifData = (undefined*)findLmpEntry("load_it.lmp", "loading_it.vif");
            pcVar10 = "load_it.lmp";
            entryName = "loading_it.tex";
            break;
        case 3:
            pcVar10 = "load_fr.lmp";
            pLoadingVifData = (undefined*)findLmpEntry("load_fr.lmp", "loading_fr.vif");
            entryName = "loading_fr.tex";
            break;
        case 4:
            pcVar10 = "load_gr.lmp";
            pLoadingVifData = (undefined*)findLmpEntry("load_gr.lmp", "loading_gr.vif");
            entryName = "loading_gr.tex";
            break;
        default:
            goto switchD_00202994_caseD_5;
        }
        pLoadingTexData = (undefined*)findLmpEntry(pcVar10, entryName);
    switchD_00202994_caseD_5:

    */
    setLmpGeneration(10);
    /*
        FUN_ram_00143780();
        FUN_ram_0016e648();

        displayEnvironment.dispfb &= 0xfffffe00;
        setDisplayRegs(&displayEnvironment);
        disableDisplay();
        INT_ram_00325c38 = 2;
    */
    isInterlaced = 1;
    gameExited = 0;

    //resetVus();
    //FLOAT_ram_00325c1c = 1.0;
    textBrightness[0] = 1.0f;
    while (gameExited == 0) {
        /*
        FUN_ram_00166948();
        if (curLevelId[0] != '\0') {
            if ((DAT_ram_00324474 == '\0') && (DAT_ram_00324476 == '\0')) {
                curRandomNum = curRandomNum * 0x19660d + 0x3c6ef35f;
                DAT_ram_00325ce4 = (curRandomNum >> 0x10) % 0x5dc + 0x32;
            }
            displayEnableCountdown = 3;
            disableDisplay();
            etexData = NULL;
            FUN_ram_00200ab0();
            DAT_ram_00325c3c = 10;
            DAT_ram_00325c40 = frameCount;
            ScriptDemonFunc();
            while (DAT_ram_00325c3c != 0) {
                DAT_ram_00325c48 = 1;
                doCamera();
                doDrawWorld();
                if (DAT_ram_003248b8 != '\0') {
                    cutFrame();
                }
                dealWithCdStreaming();
                processWorld();
                REG_RCNT0_COUNT = 0;
                uploadLevelTextureList();
                iVar8 = 10;
                if (DAT_ram_00325c48 != 0) {
                    bVar6 = levelTexUploadPending();
                    if (bVar6) {
                        iVar8 = 10;
                    } else {
                        iVar8 = DAT_ram_00325c3c -1;
                    }
                }
                DAT_ram_00325c3c = iVar8;
                if (500 < (int)(frameCount - DAT_ram_00325c40)) {
                    DAT_ram_00325c3c = 0;
                }
                if ((((DAT_ram_0029bce8 != 0) && (iVar8 = FUN_ram_0015bc60(DAT_ram_0029bce8), iVar8 != 0)) || ((DAT_ram_0029f5c8 != 0 && (iVar8 = FUN_ram_0015bc60(DAT_ram_0029f5c8), iVar8 != 0)))) && (DAT_ram_00325c3c < 3)) {
                    DAT_ram_00325c3c = 3;
                }
            }
            DAT_ram_00325cf0 = 1;
            loadingDoneFlag = 1;
            DAT_ram_00325c3c = 0;
            do {
                iVar8 = ReferThreadStatus(DAT_ram_00325ce8, NULL);
            } while (iVar8 != 0x10);
            DAT_ram_00325ced = 0;
            isLoading = 0;

            FUN_ram_0014fe30();
            initSlots();
            REG_GS_PMODE = 0;
            displayEnableCountdown = 6;
            DAT_ram_00325c74 = 0;
            DAT_ram_00325c78 = 0;
            INT_ram_00325c7c = 0;
            INT_ram_00325c80 = 0;
            if (etexData != NULL) {
                d = (sceDmaChan*)sceDmaGetChan(2);
                pTVar4 = etexData;
                fixupTex(etexData);
                addr = (void*)pTVar4->gif_madr_val;
                *(undefined2*)(((uint)addr | 0x20000000) + 0x14) = 0x3200;
                SYNC(0);
                sceDmaSendN(d, addr, (uint)pTVar4->qwc);
            }
            gauntletFramesLeft = 45000;
        }
        */

        //resetVus();
        startFrame();
        FlushCache(0);
        kickoffDMA();
        curDMABufHead = (u64*)UCAB_SEG(dmaBuffers[frameCount & 1]);
        curDMABufTail = curDMABufHead;

        /*
        if ((((DAT_ram_00325c4c == 1) && (DAT_ram_00324530 == 0)) && (DAT_ram_003248b8 == '\0')) &&
            (bVar6 = isGameRunning(), !bVar6)) {
            fVar26 = sin(sceneFrameNum << 10);
            uVar25 = (uint)(fVar26 * 10.0 + 50.0);
            beginText(pMenuFont, 6, 1, uVar25 | 0x80000000 | uVar25 << 0x10 | uVar25 << 8);
            displayTextCentered(0x140, 0x1d0, "Demo");
            displayTextCentered(0x140, 0x1e4, "Press Start");
            flushText();
            if (startButtonPressed != '\0') {
                gameExited = 1;
            }
            bVar6 = FUN_ram_0013c330();
            if (bVar6) {
                gameExited = 1;
            }
        }
        */

        /*
        iVar8 = strcmp(curLevelName, "gauntlet");
        if (iVar8 == 0) {
            iVar8 = gauntletFramesLeft / 3000;
            if (0 < gauntletFramesLeft) {
                if ((iVar8 * 3000 == gauntletFramesLeft) && (1 < iVar8)) {
                    pwVar9 = translateText("gauntletMinLeft");
                    simple_wsprintf((float)iVar8, 0, 0, 0, (wchar_t*)&messageText, pwVar9);
                    messageAlpha = 0xfa;
                }
                if (iVar8 == 0) {
                    pwVar9 = translateText("gauntletSecLeft");
                    simple_wsprintf((float)(gauntletFramesLeft / 0x32), 0, 0, 0, (wchar_t*)&messageText, pwVar9);
                    messageAlpha = 0xfa;
                }
            }
            bVar6 = isGameRunning();
            if (((!bVar6) && (gauntletFramesLeft += -1, gauntletFramesLeft < 1)) &&
                (DAT_ram_0029bce8 != 0)) {
                (**(code**)(*(int*)(DAT_ram_0029bce8 + 0x38) + 0x14))((long)(DAT_ram_0029bce8 + *(short*)(*(int*)(DAT_ram_0029bce8 + 0x38) + 0x10)), (long)(int)DAT_ram_00298a24, 0, 0x50);
            }
        }
        */

        //displayMessage();
        readInput();
        /*
            DAT_ram_00325c48 = 1;
        */
        int savedButtons_0 = convertedControllerInput[0].updatedButtons;
        int savedButtons2_0 = convertedControllerInput[0].updatedButtons2;
        int savedButtons_1 = convertedControllerInput[1].updatedButtons;
        int savedButtons2_1 = convertedControllerInput[1].updatedButtons2;

        int iVar8 = 2;
        //if (framesToProcess < 3) {
        //    iVar8 = framesToProcess;
        //}
        int idx = 0;
        while (idx < iVar8) {
            // Execute all frame functions with a priority less than 20
            prevExecutedFrameFunctionIdx = 0;
            while (prevExecutedFrameFunctionIdx < numRegisteredFrameFunctions) {
                if (frameFunctions[prevExecutedFrameFunctionIdx + 1].priority < 0x14) {
                    (*frameFunctions[prevExecutedFrameFunctionIdx + 1].func)();
                }
                prevExecutedFrameFunctionIdx += 1;
            }

            ++idx;

            /*
            if (!isGameRunning() && (DAT_ram_00325c3c == 0)) {
                sceneFrameNum += 1;
            }
            */
            convertedControllerInput[0].updatedButtons = 0;
            convertedControllerInput[0].updatedButtons2 = 0;
            convertedControllerInput[1].updatedButtons = 0;
            convertedControllerInput[1].updatedButtons2 = 0;
        }

        convertedControllerInput[0].updatedButtons = savedButtons_0;
        convertedControllerInput[0].updatedButtons2 = savedButtons2_0;
        convertedControllerInput[1].updatedButtons = savedButtons_1;
        convertedControllerInput[1].updatedButtons2 = savedButtons2_1;

        prevExecutedFrameFunctionIdx = 0;

        if (0 < numRegisteredFrameFunctions) {
            do {
                if (0x13 < frameFunctions[prevExecutedFrameFunctionIdx + 1].priority) {
                    (*frameFunctions[prevExecutedFrameFunctionIdx + 1].func)();
                }
                prevExecutedFrameFunctionIdx += 1;
            } while (prevExecutedFrameFunctionIdx < numRegisteredFrameFunctions);
        }
        /*
                if (DAT_ram_00325c44 != 0) {
                    if (((DAT_ram_00325c48 != 0) || (bVar6 = levelTexUploadPending(), !bVar6)) &&
                        (DAT_ram_00325c44 += -1, 0x40 < frameDisplayAlpha)) {
                        frameDisplayAlpha = 0x40;
                    }
                    initSlots();
                }
        */

        /*
                DAT_ram_00325ce4 -= 1;
                if (DAT_ram_00325ce4 < 1) {
                    pcVar10 = strstr(curLevelName, "burneye");
                    if (pcVar10 == NULL) {
                        curRandomNum = curRandomNum * 0x19660d + 0x3c6ef35f;
                        DAT_ram_00325ce4 = (curRandomNum >> 0x10) % 6000 + 12000;
                    } else {
                        curRandomNum = curRandomNum * 0x19660d + 0x3c6ef35f;
                        DAT_ram_00325ce4 = (curRandomNum >> 0x10) % 6000 + 21000;
                    }
                    lVar15 = (long)(int)curRandomNum;
                    iVar8 = strcmp(curLevelName, "tavern");
                    if (((iVar8 != 0) && (uVar25 = FUN_ram_001f41f8(2), (uVar25 & 0x20) == 0)) &&
                        (__haystack = getDialogFileForLevel(), __haystack != NULL)) {
                        for (; cVar1 = __haystack->name[0], __haystack->name[0] != '\0';
                             __haystack = __haystack + 1) {
                            pcVar10 = strstr(__haystack->name, "_env_");
                            if (pcVar10 != NULL) {
                                cVar1 = __haystack->name[0];
                                break;
                            }
                        }
                        if (cVar1 != '\0') {
                            speakDialog(__haystack->name, 2);
                        }
                    }
                }
            */
        //FUN_ram_001f0db0();
        //FUN_ram_00138260();
        dealWithCdStreaming();
        //processWorld();
        //uploadLevelTextureList();
        /*
        if (DAT_ram_003248b8 == '\0') {
            FUN_ram_001486a8(0);
        }
        */

        /*
         if (0 < DAT_ram_00324454) {
             if (((convertedControllerInput[0].RLUD_buttons & 0xff0fffff) == 0) &&
                 ((convertedControllerInput[1].RLUD_buttons & 0xff0fffff) == 0)) {
                 // nothing pressed (ignoring R stick)
                 DAT_ram_00324454 = 0;
             } else {
                 DAT_ram_00324454 -= 1;
             }
         }
         */
        /*
                if (DAT_ram_00325c3c != 0) {
                    bVar6 = levelTexUploadPending();
                    iVar8 = 0x14;
                    if (!bVar6) {
                        iVar8 = DAT_ram_00325c3c + -1;
                    }
                    DAT_ram_00325c3c = iVar8;
                    if (500 < (int)(frameCount - DAT_ram_00325c40)) {
                        DAT_ram_00325c3c = 0;
                    }
                    if ((2 < (int)frameCount) && (initSlots(), DAT_ram_00325c3c == 0)) {
                        DAT_ram_00325c74 = 0;
                        DAT_ram_00325c78 = 0;
                        INT_ram_00325c7c = 0;
                        INT_ram_00325c80 = 0;
                        displayEnableCountdown = 0;
                    }
                }
        */
        waitDMASema();
        sceGsSyncPath(0, 0);
        /*
                if (!isGameRunning()) {
                    if (DAT_ram_00324458 != 0) {
                        DAT_ram_00324458 -= 1;
                    }
                    if (DAT_ram_00325c10 != 0) {
                        DAT_ram_00325c10 -= 1;
                    }
                }
        */
        frameCount += 1;
        resetTextureAlloc();
        while (vblankSetsMeToFF != 0xff) {
            WaitSema(vblankSema);
        }
        if (displayEnableCountdown < 1) {
            enableDisplay();
        } else {
            disableDisplay();
            --displayEnableCountdown;
        }

        //iVar8 = REG_RCNT0_COUNT;
        //REG_RCNT0_COUNT = 0;
        //framesToProcess = (iVar8 + 0x82) / 0x104;
        endFrame();
        //bVar5 = BYTE_ram_00325c8c;

        DI();
        vblankSetsMeToFF = 0; //BYTE_ram_00325c8b == 0;
        //BYTE_ram_00325c8c = 0;
        //framesToProcess = (char)bVar5 + 1;
        EI();

        for (int i = 0; i < 2; ++i) {
            const float val = textBrightness[i];
            if (val > 1.0f) {
                textBrightness[i] = (val - 1.0) * 0.8 + 1.0;
            }
        }
    }

    //DAT_ram_0032447a = 0;
    disableDisplay();
    /*
        DAT_ram_0032447e = 0;
        initScriptFile(NULL);
        FUN_ram_00182310(1);
        FUN_ram_001f02c0(0);
        FUN_ram_001f02c0(1);
        FUN_ram_001f02c0(2);
        lmpCleanup(10);

        FUN_ram_00143750();
        FUN_ram_0014f9e8();
        clearRecordedInput();
        DAT_ram_00324894 = 0;
        gamePausedFlag = 0;
        DAT_ram_00325c4c = 0;
        pWorldData = NULL;
    */
}
