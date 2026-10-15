FROM debian:bookworm

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake git python3 ca-certificates pkg-config \
        gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib \
        libusb-1.0-0-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 --branch 2.1.1 https://github.com/raspberrypi/pico-sdk.git /opt/pico-sdk \
    && git -C /opt/pico-sdk submodule update --init --depth 1 lib/tinyusb

ENV PICO_SDK_PATH=/opt/pico-sdk

# picotool converts the ELF to UF2; install it so the SDK does not try to
# fetch and build it during every project build
RUN git clone --depth 1 --branch 2.1.1 https://github.com/raspberrypi/picotool.git /tmp/picotool \
    && cmake -S /tmp/picotool -B /tmp/picotool/build -DCMAKE_INSTALL_PREFIX=/opt/picotool -DPICOTOOL_FLAT_INSTALL=1 \
    && make -C /tmp/picotool/build -j"$(nproc)" install \
    && rm -rf /tmp/picotool

ENV picotool_DIR=/opt/picotool/picotool
