#!/bin/bash
set -e

HAL_VERSION="1.28.1"
HAL_DIR="lib/stm32f4_hal"
CMSIS_DIR="lib/stm32f4_hal/Inc/CMSIS_Core"
DEVICE_DIR="lib/stm32f4_hal/Inc"

if [ -d "$HAL_DIR/Src" ] && [ "$(ls -A $HAL_DIR/Src/*.c 2>/dev/null)" ]; then
    echo "HAL already present, skipping download."
    exit 0
fi

echo "Fetching STM32CubeF4 v${HAL_VERSION}..."

TMPDIR=$(mktemp -d)
wget -q "https://github.com/STMicroelectronics/STM32CubeF4/archive/refs/tags/v${HAL_VERSION}.tar.gz" -O "$TMPDIR/stm32cube.tar.gz"
tar -xzf "$TMPDIR/stm32cube.tar.gz" -C "$TMPDIR"
CUBE_ROOT="$TMPDIR/STM32CubeF4-${HAL_VERSION}"

mkdir -p "$HAL_DIR/Src" "$HAL_DIR/Inc" "$CMSIS_DIR" "$DEVICE_DIR"

cp "$CUBE_ROOT/Drivers/STM32F4xx_HAL_Driver/Src/"*.c "$HAL_DIR/Src/"
cp "$CUBE_ROOT/Drivers/STM32F4xx_HAL_Driver/Inc/"*.h "$HAL_DIR/Inc/"
cp "$CUBE_ROOT/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c" "$HAL_DIR/Src/"
cp "$CUBE_ROOT/Drivers/CMSIS/Device/ST/STM32F4xx/Include/"*.h "$DEVICE_DIR/"
cp "$CUBE_ROOT/Drivers/CMSIS/Include/"*.h "$CMSIS_DIR/"

rm -rf "$TMPDIR"
echo "HAL library downloaded successfully."
