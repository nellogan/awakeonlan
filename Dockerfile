FROM debian:forky-slim
WORKDIR /workspace

ENV DEBIAN_FRONTEND=noninteractive

# Install stable LLVM toolchain with explicit version-matched compiler-rt
RUN apt-get update && apt-get install -y --no-install-recommends \
    make \
    valgrind \
    clang-21 \
    clang-format-21 \
    clang-tidy-21 \
    lldb-21 \
    lld-21 \
    libc++-21-dev \
    libc++abi-21-dev \
    libunwind-21-dev \
    libclang-rt-21-dev \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-21 100 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-21 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-21 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-21 100

# Non-root user
RUN useradd -ms /bin/bash container_user
RUN mkdir -p /workspace && chown container_user:container_user /workspace
WORKDIR /workspace
USER container_user

COPY --chown=container_user:container_user src/ src/
COPY --chown=container_user:container_user include/ include/
COPY --chown=container_user:container_user tests/ tests/
COPY --chown=container_user:container_user format.sh format.sh
COPY --chown=container_user:container_user .clang-format .clang-format
COPY --chown=container_user:container_user lint.sh lint.sh
COPY --chown=container_user:container_user .clang-tidy .clang-tidy
COPY --chown=container_user:container_user Makefile Makefile

CMD ["/bin/bash"]