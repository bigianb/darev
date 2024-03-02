#pragma once

typedef int (*sceneHandler)();

void runScene(sceneHandler sceneFunc);
