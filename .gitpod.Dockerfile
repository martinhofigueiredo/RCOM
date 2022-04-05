
FROM gitpod/workspace-full

# Install custom tools, runtime, etc.
RUN sudo apt-get update \
    && sudo apt-get install socat -y \
    && sudo rm -rf /var/lib/apt/lists/*