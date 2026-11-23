#pragma once
// Browser implementation of the subset used by this game; not the DxLib library.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <emscripten.h>
using UINT = unsigned int;
using UINT8 = unsigned char;
using HWND = void*;
using TCHAR = char;
struct LARGE_INTEGER { long long QuadPart = 0; };
struct XINPUT_STATE { unsigned char LeftTrigger = 0, RightTrigger = 0; };
#define _T(x) x
constexpr int TRUE=1,FALSE=0,MB_OK=0,DX_SCREEN_BACK=-2,DX_FONTTYPE_NORMAL=0;
constexpr int DX_BLENDMODE_NOBLEND=0,DX_BLENDMODE_ALPHA=1,DX_PLAYTYPE_BACK=1,DX_PLAYTYPE_LOOP=2;
constexpr int DX_INPUT_PAD1=0,DX_INPUT_PAD2=1,MOUSE_INPUT_LEFT=1,MOUSE_INPUT_RIGHT=2;
constexpr int PAD_INPUT_UP=1,PAD_INPUT_DOWN=2,PAD_INPUT_LEFT=4,PAD_INPUT_RIGHT=8;
constexpr int PAD_INPUT_1=16,PAD_INPUT_2=32,PAD_INPUT_3=64,PAD_INPUT_4=128;
constexpr int PAD_INPUT_5=256,PAD_INPUT_6=512,PAD_INPUT_7=1024,PAD_INPUT_8=2048;
constexpr int KEY_INPUT_UP=38,KEY_INPUT_DOWN=40,KEY_INPUT_LEFT=37,KEY_INPUT_RIGHT=39;
constexpr int KEY_INPUT_ESCAPE=27,KEY_INPUT_SPACE=32,KEY_INPUT_RETURN=13,KEY_INPUT_PGUP=33,KEY_INPUT_PGDN=34,KEY_INPUT_4=52;
constexpr int KEY_INPUT_W=87,KEY_INPUT_A=65,KEY_INPUT_S=83,KEY_INPUT_D=68,KEY_INPUT_U=85;
constexpr int KEY_INPUT_I=73,KEY_INPUT_O=79,KEY_INPUT_P=80,KEY_INPUT_Q=81,KEY_INPUT_E=69;
constexpr int KEY_INPUT_F=70,KEY_INPUT_G=71,KEY_INPUT_H=72,KEY_INPUT_M=77;
inline unsigned int GetColor(int r,int g,int b){return (r<<16)|(g<<8)|b;}
inline void QueryPerformanceFrequency(LARGE_INTEGER* x){x->QuadPart=1000000;}
inline void QueryPerformanceCounter(LARGE_INTEGER* x){x->QuadPart=static_cast<long long>(emscripten_get_now()*1000);}
// Window-specific options map to the existing browser canvas rather than an OS window.
inline int ChangeWindowMode(int){return 0;}
int SetGraphMode(int,int,int);
int SetBackgroundColor(int,int,int);
inline int SetWaitVSyncFlag(int){return 0;} // ScreenFlip yields to the browser.
inline int SetUseXInputFlag(int){return 0;} // Browser Gamepad API is used instead.
inline HWND GetMainWindowHandle(){return nullptr;}
void SetWindowTextA(HWND,const char*);
void MessageBoxA(HWND,const char*,const char*,int);
int DxLib_Init();
int DxLib_End();
inline int SetDrawScreen(int){return 0;} // Single Canvas target only.
int ClearDrawScreen();
int ScreenFlip();
int ProcessMessage();
int CheckHitKey(int);
int GetMouseInput();
int GetMousePoint(int*,int*);
int GetJoypadInputState(int);
int GetJoypadXInputState(int,XINPUT_STATE*);
int GetJoypadAnalogInput(int*,int*,int);
int GetJoypadAnalogInputRight(int*,int*,int);
int LoadGraph(const char*);
int LoadDivGraph(const char*,int,int,int,int,int,int*);
int GetGraphSize(int,int*,int*);
int DeleteGraph(int);
int DrawGraph(int,int,int,int);
int DrawExtendGraph(int,int,int,int,int,int);
int DrawRotaGraphF(float,float,double,double,int,int,int=FALSE,int=FALSE);
int DrawRotaGraph3F(float,float,float,float,double,double,double,int,int,int=FALSE,int=FALSE);
int DrawBox(int,int,int,int,unsigned int,int);
int DrawBoxAA(float,float,float,float,unsigned int,int,float=1);
int DrawLine(int,int,int,int,unsigned int,int=1);
int DrawLineAA(float,float,float,float,unsigned int,float=1);
int DrawCircle(int,int,int,unsigned int,int=TRUE,int=1);
int DrawCircleAA(float,float,float,int,unsigned int,int=TRUE,float=1);
int DrawQuadrangle(int,int,int,int,int,int,int,int,unsigned int,int);
int GetDrawBlendMode(int*,int*);
int SetDrawBlendMode(int,int);
int CreateFontToHandle(const char*,int,int,int=0);
int DeleteFontToHandle(int);
int GetDrawStringWidthToHandle(const char*,int,int);
int DrawStringToHandle(int,int,const char*,unsigned int,int);
int DrawString(int,int,const char*,unsigned int);
int DrawFormatString(int,int,unsigned int,const char*,...);
int SetFontSize(int);
int ChangeFont(const char*);
int LoadSoundMem(const char*);
int DeleteSoundMem(int);
int PlaySoundMem(int,int,int=TRUE);
int StopSoundMem(int);
int CheckSoundMem(int);
int ChangeVolumeSoundMem(int,int);
