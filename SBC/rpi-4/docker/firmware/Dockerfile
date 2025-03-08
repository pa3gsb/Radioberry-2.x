# Base image.
FROM balenalib/aarch64-debian:bookworm-run

# Copy
COPY radioberry /usr/local/bin/radioberry


# Make script executable
RUN chmod +x /usr/local/bin/radioberry

# Set work directory
WORKDIR /usr/local/bin

# Set the command
CMD ["./radioberry"]
