// Copyright (C) 2012-2017 Hideaki Narita


#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include "PoFile.h"


using namespace hnrt;


int main(int argc, char* argv[])
{
    try
    {
        setlocale(LC_ALL, "");

        for (int i = 1; i < argc; i++)
        {
            PoFile po(argv[i]);
            if (po.open(stdout))
            {
                po.printUntranslated();
            }
        }

        return EXIT_SUCCESS;
    }
    catch (std::bad_alloc)
    {
        fprintf(stderr, "Error: Out of memory.\n");
    }
    catch (...)
    {
        fprintf(stderr, "Error: Unhandled exception caught.\n");
    }

    return EXIT_FAILURE;
}
