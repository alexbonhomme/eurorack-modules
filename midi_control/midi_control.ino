#include <Control_Surface.h>

USBMIDI_Interface midi;

CCPotentiometer potentiometers[]{
    // potars
    {A0, MIDI_CC::General_Purpose_Controller_1},
    {A1, MIDI_CC::General_Purpose_Controller_2},
    {A2, MIDI_CC::General_Purpose_Controller_3},
};

CCPotentiometer fader[]{
    {A3, MIDI_CC::Effect_Control_1},
};

CCButton button{A9, MIDI_CC::Effect_Control_2};

void setup()
{
    Control_Surface.begin();
}

void loop()
{
    Control_Surface.loop();
}
