#include <stdio.h>
#include <malloc.h>
#include <dirent.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

#include <sys/stat.h>

#include "source/modules/dropbox.h"
#include "source/modules/citra.h"
#include "source/modules/time.h"


int main(int argc, char** argv){
    std::string checkpointPath = "/mnt/c/3ds-sdmc-dev/3ds/Checkpoint";

    auto refreshToken = std::getenv("DROPBOX_REFRESH_TOKEN");

    std::string dropboxToken = get_dropbox_access_token(refreshToken);
    if (dropboxToken == "") {
        std::cout << "Failed to receive Dropbox access token, exiting" << std::endl;
        return 1;
    }
    downloadCitraSaves(dropboxToken, checkpointPath);
}
