FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends g++ binutils libc6-dev git ca-certificates sudo curl gnupg && \
    curl -fsSL https://deb.nodesource.com/setup_20.x | bash - && \
    apt-get install -y --no-install-recommends nodejs && \
    rm -rf /var/lib/apt/lists/*

RUN echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/ubuntu && \
    mkdir -p /home/ubuntu/.claude && \
    chown ubuntu:ubuntu /home/ubuntu/.claude

RUN npm install -g @anthropic-ai/claude-code

ARG FORGE_VERSION=v0.11.28
RUN curl -L "https://github.com/cwbaker/forge/releases/download/${FORGE_VERSION}/forge-linux-${FORGE_VERSION}.tar.gz" -o /tmp/forge.tar.gz && \
    tar -xzf /tmp/forge.tar.gz -C /opt && \
    rm /tmp/forge.tar.gz && \
    chown -R ubuntu:ubuntu /opt/forge
ENV PATH="/opt/forge/bin:${PATH}"
RUN echo 'export PATH="/opt/forge/bin:$PATH"' > /etc/profile.d/forge.sh
