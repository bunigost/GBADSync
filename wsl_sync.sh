# Helper script to sync builds from WSL to Windows C: drive

mkdir -p /mnt/c/3ds-build
cp output/3ds-arm/3DSync.3ds /mnt/c/3ds-build/
cp output/3ds-arm/3DSync.cia /mnt/c/3ds-build/
echo "Finished syncing from WSL to Windows disk"
