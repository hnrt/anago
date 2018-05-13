// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_INPUTOUTPUTGRAPH_H
#define HNRT_INPUTOUTPUTGRAPH_H


#include "ResourceHistoryGraph.h"


namespace hnrt
{
    class InputOutputGraph
        : public ResourceHistoryGraph
    {
    public:

        InputOutputGraph();
        virtual ~InputOutputGraph();
        virtual void addValue(int, unsigned long, unsigned long, const char*, const char*);
        virtual void update();

    protected:

        InputOutputGraph(const InputOutputGraph&);
        void operator =(const InputOutputGraph&);
        void drawPixmap(bool queuing = true);

        char* _bytesPerSecond;

        unsigned long _maxValue;
        unsigned long _maxValueAt;
        Glib::ustring _labels[kMaxElementCount];
    };
}


#endif //!HNRT_INPUTOUTPUTGRAPH_H
