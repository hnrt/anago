// Copyright (C) 2012-2017 Hideaki Narita


#include <time.h>
#include "Base/StringBuffer.h"
#include "ResourceHistoryGraph.h"


using namespace hnrt;


ResourceHistoryGraph::ResourceHistoryGraph()
    : _pixmapWidth(kWidth)
    , _pixmapHeight(kHeight)
    , _pointCount(0)
    , _elementCount(0)
    , _elementMask(0)
    , _textHeight(0)
    , _cx(0)
    , _cy(0)
{
    set_size_request(_pixmapWidth, _pixmapHeight);

    _colors[kColorWhite] = Gdk::Color("white");
    _colors[kColorBlack] = Gdk::Color("black");
    _colors[kColorGray] = Gdk::Color("gray");
    _colors[kColorRed] = Gdk::Color("red");
    _colors[kColorGreen] = Gdk::Color("green");
    _colors[kColorBlue] = Gdk::Color("blue");
    _colors[kColorElement0] = Gdk::Color("red");
    _colors[kColorElement1] = Gdk::Color("green");
    _colors[kColorElement2] = Gdk::Color("blue");
    _colors[kColorElement3] = Gdk::Color("yellow3");
    _colors[kColorElement4] = Gdk::Color("cyan");
    _colors[kColorElement5] = Gdk::Color("magenta");
    _colors[kColorElement6] = Gdk::Color("orange");
    _colors[kColorElement7] = Gdk::Color("dark red");
    _colors[kColorElement8] = Gdk::Color("dark green");
    _colors[kColorElement9] = Gdk::Color("dark blue");
    _colors[kColorElement10] = Gdk::Color("gold");
    _colors[kColorElement11] = Gdk::Color("dark cyan");
    _colors[kColorElement12] = Gdk::Color("dark magenta");
    _colors[kColorElement13] = Gdk::Color("light pink");
    _colors[kColorElement14] = Gdk::Color("light green");
    _colors[kColorElement15] = Gdk::Color("light blue");
    _colors[kColorElement16] = Gdk::Color("brown");
    _colors[kColorElement17] = Gdk::Color("pale green");
    _colors[kColorElement18] = Gdk::Color("navy");
    _colors[kColorElement19] = Gdk::Color("khaki");
    _colors[kColorElement20] = Gdk::Color("red3");
    _colors[kColorElement21] = Gdk::Color("green3");
    _colors[kColorElement22] = Gdk::Color("blue3");
    _colors[kColorElement23] = Gdk::Color("yellow4");
    _colors[kColorElement24] = Gdk::Color("light cyan");
    _colors[kColorElement25] = Gdk::Color("magenta3");
    _colors[kColorElement26] = Gdk::Color("dark orange");
    _colors[kColorElement27] = Gdk::Color("red4");
    _colors[kColorElement28] = Gdk::Color("green4");
    _colors[kColorElement29] = Gdk::Color("sky blue");
    _colors[kColorElement30] = Gdk::Color("gold3");
    _colors[kColorElement31] = Gdk::Color("cyan3");

    Glib::RefPtr<Gdk::Colormap> colmap = get_default_colormap();
    for( int i = 0; i < kColorCount; ++i )
    {
        colmap->alloc_color(_colors[i]);
    }

    _layout = create_pango_layout("");
    _layout->set_font_description(Pango::FontDescription("sans condensed 8"));
    _layout->set_text("ABC123");
    Pango::Rectangle r0 = _layout->get_pixel_logical_extents();
    _textHeight = r0.get_height();

    _cxb = kGraphWidth;
    _cyb = kHeight - (1 + _textHeight);
    _xb1 = kLeftColumnWidth;
    _yb1 = 0;
    _xb2 = _xb1 + _cxb;
    _yb2 = _yb1;
    _xb3 = _xb2;
    _yb3 = _yb2 + _cyb;
    _xb4 = _xb1;
    _yb4 = _yb3;
}


