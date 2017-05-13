// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_RESOURCEHISTORYGRAPH_H
#define HNRT_RESOURCEHISTORYGRAPH_H


#include <stdarg.h>
#include <gtkmm.h>


namespace hnrt
{
    class ResourceHistoryGraph
        : public Gtk::DrawingArea
    {
    public:

        enum Constants
        {
            kLeftColumnWidth = 50,
            kGraphWidth = 300,
            kRightColumnWidth = 150,
            kWidth = kLeftColumnWidth + kGraphWidth + kRightColumnWidth,
            kHeight = 200,
            kMaxPointCount = 31,
            kMaxElementCount = 32,
        };

        ResourceHistoryGraph();
        virtual ~ResourceHistoryGraph();
        virtual void addTime(unsigned long);
        virtual void addValue(int, unsigned long);
        virtual void update();
        virtual void clear();

    protected:

        enum ColorName
        {
            kColorWhite,
            kColorBlack,
            kColorGray,
            kColorRed,
            kColorGreen,
            kColorBlue,
            kColorElement0,
            kColorElement1,
            kColorElement2,
            kColorElement3,
            kColorElement4,
            kColorElement5,
            kColorElement6,
            kColorElement7,
            kColorElement8,
            kColorElement9,
            kColorElement10,
            kColorElement11,
            kColorElement12,
            kColorElement13,
            kColorElement14,
            kColorElement15,
            kColorElement16,
            kColorElement17,
            kColorElement18,
            kColorElement19,
            kColorElement20,
            kColorElement21,
            kColorElement22,
            kColorElement23,
            kColorElement24,
            kColorElement25,
            kColorElement26,
            kColorElement27,
            kColorElement28,
            kColorElement29,
            kColorElement30,
            kColorElement31,
            kColorCount,
        };

        ResourceHistoryGraph(const ResourceHistoryGraph&);
        void operator =(const ResourceHistoryGraph&);
        virtual void on_realize();
        virtual bool on_expose_event(GdkEventExpose*);
        virtual void drawPixmap(bool = true) = 0;
        void setPercentLabels();
        void setByteLabels(unsigned long);
        void drawBackground();
        void drawLeftColumn();
        void drawTimeRow();
        void drawGrid();
        void drawFrame();
        int drawTextV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList);
        int drawText(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...);
        int drawText(int x, int y, const Gdk::Color& fg, const char* format, ...);
        int drawText(int x, int y, const char* format, ...);
        int drawTextCenterV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList);
        int drawTextCenter(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...);
        int drawTextCenter(int x, int y, const Gdk::Color& fg, const char* format, ...);
        int drawTextCenter(int x, int y, const char* format, ...);
        int drawTextRightV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList);
        int drawTextRight(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...);
        int drawTextRight(int x, int y, const Gdk::Color& fg, const char* format, ...);
        int drawTextRight(int x, int y, const char* format, ...);
        int drawTextRightCenterV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList);
        int drawTextRightCenter(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...);
        int drawTextRightCenter(int x, int y, const Gdk::Color& fg, const char* format, ...);
        int drawTextRightCenter(int x, int y, const char* format, ...);
        int drawTextRightBottomV(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, va_list argList);
        int drawTextRightBottom(int x, int y, const Gdk::Color& fg, const Gdk::Color& bg, const char* format, ...);
        int drawTextRightBottom(int x, int y, const Gdk::Color& fg, const char* format, ...);
        int drawTextRightBottom(int x, int y, const char* format, ...);

        Glib::RefPtr<Gdk::Pixmap> _pixmap;
        int _pixmapWidth;
        int _pixmapHeight;
        Glib::RefPtr<Gdk::GC> _gc;
        Gdk::Color _colors[kColorCount];
        Gdk::Color _bg;
        Gdk::Color _fg;
        Glib::RefPtr<Pango::Layout> _layout;

        int _pointCount;
        int _elementCount;
        unsigned long _elementMask;
        unsigned long _time[kMaxPointCount + 1];
        unsigned long _value[kMaxPointCount + 1][kMaxElementCount];
        Glib::ustring _yLabel[5];
        int _textHeight;
        int _cxb;
        int _cyb;
        int _xb1;
        int _yb1;
        int _xb2;
        int _yb2;
        int _xb3;
        int _yb3;
        int _xb4;
        int _yb4;
        int _cx;
        int _cy;
    };
}


#endif //!HNRT_RESOURCEHISTORYGRAPH_H
