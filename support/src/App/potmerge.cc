// Copyright (C) 2012-2017 Hideaki Narita


#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "PoFile.h"


using namespace hnrt;


int main(int argc, char* argv[])
{
    try
    {
        setlocale(LC_ALL, "");

        std::vector<PoFile*> docs;

        for (int i = 1; i < argc; i++)
        {
            PoFile* doc = new PoFile(argv[i]);
            if (doc->open())
            {
                docs.push_back(doc);
            }
            else
            {
                return EXIT_FAILURE;
            }
        }

        if (docs.size())
        {
            PoFile* doc = docs[0];
            for (std::vector<PoFile*>::size_type i = 1; i < docs.size(); i++)
            {
                doc->merge(*docs[i]);
            }
            doc->print(stdout);
        }

        return EXIT_SUCCESS;
    }
    catch (std::bad_alloc)
    {
        fprintf(stderr, "Error: Insufficient memory.\n");
    }
    catch (...)
    {
        fprintf(stderr, "Error: Unhandled exception caught.\n");
    }

    return EXIT_FAILURE;
}
