#include "panel_conf.h"
#include "panel.h"
#include "mbwmidi.h"

// TODO: put all wids in one .h ?

void panelConstruct(SDL_Renderer* rend);
void panelLoop(uint32_t ms);

typedef struct {
    const char* name;
    const char* hint;
    const SDL_KeyCode keycode;
    const uint8_t midikey;
} ButtonDef;

static ButtonDef bdef[] = {

    { .name = "f1", .hint = "p", .keycode = SDLK_p, .midikey = 0 },
    { .name = "f2", .hint = "[", .keycode = SDLK_LEFTBRACKET, .midikey = 1 },
    { .name = "f3", .hint = "]", .keycode = SDLK_RIGHTBRACKET, .midikey = 2 },
    { .name = "<", .hint = "<", .keycode = SDLK_LEFT, .midikey = 6 },
    { .name = "enc", .hint = "ret", .keycode = SDLK_RETURN, .midikey = 4 },
    { .name = ">", .hint = ">", .keycode = SDLK_RIGHT, .midikey = 5 },
    { .name = "s1", .hint = "a", .keycode = SDLK_a, .midikey = 11 },
    { .name = "s2", .hint = "s", .keycode = SDLK_s, .midikey = 12 },
    { .name = "s3", .hint = "d", .keycode = SDLK_d, .midikey = 13 },
    { .name = "s4", .hint = "f", .keycode = SDLK_f, .midikey = 14 },
    { .name = "t1", .hint = "z", .keycode = SDLK_z, .midikey = 7 },
    { .name = "t2", .hint = "x", .keycode = SDLK_x, .midikey = 8 },
    { .name = "t3", .hint = "c", .keycode = SDLK_c, .midikey = 9 },
    { .name = "t4", .hint = "v", .keycode = SDLK_v, .midikey = 10 },
};

#define BUTTONS_COUNT (sizeof(bdef) / sizeof(ButtonDef))

static WidgetButton buttons[BUTTONS_COUNT];
static WidgetEnc encoder;
WID_DISPLAY_MONO_DEFINE(display_mono, 122, 32) // static monochrome display
static WidgetFrameCounter framecounter;

#define LCD_SCALE 3
#define LCD_DEF_WTD ((122 + 6) * LCD_SCALE)
#define LCD_DEF_HTH ((32 + 6) * LCD_SCALE)


void panelConstruct(SDL_Renderer* rend)
{
    wFrameCounterInit(&framecounter, 5, 5, rend);

    uint16_t xinit = PAN_OFFSET;

    uint16_t y = PAN_OFFSET;
    uint16_t x = xinit + (PAN_ELEMENTS_WIDTH - LCD_DEF_WTD) / 2;

    wDisplayMonoInit(&display_mono, &display_mono_mgldisp, display_mono_framebuffer, x, y, LCD_SCALE, rend);
    mgsDisplay(&display_mono_mgldisp);
    mgdString("Hello!", COLOR_ON);

    y += LCD_DEF_HTH + PAN_BUT_INTERVAL * PANEL_SCALE;
    x = xinit + PAN_1ST_ROW_INT;
    uint16_t but = 0;
    for (; but < 3; but++) {
        wButtonInit(&buttons[but], bdef[but].name, bdef[but].hint, bdef[but].keycode, bdef[but].midikey, x, y, rend);
        x += PAN_1ST_ROW_INT + PANEL_UNIT_SIZE * PANEL_SCALE;
    }
    y += PAN_BUT_INC;
    x = xinit;
    for (; but < 5; but++) {
        wButtonInit(&buttons[but], bdef[but].name, bdef[but].hint, bdef[but].keycode, bdef[but].midikey, x, y, rend);
        x += PAN_BUT_INC;
    }
    wEncInit(&encoder, "enc", "^ v", SDLK_UP, SDLK_DOWN, 0, x, y, rend);
    x += PAN_BUT_INC;
    wButtonInit(&buttons[but], bdef[but].name, bdef[but].hint, bdef[but].keycode, bdef[but].midikey, x, y, rend);
    but++;
    y += PAN_BUT_INC;
    x = xinit;
    for (; but < 10; but++) {
        wButtonInit(&buttons[but], bdef[but].name, bdef[but].hint, bdef[but].keycode, bdef[but].midikey, x, y, rend);
        x += PAN_BUT_INC;
    }
    y += PAN_BUT_INC;
    x = xinit;
    for (; but < 14; but++) {
        wButtonInit(&buttons[but], bdef[but].name, bdef[but].hint, bdef[but].keycode, bdef[but].midikey, x, y, rend);
        x += PAN_BUT_INC;
    }
}

void panelLoop(uint32_t ms)
{
    // static uint8_t line = 0;
    // MidiTsMessageT mt;
    // if (MIDI_RET_OK == midiRead(&mt)) {
    //     mgsDisplay(&display_multi_mgldisp);
    //     mgsCursorAbs(0, line * 8);
    //     line++;
    //     if (line == 8)
    //         line = 0;
    //     mgdHex32(mt.mes.full_word, mgColorHsv(0, 255, 128));
    //     // mgdChar('-', mgColorHsv(0, 255, 128));
    //     mgdHex32(mt.timestamp, mgColorHsv(0, 255, 128));

    //     // lock test
    //     if (mt.mes.cn == MIDI_CN_LOCALPANEL) {
    //         if ((MIDI_CIN_NOTEON == mt.mes.cin)
    //             && (mt.mes.byte2 == 0)) {
    //             //
    //             potLock(&pots[0].potdata, 1, 1);
    //         }
    //         if ((MIDI_CIN_NOTEON == mt.mes.cin)
    //             && (mt.mes.byte2 == 1)) {
    //             //
    //             potLock(&pots[1].potdata, 128 * 16, 1);
    //         }
    //         if ((MIDI_CIN_NOTEON == mt.mes.cin)
    //             && (mt.mes.byte2 == 2)) {
    //             //
    //             potLock(&pots[2].potdata, 128 * 64, 1);
    //         }
    //         if ((MIDI_CIN_NOTEON == mt.mes.cin)
    //             && (mt.mes.byte2 == 3)) {
    //             //
    //             potLock(&pots[3].potdata, 128 * 128 - 1, 1);
    //         }
    //     }
    // }
    // if (MIDI_RET_OK == midiSysexRead(&mt.mes)) {
    //     mgsDisplay(&display_multi_mgldisp);
    //     mgsCursorAbs(0, line * 8);
    //     line++;
    //     if (line == 8)
    //         line = 0;
    //     mgdHex32(mt.mes.full_word, mgColorHsv(0, 255, 128));
    //     mgdString("-syx-", mgColorHsv(0, 255, 128));
    // }
}
