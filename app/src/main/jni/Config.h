#pragma once

struct EspCfg {
    bool  enabled = true, box = true, line = false, name = true,
          distance = true, teamCheck = true, visibleOnly = false;
    float maxDist = 400.f;
    float boxColor[4]  = {0,1,0,1};
    float lineColor[4] = {1,1,1,1};
};
struct AimbotCfg {
    bool  enabled = false, visibleOnly = true, ignoreKnocked = true,
          teamCheck = true;
    int   bone = 0;
    float fov = 150.f, smooth = 4.f, maxDist = 300.f;
};

inline EspCfg g_esp;
inline AimbotCfg g_aim;
