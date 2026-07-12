#!/bin/bash

# *
# *	This file is part of gooseOS.
# *
# *	gooseOS is free software: you can redistribute it and/or modify
# *	it under the terms of the GNU General Public License as published by
# *	the Free Software Foundation, either version 3 of the License, or
# *	(at your option) any later version.
# *
# *	gooseOS is distributed in the hope that it will be useful,
# *	but WITHOUT ANY WARRANTY; without even the implied warranty of
# *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *	GNU General Public License for more details.
# *
# *	You should have received a copy of the GNU General Public License
# *	along with gooseOS.  If not, see <https://www.gnu.org/licenses/>.
# *
# *	Copyright(c) 2026 EyeDev
# *

set -e
set -euo pipefail

# Simple usage check
if [ $# -eq 0 ]; then
    echo "Usage: $0 <kernel.elf> [output.img] <extrafiles/>"
    exit 1
fi

KERNEL="$1"
IMAGE="$2"
EXTRAFILES="$3"

printf "create-limine.sh v1.0\n\n"
printf "This script will create a new EFI Limine image with your kernel in it!\n\n"

printf "NOTICE: To put extra files inside of the IMG, put them in the folder passed in as the third argument\n"
printf "NOTICE: Those files WILL be put inside of the /boot directory\n"

printf "KERNEL: ${KERNEL}\n"
printf "IMAGE: ${IMAGE}\n"
printf "EXTRAFILES: ${EXTRAFILES}\n"

## CONFIG START

LIMINE_BINARY_DOWNLOAD="https://github.com/Limine-Bootloader/Limine/releases/download/v12.3.3/limine-binary.zip"

## CONFIG END

# Define the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Check for all dependencies
deps=("wget" "dd" "parted" "mkfs.fat" "losetup" "mount" "umount" "unzip")
missing=()
for cmd in "${deps[@]}"; do
    if ! command -v "$cmd" >/dev/null; then
        missing+=("$cmd")
    fi
done

if [ ${#missing[@]} -eq 0 ]; then
    # If all the dependencies are found, continue on with making the image
    printf "[\033[32mOK\033[0m] Found all dependencies, creating image!\n" 
else
    printf "[\033[31mFAIL\033[0m] Missing: ${missing[*]}\n"
    exit 1
fi

printf "[\033[36mINFO\033[0m] Installing limine to limine folder!\n"

if [ -d "${SCRIPT_DIR}/tmp/limine-binary" ]; then
    printf "[\033[36mINFO\033[0m] Limine allready installed!"
else
    # Limine is not allready downloaded, so we should use wget to download the zip
    wget "${LIMINE_BINARY_DOWNLOAD}"

    printf "[\033[32mOK\033[0m] Downloaded Limine from github! Unzipping archive\n"

    # Now unzip the archive
    unzip limine-binary.zip -d "${SCRIPT_DIR}/tmp/"
    rm -rf limine-binary.zip # Then delete the archive
fi

# Now that we have downloaded the needed files, lets create the image

# Create the folder
mkdir -p "${SCRIPT_DIR}/tmp/img_root/EFI/BOOT"
mkdir -p "${SCRIPT_DIR}/tmp/img_root/boot"

printf "[\033[32mOK\033[0m] Created folders\n"

# Copy the files

#cp "${SCRIPT_DIR}/tmp/limine-binary/BOOTX64.EFI" "${SCRIPT_DIR}/tmp/img_root/EFI/BOOT" # EFI boot entry
cp "${KERNEL}" "${SCRIPT_DIR}/tmp/img_root/boot/kernel" # Kernel ELF

if [ -d "${EXTRAFILES}" ]; then
    # The "/." tells bash to copy everything INSIDE the folder, not the folder itself
    cp -r "${EXTRAFILES}/." "${SCRIPT_DIR}/tmp/img_root/boot/"
fi

printf "[\033[32mOK\033[0m] Copied files\n"

# Create the limine config

create_default_config() {
    cat > "$1" << EOF
# Timeout in seconds that Limine will use before automatically booting.
timeout: 3

# The entry name that will be displayed in the boot menu.
/Limine Template
    # We use the Limine boot protocol.
    protocol: limine

    # Path to the kernel to boot. boot():/ represents the partition on which limine.conf is located.
    path: boot():/boot/kernel
EOF
}

if [ -f "limine.conf" ]; then
    printf "[\033[36mINFO\033[0m] Using existing limine.conf\n"
    cp limine.conf "${SCRIPT_DIR}/tmp/img_root/boot"
else
    printf "[\033[36mINFO\033[0m] Creating default limine.conf\n"

    echo "Creating default limine.conf"
    create_default_config "${SCRIPT_DIR}/tmp/img_root/boot/limine.conf"
fi


printf "[\033[36mINFO\033[0m] Creating blank 256 mb image, formatting to FAT32\n"
dd if=/dev/zero of="${IMAGE}" bs=1M count=256 # Write 256 MBs of zeros

parted -s "${IMAGE}" mklabel gpt

parted -s "${IMAGE}" mkpart ESP fat32 1MiB 64MiB
parted -s "${IMAGE}" set 1 esp on

parted -s "${IMAGE}" mkpart ROOT fat32 64MiB 100%

# Get the loop device
LOOP=$(sudo losetup --find --show -P "${IMAGE}")

sudo mkfs.fat -F 32 ${LOOP}p1
sudo mkfs.fat -F 32 ${LOOP}p2

printf "[\033[36mINFO\033[0m] Created image! Writing EFI partition\n"

mkdir -p "${SCRIPT_DIR}/tmp/mnt/efi" "${SCRIPT_DIR}/tmp/mnt/data" # Make folders

sudo mount ${LOOP}p1 ${SCRIPT_DIR}/tmp/mnt/efi
sudo mount ${LOOP}p2 ${SCRIPT_DIR}/tmp/mnt/data
sudo mkdir -p "${SCRIPT_DIR}/tmp/mnt/efi/EFI/BOOT"

# Copy EFI files
sudo cp -r "${SCRIPT_DIR}/tmp/limine-binary/BOOTX64.EFI" "${SCRIPT_DIR}/tmp/mnt/efi/EFI/BOOT/"

# Copy data from root
printf "[\033[36mINFO\033[0m] Writing image data\n"

sudo cp -r "${SCRIPT_DIR}/tmp/img_root"/* "${SCRIPT_DIR}/tmp/mnt/data"

# Unmount
sync
sudo umount "${SCRIPT_DIR}/tmp/mnt/efi"
sudo umount "${SCRIPT_DIR}/tmp/mnt/data"
sudo losetup -d $LOOP



printf "[\033[32mOK\033[0m] Created image!\n"

# Print end instructions
printf "====================================\n\n"
printf "Your image has now been created! It includes the UEFI version of Limine, and your kernel\n"
printf "Now you can run it using QEMU or another emulator!\n\n"
printf "====================================\n"