// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CPUGRAPH_H
#define HNRT_CPUGRAPH_H


#include "ResourceHistoryGraph.h"


namespace hnrt
{
    class CpuGraph
        : public ResourceHistoryGraph
    {
    public:

        CpuGraph();
        virtual ~CpuGraph();

    protected:

        CpuGraph(const CpuGraph&);
        void operator =(const CpuGraph&);
        void drawPixmap(bool = true);
    };
}


#endif //!HNRT_CPUGRAPH_H
