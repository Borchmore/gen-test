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

#define MAX_SPEED       FIX16(6)
#define BRAKE_SPEED     FIX16(2)

#define MAX_SPRITES     FIX16(80)

#define ACCEL           FIX16(1)

#define MIN_POSX        FIX16(0)
#define MAX_POSX        FIX16(288)
#define MIN_POSY        FIX16(0)
#define MAX_POSY        FIX16(208)


// forward
static void handleInput();

static void shipAction();
static void shotAction();
static void bgAction();

// sprites structure (pointer of Sprite)
Sprite* ship;
Sprite* shots[24];
Sprite* enemies[35];
Sprite* other[20];

fix16 posx;
fix16 posy;
s16 shotPosx[24];
s16 shotPosy[24];
s16 bgaScroll;
s16 bgbScroll;
fix16 vSpeedShip;
fix16 hSpeedShip;
u8 lockout;
u8 bPress;

int main(){

    u16 palette[64];
    u16 ind;
    posx = FIX16(160);
    posy = FIX16(112);
    bPress = 0;
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
    VDP_drawImageEx(PLAN_A, &bga_image, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += bga_image.tileset->numTile;

    // VDP process done, we can re enable interrupts
    SYS_enableInts();

    // init ship sprite
    ship = SPR_addSprite(&ship_sprite, fix16ToInt(posx), fix16ToInt(posy), TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_update();

    // prepare palettes
    memcpy(&palette[0], bgb_image.palette->data, 16 * 2);
    memcpy(&palette[16], bga_image.palette->data, 16 * 2);
    memcpy(&palette[32], ship_sprite.palette->data, 16 * 2);
    //next line needs to be replaced with enemy sprite palette
    memcpy(&palette[48], ship_sprite.palette->data, 16 * 2);

    // fade in
    VDP_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE);

    while(TRUE){

        handleInput();

        shipAction();
        shotAction();
        bgAction();

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

    SPR_setPosition(ship, fix16ToInt(posx), fix16ToInt(posy));

}

static void bgAction(){

    bgaScroll = bgaScroll - 10;
    bgbScroll = bgbScroll - 1;

    VDP_setHorizontalScroll(PLAN_A, bgaScroll);
    VDP_setHorizontalScroll(PLAN_B, bgbScroll);

}

static void shotAction(){

    u8 i = 0;

    if(lockout == 0){
        if((JOY_readJoypad(JOY_1)) & (BUTTON_B)){
            while(i < 24){
                if(shots[i] == NULL){
                    shots[i] = SPR_addSprite(&shot_sprite, fix16ToInt(posx) + 24, fix16ToInt(posy) + 4, TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
                    shotPosx[i] = fix16ToInt(posx) + 24;
                    shotPosy[i] = fix16ToInt(posy) + 4;
                    lockout = 19;
                    i = 24;
                }
                i++;
            }
        }
    }
    else{
        lockout--;
    }

    i = 0;

    while(i < 24){
        if (shots[i] != NULL){
            if(shotPosx[i] > 320){
                SPR_releaseSprite(shots[i]);
                shots[i] = NULL;
            }
            else{
                shotPosx[i] = shotPosx[i] + 8;
                SPR_setPosition(shots[i], shotPosx[i], shotPosy[i]);
            }
        }
        i++;
    }

}
