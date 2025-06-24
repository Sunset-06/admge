#include"include/core.h"

int main(int argc, char *argv[]) {
    while(!quit_flag){
        fetch();
        decode();
        execute();
    }
}