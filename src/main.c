#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"

#define SFX_SHOT        64
#define SFX_KILL        65
#define SFX_PWUP        66

#define SHIP_ANIM_STAND 0

#define SHIP_MAX_SPEED  FIX16(6)

#define MAX_SPRITES     FIX16(80)

#define SHIP_ACCEL      FIX16(1)

#define SHIP_MIN_POSX   FIX16(0)
#define SHIP_MAX_POSX   FIX16(288)
#define SHIP_MIN_POSY   FIX16(0)
#define SHIP_MAX_POSY   FIX16(208)


// currently unused, should probably implement this later to optimize code
static void handleInput();
//functions to handle different sprite actions
static void shipAction();
static void shotAction();
static void bgAction();
static void enemyAction();
//currently unused
static void otherAction();

//sprites structures (pointer of Sprite)
Sprite* ship;
Sprite* shots[24];
Sprite* enemies[35];
Sprite* other[20];

//global variable declarations
fix16 shipPosX;
fix16 shipPosY;
s16 shotPosX[24];
s16 shotPosY[24];
fix16 enemyPosX[35];
fix16 enemyPosY[35];
s16 bgaScroll;
s16 bgbScroll;
fix16 shipVSpeed;
fix16 shipHSpeed;
u8 lockout;
//used as a test; enemies spawn once every second
u8 test;

int main(){

    //local variable declarations, assignments
    u16 palette[64];
    u16 ind;
    shipPosX = FIX16(160);
    shipPosY = FIX16(112);

    // disable interrupt when accessing VDP
    SYS_disableInts();
    // initialization
    VDP_setScreenWidth320();

    // init SFX
    //SND_setPCM_XGM(SFX_SHOT, shotSfxNameHere, sizeof(shotSfxNameHere));
    //SND_setPCM_XGM(SFX_KILL, killSfxNameHere, sizeof(killSfxNameHere));
    //SND_setPCM_XGM(SFX_PWUP, pwupSfxNameHere, sizeof(pwupSfxNameHere));
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
    ship = SPR_addSprite(&ship_sprite, fix16ToInt(shipPosX), fix16ToInt(shipPosY), TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_update();

    // prepare palettes
    memcpy(&palette[0], bgb_image.palette->data, 16 * 2);
    memcpy(&palette[16], bga_image.palette->data, 16 * 2);
    memcpy(&palette[32], ship_sprite.palette->data, 16 * 2);
    memcpy(&palette[48], enemy_sprite.palette->data, 16 * 2);

    // fade in
    VDP_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE);

    while(TRUE){

        //handle on screen actions
        //handleInput();
        shipAction();
        shotAction();
        bgAction();
        enemyAction();
        //otherAction();

        //update sprites
        SPR_update();

        VDP_waitVSync();
    }

    return 0;
}

static void handleInput(){}

