// Copyright (C) 2012-2018 Hideaki Narita


#include <math.h>
#include <time.h>
#include "CpuGraph.h"


using namespace hnrt;


CpuGraph::CpuGraph()
{
    setPercentLabels();
}


CpuGraph::~CpuGraph()
{
}


void CpuGraph::drawPixmap(bool queuing)
{
    if (!_gc || !_pixmap)
    {
        return;
    }

    int x, y, cx, cy;

    //////////////////////////////////////////////////////////////////////
    //
    // determining sizes and positions
    //
    //////////////////////////////////////////////////////////////////////

    int cx7 = _textHeight; // line length of a legend

    _layout->set_text(_elementCount <= 10 ? "CPU0" : "CPU00");
    Pango::Rectangle r8 = _layout->get_pixel_logical_extents();
    int cx8 = r8.get_width();

    _layout->set_text("100%");
    Pango::Rectangle r9 = _layout->get_pixel_logical_extents();
    int cx9 = r9.get_width() + _textHeight;

    int cxm = _textHeight; // margin width between legend columns
    int cym = 2; // margin height between legends

    int rightRows = _elementCount;

    if (_elementCount > 0)
    {
        cx = cx7 + cx8 + cx9;
        cy = _textHeight;
        for (int columns = 1;; columns++)
        {
            int rows = (_elementCount + columns - 1) / columns;
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
            _gc->set_foreground(_colors[kColorElement0 + j]);
            int v = static_cast<int>(_value[0][j]);
            if (v < 0) v = 0; else if (v > 100) v = 100;
            int x1 = _xb4;
            int y1 = _yb4 - _cyb * v / 100;
            for (int i = 1; i < _pointCount; i++)
            {
                v = static_cast<int>(_value[i][j]);
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

    if (_elementCount > 0)
    {
        cx = cx7 + cx8 + cx9;
        x = _xb2 + cxm;
        y = _yb2 + 1;
        for (int i = 0; i < _elementCount; i++)
        {
            int v = static_cast<int>(_value[_pointCount - 1][i]);
            if (v < 0) v = 0; else if (v > 100) v = 100;
            _gc->set_foreground(_colors[kColorElement0 + i]);
            _gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_BUTT, Gdk::JOIN_MITER);
            _pixmap->draw_line(_gc, x, y + _textHeight / 2, x + cx7, y + _textHeight / 2);
            drawText(x + cx7, y, "CPU%d", i);
            drawTextRight(x + cx7 + cx8 + cx9, y, v < 90 ? _fg : _colors[kColorRed], "%d%%", v);
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
