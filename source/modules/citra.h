#include <stdio.h>
#include <malloc.h>
#include <dirent.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

#include <sys/stat.h>

#include "dropbox.h"

std::vector<std::string> getUpdateTimestampsForCitraSave(std::string dropboxToken, std::string fullPath, std::vector<std::string>& filenames) {
    Dropbox dropbox(dropboxToken);
    std::vector<std::string> strings;
    strings.empty();
    filenames.empty();
    auto results = dropbox.list_folder(fullPath + "/data/00000001");
    for (auto lr : results) {
        if (lr.name.substr(0, 4) == "user" || lr.name.substr(0, 6) == "system") {
            // filenames will store the names of the user<number> and system files, if any are found
            // This vector will be used by downloadCitraSavesToCheckpoint() to download all the save slots with valid timestamp
            // instead of only the first one, like up until now
            filenames.push_back(lr.name);
            strings.push_back(lr.server_modified);
        }
        else {
            std::cout << lr.name << ": Could not find timestamp, skipping" << std::endl; 
        }
    }
    return strings;
}

std::string checkpointDirToCitraGameCode(std::string checkpointGameSaveDir) {
    if (checkpointGameSaveDir.rfind("0x", 0) == 0) {
        std::string gameCode = "0" + checkpointGameSaveDir.substr(2,5) + "00";
        for (auto &elem : gameCode) {
            elem = std::tolower(elem);
        }
        return gameCode;
    }
    return "";
}

std::map<std::string, std::string> findCheckpointSaves(std::string checkpointPath) {
    std::map<std::string, std::string> pathmap;

    std::string path(checkpointPath + "/saves");
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(path.c_str())) != NULL){
        while((ent = readdir(dir)) != NULL){
            std::string dirname = ent->d_name;

            if (dirname == "." || dirname == "..") {
                continue;
            }

            std::string readpath(path + "/" + dirname);

            std::string gameCode = checkpointDirToCitraGameCode(dirname);
            if (gameCode != "") {
                pathmap[gameCode] = dirname;
            } else {
                std::cout << "Invalid game directory, skipping: " << dirname << std::endl;
            }
        }
    }
    return pathmap;
}

void downloadCitraSavesToCheckpoint(std::string dropboxToken, std::string timestamp, std::string checkpointPath, 
                        std::map<std::string, std::string> pathmap, std::string gameCode, std::string fullPath, std::vector<std::string> saveFileNames) {
    Dropbox dropbox(dropboxToken);

    std::string baseSaveDir = checkpointPath + "/saves";

    if (!pathmap.count(gameCode)) {
        std::cout << "Game code " << gameCode << " not found in local Checkpoint saves, skipping";
        return;
    }

    std::string gameDirname = pathmap[gameCode];
    std::string gameSaveDir = baseSaveDir + "/" + gameDirname;
    // TODO: Currently only the latest Citra save is retained per game. 
    //       Could append timestamp if wanted, but adding raw timestamp caused segfaults on 3DS.
    //       Investigate further if want to keep older saves.
    // std::string destPath = gameSaveDir + "/Citra_" + timestamp;
    std::string destPath = gameSaveDir + "/Citra";

    std::cout << "Citra save found for " << gameDirname << " with timestamp: " << timestamp << std::endl;

    struct stat info;

    if (stat(destPath.c_str(), &info) != 0) {
        std::cout << "Creating dir: " << destPath << std::endl;
        int status = mkdir(destPath.c_str(), 0777);
        if (status != 0) {
            std::cout << "Failed to create Checkpoint save dir " << destPath << ", skipping" << std::endl;
            return;
        }
    } else if (info.st_mode & !S_IFDIR) {
        std::cout << "File already exists at Checkpoint save dir, delete the file and try again:\n" << destPath << std::endl;
    } else {
        std::cout << "Checkpoint save dir already exists: " << destPath << std::endl;
    }

    for (std::string fileName : saveFileNames) {
        dropbox.download(fullPath + "/data/00000001/" + fileName,  destPath + "/" + fileName);
    }

}

void downloadCitraSaves(std::string dropboxToken, std::string checkpointPath) {
    auto pathmap = findCheckpointSaves(checkpointPath);

    // This vector will have the same size as timestamps after calling getUpdateTimestampsForCitraSave()!
    std::vector<std::string> saveFileNames;

    Dropbox dropbox(dropboxToken);
    auto folder = dropbox.list_folder("/sdmc/Nintendo 3DS/00000000000000000000000000000000/00000000000000000000000000000000/title/00040000");
    for (auto lr : folder) {
        std::vector<std::string> timestamps = getUpdateTimestampsForCitraSave(dropboxToken, lr.path_display, saveFileNames);

        downloadCitraSavesToCheckpoint(
        dropboxToken,
        timestamps.at(0),       // Now this is a vector
        checkpointPath,
        pathmap,
        lr.name,
        lr.path_display,
        saveFileNames           // Can be "user<number>" or "system"
        );

    }
}
