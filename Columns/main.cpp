#include <iostream>
#include <fstream>

#include "common.h"
#include "columns.h"
#include "xlog.h"

using namespace std;

static int entry()
{
#if defined(DEBUG) | defined (_DEBUG)
    // Turn on memory leak checking
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    int result;
    {
        columns app;
        result = app();
    }   // ~app()
    xlog << "Clean shutdown" << endl;
    return result;
}

int main(int argc, char** argv)
{
    // Command line dump
    for (int i = 0; i != argc; ++i)
        xlog << argv[i] << " ";
    xlog << endl;
    // Call common entry point
    return entry();
}

int CALLBACK WinMain(HINSTANCE /* hInstance */, HINSTANCE /* hPrevInstance */, LPSTR lpCmdLine, int /* nCmdShow */)
{
    // Command line dump
    xlog << lpCmdLine << endl;
    // Call common entry point
    return entry();
}