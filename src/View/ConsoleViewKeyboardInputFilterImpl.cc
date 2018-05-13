// Copyright (C) 2012-2018 Hideaki Narita


#include <gtkmm.h>
#include "ConsoleViewKeyboardInputFilterImpl.h"


using namespace hnrt;


RefPtr<ConsoleViewKeyboardInputFilterImpl> ConsoleViewKeyboardInputFilterImpl::create()
{
    return RefPtr<ConsoleViewKeyboardInputFilterImpl>(new ConsoleViewKeyboardInputFilterImpl);
}


ConsoleViewKeyboardInputFilterImpl::ConsoleViewKeyboardInputFilterImpl()
    : _sasAlternativeKeycode(KEYCODE_INSERT)
    , _us101conversionEnabled(false)
{
    setUnfullscreenKeycodes(KEYCODE_SHIFT_R, KEYCODE_CONTROL_R);
}


void ConsoleViewKeyboardInputFilterImpl::reset()
{
    _unfullscreenKeycodeFlags = 0;
}


bool ConsoleViewKeyboardInputFilterImpl::filter(unsigned char downFlag, unsigned int& keyval, unsigned int& state, unsigned int& keycode)
{
    if (_us101conversionEnabled)
    {
        keyval = us101convert(keyval, state, keycode);
    }

    if (_unfullscreenKeycodeCount == 2)
    {
        if (keycode == _unfullscreenKeycodes[0])
        {
            if (downFlag)
            {
                _unfullscreenKeycodeFlags |= (1 << 0);
            }
            else
            {
                _unfullscreenKeycodeFlags &= ~(1 << 0);
            }
        }
        else if (keycode == _unfullscreenKeycodes[1])
        {
            if (downFlag)
            {
                _unfullscreenKeycodeFlags |= (1 << 1);
            }
            else
            {
                _unfullscreenKeycodeFlags &= ~(1 << 1);
            }
        }
        else
        {
            goto check2;
        }
        if (_unfullscreenKeycodeFlags == ((1 << 2) - 1))
        {
            _unfullscreenKeycodeFlags = 0;
            _sigUnfullscreen.emit();
        }
    }
    else if (_unfullscreenKeycodeCount == 3)
    {
        if (keycode == _unfullscreenKeycodes[0])
        {
            if (downFlag)
            {
                _unfullscreenKeycodeFlags |= (1 << 0);
            }
            else
            {
                _unfullscreenKeycodeFlags &= ~(1 << 0);
            }
        }
        else if (keycode == _unfullscreenKeycodes[1])
        {
            if (downFlag)
            {
                _unfullscreenKeycodeFlags |= (1 << 1);
            }
            else
            {
                _unfullscreenKeycodeFlags &= ~(1 << 1);
            }
        }
        else if (keycode == _unfullscreenKeycodes[2])
        {
            if (downFlag)
            {
                _unfullscreenKeycodeFlags |= (1 << 2);
            }
            else
            {
                _unfullscreenKeycodeFlags &= ~(1 << 2);
            }
        }
        else
        {
            goto check2;
        }
        if (_unfullscreenKeycodeFlags == ((1 << 3) - 1))
        {
            _unfullscreenKeycodeFlags = 0;
            _sigUnfullscreen.emit();
        }
    }

check2:

    if ((state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) == (GDK_CONTROL_MASK | GDK_MOD1_MASK))
    {
        if (keycode == _sasAlternativeKeycode)
        {
            keycode = KEYCODE_DELETE;
        }
    }

    return false;
}


void ConsoleViewKeyboardInputFilterImpl::setUnfullscreenKeycodes(unsigned int v1, unsigned int v2)
{
    _unfullscreenKeycodeCount = 2;
    _unfullscreenKeycodes[0] = v1;
    _unfullscreenKeycodes[1] = v2;
    _unfullscreenKeycodes[2] = 0;
    _unfullscreenKeycodeFlags = 0;
}


void ConsoleViewKeyboardInputFilterImpl::setUnfullscreenKeycodes(unsigned int v1, unsigned int v2, unsigned int v3)
{
    _unfullscreenKeycodeCount = 3;
    _unfullscreenKeycodes[0] = v1;
    _unfullscreenKeycodes[1] = v2;
    _unfullscreenKeycodes[2] = v3;
    _unfullscreenKeycodeFlags = 0;
}


