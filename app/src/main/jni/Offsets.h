#pragma once
#include <cstdint>

namespace Off {
    inline uintptr_t InitBase            = 0x8CA88B4;
    inline uintptr_t StaticClass         = 0x5C;
    inline uintptr_t CurrentMatch        = 0x50;
    inline uintptr_t LocalPlayer         = 0x44;
    inline uintptr_t DictionaryEntities  = 0x68;
    inline uintptr_t Player_IsDead       = 0x4C;
    inline uintptr_t Player_Name         = 0x224;
    inline uintptr_t Player_Data         = 0x44;
    inline uintptr_t Player_IsTeam       = 0x51;
    inline uintptr_t Player_Health       = 0x1C0;
    inline uintptr_t AvatarManager       = 0x3F4;
    inline uintptr_t Avatar              = 0x94;
    inline uintptr_t Avatar_IsVisible    = 0x7C;
    inline uintptr_t FollowCamera        = 0x384;
    inline uintptr_t Camera              = 0x14;
    inline uintptr_t AimRotation         = 0x33C;
    inline uintptr_t ViewMatrix          = 0x98;

    enum Bone : uintptr_t {
        Head = 0x38C, Neck = 0x390, Spine = 0x394, Hip = 0x39C, Root = 0x3A0,
        LeftShoulder = 0x3C0, RightShoulder = 0x3C4, LeftElbow = 0x3D0,
        RightElbow = 0x3D4, LeftHand = 0x3B8, RightWrist = 0x3B4,
        LeftFoot = 0x3B0, RightFoot = 0x3AC,
    };
}
