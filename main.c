#include "raylib.h"
#include "minesweeper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BASE_CELL      32
#define BASE_PADDING   8
#define BASE_HEADER    48
#define BASE_TOP_PANEL   80
#define BASE_BOTTOM_BAR  36
#define MIN_CELL         22

static Color num_colors[9] = {
    {0,0,0,255},{0,0,255,255},{0,128,0,255},{255,0,0,255},
    {0,0,128,255},{128,0,0,255},{0,128,128,255},{0,0,0,255},{128,128,128,255}
};

static Color lighter(Color c, int a) {
    return Color{ (unsigned char)fmin(c.r+a,255), (unsigned char)fmin(c.g+a,255), (unsigned char)fmin(c.b+a,255), 255 };
}
static Color darker(Color c, int a) {
    return Color{ (unsigned char)fmax(c.r-a,0), (unsigned char)fmax(c.g-a,0), (unsigned char)fmax(c.b-a,0), 255 };
}

static void draw_btn(int x,int y,int w,int h,bool sunken) {
    Color c = sunken ? Color{170,170,170,255} : Color{208,208,208,255};
    DrawRectangle(x,y,w,h,c);
    if(sunken){
        DrawRectangle(x,y,w,2,GRAY); DrawRectangle(x,y,2,h,GRAY);
        DrawRectangle(x+2,y+2,w-4,1,DARKGRAY); DrawRectangle(x+2,y+2,1,h-4,DARKGRAY);
        DrawRectangle(x,y+h-2,w,2,WHITE); DrawRectangle(x+w-2,y,2,h,WHITE);
    } else {
        DrawRectangle(x,y,w,2,WHITE); DrawRectangle(x,y,2,h,WHITE);
        DrawRectangle(x+2,y+2,w-4,1,lighter(c,40)); DrawRectangle(x+2,y+2,1,h-4,lighter(c,40));
        DrawRectangle(x,y+h-2,w,2,GRAY); DrawRectangle(x+w-2,y,2,h,GRAY);
        DrawRectangle(x,y+h-3,w,1,darker(c,40)); DrawRectangle(x+w-3,y,1,h,darker(c,40));
    }
}

static void draw_cell_raised(int x,int y,int s) {
    Color c={208,208,208,255};
    DrawRectangle(x,y,s,s,c); DrawRectangle(x,y,s,2,WHITE); DrawRectangle(x,y,2,s,WHITE);
    DrawRectangle(x+2,y+2,s-4,1,lighter(c,40)); DrawRectangle(x+2,y+2,1,s-4,lighter(c,40));
    DrawRectangle(x,y+s-2,s,2,GRAY); DrawRectangle(x+s-2,y,2,s,GRAY);
    DrawRectangle(x,y+s-3,s,1,darker(c,40)); DrawRectangle(x+s-3,y,1,s,darker(c,40));
}

static void draw_cell_sunken(int x,int y,int s) {
    Color c={188,188,188,255};
    DrawRectangle(x,y,s,s,c); DrawRectangle(x,y,s,2,GRAY); DrawRectangle(x,y,2,s,GRAY);
    DrawRectangle(x+2,y+2,s-4,1,DARKGRAY); DrawRectangle(x+2,y+2,1,s-4,DARKGRAY);
    DrawRectangle(x,y+s-2,s,2,WHITE); DrawRectangle(x+s-2,y,2,s,WHITE);
}

static void draw_flag(int x,int y,int s) {
    int cx=x+s/2,cy=y+s/2,fh=s*5/8,fw=s*3/8,top=cy-fh/2;
    if(fh<6)fh=6; if(fw<4)fw=4;
    DrawRectangle(cx-1,top-1,2,fh+3,BLACK);
    Vector2 vt={(float)(cx-1),(float)top},vb={(float)(cx-1),(float)(top+fh)},vm={(float)(cx-fw-1),(float)cy};
    DrawTriangle(vt,vm,vb,RED); DrawTriangleLines(vt,vm,vb,BLACK);
    DrawRectangle(cx-3,top+fh+1,8,2,BLACK);
}

static void draw_mine(int x,int y,int s) {
    int cx=x+s/2,cy=y+s/2,r=(s-14)/2,d=r*7/10;
    DrawCircle(cx,cy,(float)r,BLACK); DrawCircle(cx,cy,(float)(r/4),BLACK);
    DrawLine(cx-r+2,cy,cx+r-2,cy,WHITE); DrawLine(cx,cy-r+2,cx,cy+r-2,WHITE);
    DrawLine(cx-d,cy-d,cx+d,cy+d,WHITE); DrawLine(cx+d,cy-d,cx-d,cy+d,WHITE);
}

