// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_CONSOLEVIEWKEYBOARDINPUTFILTERIMPL_H
#define HNRT_CONSOLEVIEWKEYBOARDINPUTFILTERIMPL_H


#include <sigc++/sigc++.h>
#include "Base/RefPtr.h"
#include "ConsoleViewKeyboardInputFilter.h"


namespace hnrt
{
    class ConsoleViewKeyboardInputFilterImpl
        : public ConsoleViewKeyboardInputFilter
    {
    public:

        static RefPtr<ConsoleViewKeyboardInputFilterImpl> create();

        virtual void reset();
        virtual bool filter(unsigned char downFlag, unsigned int& keyval, unsigned int& state, unsigned int& keycode);
        unsigned int getSasAlternativeKeycode() const { return _sasAlternativeKeycode; }
        void setSasAlternativeKeycode(unsigned int value) { _sasAlternativeKeycode = value; }
        unsigned int getUnfullscreenKeycodeCount() const { return _unfullscreenKeycodeCount; }
        unsigned int getUnfullscreenKeycode(unsigned int index) const { return index < _unfullscreenKeycodeCount ? _unfullscreenKeycodes[index] : 0; }
        void setUnfullscreenKeycodes(unsigned int v1, unsigned int v2);
        void setUnfullscreenKeycodes(unsigned int v1, unsigned int v2, unsigned int v3);
        sigc::signal<void> signalUnfullscreen() { return _sigUnfullscreen; }
        void enableConvertUS101(bool value) { _us101conversionEnabled = value; }

    private:

        ConsoleViewKeyboardInputFilterImpl();
        ConsoleViewKeyboardInputFilterImpl(const ConsoleViewKeyboardInputFilterImpl&);
        void operator =(const ConsoleViewKeyboardInputFilterImpl&);
        unsigned int us101convert(unsigned int keyval, unsigned int state, unsigned int keycode);

        unsigned int _sasAlternativeKeycode;
        unsigned int _unfullscreenKeycodeCount;
        unsigned int _unfullscreenKeycodes[3];
        unsigned int _unfullscreenKeycodeFlags;
        sigc::signal<void> _sigUnfullscreen;
        bool _us101conversionEnabled;
    };
}


#define KEYCODE_SHIFT_R    62
#define KEYCODE_CONTROL_R 105
#define KEYCODE_INSERT    118
#define KEYCODE_DELETE    119


#endif //!HNRT_CONSOLEVIEWKEYBOARDINPUTFILTERIMPL_H
