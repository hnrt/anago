// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MEMORYGRAPH_H
#define HNRT_MEMORYGRAPH_H


#include "ResourceHistoryGraph.h"


namespace hnrt
{
    class MemoryGraph
        : public ResourceHistoryGraph
    {
    public:

        MemoryGraph();
        virtual ~MemoryGraph();

    protected:

        MemoryGraph(const MemoryGraph&);
        void operator =(const MemoryGraph&);
        virtual void drawPixmap(bool = true);

        char* _totalMemory;
        char* _currentlyUsed;
        char* _availableMemory;

        int _cx7;
        int _cy7;
        int _x7;
        int _y71;
        int _y72;
        int _y73;
        int _x8;
        int _x9;
    };
}


#endif //!HNRT_MEMORYGRAPH_H
