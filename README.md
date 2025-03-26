# 3DSync

Homebrew for Nintendo 3DS/2DS console family that allows synchronization of saves and files to a cloud, to another console or to a PC

Configurator has built-in support for Checkpoint and JKSM folders, but you can add custom one if you like

**currently in early stage, supports only Dropbox to upload/download files**

Modified (April 2024) so that when syncing Citra saves into the 3ds, all the save slots are downloaded instead of only the first one.
It also includes the code for buildtools, which was previously a submodule and became unaccessible since the [original repository](https://github.com/Steveice10/buildtools/) was archived/deleted and returns a 404 error.

## Usage

1. Follow steps on the [configurator page](https://concreted.github.io/3DSync/) to obtain the configuration file
2. Place the configurator file in the following folder of the console SD card: `/3ds/3DSync/3DSync.ini`
3. Download and install .cia file **or** run the .3dsx from the homebrew launcher

### Checkpoint Citra save downloading 

The application will attempt to find and download Citra saves in Dropbox to the 3DS's Checkpoint directory. To enable, place your entire Citra `sdmc` directory in your Dropbox's `Apps/3DSyncU` directory. The final directory structure in Dropbox should look something like:

```
Apps
|_3DSyncU
  |_sdmc
    |_Nintendo 3DS
      |_...
```

## Development

Follow the steps below to install build dependencies and compile on Ubuntu:

```bash
sudo apt install zip unzip -y

# Install devkitpro with 3DS support
sudo apt-get install gdebi-core
wget https://github.com/devkitPro/pacman/releases/download/v1.0.2/devkitpro-pacman.amd64.deb    # Missing (1.0.2 is deprecated and not included in the releases section of the repo), check the compilation_tools_and_dependencies folder
sudo gdebi devkitpro-pacman.amd64.deb
sudo dkp-pacman -S 3ds-dev

source /etc/profile.d/devkit-env.sh

# Install other 3DS build dependencies
sudo dkp-pacman -S 3ds-curl 3ds-mbedtls 3ds-libjson-c

wget https://github.com/Steveice10/bannertool/releases/download/1.2.0/bannertool.zip    # Missing, check the compilation_tools_and_dependencies folder
unzip bannertool.zip -d bannertool
sudo cp bannertool/linux-x86_64/bannertool /usr/local/bin

wget https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.18.3/makerom-v0.18.3-ubuntu_x86_64.zip   # Not missing, but included in the compilation_tools_and_dependencies folder just in case
unzip makerom-v0.18.3-ubuntu_x86_64.zip -d makerom
sudo cp makerom/makerom /usr/local/bin
sudo chmod +x /usr/local/bin/makerom

# Build 3DS binaries
mkdir -p ~/src
cd ~/src
git clone https://github.com/PedroPerez14/3DSync-multisave.git
cd 3DSync-multisave
make
```

In the aforementioned sequence of bash commmands for compilation, some dependencies will no longer work and those commands will return an error. The missing packages and tools have been added to the ```compilation_tools_and_dependencies``` folder to save lots of time. Tested on Ubuntu 20.04 (testing binary hasn't been tested, only the 3ds application).

A testing binary can be built to test functionality locally:

```bash
# Build
## Before building, you may have to comment out line 8 in source/utils/curl.cpp (referring to VERSION_STRING)
g++ -o test test.cpp source/modules/dropbox.cpp source/utils/curl.cpp -lcurl -ljson-c

# Run the test binary
## Set your Dropbox refresh token as env var
export DROPBOX_REFRESH_TOKEN=<your Dropbox refresh token>
./test
```