static void shipAction(){

    //set shipVSpeed; first handles up input, then down input, then no input
    if((JOY_readJoypad(JOY_1)) & (BUTTON_UP)){
        if(shipVSpeed > (-1)*SHIP_MAX_SPEED){
            shipVSpeed = shipVSpeed - SHIP_ACCEL;
        }
    }
    else if((JOY_readJoypad(JOY_1)) & (BUTTON_DOWN)){
        if(shipVSpeed < SHIP_MAX_SPEED){
            shipVSpeed = shipVSpeed + SHIP_ACCEL;
        }
    }
    else {
        if((shipVSpeed < SHIP_ACCEL) & (shipVSpeed > (-1)*SHIP_ACCEL)){
            shipVSpeed = 0;
        }
        else{
            if (shipVSpeed < 0){
                shipVSpeed = shipVSpeed + SHIP_ACCEL;
            }
            if (shipVSpeed > 0){
                shipVSpeed = shipVSpeed - SHIP_ACCEL;
            }
        }
    }

    //set shipHSpeed; first handles left input, then up input, then no input
    if((JOY_readJoypad(JOY_1)) & (BUTTON_LEFT)){
        if(shipHSpeed > (-1)*SHIP_MAX_SPEED){
            shipHSpeed = shipHSpeed - SHIP_ACCEL;
        }
    }
    else if((JOY_readJoypad(JOY_1)) & (BUTTON_RIGHT)){
        if(shipHSpeed < SHIP_MAX_SPEED){
            shipHSpeed = shipHSpeed + SHIP_ACCEL;
        }
    }
    else {
        if((shipHSpeed < SHIP_ACCEL) & (shipHSpeed > (-1)*SHIP_ACCEL)){
            shipHSpeed = 0;
        }
        else{
            if (shipHSpeed < 0){
                shipHSpeed = shipHSpeed + SHIP_ACCEL;
            }
            if (shipHSpeed > 0){
                shipHSpeed = shipHSpeed - SHIP_ACCEL;
            }
        }
    }

    //updates position based on speed
    shipPosX = shipPosX + shipHSpeed;
    shipPosY = shipPosY + shipVSpeed;

    //screen boundary
    if(shipPosX < SHIP_MIN_POSX){
        shipPosX = SHIP_MIN_POSX;
        shipHSpeed = 0;
    }
    if(shipPosX > SHIP_MAX_POSX){
        shipPosX = SHIP_MAX_POSX;
        shipHSpeed = 0;
    }
    if(shipPosY < SHIP_MIN_POSY){
        shipPosY = SHIP_MIN_POSY;
        shipVSpeed = 0;
    }
    if(shipPosY > SHIP_MAX_POSY){
        shipPosY = SHIP_MAX_POSY;
        shipVSpeed = 0;
    }

    //sets position
    SPR_setPosition(ship, fix16ToInt(shipPosX), fix16ToInt(shipPosY));

}

static void bgAction(){

    //updates scrolling
    bgaScroll = bgaScroll - 10;
    bgbScroll = bgbScroll - 1;

    //sets scrolling
    VDP_setHorizontalScroll(PLAN_A, bgaScroll);
    VDP_setHorizontalScroll(PLAN_B, bgbScroll);

}

static void shotAction(){

    u8 i = 0;

    //first checks to see if the user is locked out from shot input
    if(lockout == 0){
        //checks if B button is pressed
        if((JOY_readJoypad(JOY_1)) & (BUTTON_B)){
            //searches shots[] array from open spot
            while(i < 24){
                if(shots[i] == NULL){
                    shotPosX[i] = fix16ToInt(shipPosX) + 24;
                    shotPosY[i] = fix16ToInt(shipPosY) + 4;
                    shots[i] = SPR_addSprite(&shot_sprite, shotPosX[i], shotPosY[i], TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
                    lockout = 19;
                    i = 24;
                }
                i++;
            }
        }
    }
    //counts down lockout time if user is locked out
    else{
        lockout--;
    }

    i = 0;

    //sets position for each non-null element of the shots[] array
    while(i < 24){
        if (shots[i] != NULL){
            if(shotPosX[i] > 320){
                SPR_releaseSprite(shots[i]);
                shots[i] = NULL;
            }
            else{
                shotPosX[i] = shotPosX[i] + 8;
                SPR_setPosition(shots[i], shotPosX[i], shotPosY[i]);
            }
        }
        ++i;
    }

}

static void enemyAction(){

    u8 i = 0;

    if(test == 0){
        while(i < 35){
            if(enemies[i] == NULL){
                enemyPosX[i] = FIX16(320);
                enemyPosY[i] = FIX16(random() % 224);
                enemies[i] = SPR_addSprite(&enemy_sprite, enemyPosX[i], enemyPosY[i], TILE_ATTR(PAL3, FALSE, FALSE, FALSE));
                i = 35;
                test = 59;
            }
            ++i;
        }
    }
    else{
        test--;
    }

    while(i < 35){
        if (enemies[i] != NULL){
            if(enemyPosX[i] < FIX16(-32)){
                SPR_releaseSprite(enemies[i]);
                enemies[i] = NULL;
            }
            else{
                enemyPosX[i] = enemyPosX[i] - FIX16(3);
                enemyPosY[i] = enemyPosY[i] + sinFix16(test*17);
                SPR_setPosition(enemies[i], fix16ToInt(enemyPosX[i]), fix16ToInt(enemyPosY[i]));
            }
        }
        ++i;
    }
}

static void otherAction(){}
