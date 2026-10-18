#include "shell/Shell.hpp"

int main()
{
    Shell::installSignalHandler();
    Shell shell;
    return shell.run();
}
