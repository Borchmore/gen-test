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
static void collisionAction(u16 sp1, u16 val1, u16 sp2);

//sprites structures (pointer of Sprite)
Sprite* ship;
Sprite* shots[24];
Sprite* enemies[35];
Sprite* other[20];

//global variable declarations
fix16 shipPosX;
fix16 shipPosY;
s16 shipHP; //maybe change these HP variables to be a data attribute for each sprite?
fix16 shotPosX[24];
fix16 shotPosY[24];
s16 shotHP[24]; //?
fix16 enemyPosX[35];
fix16 enemyPosY[35];
s16 enemyHP[35]; //?
s16 bgaScroll;
s16 bgbScroll;
fix16 shipVSpeed;
fix16 shipHSpeed;
u16 lockout;
u16 test; //used as a test; enemies spawn once every second
u16 i; //used as an incremental counter in a couple methods, might change name to "increment" when I get less lazy
u16 sp1; //identifies the first sprite in the collisionAction function
u16 sp2; //"    "   "   " second sprite "   "   "   "   "   "   "   "
u16 val1; //identifies array value for sp1

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
    shipHP = 1;
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
        if((shipVSpeed < SHIP_ACCEL) && (shipVSpeed > (-1)*SHIP_ACCEL)){
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
        if((shipHSpeed < SHIP_ACCEL) && (shipHSpeed > (-1)*SHIP_ACCEL)){
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

    collisionAction(0, 0, 2);

    if(shipHP == 0){
        SPR_setVisibility(ship, HIDDEN);
    }
    else{
    //sets position
        SPR_setPosition(ship, fix16ToInt(shipPosX), fix16ToInt(shipPosY));
    }

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

    i = 0;
    //first checks to see if the user is locked out from shot input
    if(lockout == 0){
        //checks if B button is pressed
        if((JOY_readJoypad(JOY_1)) & (BUTTON_B)){
            //searches shots[] array from open spot
            while(i < 24){
                if(shots[i] == NULL){
                    shotPosX[i] = shipPosX + FIX16(24);
                    shotPosY[i] = shipPosY + FIX16(4);
                    shotHP[i] = 1;
                    shots[i] = SPR_addSprite(&shot_sprite, fix16ToInt(shotPosX[i]), fix16ToInt(shotPosY[i]), TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
                    lockout = 19;
                    i = 24;
                }
                ++i;
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
            collisionAction(1, i, 2);
            if(shotPosX[i] > FIX16(320) || (shotHP[i] == 0)){
                SPR_releaseSprite(shots[i]);
                shots[i] = NULL;
            }
            else{
                shotPosX[i] = shotPosX[i] + FIX16(8);
                SPR_setPosition(shots[i], fix16ToInt(shotPosX[i]), fix16ToInt(shotPosY[i]));
            }
        }
        ++i;
    }

}

static void enemyAction(){

    i = 0;

    if(test == 0){
        while(i < 35){
            if(enemies[i] == NULL){
                enemyPosX[i] = FIX16(320);
                enemyPosY[i] = FIX16(random() % 224);
                enemyHP[i] = 1;
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
            if((enemyPosX[i] < FIX16(-32)) || (enemyHP[i] == 0)){
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

static void collisionAction(u16 sp1, u16 val1, u16 sp2){

    //the values in the header correspond to sprite types; 0 = ship, 1 = shot, 2 = enemy
    //this will have to change when as the game gets more complex to accommodate non-binary HP and damage values

    u8 i = 0;

    if(sp1 == 0){
        if(sp2 == 2){
            while(i < 35){
                if(((enemyPosX[i] >= shipPosX) && (enemyPosX[i] <= shipPosX +FIX16(32))) || ((shipPosX >= enemyPosX[i]) && (shipPosX <= enemyPosX[i] +FIX16(32)))){
                    //Fuck me, that's a long ass if statement! Oh shit now we have to do it again!! (or make the above if statement even longer)
                    if(((enemyPosY[i] >= shipPosY) && (enemyPosY[i] <= shipPosY +FIX16(16))) || ((shipPosY >= enemyPosY[i]) && (shipPosY <= enemyPosY[i] +FIX16(16)))){
                        shipHP = 0;
                        enemyHP[i] = 0;
                        i = 35;
                    }
                }
                ++i;
            }
        }
    }

    i = 0;

    if(sp1 == 1){
        if(sp2 == 2){
            while(i < 35){
                if(((enemyPosX[i] >= shotPosX[val1]) && (enemyPosX[i] <= shotPosX[val1] +FIX16(16))) || ((shotPosX[val1] >= enemyPosX[i]) && (shotPosX[val1] <= enemyPosX[i] +FIX16(32)))){
                    //Fuck me, that's a long ass if statement! Oh shit now we have to do it again!! (or make the above if statement even longer)
                    if(((enemyPosY[i] >= shotPosY[val1]) && (enemyPosY[i] <= shotPosY[val1] +FIX16(8))) || ((shotPosY[val1] >= enemyPosY[i]) && (shotPosY[val1] <= enemyPosY[i] +FIX16(16)))){
                        shotHP[val1] = 0;
                        enemyHP[i] = 0;
                        i = 35;
                    }
                }
                ++i;
            }
        }
    }

    /*may need to be implemented when "other" objects get implemented
    if(sp1->status == enemies->status){

    } but until then...*/
}

