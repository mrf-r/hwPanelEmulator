#include "panel_conf.h"
#include "panel.h"
#include "mbwmidi.h"
#include "synth.h"

void panelConstruct(SDL_Renderer* rend);
void panelLoop(uint32_t clock);
void synthAudioCallback(int16_t* const buffer_in, int16_t* const buffer_out, const uint16_t length);

typedef struct {
    const char* name;
    const char* hint;
    const SDL_KeyCode keycode;
    const uint8_t midikey;
} ButtonDef;

static char* pdef[] = {
    "Pit",
    "Oct",
    "Mix",
    "Pha",
    "P2",
    "Snc"
};

#define POT_FIRST_CC_NUM 0x10

static ButtonDef bdef[] = {

    { .name = "cal", .hint = "c", .keycode = SDLK_c, .midikey = 0 },
    { .name = "!", .hint = "__", .keycode = SDLK_SPACE, .midikey = 36 }
};

#define POTS_COUNT (sizeof(pdef) / sizeof(pdef[0]))
#define BUTTONS_COUNT (sizeof(bdef) / sizeof(ButtonDef))

static WidgetPot pots[POTS_COUNT];
static WidgetButton buttons[BUTTONS_COUNT];
WID_DISPLAY_MONO_DEFINE(display_mono, DISP_XSIZE, DISP_YSIZE) // static monochrome display
WID_AUDIO_CALLBACK_DEFINE(paCallback, synthAudioCallback)
static WidgetFrameCounter framecounter;
static WidgetMidi wmidi_io;
static WidgetAudio waudio;

#define LCD_SCALE 3
#define LCD_DEF_WTD ((DISP_XSIZE + 6) * LCD_SCALE)
#define LCD_DEF_HTH ((DISP_YSIZE + 6) * LCD_SCALE)

extern const MglFont _5monotxt;
void panelConstruct(SDL_Renderer* rend)
{
    // first we will init backend
    synthInit();

    // and then frontend
    wFrameCounterInit(&framecounter, 20, 0, rend);
    wMidiInit(&wmidi_io, 70, 0, rend, "MPK", "MPK", 31250, 115200);
    wAudioInit(&waudio, 120, 0, rend, 0, 0, 48000, 32, paCallback);

    uint16_t xinit = PAN_BORDER;
    uint16_t y = PAN_BORDER;
    uint16_t x = xinit + (PAN_ELEMENTS_WIDTH - LCD_DEF_WTD) / 2;

    wDisplayMonoInit(&display_mono, &display_mono_mgldisp, display_mono_framebuffer, x, y, LCD_SCALE, rend);
    mgsDisplay(&display_mono_mgldisp);
    mgsFont(&_5monotxt);
    mgdString("Hello!", COLOR_ON);

    y += LCD_DEF_HTH + PANEL_UNIT_GAP * PANEL_SCALE;
    x = xinit;
    uint16_t pot = 0;

    for (; pot < 4; pot++) {
        wPotInit(&pots[pot], pdef[pot], pot + POT_FIRST_CC_NUM, x, y, rend);
        potThresholdSet(&pots[pot].potdata, 2);
        potLockJump(&pots[pot].potdata);
        x += PAN_BUT_INC;
    }
    y += PAN_BUT_INC;
    x = xinit;
    for (; pot < POTS_COUNT; pot++) {
        wPotInit(&pots[pot], pdef[pot], pot + POT_FIRST_CC_NUM, x, y, rend);
        potThresholdSet(&pots[pot].potdata, 2);
        potLockJump(&pots[pot].potdata);
        x += PAN_BUT_INC;
    }
    uint16_t but = 0;
    for (; but < BUTTONS_COUNT; but++) {
        wButtonInit(&buttons[but], bdef[but].name, bdef[but].hint, bdef[but].keycode, bdef[but].midikey, x, y, rend);
        x += PAN_BUT_INC;
    }

    GadgetEeprom eeprom = {
        .name = "EEPROM_test",
        .filepath = "EEPROM/eeprom.bin",
        .size = 64
    };
    // wEepromInit(&eeprom);
}

static inline void panelHandleMidi(MidiMessageT m)
{
    if (MIDI_CN_LOCALPANEL != m.cn) {
        // update local panel from external midi controller
        if (MIDI_CIN_CONTROLCHANGE == m.cin) {
            if ((POT_FIRST_CC_NUM >= m.byte2)
                && (m.byte2 < POT_FIRST_CC_NUM + POTS_COUNT)) {
                uint16_t old_val = pots[m.byte2 - POT_FIRST_CC_NUM].potdata.locked;
                uint16_t new_val = parameterReceiveMsb(old_val, m.byte3);
                potLockFetch(&pots[m.byte2 - POT_FIRST_CC_NUM].potdata, new_val);
            }
            if ((POT_FIRST_CC_NUM + 0x20 >= m.byte2)
                && (m.byte2 < POT_FIRST_CC_NUM + 0x20 + POTS_COUNT)) {
                uint16_t old_val = pots[m.byte2 - POT_FIRST_CC_NUM].potdata.locked;
                uint16_t new_val = parameterReceiveLsb(old_val, m.byte3);
                potLockFetch(&pots[m.byte2 - POT_FIRST_CC_NUM - 0x20].potdata, new_val);
            }
        } else if (MIDI_CIN_NOTEON == m.cin) {
            if (m.byte2 == 0) {
                widgetLed(&buttons[0].v, 0x00FF00FF);
            } else {
                widgetLed(&buttons[1].v, 0x00FF00FF);
            }
        } else if (MIDI_CIN_NOTEOFF == m.cin) {
            if (m.byte2 == 0) {
                widgetLed(&buttons[0].v, 0);
            } else {
                widgetLed(&buttons[1].v, 0);
            }
        }
    } else {
        // add visual feedback for local buttons
        if (MIDI_CIN_NOTEON == m.cin) {
            if (m.byte2 == 0) {
                widgetLed(&buttons[0].v, 0x0000FF00);
            } else {
                widgetLed(&buttons[1].v, 0x0000FF00);
            }
        } else if (MIDI_CIN_NOTEOFF == m.cin) {
            if (m.byte2 == 0) {
                widgetLed(&buttons[0].v, 0);
            } else {
                widgetLed(&buttons[1].v, 0);
            }
        }
    }
}

void panelLoop(uint32_t clock)
{
    MidiTsMessageT mt;
    while (MIDI_RET_OK == midiRead(&mt)) {
        synthHandleMidi(mt.mes);
        panelHandleMidi(mt.mes);
    }
    // mgsDisplay(&display_mono_mgldisp);
    scopeXYframe(&scope, 0, 0, display_mono_mgldisp.size_x, display_mono_mgldisp.size_y, COLOR_ON, COLOR_OFF);
}