ResourceHistoryGraph::~ResourceHistoryGraph()
{
}


void ResourceHistoryGraph::on_realize()
{
    Gtk::DrawingArea::on_realize();

    Glib::RefPtr<Gdk::Window> window = get_window();
    _pixmap = Gdk::Pixmap::create(window, _pixmapWidth, _pixmapHeight);
    _gc = Gdk::GC::create(_pixmap);
    _bg = get_style()->get_bg(Gtk::STATE_NORMAL);
    _fg = get_style()->get_fg(Gtk::STATE_NORMAL);
    drawPixmap(false);
}


bool ResourceHistoryGraph::on_expose_event(GdkEventExpose* event)
{
    Glib::RefPtr<Gdk::Window> window = get_window();

    window->draw_drawable(get_style()->get_fg_gc(get_state()),
                          _pixmap,
                          event->area.x, event->area.y, event->area.x, event->area.y,
                          event->area.width, event->area.height);

    return true;
}


void ResourceHistoryGraph::setPercentLabels()
{
    _yLabel[0] = "0%%";
    _yLabel[1] = "25%%";
    _yLabel[2] = "50%%";
    _yLabel[3] = "75%%";
    _yLabel[4] = "100%%";
}


void ResourceHistoryGraph::setByteLabels(unsigned long maxValue)
{
    const unsigned long kGiB = 1024UL * 1024UL * 1024UL;
    const unsigned long kMiB = 1024UL * 1024UL;
    const unsigned long kKiB = 1024UL;
    double unit = 1.0;
    const char* format;

    if (maxValue >= kGiB)
    {
        unit = kGiB;
        format = "%g GiB";
    }
    else if (maxValue >= kMiB)
    {
        unit = kMiB;
        format = "%g MiB";
    }
    else if (maxValue >= kKiB)
    {
        unit = kKiB;
        format = "%g KiB";
    }
    else
    {
        unit = 1.0;
        format = "%g";
    }

    _yLabel[0] = StringBuffer().format(format, 0.0);
    _yLabel[1] = StringBuffer().format(format, maxValue * 0.25 / unit);
    _yLabel[2] = StringBuffer().format(format, maxValue * 0.50 / unit);
    _yLabel[3] = StringBuffer().format(format, maxValue * 0.75 / unit);
    _yLabel[4] = StringBuffer().format(format, maxValue / unit);
}


void ResourceHistoryGraph::drawBackground()
{
    _gc->set_rgb_fg_color(_bg);
    _gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_BUTT, Gdk::JOIN_MITER);
    _pixmap->draw_rectangle(_gc, true, 0, 0, _pixmapWidth, _pixmapHeight);
}


void ResourceHistoryGraph::drawLeftColumn()
{
    drawTextRight(_xb1 - 1, _yb1 + 1, _yLabel[4].c_str());
    drawTextRightBottom(_xb4 - 1, _yb4 - 1, _yLabel[0].c_str());

    switch (_cyb / _textHeight)
    {
    default:
        drawTextRightCenter(_xb1 - 1, _yb1 + _cyb * 1 / 4, _yLabel[3].c_str());
        drawTextRightCenter(_xb1 - 1, _yb1 + _cyb * 3 / 4, _yLabel[1].c_str());
        //FALLTHROUGH
    case 4:
    case 3:
        drawTextRightCenter(_xb1 - 1, _yb1 + _cyb * 2 / 4, _yLabel[2].c_str());
        break;
    case 2:
    case 1:
    case 0:
        break;
    }
}


