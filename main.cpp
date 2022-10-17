#include "Quash.h"

int main(int argc, char* argv[]) {
    Quash shell;

    shell.run();

    return 0;
}


/*
atexit(3) - kill
chdir(2) - cd
close - pipes
dup2 - pipes
exexvp - everywhere
exit - kill
fork - pipes
getenv - env var
getwd (get_current_dir_name) - call relative path executables
kill - kill
open - ?
pipe - pipes
printf - echo
setenv - setting env var
waitpid - pipes
*/