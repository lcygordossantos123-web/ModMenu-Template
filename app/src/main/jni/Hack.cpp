#include <jni.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <pthread.h>          // <-- ADICIONADO (pthread_create / pthread_t)
#include <GLES/gl.h>          // <-- ADICIONADO (GLES1: glMatrixMode, glLoadIdentity, glOrthof, glColor4fv, glVertexPointer, glEnableClientState...)
#include <GLES2/gl2.h>        // mantido (glDrawArrays, glViewport, GL_LINE_LOOP...)
#include "Offsets.h"
#include "Config.h"
#include "KittyMemory/KittyMemory.h"

struct Vec3 { float x,y,z; };
struct Vec2 { float x,y; };
struct Quat { float x,y,z,w; };
static inline float vlen(const Vec3&v){ return sqrtf(v.x*v.x+v.y*v.y+v.z*v.z); }
static inline Vec3  vsub(const Vec3&a,const Vec3&b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }

static uintptr_t g_base = 0;
static int g_scrW = 1080, g_scrH = 2400;
static float g_view[16] = {0};

static inline void* R(uintptr_t o){ return (void*)(g_base+o); }
template<typename T> static inline T rd(uintptr_t a){ return *(T*)a; }

static uintptr_t findLib(const char* n){
    FILE* f=fopen("/proc/self/maps","r"); if(!f) return 0;
    char l[512]; uintptr_t b=0;
    while(fgets(l,sizeof(l),f)){ if(strstr(l,n)){ b=(uintptr_t)strtoull(l,nullptr,16); break; } }
    fclose(f); return b;
}
static uintptr_t staticClass(){
    uintptr_t p=(uintptr_t)R(Off::InitBase); if(!p) return 0;
    p=rd<uintptr_t>(p); if(!p) return 0;
    return rd<uintptr_t>(p+Off::StaticClass);
}
static uintptr_t matchPtr(){ uintptr_t s=staticClass(); return s?rd<uintptr_t>(s+Off::CurrentMatch):0; }
static uintptr_t localPlayer(){ uintptr_t m=matchPtr(); return m?rd<uintptr_t>(m+Off::LocalPlayer):0; }

