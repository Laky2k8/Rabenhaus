clear
printf "Rabenhaus Builder v0.0.1\n"
read -p "Press any key to start build..."

printf "\nCompiling Rabenhaus...\n"

gmake

read -p "Done! Press any key to continue."

printf "Compiling Limine...\n"
make -C limine

read -p "Done! Press any key to continue."

printf "Copying files...\n"

mkdir -p iso_root
mkdir -p iso_root/boot
cp -v bin/Rabenhaus iso_root/boot/
mkdir -p iso_root/boot/limine
cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin \
	limine/limine-uefi-cd.bin iso_root/boot/limine/

read -p "Done! Press any key to continue."

echo "Creating EFI boot tree...\n"

mkdir -p iso_root/EFI/BOOT
cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/

read -p "Done! Press any key to continue."

echo "Creating bootable ISO...\n"

xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o Rabenhaus.iso

# Install Limine stage 1 and 2 for Legacy BIOS boot

./limine/limine bios-install Rabenhaus.iso

read -p "ISO successfully built!"