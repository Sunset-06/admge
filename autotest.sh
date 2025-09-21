#!/bin/bash
# A script to automatically run all test ROMs in the directory.

BIN="./bin/admge" 

ROMS_DIR="./roms/acceptance/ppu"

find "$ROMS_DIR" -maxdepth 1 -type f -name "*.gb" -print0 | while IFS= read -r -d $'\0' rom_file; do
    echo "================================================="
    echo "===> Running test ROM: $rom_file"
    echo "================================================="
    
    "$BIN" "$rom_file" "-noboot"
    
    #read -p "Press [Enter] to run the next ROM..."
done

echo ""
echo "All .gb ROMs in '$ROMS_DIR' have been processed."