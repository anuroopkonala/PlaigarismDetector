#include <iostream>
#include "CLI.h"

int CLI::run(int argc,char** argv){
    if(argc<2){
        std::cout<<"Usage: detector <submission_folder>\n";
        return 1;
    }
    std::cout<<"Phase 1: Project scaffold\n";
    std::cout<<"Input folder: "<<argv[1]<<"\n";
    return 0;
}
