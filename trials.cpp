//
// Created by ych on 16.10.16.
//

#include <stdio.h>
#include "trials.h"
#include <QApplication>
#include "MainWindow.h"


int main (int argc, char *argv[])
{
    if (argc < 2)
    {
        QApplication app (argc, argv);
        MainWindow mainWindow;
        mainWindow.show();
        return app.exec();
    }
    std::string flag1 (argv[1]);
    if (flag1.compare("-v") == 0) {
        fprintf(stdout, "%s Version %d.%d\n",
                Sandbar_APPLICATION_NAME,
                Sandbar_VERSION_MAJOR,
                Sandbar_VERSION_MINOR);
    }
    else if (flag1.compare("-h") == 0)
    {
        printf("This will print the help file.\n");
    }
    else
    {
        printf("Unknown option %s\n", argv[1]);
    }
    return 0;
}