static void draw_smiley(Game *g,int x,int y,int s) {
    int r=s/2-1,cx=x+s/2,cy=y+s/2;
    DrawCircle(cx,cy,(float)r,YELLOW); DrawCircleLines(cx,cy,(float)r,BLACK);
    if(g->face==FACE_WON){
        DrawRectangle(cx-r/2-4,cy-r/3-3,8,6,BLACK); DrawRectangle(cx+r/2-4,cy-r/3-3,8,6,BLACK);
        DrawLine(cx-r/2+4,cy-r/3,cx+r/2-4,cy-r/3,BLACK);
        DrawRectangle(cx-r/3,cy+2,r*2/3,r/2,BLACK); DrawRectangle(cx-r/3+1,cy+3,r*2/3-2,r/2-2,YELLOW);
    } else if(g->face==FACE_LOST){
        DrawLine(cx-r/2-3,cy-r/3-3,cx-r/2+3,cy-r/3+3,BLACK);
        DrawLine(cx-r/2+3,cy-r/3-3,cx-r/2-3,cy-r/3+3,BLACK);
        DrawLine(cx+r/2-3,cy-r/3-3,cx+r/2+3,cy-r/3+3,BLACK);
        DrawLine(cx+r/2+3,cy-r/3-3,cx+r/2-3,cy-r/3+3,BLACK);
        DrawRectangle(cx-r/3,cy-r/3-3,r*2/3,3,BLACK);
    } else if(g->face==FACE_SURPRISE){
        DrawCircle(cx-r/3,cy-r/3,(float)(r/3),BLACK); DrawCircle(cx+r/3,cy-r/3,(float)(r/3),BLACK);
        DrawCircle(cx,cy+r/4,(float)(r/3),BLACK);
    } else {
        DrawCircle(cx-r/3,cy-r/3,(float)(r/5),BLACK); DrawCircle(cx+r/3,cy-r/3,(float)(r/5),BLACK);
        for(int i=0;i<3;i++) DrawRectangle(cx-r/3+i,cy+i+2,r*2/3-i*2,1,BLACK);
    }
}

static Sound gen_tone(float f,float d,float v){
    int sr=22050,c=(int)(sr*d); short *b=(short*)calloc((size_t)c,sizeof(short));
    if(!b){Sound e={0};return e;}
    for(int i=0;i<c;i++) b[i]=(short)(v*32767.0*sin(2.0*PI*f*(double)i/sr));
    Wave w={(unsigned int)c,(unsigned int)sr,16,1,b}; Sound s=LoadSoundFromWave(w); UnloadWave(w); return s;
}

static Sound gen_sweep(float fs,float fe,float d,float v){
    int sr=22050,c=(int)(sr*d); short *b=(short*)calloc((size_t)c,sizeof(short));
    if(!b){Sound e={0};return e;}
    for(int i=0;i<c;i++){double t=(double)i/sr;b[i]=(short)(v*32767.0*sin(2.0*PI*(fs+(fe-fs)*t/d)*t));}
    Wave w={(unsigned int)c,(unsigned int)sr,16,1,b}; Sound s=LoadSoundFromWave(w); UnloadWave(w); return s;
}

static Sound gen_arpeggio(float freqs[],int n,float ed,float v){
    int sr=22050,per=(int)(sr*ed),c=per*n; short *b=(short*)calloc((size_t)c,sizeof(short));
    if(!b){Sound e={0};return e;}
    for(int seg=0;seg<n;seg++)for(int i=0;i<per;i++){
        double t=(double)i/sr; float env=1.0f;
        if(i<per/8)env=(float)i/(per/8);else if(i>per-per/8)env=(float)(per-i)/(per/8);
        b[seg*per+i]=(short)(v*env*32767.0*sin(2.0*PI*freqs[seg]*t));
    }
    Wave w={(unsigned int)c,(unsigned int)sr,16,1,b}; Sound s=LoadSoundFromWave(w); UnloadWave(w); return s;
}

static int calc_cell_size(Game *g,int ww,int wh){
    int cs=(ww-16)/g->cols,cs2=(int)((wh-16)/(80.0f/32+36.0f/32+g->rows));
    cs=cs<cs2?cs:cs2; return cs<22?22:cs;
}

