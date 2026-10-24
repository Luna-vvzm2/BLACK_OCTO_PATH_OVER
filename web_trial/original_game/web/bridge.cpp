#include "DxLib.h"
#include <cstdarg>
#include <vector>
EM_JS(int, web_init, (), { return Module.web.init(); });
EM_ASYNC_JS(int, web_image, (const char* path), {
    return await Module.web.loadImage(UTF8ToString(path));
});
EM_ASYNC_JS(int, web_sound, (const char* path), {
    return await Module.web.loadSound(UTF8ToString(path));
});
EM_JS(int, image_part, (int id,int x,int y,int w,int h), {
    const src=Module.web.images[id]; if(!src) return -1;
    return Module.web.images.push({image:src.image,x:src.x+x,y:src.y+y,w:w,h:h})-1;
});
EM_JS(int, image_size, (int id,int axis), {
    const i=Module.web.images[id]; return i ? (axis?i.h:i.w) : 0;
});
EM_JS(void, render_image, (int id,double x,double y,double cx,double cy,double sx,double sy,double angle), {
    const g=Module.web.images[id];if(!g)return;
    const c=Module.web.ctx;c.save();c.translate(x,y);c.rotate(angle);c.scale(sx,sy);
    c.drawImage(g.image,g.x,g.y,g.w,g.h,-cx,-cy,g.w,g.h);c.restore();
});
EM_JS(void, shape, (int kind,double x,double y,double a,double b,unsigned int color,int fill,double line), {
    const c=Module.web.ctx;c.fillStyle=c.strokeStyle='#'+color.toString(16).padStart(6,'0');
    c.lineWidth=line;c.beginPath();
    if(kind===0)c.rect(x,y,a-x,b-y);
    if(kind===1){c.moveTo(x,y);c.lineTo(a,b);}
    if(kind===2)c.arc(x,y,a,0,Math.PI*2);
    if(fill)c.fill();else c.stroke();
});
EM_JS(void, text_draw,(int x,int y,const char* t,unsigned int color,int size),{
    const c=Module.web.ctx;c.fillStyle='#'+color.toString(16).padStart(6,'0');
    c.font=size+'px sans-serif';c.textBaseline='top';
    UTF8ToString(t).split('\n').forEach((line,i)=>c.fillText(line,x,y+i*size));
});
static int blend=0,alpha=255,fontSize=24;
static std::vector<int> fonts{24};
int DxLib_Init(){return web_init();}
int DxLib_End(){EM_ASM({Module.web.stopSounds();});return 0;}
int SetGraphMode(int w,int h,int){EM_ASM({Module.canvas.width=$0;Module.canvas.height=$1;},w,h);return 0;}
int SetBackgroundColor(int r,int g,int b){EM_ASM({Module.web.background='#'+$0.toString(16).padStart(6,'0');},GetColor(r,g,b));return 0;}
void SetWindowTextA(HWND,const char* name){EM_ASM({document.title=UTF8ToString($0)+' — Web試作';},name);}
void MessageBoxA(HWND,const char* text,const char*,int){EM_ASM({console.error(UTF8ToString($0));},text);}
int ClearDrawScreen(){EM_ASM({const w=Module.web;const c=w.ctx;c.save();c.globalAlpha=1;c.fillStyle=w.background;c.fillRect(0,0,Module.canvas.width,Module.canvas.height);c.restore();});return 0;}
int ScreenFlip(){emscripten_sleep(16);return 0;}
int ProcessMessage(){return 0;} // Browser remains active; game handles Escape.
int CheckHitKey(int code){return EM_ASM_INT({return Module.web.keys[$0]?1:0;},code);}
int GetMouseInput(){return EM_ASM_INT({return Module.web.mouse.buttons;});}
int GetMousePoint(int* x,int* y){*x=EM_ASM_INT({return Module.web.mouse.x;});*y=EM_ASM_INT({return Module.web.mouse.y;});return 0;}
int GetJoypadInputState(int id){return EM_ASM_INT({
 const p=navigator.getGamepads?.()[$0];if(!p)return 0;
 const map=([12,13,14,15,0,1,2,3,4,5,8,9]);let result=0;
 map.forEach((b,i)=>{if(p.buttons[b]?.pressed)result|=1<<i;});return result;
},id);}
int GetJoypadXInputState(int id,XINPUT_STATE* state){
 if(!EM_ASM_INT({return !!navigator.getGamepads?.()[$0];},id))return -1;
 state->LeftTrigger=EM_ASM_INT({return navigator.getGamepads()[$0].buttons[6].value*255;},id);
 state->RightTrigger=EM_ASM_INT({return navigator.getGamepads()[$0].buttons[7].value*255;},id);return 0;
}
static int analog(int* x,int* y,int id,int axis){
 *x=EM_ASM_INT({return (navigator.getGamepads?.()[$0]?.axes[$1]||0)*1000;},id,axis);
 *y=EM_ASM_INT({return (navigator.getGamepads?.()[$0]?.axes[$1+1]||0)*1000;},id,axis);return 0;
}
int GetJoypadAnalogInput(int* x,int* y,int id){return analog(x,y,id,0);}
int GetJoypadAnalogInputRight(int* x,int* y,int id){return analog(x,y,id,2);}
int LoadGraph(const char* p){return web_image(p);}
int LoadDivGraph(const char* p,int n,int cols,int rows,int w,int h,int* result){
 int base=LoadGraph(p);if(base<0 || cols<=0 || rows<=0 || n>cols*rows)return -1;
 for(int i=0;i<n;++i) result[i]=image_part(base,(i%cols)*w,(i/cols)*h,w,h);
 DeleteGraph(base);return 0;
}
int GetGraphSize(int id,int* w,int* h){*w=image_size(id,0);*h=image_size(id,1);return *w?0:-1;}
int DeleteGraph(int id){EM_ASM({delete Module.web.images[$0];},id);return 0;}
int DrawRotaGraph3F(float x,float y,float cx,float cy,double sx,double sy,double a,int id,int,int fh,int fv){render_image(id,x,y,cx,cy,fh?-sx:sx,fv?-sy:sy,a);return 0;}
int DrawRotaGraphF(float x,float y,double s,double a,int id,int t,int fh,int fv){return DrawRotaGraph3F(x,y,image_size(id,0)/2.0f,image_size(id,1)/2.0f,s,s,a,id,t,fh,fv);}
int DrawGraph(int x,int y,int id,int t){return DrawRotaGraph3F(x,y,0,0,1,1,0,id,t);}
int DrawExtendGraph(int x,int y,int x2,int y2,int id,int t){int w=image_size(id,0),h=image_size(id,1);if(!w||!h)return -1;return DrawRotaGraph3F(x,y,0,0,double(x2-x)/w,double(y2-y)/h,0,id,t);}
int DrawBoxAA(float x,float y,float a,float b,unsigned int c,int f,float l){shape(0,x,y,a,b,c,f,l);return 0;}
int DrawBox(int x,int y,int a,int b,unsigned int c,int f){return DrawBoxAA(x,y,a,b,c,f);}
int DrawLineAA(float x,float y,float a,float b,unsigned int c,float l){shape(1,x,y,a,b,c,0,l);return 0;}
int DrawLine(int x,int y,int a,int b,unsigned int c,int l){return DrawLineAA(x,y,a,b,c,l);}
int DrawCircleAA(float x,float y,float r,int,unsigned int c,int f,float l){shape(2,x,y,r,0,c,f,l);return 0;}
int DrawCircle(int x,int y,int r,unsigned int c,int f,int l){return DrawCircleAA(x,y,r,32,c,f,l);}
int DrawQuadrangle(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4,unsigned int c,int f){
 EM_ASM({const c=Module.web.ctx;c.fillStyle=c.strokeStyle='#'+$8.toString(16).padStart(6,'0');c.beginPath();c.moveTo($0,$1);c.lineTo($2,$3);c.lineTo($4,$5);c.lineTo($6,$7);c.closePath();if($9)c.fill();else c.stroke();},x1,y1,x2,y2,x3,y3,x4,y4,c,f);return 0;
}
int GetDrawBlendMode(int* b,int* a){*b=blend;*a=alpha;return 0;}
int SetDrawBlendMode(int b,int a){blend=b;alpha=a;EM_ASM({Module.web.ctx.globalAlpha=$0===1?$1/255:1;},b,a);return 0;}
int CreateFontToHandle(const char*,int size,int,int){fonts.push_back(size);return static_cast<int>(fonts.size())-1;}
int DeleteFontToHandle(int id){if(id>=0&&id<static_cast<int>(fonts.size()))fonts[id]=0;return 0;}
int GetDrawStringWidthToHandle(const char* t,int,int id){return EM_ASM_INT({const c=Module.web.ctx;c.font=$1+'px sans-serif';return Math.ceil(c.measureText(UTF8ToString($0)).width);},t,fonts.at(id));}
int DrawStringToHandle(int x,int y,const char* t,unsigned int c,int id){text_draw(x,y,t,c,fonts.at(id));return 0;}
int DrawString(int x,int y,const char* t,unsigned int c){text_draw(x,y,t,c,fontSize);return 0;}
int DrawFormatString(int x,int y,unsigned int c,const char* format,...){char buf[2048];va_list args;va_start(args,format);vsnprintf(buf,sizeof(buf),format,args);va_end(args);return DrawString(x,y,buf,c);}
int SetFontSize(int size){fontSize=size;return 0;}
int ChangeFont(const char*){return 0;} // Browser sans-serif fallback, documented limitation.
int LoadSoundMem(const char* p){return web_sound(p);}
int StopSoundMem(int id){EM_ASM({const s=Module.web.sounds[$0];if(s?.source){s.source.stop();s.source=null;}},id);return 0;}
int DeleteSoundMem(int id){StopSoundMem(id);EM_ASM({delete Module.web.sounds[$0];},id);return 0;}
int PlaySoundMem(int id,int type,int restart){return EM_ASM_INT({
 const w=Module.web;const s=w.sounds[$0];if(!s)return -1;if(s.source&&!$2)return 0;
 if(s.source)s.source.stop();const src=w.audio.createBufferSource();src.buffer=s.buffer;src.loop=$1===2;
 src.connect(s.gain);s.source=src;src.onended=()=>{if(s.source===src)s.source=null;};src.start();return 0;
},id,type,restart);}
int CheckSoundMem(int id){return EM_ASM_INT({return Module.web.sounds[$0]?.source?1:0;},id);}
int ChangeVolumeSoundMem(int volume,int id){return EM_ASM_INT({const s=Module.web.sounds[$1];if(!s)return -1;s.gain.gain.value=$0/255;return 0;},volume,id);}