void ResourceHistoryGraph::drawTimeRow()
{
    int x0 = _pixmapWidth;
    int count = 0;
    _cx = 0;
    for (int i = _pointCount - 1; i >= 0 ; i--)
    {
        struct tm buf;
        localtime_r(&reinterpret_cast<time_t&>(_time[i]), &buf);
        if (count && buf.tm_sec)
        {
            continue;
        }
        int x = _xb3 - _cxb * ((kMaxPointCount - 1) - i) / (kMaxPointCount - 1);
        int y = _yb3 + 1;
        if (x + _cx / 2 <= x0)
        {
            drawTextCenter(x, y, "%d:%02d:%02d", buf.tm_hour, buf.tm_min, buf.tm_sec);
            x0 = x - (_cy + _cx / 2);
            count++;
        }
    }
}


void ResourceHistoryGraph::drawGrid()
{
    _gc->set_rgb_fg_color(_colors[kColorWhite]);
    _pixmap->draw_rectangle(_gc, true, _xb1, _yb1, _cxb, _cyb);

    _gc->set_foreground(_colors[kColorGray]);
    _gc->set_line_attributes(1, Gdk::LINE_ON_OFF_DASH, Gdk::CAP_BUTT, Gdk::JOIN_MITER);
    //_gc->set_line_attributes(1, Gdk::LINE_DOUBLE_DASH, Gdk::CAP_BUTT, Gdk::JOIN_MITER);

    switch (_cyb / _textHeight)
    {
    default:
        _pixmap->draw_line(_gc, _xb1, _yb1 + _cyb * 1 / 4, _xb2, _yb2 + _cyb * 1 / 4);
        _pixmap->draw_line(_gc, _xb1, _yb1 + _cyb * 3 / 4, _xb2, _yb2 + _cyb * 3 / 4);
        //FALLTHROUGH
    case 4:
    case 3:
        _pixmap->draw_line(_gc, _xb1, _yb1 + _cyb * 2 / 4, _xb2, _yb2 + _cyb * 2 / 4);
        break;
    case 2:
    case 1:
    case 0:
        break;
    }

    int n = _pointCount < kMaxPointCount - 1 ? _pointCount : kMaxPointCount - 1;
    for (int i = 1; i < n; i++)
    {
        struct tm buf;
        localtime_r(&reinterpret_cast<time_t&>(_time[i]), &buf);
        if (buf.tm_sec)
        {
            continue;
        }
        int x = _xb3 - _cxb * ((kMaxPointCount - 1) - i) / (kMaxPointCount - 1);
        _pixmap->draw_line(_gc, x, _yb1, x, _yb4);
    }
}


void ResourceHistoryGraph::drawFrame()
{
    _gc->set_rgb_fg_color(_colors[kColorBlack]);
    _gc->set_line_attributes(1, Gdk::LINE_SOLID, Gdk::CAP_BUTT, Gdk::JOIN_MITER);
    _pixmap->draw_rectangle(_gc, false, _xb1, _yb1, _cxb, _cyb);
}


int ResourceHistoryGraph::drawTextV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList)
{
    StringBuffer text;
    text.formatV(format, argList);
    _layout->set_text(text.str());
    _layout->get_pixel_size(_cx, _cy);
    _pixmap->draw_layout(_gc, x, y, _layout, fg, bg);
    return x + _cx;
}


