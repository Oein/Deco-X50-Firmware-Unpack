FROM debian:bookworm

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential clang flex bison g++ gawk gettext git \
        libncurses-dev libssl-dev libelf-dev zlib1g-dev \
        python3 python3-setuptools python3-dev python3-pyelftools \
        rsync swig unzip file wget ca-certificates \
        xz-utils zstd bzip2 patch perl time quilt \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -u 1000 builder && mkdir -p /src /out && chown builder:builder /src /out
USER builder
WORKDIR /src
