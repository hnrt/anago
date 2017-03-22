// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include "Base/StringBuffer.h"
#include "InputOutputGraph.h"


using namespace hnrt;


InputOutputGraph::InputOutputGraph()
    : _bytesPerSecond(gettext("%'lu bytes/sec"))
    , _maxValue(1024)
    , _maxValueAt(INT_MAX)
{
    setByteLabels(_maxValue);
}


InputOutputGraph::~InputOutputGraph()
{
}


void InputOutputGraph::addValue(int index, unsigned long rxValue, unsigned long txValue, const char* rxLabel, const char* txLabel)
{
    if (index < 0 || kMaxElementCount < index * 2 + 2)
    {
        return;
    }

    ResourceHistoryGraph::addValue(index * 2 + 0, rxValue);
    ResourceHistoryGraph::addValue(index * 2 + 1, txValue);

    _labels[index * 2 + 0] = rxLabel;
    _labels[index * 2 + 1] = txLabel;

    unsigned long value = rxValue > txValue ? rxValue : txValue;

    if (_maxValue < value)
    {
        _maxValue = static_cast<unsigned long>(pow(2, ceil(log2(value))));
        _maxValueAt = _time[_pointCount];
        //printf("InputOutputGraph::addValue(%d,%lu,%lu): max=%lu\n", index, rxValue, txValue, _maxValue);
        setByteLabels(_maxValue);
    }
}


void InputOutputGraph::update()
{
    if (_maxValueAt + 5 * kMaxPointCount < _time[_pointCount])
    {
        _maxValue = 1024;
        _maxValueAt = INT_MAX;
        for (int i = 1; i <= _pointCount; i++)
        {
            for (int j = 0; j < _elementCount; j++)
            {
                if (_maxValue < _value[i][j])
                {
                    _maxValue = static_cast<unsigned long>(pow(2, ceil(log2(_value[i][j]))));
                    _maxValueAt = _time[i];
                }
            }
        }
        //printf("InputOutputGraph::update: max=%lu\n", _maxValue);
        setByteLabels(_maxValue);
    }

    ResourceHistoryGraph::update();
}


void InputOutputGraph::drawPixmap(bool queuing)
{
    if (!_gc || !_pixmap)
    {
        return;
    }

    int x, y, cx, cy;
    StringBuffer text;

    //////////////////////////////////////////////////////////////////////
    //
    // determining sizes and positions
    //
    //////////////////////////////////////////////////////////////////////

    int count = 0;

    int cx7 = _textHeight; // line length of a legend

    int cx8 = 0;

    for (int i = 0; i < _elementCount; i++)
    {
        if (!(_elementMask & (1 << i)))
        {
            continue;
        }
        count++;
        text.format(_labels[i].c_str(), i);
        _layout->set_text(text.str());
        Pango::Rectangle r81 = _layout->get_pixel_logical_extents();
        int cx81 = r81.get_width();
        if (cx8 < cx81)
        {
            cx8 = cx81;
        }
    }

    text.format(_bytesPerSecond, _maxValue);
    _layout->set_text(text.str());
    Pango::Rectangle r9 = _layout->get_pixel_logical_extents();
    int cx9 = r9.get_width() + _textHeight;

    int cxm = _textHeight; // margin width between legend columns
    int cym = 2; // margin height between legends

    int rightRows = count;

    if (count > 0)
    {
        cx = cx7 + cx8 + cx9;
        cy = _textHeight;
        for (int columns = 1;; columns++)
        {
            int rows = (count + columns - 1) / columns;
            x = _xb2 + cxm + cx * columns + cxm * (columns - 1);
            y = _yb2 + 1 + cy * rows + cym * (rows - 1) + 1;
            if (y <= _cyb)
            {
                rightRows = rows;
                if (_pixmapWidth < x)
                {
                    _pixmapWidth = x;
                    _pixmap = Gdk::Pixmap::create(get_window(), _pixmapWidth, _pixmapHeight);
                    set_size_request(_pixmapWidth, _pixmapHeight);
                }
                break;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    //
    // drawing
    //
    //////////////////////////////////////////////////////////////////////

    drawBackground();

    //////////////////////////////////////////////////////////////////////
    //
    // drawing the left column part (percent labels)
    //
    //////////////////////////////////////////////////////////////////////

    drawLeftColumn();

    //////////////////////////////////////////////////////////////////////
    //
    // drawing the middle column upper part (graph)
    //
    //////////////////////////////////////////////////////////////////////

    drawGrid();

    if (_pointCount >= 2)
    {
        _gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_BUTT, Gdk::JOIN_MITER);
        for (int j = _elementCount - 1; j >= 0; j--)
        {
            if (!(_elementMask & (1 << j)))
            {
                continue;
            }
            _gc->set_foreground(_colors[kColorElement0 + j]);
            int v = static_cast<int>(round(_value[0][j] * 100.0 / _maxValue));
            if (v < 0) v = 0; else if (v > 100) v = 100;
            int x1 = _xb4;
            int y1 = _yb4 - _cyb * v / 100;
            for (int i = 1; i < _pointCount; i++)
            {
                v = static_cast<int>(round(_value[i][j] * 100.0 / _maxValue));
                if (v < 0) v = 0; else if (v > 100) v = 100;
                int x2 = _xb3 - _cxb * ((kMaxPointCount - 1) - i) / (kMaxPointCount - 1);
                int y2 = v == 100 ? (_yb1 + 1) : v == 0 ? (_yb4 - 1) : (_yb4 - _cyb * v / 100);
                _pixmap->draw_line(_gc, x1, y1, x2, y2);
                x1 = x2;
                y1 = y2;
            }
        }
    }

    drawFrame();

    //////////////////////////////////////////////////////////////////////
    //
    // drawing the middle column lower part (time labels)
    //
    //////////////////////////////////////////////////////////////////////

    drawTimeRow();

    //////////////////////////////////////////////////////////////////////
    //
    // drawing the right column part (legends)
    //
    //////////////////////////////////////////////////////////////////////

    if (count > 0)
    {
        cx = cx7 + cx8 + cx9;
        x = _xb2 + cxm;
        y = _yb2 + 1;
        for (int i = 0; i < _elementCount; i++)
        {
            if (!(_elementMask & (1 << i)))
            {
                continue;
            }
            _gc->set_foreground(_colors[kColorElement0 + i]);
            _gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_BUTT, Gdk::JOIN_MITER);
            _pixmap->draw_line(_gc, x, y + _textHeight / 2, x + cx7, y + _textHeight / 2);
            drawText(x + cx7, y, _labels[i].c_str(), i >> 1);
            drawTextRight(x + cx7 + cx8 + cx9, y, _bytesPerSecond, _value[_pointCount - 1][i]);
            if (((i + 1) % rightRows))
            {
                y += _textHeight + cym;
            }
            else
            {
                x += cx + cxm;
                y = _yb2 + 1;
            }
        }
    }

    if (queuing)
    {
        queue_draw();
    }
}