// This function converts keyval into the one corresponding to the US keyboard.
unsigned int ConsoleViewKeyboardInputFilterImpl::us101convert(unsigned int keyval, unsigned int state, unsigned int keycode)
{
    switch (keycode)
    {
    case 49: return !(state & GDK_SHIFT_MASK) ? '`' : '~';
    case 10: return !(state & GDK_SHIFT_MASK) ? '1' : '!';
    case 11: return !(state & GDK_SHIFT_MASK) ? '2' : '@';
    case 12: return !(state & GDK_SHIFT_MASK) ? '3' : '#';
    case 13: return !(state & GDK_SHIFT_MASK) ? '4' : '$';
    case 14: return !(state & GDK_SHIFT_MASK) ? '5' : '%';
    case 15: return !(state & GDK_SHIFT_MASK) ? '6' : '^';
    case 16: return !(state & GDK_SHIFT_MASK) ? '7' : '&';
    case 17: return !(state & GDK_SHIFT_MASK) ? '8' : '*';
    case 18: return !(state & GDK_SHIFT_MASK) ? '9' : '(';
    case 19: return !(state & GDK_SHIFT_MASK) ? '0' : ')';
    case 20: return !(state & GDK_SHIFT_MASK) ? '-' : '_';
    case 21: return !(state & GDK_SHIFT_MASK) ? '=' : '+';
    case 24: return !(state & GDK_SHIFT_MASK) ? 'q' : 'Q';
    case 25: return !(state & GDK_SHIFT_MASK) ? 'w' : 'W';
    case 26: return !(state & GDK_SHIFT_MASK) ? 'e' : 'E';
    case 27: return !(state & GDK_SHIFT_MASK) ? 'r' : 'R';
    case 28: return !(state & GDK_SHIFT_MASK) ? 't' : 'T';
    case 29: return !(state & GDK_SHIFT_MASK) ? 'y' : 'Y';
    case 30: return !(state & GDK_SHIFT_MASK) ? 'u' : 'U';
    case 31: return !(state & GDK_SHIFT_MASK) ? 'i' : 'I';
    case 32: return !(state & GDK_SHIFT_MASK) ? 'o' : 'O';
    case 33: return !(state & GDK_SHIFT_MASK) ? 'p' : 'P';
    case 34: return !(state & GDK_SHIFT_MASK) ? '[' : '{';
    case 35: return !(state & GDK_SHIFT_MASK) ? ']' : '}';
    case 51: return !(state & GDK_SHIFT_MASK) ? '\\' : '|';
    case 38: return !(state & GDK_SHIFT_MASK) ? 'a' : 'A';
    case 39: return !(state & GDK_SHIFT_MASK) ? 's' : 'S';
    case 40: return !(state & GDK_SHIFT_MASK) ? 'd' : 'D';
    case 41: return !(state & GDK_SHIFT_MASK) ? 'f' : 'F';
    case 42: return !(state & GDK_SHIFT_MASK) ? 'g' : 'G';
    case 43: return !(state & GDK_SHIFT_MASK) ? 'h' : 'H';
    case 44: return !(state & GDK_SHIFT_MASK) ? 'j' : 'J';
    case 45: return !(state & GDK_SHIFT_MASK) ? 'k' : 'K';
    case 46: return !(state & GDK_SHIFT_MASK) ? 'l' : 'L';
    case 47: return !(state & GDK_SHIFT_MASK) ? ';' : ':';
    case 48: return !(state & GDK_SHIFT_MASK) ? '\'' : '\"';
    case 52: return !(state & GDK_SHIFT_MASK) ? 'z' : 'Z';
    case 53: return !(state & GDK_SHIFT_MASK) ? 'x' : 'X';
    case 54: return !(state & GDK_SHIFT_MASK) ? 'c' : 'C';
    case 55: return !(state & GDK_SHIFT_MASK) ? 'v' : 'V';
    case 56: return !(state & GDK_SHIFT_MASK) ? 'b' : 'B';
    case 57: return !(state & GDK_SHIFT_MASK) ? 'n' : 'N';
    case 58: return !(state & GDK_SHIFT_MASK) ? 'm' : 'M';
    case 59: return !(state & GDK_SHIFT_MASK) ? ',' : '<';
    case 60: return !(state & GDK_SHIFT_MASK) ? '.' : '>';
    case 61: return !(state & GDK_SHIFT_MASK) ? '/' : '?';
    default: return keyval;
    }
}
