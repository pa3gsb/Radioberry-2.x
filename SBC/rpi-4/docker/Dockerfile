# Base image.
FROM balenalib/aarch64-debian:bookworm-build AS build

# update and install required packages.
RUN apt-get update && apt-get install -y \
    build-essential \
    kmod \
	device-tree-compiler \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Copy
COPY ../device_driver/driver /driver
COPY build.sh /driver/build.sh


# Make script executable
RUN chmod +x /driver/build.sh 

# Set work directory
WORKDIR /driver

# Set the command
CMD ["/driver/build.sh"]
