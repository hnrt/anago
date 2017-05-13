// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include <math.h>
#include <time.h>
#include "Base/StringBuffer.h"
#include "MemoryGraph.h"


using namespace hnrt;


MemoryGraph::MemoryGraph()
    : _totalMemory(gettext("Total memory"))
    , _currentlyUsed(gettext("Currently used"))
    , _availableMemory(gettext("Available memory"))
{
    setPercentLabels();

    _layout->set_text(_totalMemory);
    Pango::Rectangle r71 = _layout->get_pixel_logical_extents();
    int cx71 = r71.get_width();
    int cy71 = r71.get_height();
    _layout->set_text(_currentlyUsed);
    Pango::Rectangle r72 = _layout->get_pixel_logical_extents();
    int cx72 = r72.get_width();
    int cy72 = r72.get_height();
    _layout->set_text(_availableMemory);
    Pango::Rectangle r73 = _layout->get_pixel_logical_extents();
    int cx73 = r73.get_width();
    int cy73 = r73.get_height();

    _cx7 = cx71 > cx73 ? (cx71 > cx72 ? cx71 : cx72) : (cx72 > cx73 ? cx72 : cx73);
    _cy7 = cy71 > cy73 ? (cy71 > cy72 ? cy71 : cy72) : (cy72 > cy73 ? cy72 : cy73);

    int cxm = _cy7; // margin width between graph and current stats
    int cym = 2; // space between rows
   
    _x7 = _xb2 + cxm;
    _y71 = _yb2 + 1 + (_cy7 + cym) * 0;
    _y72 = _yb2 + 1 + (_cy7 + cym) * 1;
    _y73 = _yb2 + 1 + (_cy7 + cym) * 2;
}


MemoryGraph::~MemoryGraph()
{
}


void MemoryGraph::drawPixmap(bool queuing)
{
    if (!_gc || !_pixmap)
    {
        return;
    }

    int x, y, v;

    //////////////////////////////////////////////////////////////////////
    //
    // determining sizes and positions
    //
    //////////////////////////////////////////////////////////////////////

    StringBuffer text;
    text.format("%'lu MiB", _value[_pointCount - 1][0]);
    _layout->set_text(text.str());
    Pango::Rectangle r8 = _layout->get_pixel_logical_extents();
    int cx8 = r8.get_width();

    _layout->set_text("(100%)");
    Pango::Rectangle r9 = _layout->get_pixel_logical_extents();
    int cx9 = r9.get_width();

    int cxm = _cy7; // space between columns

    _x8 = _x7 + _cx7 + cxm + cx8;
    _x9 = _x8 + cxm;

    x = _x9 + cx9;

    if (_pixmapWidth < x)
    {
        _pixmapWidth = x;
        _pixmap = Gdk::Pixmap::create(get_window(), _pixmapWidth, _pixmapHeight);
        set_size_request(_pixmapWidth, _pixmapHeight);
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
        _gc->set_foreground(_colors[kColorGreen]);
        _gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_BUTT, Gdk::JOIN_MITER);
        v = _value[0][0] ? static_cast<int>(round(_value[0][1] * 100.0 / _value[0][0])) : 0;
        if (v < 0) v = 0; else if (v > 100) v = 100;
        int x1 = _xb4;
        int y1 = v == 100 ? (_yb1 + 1) : v == 0 ? (_yb4 - 1) : (_yb4 - _cyb * v / 100);
        for (int i = 1; i < _pointCount; i++)
        {
            v = _value[i][0] ? static_cast<int>(round(_value[i][1] * 100.0 / _value[i][0])) : 0;
            if (v < 0) v = 0; else if (v > 100) v = 100;
            int x2 = _xb3 - _cxb * ((kMaxPointCount - 1) - i) / (kMaxPointCount - 1);
            int y2 = v == 100 ? (_yb1 + 1) : v == 0 ? (_yb4 - 1) : (_yb4 - _cyb * v / 100);
            _pixmap->draw_line(_gc, x1, y1, x2, y2);
            x1 = x2;
            y1 = y2;
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
    // drawing the right column part (current stats)
    //
    //////////////////////////////////////////////////////////////////////

    y = _y71;
    drawText(_x7, y, _totalMemory);
    drawTextRight(_x8, y, "%'lu MiB", _value[_pointCount - 1][0]);

    y = _y72;
    drawText(_x7, y, _currentlyUsed);
    drawTextRight(_x8, y, "%'lu MiB", _value[_pointCount - 1][1]);
    v = _value[_pointCount - 1][0] ? static_cast<int>(round(_value[_pointCount - 1][1] * 100.0 / _value[_pointCount - 1][0])) : 0;
    if (v < 0) v = 0; else if (v > 100) v = 100;
    x = drawText(_x9, y, "(");
    x = drawText(x, y, v < 90 ? _fg : _colors[kColorRed], "%u%%", v);
    x = drawText(x, y, ")");

    y = _y73;
    drawText(_x7, y, _availableMemory);
    drawTextRight(_x8, y, "%'lu MiB", _value[_pointCount - 1][0] - _value[_pointCount - 1][1]);
    v = _value[_pointCount - 1][0] ? static_cast<int>(round((_value[_pointCount - 1][0] - _value[_pointCount - 1][1]) * 100.0 / _value[_pointCount - 1][0])) : 0;
    if (v < 0) v = 0; else if (v > 100) v = 100;
    drawText(_x9, y, "(%u%%)", v);

    if (queuing)
    {
        queue_draw();
    }
}