int ResourceHistoryGraph::drawText(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextV(x, y, fg, bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawText(int x, int y, const Gdk::Color& fg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextV(x, y, fg, _bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawText(int x, int y, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextV(x, y, _fg, _bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextCenterV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList)
{
    StringBuffer text;
    text.formatV(format, argList);
    _layout->set_text(text.str());
    _layout->get_pixel_size(_cx, _cy);
    _pixmap->draw_layout(_gc, x - _cx / 2, y, _layout, fg, bg);
    return x;
}


int ResourceHistoryGraph::drawTextCenter(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextCenterV(x, y, fg, bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextCenter(int x, int y, const Gdk::Color& fg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextCenterV(x, y, fg, _bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextCenter(int x, int y, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextCenterV(x, y, _fg, _bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextRightV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList)
{
    StringBuffer text;
    text.formatV(format, argList);
    _layout->set_text(text.str());
    _layout->get_pixel_size(_cx, _cy);
    _pixmap->draw_layout(_gc, x - _cx, y, _layout, fg, bg);
    return x;
}


int ResourceHistoryGraph::drawTextRight(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextRightV(x, y, fg, bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextRight(int x, int y, const Gdk::Color& fg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextRightV(x, y, fg, _bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextRight(int x, int y, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextRightV(x, y, _fg, _bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextRightCenterV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList)
{
    StringBuffer text;
    text.formatV(format, argList);
    _layout->set_text(text.str());
    _layout->get_pixel_size(_cx, _cy);
    _pixmap->draw_layout(_gc, x - _cx, y - _cy / 2, _layout, fg, bg);
    return x;
}


int ResourceHistoryGraph::drawTextRightCenter(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextRightCenterV(x, y, fg, bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextRightCenter(int x, int y, const Gdk::Color& fg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextRightCenterV(x, y, fg, _bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextRightCenter(int x, int y, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextRightCenterV(x, y, _fg, _bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextRightBottomV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList)
{
    StringBuffer text;
    text.formatV(format, argList);
    _layout->set_text(text.str());
    _layout->get_pixel_size(_cx, _cy);
    _pixmap->draw_layout(_gc, x - _cx, y - _cy, _layout, fg, bg);
    return x;
}


int ResourceHistoryGraph::drawTextRightBottom(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextRightBottomV(x, y, fg, bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextRightBottom(int x, int y, const Gdk::Color& fg, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextRightBottomV(x, y, fg, _bg, format, argList);
    va_end(argList);
    return x;
}


int ResourceHistoryGraph::drawTextRightBottom(int x, int y, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    x = drawTextRightBottomV(x, y, _fg, _bg, format, argList);
    va_end(argList);
    return x;
}


void ResourceHistoryGraph::addTime(unsigned long t)
{
    if (_pointCount)
    {
        // assume 5-second interval of RRD
        unsigned long steps = (t - (_time[_pointCount - 1] + 5)) / 5;
        if (steps > 0)
        {
            if (static_cast<unsigned long>(kMaxPointCount) < _pointCount + steps)
            {
                if (steps < static_cast<unsigned long>(kMaxPointCount))
                {
                    int newPointCount = kMaxPointCount - static_cast<int>(steps);
                    int delta = _pointCount - newPointCount;
                    memmove(&_time[0], &_time[delta], newPointCount * sizeof(_time[delta]));
                    memmove(_value[0], _value[delta], newPointCount * sizeof(_value[delta]));
                    _pointCount = newPointCount;
                }
                else
                {
                    steps = kMaxPointCount;
                    _pointCount = 0;
                }
            }
            _time[_pointCount] = t - 5 * steps;
            for (int i = 1; i < static_cast<int>(steps); i++)
            {
                _time[_pointCount + i] = _time[_pointCount + i - 1] + 5;
            }
            memset(_value[_pointCount], 0, steps * sizeof(_value[_pointCount]));
            _pointCount += static_cast<int>(steps);
        }
    }
    _time[_pointCount] = t;
    memset(_value[_pointCount], 0, sizeof(_value[_pointCount]));
    _elementCount = 0;
    _elementMask = 0;
}


void ResourceHistoryGraph::addValue(int i, unsigned long v)
{
    if (0 <= i && i < kMaxElementCount)
    {
        _value[_pointCount][i] = v;
        if (_elementCount < i + 1)
        {
            _elementCount = i + 1;
        }
        _elementMask |= (1UL << i);
    }
}


void ResourceHistoryGraph::update()
{
    if (_pointCount < kMaxPointCount)
    {
        _pointCount++;
    }
    else
    {
        memmove(&_time[0], &_time[1], _pointCount * sizeof(_time[1]));
        memmove(_value[0], _value[1], _pointCount * sizeof(_value[1]));
    }
    drawPixmap();
}


void ResourceHistoryGraph::clear()
{
    _pointCount = 0;
}
