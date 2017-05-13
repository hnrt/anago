// Copyright (C) 2012-2017 Hideaki Narita


#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "PoFile.h"


using namespace hnrt;


int main(int argc, char* argv[])
{
    try
    {
        setlocale(LC_ALL, "");

        bool copyId = false;
        PoFile* pot = NULL;
        PoFile* po = NULL;

        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                if (!strcmp(&argv[i][1], "copy"))
                {
                    if (pot)
                    {
                        goto syntax;
                    }
                    copyId = true;
                }
                else
                {
                    fprintf(stderr, "Error: Bad option: %s\n", argv[i]);
                    goto syntax;
                }
            }
            else if (!copyId && !pot)
            {
                pot = new PoFile(argv[i]);
                if (!pot->open())
                {
                    return EXIT_FAILURE;
                }
            }
            else if (!po)
            {
                po = new PoFile(argv[i]);
                if (!po->open())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                fprintf(stderr, "Error: Extra parameter: %s\n", argv[i]);
                goto syntax;
            }
        }

        if (copyId)
        {
            if (!po->translateWithId())
            {
                return EXIT_FAILURE;
            }
        }
        else if (pot && po)
        {
            if (!po->updateWithTemplate(*pot))
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
        syntax:
            char* name = strrchr(argv[0], '/');
            if (name)
            {
                name++;
            }
            else
            {
                name = argv[0];
            }
            fprintf(stderr, "Usage: %s FILE.pot FILE.po\n", name);
            fprintf(stderr, "       %s -copy FILE.po\n", name);
            return EXIT_FAILURE;
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