static void getPlayers(std::vector<uintptr_t>& out){
    out.clear();
    uintptr_t m=matchPtr(); if(!m) return;
    uintptr_t d=rd<uintptr_t>(m+Off::DictionaryEntities); if(!d) return;
    uintptr_t e=rd<uintptr_t>(d+0x18); int c=rd<int>(d+0x20);
    if(!e||c<=0||c>200) return;
    for(int i=0;i<c;i++){
        uintptr_t en=e+(uintptr_t)i*0x18;
        if(rd<int>(en)<0) continue;
        uintptr_t p=rd<uintptr_t>(en+0x10);
        if(p) out.push_back(p);
    }
}
static Vec3 bonePos(uintptr_t p,uintptr_t off){
    uintptr_t d=rd<uintptr_t>(p+Off::Player_Data); if(!d) return {0,0,0};
    uintptr_t t=rd<uintptr_t>(d+off); if(!t) return {0,0,0};
    return rd<Vec3>(t+0x90);
}
static bool isTeammate(uintptr_t p){
    uintptr_t d=rd<uintptr_t>(p+Off::Player_Data); if(!d) return false;
    uintptr_t a=rd<uintptr_t>(d+Off::Avatar); if(!a) return false;
    return rd<uint8_t>(a+Off::Player_IsTeam)!=0;
}
static bool isVisible(uintptr_t p){
    uintptr_t m=rd<uintptr_t>(p+Off::AvatarManager); if(!m) return true;
    uintptr_t a=rd<uintptr_t>(m+Off::Avatar); if(!a) return true;
    return rd<uint8_t>(a+Off::Avatar_IsVisible)!=0;
}
static void updateView(){
    uintptr_t l=localPlayer(); if(!l) return;
    uintptr_t f=rd<uintptr_t>(l+Off::FollowCamera); if(!f) return;
    uintptr_t c=rd<uintptr_t>(f+Off::Camera); if(!c) return;
    uintptr_t v=c+Off::ViewMatrix+0x24;
    for(int i=0;i<16;i++) g_view[i]=rd<float>(v+i*4);
}
static bool w2s(const Vec3& w,Vec2& s){
    float x=g_view[0]*w.x+g_view[4]*w.y+g_view[8]*w.z+g_view[12];
    float y=g_view[1]*w.x+g_view[5]*w.y+g_view[9]*w.z+g_view[13];
    float z=g_view[2]*w.x+g_view[6]*w.y+g_view[10]*w.z+g_view[14];
    if(z<0.01f) return false;
    s.x=(g_scrW*0.5f)*(1.f+x/z);
    s.y=(g_scrH*0.5f)*(1.f-y/z);
    return true;
}
static void glBox(float x,float y,float w,float h,float c[4]){
    glColor4fv(c); float v[]={x,y,x+w,y,x+w,y+h,x,y+h};
    glEnableClientState(GL_VERTEX_ARRAY); glVertexPointer(2,GL_FLOAT,0,v);
    glDrawArrays(GL_LINE_LOOP,0,4); glDisableClientState(GL_VERTEX_ARRAY);
}
void renderESP(){
    if(!g_esp.enabled) return;
    glViewport(0,0,g_scrW,g_scrH);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrthof(0,g_scrW,g_scrH,0,-1,1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    updateView();
    uintptr_t lp=localPlayer();
    std::vector<uintptr_t> ps; getPlayers(ps);
    for(uintptr_t p:ps){
        if(!p||p==lp) continue;
        if(g_esp.teamCheck&&isTeammate(p)) continue;
        if(rd<uint8_t>(p+Off::Player_IsDead)) continue;
        if(g_esp.visibleOnly&&!isVisible(p)) continue;
        Vec3 h=bonePos(p,Off::Head), f=bonePos(p,Off::Root);
        Vec2 sh,sf; if(!w2s(h,sh)||!w2s(f,sf)) continue;
        float hh=sf.y-sh.y; if(hh<=0) continue;
        float ww=hh*0.5f, xx=sh.x-ww*0.5f;
        if(g_esp.box) glBox(xx,sh.y,ww,hh,g_esp.boxColor);
    }
}
static Quat fromEuler(float p,float y){
    float cp=cosf(p*0.5f),sp=sinf(p*0.5f),cy=cosf(y*0.5f),sy=sinf(y*0.5f);
    return {cy*sp,sy*cp,-sy*sp,cy*cp};
}
static Quat slerp(const Quat&a,Quat b,float t){
    float d=a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;
    if(d<0){b.x=-b.x;b.y=-b.y;b.z=-b.z;b.w=-b.w;d=-d;}
    if(d>0.9995f){
        Quat r{a.x+t*(b.x-a.x),a.y+t*(b.y-a.y),a.z+t*(b.z-a.z),a.w+t*(b.w-a.w)};
        float n=sqrtf(r.x*r.x+r.y*r.y+r.z*r.z+r.w*r.w);
        return {r.x/n,r.y/n,r.z/n,r.w/n};
    }
    float th=acosf(d),s=sinf(th),wa=sinf((1-t)*th)/s,wb=sinf(t*th)/s;
    return {a.x*wa+b.x*wb,a.y*wa+b.y*wb,a.z*wa+b.z*wb,a.w*wa+b.w*wb};
}
static Quat lookRot(Vec3 d){
    float l=vlen(d); if(l<0.0001f) return {0,0,0,1};
    d.x/=l;d.y/=l;d.z/=l;
    return fromEuler(asinf(-d.y),atan2f(d.x,d.z));
}
static uintptr_t boneIdx(int i){
    switch(i){case 1:return Off::Neck;case 2:return Off::Spine;case 3:return Off::Hip;default:return Off::Head;}
}
void aimbotTick(){
    if(!g_aim.enabled) return;
    uintptr_t lp=localPlayer(); if(!lp) return;
    Vec3 cam=bonePos(lp,Off::Head);
    std::vector<uintptr_t> ps; getPlayers(ps);
    float best=g_aim.fov; uintptr_t tgt=0; Vec3 tb{};
    for(uintptr_t p:ps){
        if(!p||p==lp) continue;
        if(g_aim.teamCheck&&isTeammate(p)) continue;
        if(rd<uint8_t>(p+Off::Player_IsDead)) continue;
        if(g_aim.visibleOnly&&!isVisible(p)) continue;
        Vec3 b=bonePos(p,boneIdx(g_aim.bone));
        if(vlen(vsub(b,cam))>g_aim.maxDist) continue;
        Vec2 s; if(!w2s(b,s)) continue;
        float dx=s.x-g_scrW*0.5f, dy=s.y-g_scrH*0.5f;
        float fov=sqrtf(dx*dx+dy*dy);
        if(fov<best){best=fov;tgt=p;tb=b;}
    }
    if(!tgt) return;
    Quat t=lookRot(vsub(tb,cam));
    uintptr_t a=lp+Off::AimRotation;
    Quat c=rd<Quat>(a);
    float k=g_aim.smooth<=1.f?1.f:1.f/g_aim.smooth;
    *(Quat*)a=slerp(c,t,k);
}
static void* hackThread(void*){
    while(!g_base){ g_base=findLib("libil2cpp.so"); usleep(100000); }
    while(true){ aimbotTick(); std::this_thread::sleep_for(std::chrono::milliseconds(8)); }
    return nullptr;
}
extern "C" void StartHack(){
    pthread_t t; pthread_create(&t,nullptr,hackThread,nullptr);
}
