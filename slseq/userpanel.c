#include "panel_conf.h"
#include "panel.h"
#include "mbwmidi.h"

// TODO: put all wids in one .h ?

void panelConstruct(SDL_Renderer* rend);
void panelLoop(uint32_t clock);

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
static WidgetMidi wmidi_io;

#define LCD_SCALE 3
#define LCD_DEF_WTD ((122 + 6) * LCD_SCALE)
#define LCD_DEF_HTH ((32 + 6) * LCD_SCALE)

extern const MglFont _5monotxt;
void panelConstruct(SDL_Renderer* rend)
{
    wFrameCounterInit(&framecounter, 20, 0, rend);
    wmidi_io.name_in[0] = wmidi_io.name_out[0] = 'M';
    wmidi_io.name_in[1] = wmidi_io.name_out[1] = 'P';
    wmidi_io.name_in[2] = wmidi_io.name_out[2] = 'K';
    wMidiInit(&wmidi_io, 70, 0, rend);

    uint16_t xinit = PAN_BORDER;

    uint16_t y = PAN_BORDER;
    uint16_t x = xinit + (PAN_ELEMENTS_WIDTH - LCD_DEF_WTD) / 2;

    wDisplayMonoInit(&display_mono, &display_mono_mgldisp, display_mono_framebuffer, x, y, LCD_SCALE, rend);
    mgsDisplay(&display_mono_mgldisp);
    mgsFont(&_5monotxt);
    mgdString("Hello!", COLOR_ON);

    y += LCD_DEF_HTH + PANEL_UNIT_GAP * PANEL_SCALE;
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

#define DISPLAY_LINES 5
#define DISPLAY_FONTHGTH 6

void panelLoop(uint32_t clock)
{
    static uint8_t line = 0;
    MidiTsMessageT mt;
    // if
    while (MIDI_RET_OK == midiRead(&mt)) {
        mgsDisplay(&display_mono_mgldisp);
        mgsCursorAbs(0, line * DISPLAY_FONTHGTH);
        line++;
        if (line == DISPLAY_LINES)
            line = 0;
        mgdHex32(mt.mes.full_word, COLOR_ON);
        // mgdChar('-', mgColorHsv(0, 255, 128));
        mgdHex32(mt.timestamp, COLOR_ON);
    }
    while (MIDI_RET_OK == midiSysexRead(&mt.mes)) {
        mgsDisplay(&display_mono_mgldisp);
        mgsCursorAbs(0, line * DISPLAY_FONTHGTH);
        line++;
        if (line == DISPLAY_LINES)
            line = 0;
        mgdHex32(mt.mes.full_word, COLOR_ON);
        mgdString("-syx-", COLOR_ON);
    }
}