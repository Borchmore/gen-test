#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"

#define SFX_SHOT        64
#define SFX_KILL        65
#define SFX_PWUP        66

#define ANIM_STAND      0
#define ANIM_WAIT       1
#define ANIM_WALK       2
#define ANIM_RUN        3
#define ANIM_BRAKE      4
#define ANIM_UP         5
#define ANIM_CROUNCH    6
#define ANIM_ROLL       7

#define MAX_SPEED       FIX32(6)
#define BRAKE_SPEED     FIX32(2)

#define MAX_SPRITES     FIX32(80)

#define ACCEL           FIX32(1)

#define MIN_POSX        FIX32(0)
#define MAX_POSX        FIX32(288)
#define MIN_POSY        FIX32(0)
#define MAX_POSY        FIX32(208)


// forward
static void handleInput();

static void shipAction();
static void shotAction();
static void bgAction();

// sprites structure (pointer of Sprite)
Sprite* sprites[80];

fix32 posx = FIX32(160);
fix32 posy = FIX32(120);
fix32 bgaScroll;
fix32 bgbScroll;
fix32 vSpeedShip;
fix32 hSpeedShip;

int main()
{
    u16 palette[64];
    u16 ind;

    // disable interrupt when accessing VDP
    SYS_disableInts();
    // initialization
    VDP_setScreenWidth320();

    // init SFX
    //SND_setPCM_XGM(SFX_JUMP, sonic_jump_sfx, sizeof(sonic_jump_sfx));
    //SND_setPCM_XGM(SFX_ROLL, sonic_roll_sfx, sizeof(sonic_roll_sfx));
    //SND_setPCM_XGM(SFX_STOP, sonic_stop_sfx, sizeof(sonic_stop_sfx));
    // start music
    //SND_startPlay_XGM(sonic_music);

    // init sprites engine
    SPR_init(80, 768, 512);

    // set all palette to black
    VDP_setPaletteColors(0, (u16*) palette_black, 64);

    // load background
    ind = TILE_USERINDEX;
    VDP_drawImageEx(PLAN_B, &bgb_image, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += bgb_image.tileset->numTile;
    VDP_drawImageEx(PLAN_A, &bga_image, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += bga_image.tileset->numTile;



    // VDP process done, we can re enable interrupts
    SYS_enableInts();

    // init ship sprite
    sprites[0] = SPR_addSprite(&ship_sprite, fix32ToInt(posx), fix32ToInt(posy), TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    SPR_update();

    // prepare palettes
    memcpy(&palette[0], bgb_image.palette->data, 16 * 2);
    memcpy(&palette[16], bga_image.palette->data, 16 * 2);
    memcpy(&palette[32], ship_sprite.palette->data, 16 * 2);
    //next line needs to be replaced with enemy sprite palette
    memcpy(&palette[48], ship_sprite.palette->data, 16 * 2);

    // fade in
    VDP_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE);

    while(TRUE)
    {
        handleInput();

        shipAction();
        //shotAction();
        bgAction();
        //camAction();

        // update sprites
        SPR_update();

        VDP_waitVSync();
    }

    return 0;
}

static void handleInput(){
}

static void shipAction(){

    if((JOY_readJoypad(JOY_1)) & (BUTTON_UP)){
        if(vSpeedShip > (-1)*MAX_SPEED){
            vSpeedShip = vSpeedShip - ACCEL;
        }
    }
    else if((JOY_readJoypad(JOY_1)) & (BUTTON_DOWN)){
        if(vSpeedShip < MAX_SPEED){
            vSpeedShip = vSpeedShip + ACCEL;
        }
    }
    else {
        if((vSpeedShip < ACCEL) & (vSpeedShip > (-1)*ACCEL)){
            vSpeedShip = 0;
        }
        else{
            if (vSpeedShip < 0){
                vSpeedShip = vSpeedShip + ACCEL;
            }
            if (vSpeedShip > 0){
                vSpeedShip = vSpeedShip - ACCEL;
            }
        }
    }

    if((JOY_readJoypad(JOY_1)) & (BUTTON_LEFT)){
        if(hSpeedShip > (-1)*MAX_SPEED){
            hSpeedShip = hSpeedShip - ACCEL;
        }
    }
    else if((JOY_readJoypad(JOY_1)) & (BUTTON_RIGHT)){
        if(hSpeedShip < MAX_SPEED){
            hSpeedShip = hSpeedShip + ACCEL;
        }
    }
    else {
        if((hSpeedShip < ACCEL) & (hSpeedShip > (-1)*ACCEL)){
            hSpeedShip = 0;
        }
        else{
            if (hSpeedShip < 0){
                hSpeedShip = hSpeedShip + ACCEL;
            }
            if (hSpeedShip > 0){
                hSpeedShip = hSpeedShip - ACCEL;
            }
        }
    }

    posx = posx + hSpeedShip;
    posy = posy + vSpeedShip;

    if(posx < MIN_POSX){
        posx = MIN_POSX;
        hSpeedShip = 0;
    }
    if(posx > MAX_POSX){
        posx = MAX_POSX;
        hSpeedShip = 0;
    }
    if(posy < MIN_POSY){
        posy = MIN_POSY;
        vSpeedShip = 0;
    }
    if(posy > MAX_POSY){
        posy = MAX_POSY;
        vSpeedShip = 0;
    }

    SPR_setPosition(sprites[0], fix32ToInt(posx), fix32ToInt(posy));

}

static void bgAction(){

    bgaScroll = bgaScroll - 10;
    bgbScroll = bgbScroll - 1;

    VDP_setHorizontalScroll(PLAN_A, bgaScroll);
    VDP_setHorizontalScroll(PLAN_B, bgbScroll);

}