int main(void){
    Game game; game_init(&game,DIFF_BEGINNER);
    int sw=game.cols*32+16,sh=game.rows*32+80+36+16;
    InitWindow(sw,sh,"Minesweeper"); SetTargetFPS(60); SetWindowState(FLAG_WINDOW_RESIZABLE);
    InitAudioDevice();
    Sound snd_r=gen_tone(800,0.025f,0.6f),snd_f=gen_tone(600,0.03f,0.5f),snd_l=gen_sweep(300,80,0.45f,0.7f);
    float wf[]={523,659,784};Sound snd_w=gen_arpeggio(wf,3,0.12f,0.6f);
    while(!WindowShouldClose()){
        int ww=GetScreenWidth(),wh=GetScreenHeight();
        int cs=calc_cell_size(&game,ww,wh);
        float sc=(float)cs/32;
        int gw=game.cols*cs,gh=game.rows*cs;
        int gx=(ww-gw)/2;if(gx<8)gx=8;
        int gy=(int)(80*sc)+8;
        int hh=(int)(48*sc),hy=8;
        int ss=hh-6;if(ss<20)ss=20;
        int sx=gx+gw/2-ss/2,sy=hy+(hh-ss)/2;
        int cw=(int)(50*sc),ch=(int)(32*sc),cy=hy+(hh-ch)/2;
        int dbh=(int)(22*sc);if(dbh<16)dbh=16;
        int dbw=(int)(84*sc);if(dbw<64)dbw=64;
        int dby=hy+hh+4,dbg=(gw-3*dbw)/4;if(dbg<2)dbg=2;
        int bbh=(int)(36*sc);if(bbh<30)bbh=30;
        int rbh=(int)(26*sc);if(rbh<18)rbh=18;
        int rbw=(int)(100*sc);if(rbw<70)rbw=70;
        int rbx=gx+(gw-rbw)/2,rby=gy+gh+(bbh-rbh)/2;
        int fn=(int)(18*sc);if(fn<10)fn=10;
        int fc=(int)(22*sc);if(fc<10)fc=10;
        int fd=(int)(14*sc);if(fd<8)fd=8;

        int mx=GetMouseX(),my=GetMouseY(),mc=(mx-gx)/cs,mr=(my-gy)/cs;
        bool ld=IsMouseButtonDown(MOUSE_LEFT_BUTTON),rd=IsMouseButtonDown(MOUSE_RIGHT_BUTTON);

        if(mx>gx&&mx<gx+gw&&my>gy&&my<gy+gh&&mr>=0&&mr<game.rows&&mc>=0&&mc<game.cols){
            game.mouse_grid_x=mc;game.mouse_grid_y=mr;
            if(!game.game_over){
                Cell*hc=game_cell(&game,mr,mc);
                game.face=((ld&&!rd&&!hc->revealed&&!hc->flagged)||(rd&&!ld&&!hc->revealed)||(ld&&rd))?FACE_SURPRISE:FACE_SMILE;
            }
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                if(rd){game_chord(&game,mr,mc);PlaySound(snd_r);}
                else{game_reveal(&game,mr,mc);PlaySound(game.game_over?(game.won?snd_w:snd_l):snd_r);}
            }
            if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)){
                if(ld){game_chord(&game,mr,mc);PlaySound(snd_r);}
                else{game_toggle_flag(&game,mr,mc);PlaySound(snd_f);}
            }
        }else{game.mouse_grid_x=-1;game.mouse_grid_y=-1;if(!game.game_over)game.face=FACE_SMILE;}
        if(!ld&&!rd&&!game.game_over)game.face=FACE_SMILE;

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            if(mx>=sx&&mx<=sx+ss&&my>=sy&&my<=sy+ss)game_reset(&game);
            const char*lb[]={"Beginner","Intermed","Expert"};
            for(int i=0;i<3;i++){
                int bx=gx+dbg+i*(dbw+dbg);
                if(mx>=bx&&mx<=bx+dbw&&my>=dby&&my<=dby+dbh&&i!=(int)game.difficulty)
                {game_destroy(&game);game_init(&game,(Difficulty)i);}
            }
            if(mx>=rbx&&mx<=rbx+rbw&&my>=rby&&my<=rby+rbh)game_reset(&game);
        }

        game_update_elapsed(&game);

        BeginDrawing();
        ClearBackground(Color{198,198,198,255});

        DrawRectangle(gx,hy,gw,hh,Color{212,212,212,255});
        DrawRectangle(gx,hy,gw,2,WHITE);DrawRectangle(gx,hy,2,hh,WHITE);
        DrawRectangle(gx,hy+hh-2,gw,2,GRAY);DrawRectangle(gx+gw-2,hy,2,hh,GRAY);
        {int rm=game.total_mines-game.flags_placed;char b[8];snprintf(b,8,"%03d",rm);
         DrawRectangle(gx+8+MeasureText("Mines:",fd)+4,cy,cw,ch,BLACK);
         DrawRectangle(gx+12+MeasureText("Mines:",fd),cy+2,cw-4,ch-4,Color{40,0,0,255});
         int tw=MeasureText(b,fc);DrawText(b,gx+12+MeasureText("Mines:",fd)+(cw-tw)/2,cy+(ch-fc)/2,fc,RED);
         DrawText("Mines:",gx+8,cy+(ch-fd)/2,fd,BLACK);}
        {int et=(int)game.elapsed;if(et>999)et=999;char b[8];snprintf(b,8,"%03d",et);
         int lw=MeasureText("Time:",fd),cx=gx+gw-8-cw;
         DrawRectangle(cx,cy,cw,ch,BLACK);DrawRectangle(cx+2,cy+2,cw-4,ch-4,Color{40,0,0,255});
         int tw=MeasureText(b,fc);DrawText(b,cx+(cw-tw)/2,cy+(ch-fc)/2,fc,RED);
         DrawText("Time:",cx-lw-4,cy+(ch-fd)/2,fd,BLACK);}

        draw_smiley(&game,sx,sy,ss);
        DrawRectangle(gx,gy,gw,gh,Color{196,196,196,255});
        DrawRectangle(gx,gy,gw,2,GRAY);DrawRectangle(gx,gy,2,gh,GRAY);
        DrawRectangle(gx,gy+gh-2,gw,2,WHITE);DrawRectangle(gx+gw-2,gy,2,gh,WHITE);

        for(int r=0;r<game.rows;r++)for(int c=0;c<game.cols;c++){
            Cell*cl=game_cell(&game,r,c);
            int cx=gx+c*cs,cy=gy+r*cs;
            if(cl->revealed){
                if(cl->mine&&!game.won){DrawRectangle(cx,cy,cs,cs,RED);draw_mine(cx,cy,cs);}
                else{draw_cell_sunken(cx,cy,cs);
                    if(cl->mine)draw_mine(cx,cy,cs);
                    else if(cl->adjacent>0){char n[2]={(char)('0'+cl->adjacent),0};int tw=MeasureText(n,fn);DrawText(n,cx+(cs-tw)/2,cy+(cs-fn)/2+1,fn,num_colors[cl->adjacent]);}
                }
            }else{
                bool down=((r==game.mouse_grid_y&&c==game.mouse_grid_x)&&!game.game_over&&((ld&&!rd&&!cl->flagged)||(rd&&!ld)||(ld&&rd)));
                if(down){DrawRectangle(cx,cy,cs,cs,Color{176,176,176,255});draw_cell_sunken(cx,cy,cs);if(cl->flagged)draw_flag(cx,cy,cs);}
                else{draw_cell_raised(cx,cy,cs);if(cl->flagged)draw_flag(cx,cy,cs);}
                if(game.game_over&&!game.won&&cl->flagged&&!cl->mine){
                    DrawRectangle(cx,cy,cs,cs,Color{188,188,188,255});draw_cell_sunken(cx,cy,cs);draw_flag(cx,cy,cs);
                    int tw=MeasureText("X",fn);DrawText("X",cx+(cs-tw)/2,cy+(cs-fn)/2+1,fn,RED);
                }
            }
        }

        const char*lb[]={"Beginner","Intermed","Expert"};
        for(int i=0;i<3;i++){
            int bx=gx+dbg+i*(dbw+dbg);
            draw_btn(bx,dby,dbw,dbh,i==(int)game.difficulty);
            int tw=MeasureText(lb[i],fd);DrawText(lb[i],bx+(dbw-tw)/2,dby+(dbh-fd)/2,fd,BLACK);
        }

        draw_btn(rbx,rby,rbw,rbh,false);
        int tw=MeasureText("New Game",fd);DrawText("New Game",rbx+(rbw-tw)/2,rby+(rbh-fd)/2,fd,BLACK);

        if(game.game_over){
            DrawRectangle(gx,gy,gw,gh,Color{0,0,0,160});
            int yc=gy+gh/2;
            int ts=(int)(cs*1.2f);if(ts<20)ts=20;
            const char*t=game.won?"YOU WIN!":"GAME OVER";
            tw=MeasureText(t,ts);DrawText(t,gx+(gw-tw)/2,yc-ts-5,ts,game.won?GOLD:RED);
            char ib[64];snprintf(ib,64,"Time: %d s",(int)game.elapsed);
            int is=(int)(cs*0.5f);if(is<12)is=12;
            int iw=MeasureText(ib,is);DrawText(ib,gx+(gw-iw)/2,yc+5,is,WHITE);
            const char*h="Click face to restart";
            int hs=(int)(cs*0.4f);if(hs<10)hs=10;
            int hw=MeasureText(h,hs);DrawText(h,gx+(gw-hw)/2,yc+is+12,hs,WHITE);
        }
        EndDrawing();
    }
    game_destroy(&game);
    UnloadSound(snd_r);UnloadSound(snd_f);UnloadSound(snd_l);UnloadSound(snd_w);
    CloseAudioDevice();CloseWindow();
    return 0;
}
