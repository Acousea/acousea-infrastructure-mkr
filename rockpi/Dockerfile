FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

# Install sudo only (the rest is done by bootstrap.sh)
RUN apt update && apt install -y sudo && rm -rf /var/lib/apt/lists/*

# Create a non-root user with passwordless sudo
RUN useradd -ms /bin/bash testuser && \
    echo "testuser ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER testuser
WORKDIR /home/testuser/project

# Copy the project (includes bootstrap.sh, daemon/, overlays/, etc.)
COPY --chown=testuser:testuser . .

# Enter into bash to run ./bootstrap.sh manually
CMD ["/bin/bash"]

# Optional: Uncomment the following lines to run bootstrap.sh automatically
#ENTRYPOINT ["./bootstrap.sh"]
#CMD ["all"]

