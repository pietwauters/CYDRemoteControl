#!/usr/bin/env python3
"""
Merge ESP32 firmware binaries into a single file for distribution.
This creates a complete firmware image that can be flashed at offset 0x0.
"""

import os
import sys

# Flash memory size (16MB = 0x1000000)
FLASH_SIZE = 0x1000000

# File offsets (must match partition table)
BOOTLOADER_OFFSET = 0x1000
PARTITION_TABLE_OFFSET = 0x8000
FIRMWARE_OFFSET = 0x10000

# Build directory
BUILD_DIR = ".pio/build/nodemcu-32s"

def merge_binaries():
    """Merge bootloader, partition table, and firmware into single binary."""
    
    # Check if build directory exists
    if not os.path.exists(BUILD_DIR):
        print(f"Error: Build directory '{BUILD_DIR}' not found.")
        print("Please run 'platformio run' first to build the firmware.")
        sys.exit(1)
    
    # Input files
    bootloader_file = os.path.join(BUILD_DIR, "bootloader.bin")
    partitions_file = os.path.join(BUILD_DIR, "partitions.bin")
    firmware_file = os.path.join(BUILD_DIR, "firmware.bin")
    
    # Check if all files exist
    missing_files = []
    for f in [bootloader_file, partitions_file, firmware_file]:
        if not os.path.exists(f):
            missing_files.append(f)
    
    if missing_files:
        print("Error: Missing build files:")
        for f in missing_files:
            print(f"  - {f}")
        print("\nPlease run 'platformio run' first to build the firmware.")
        sys.exit(1)
    
    # Output file
    output_file = "firmware_merged.bin"
    
    print(f"Merging firmware files...")
    print(f"  Bootloader:      {bootloader_file} @ 0x{BOOTLOADER_OFFSET:05X}")
    print(f"  Partition Table: {partitions_file} @ 0x{PARTITION_TABLE_OFFSET:05X}")
    print(f"  Firmware:        {firmware_file} @ 0x{FIRMWARE_OFFSET:05X}")
    
    # Create merged binary filled with 0xFF (erased flash state)
    # Only allocate enough space to include the firmware
    firmware_size = os.path.getsize(firmware_file)
    total_size = FIRMWARE_OFFSET + firmware_size
    
    merged = bytearray([0xFF] * total_size)
    
    # Read and insert bootloader
    with open(bootloader_file, "rb") as f:
        bootloader_data = f.read()
        merged[BOOTLOADER_OFFSET:BOOTLOADER_OFFSET + len(bootloader_data)] = bootloader_data
        print(f"  ✓ Bootloader added ({len(bootloader_data)} bytes)")
    
    # Read and insert partition table
    with open(partitions_file, "rb") as f:
        partitions_data = f.read()
        merged[PARTITION_TABLE_OFFSET:PARTITION_TABLE_OFFSET + len(partitions_data)] = partitions_data
        print(f"  ✓ Partition table added ({len(partitions_data)} bytes)")
    
    # Read and insert firmware
    with open(firmware_file, "rb") as f:
        firmware_data = f.read()
        merged[FIRMWARE_OFFSET:FIRMWARE_OFFSET + len(firmware_data)] = firmware_data
        print(f"  ✓ Firmware added ({len(firmware_data)} bytes)")
    
    # Write merged binary
    with open(output_file, "wb") as f:
        f.write(merged)
    
    print(f"\n✓ Merged firmware created: {output_file}")
    print(f"  Total size: {len(merged):,} bytes ({len(merged) / 1024 / 1024:.2f} MB)")
    print(f"\nTo flash manually:")
    print(f"  esptool.py write_flash 0x0 {output_file}")
    print(f"\nFor esp-launchpad distribution:")
    print(f"  Upload {output_file} along with manifest.json and flash.html")

if __name__ == "__main__":
    merge_binaries()
