#!/bin/bash

# Enable strict error handling
set -euo pipefail
IFS=$'\n\t'

# Default values for options
LOG_FILE="setup_and_compile.log"
VERBOSE=false

# Function to display usage information
usage() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  -v, --verbose         Enable verbose output"
    echo "  -h, --help            Display this help message and exit"
    echo "  -l, --logfile FILE    Specify a log file (default: setup_and_compile.log)"
    exit 1
}

# Function to handle errors
error_exit() {
    echo "Error on line $1: $2" | tee -a "$LOG_FILE" >&2
    exit 1
}

# Trap errors
trap 'error_exit ${LINENO} "$BASH_COMMAND"' ERR

# Parse command-line options
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        -l|--logfile)
            if [[ -n ${2-} ]]; then
                LOG_FILE="$2"
                shift 2
            else
                echo "Error: --logfile requires a filename argument."
                usage
            fi
            ;;
        *)
            echo "Unknown option: $1"
            usage
            ;;
    esac
done

# Redirect output to log file
if $VERBOSE; then
    exec > >(tee -a "$LOG_FILE") 2>&1
else
    exec > "$LOG_FILE" 2>&1
fi

echo "Starting setup and compilation process..."

# Update package lists
echo "Updating package lists..."
sudo apt-get update

# Install essential build tools and git
echo "Installing build-essential and git..."
sudo apt-get install -y build-essential git

# Install libcurl development files
echo "Installing libcurl development files..."
sudo apt-get install -y libcurl4-openssl-dev

# Install Python 3 and development headers
echo "Installing Python 3 and development headers..."
sudo apt-get install -y python3 python3-dev python3-pip

# Install Python packages: matplotlib and numpy
echo "Installing Python packages: matplotlib and numpy..."
pip3 install --user matplotlib numpy

# Install nlohmann/json (JSON for Modern C++)
echo "Installing nlohmann/json..."
sudo apt-get install -y nlohmann-json3-dev

# Clone matplotlib-cpp repository
echo "Cloning matplotlib-cpp repository..."
git clone https://github.com/lava/matplotlib-cpp.git

# Create include directory if it doesn't exist
echo "Creating include directory..."
mkdir -p include

# Copy matplotlibcpp.h to include directory
echo "Copying matplotlibcpp.h to include directory..."
cp matplotlib-cpp/matplotlibcpp.h include/

# Ensure json.hpp is available
if [ ! -f /usr/include/nlohmann/json.hpp ]; then
    echo "json.hpp not found in system include path. Downloading..."
    wget -O include/json.hpp https://github.com/nlohmann/json/releases/latest/download/json.hpp
else
    echo "json.hpp found. Creating symlink in include directory..."
    ln -sf /usr/include/nlohmann/json.hpp include/json.hpp
fi

# Clean up cloned repositories
echo "Cleaning up cloned repositories..."
rm -rf matplotlib-cpp

# Prompt for Reddit API credentials
echo "Please enter your Reddit API credentials and settings."

read -p "Client ID: " client_id
if [ -z "$client_id" ]; then
    error_exit ${LINENO} "Client ID cannot be empty."
fi

read -p "Client Secret: " client_secret
if [ -z "$client_secret" ]; then
    error_exit ${LINENO} "Client Secret cannot be empty."
fi

read -p "Reddit Username: " username
if [ -z "$username" ]; then
    error_exit ${LINENO} "Reddit Username cannot be empty."
fi

read -sp "Reddit Password: " password
echo
if [ -z "$password" ]; then
    error_exit ${LINENO} "Reddit Password cannot be empty."
fi

read -p "User Agent (e.g., YourBot/0.1 by YourUsername): " user_agent
if [ -z "$user_agent" ]; then
    error_exit ${LINENO} "User Agent cannot be empty."
fi

# Prompt for other settings
read -p "Subreddit to analyze (default is 'cpp'): " subreddit_name
subreddit_name=${subreddit_name:-cpp}

read -p "Number of submissions to fetch (default is 100): " submission_limit
submission_limit=${submission_limit:-100}
if ! [[ "$submission_limit" =~ ^[0-9]+$ ]]; then
    error_exit ${LINENO} "Submission limit must be a positive integer."
fi

# Create config.ini file
echo "Creating config.ini file..."
cat > config.ini <<EOL
[reddit]
client_id = $client_id
client_secret = $client_secret
username = $username
password = $password
user_agent = $user_agent

[settings]
subreddit_name = $subreddit_name
submission_limit = $submission_limit
EOL

# Set permissions on config.ini
chmod 600 config.ini

# Check if main.cpp exists
if [ ! -f main.cpp ]; then
    error_exit ${LINENO} "main.cpp not found in the current directory."
fi

# Compile the program
echo "Compiling the program..."

# Use python3-config to get include and library paths
if ! PYTHON_INCLUDE=$(python3-config --includes); then
    error_exit ${LINENO} "Failed to get Python include paths."
fi

if ! PYTHON_LDFLAGS=$(python3-config --ldflags --embed); then
    error_exit ${LINENO} "Failed to get Python linker flags."
fi

# Compile the program with the necessary flags
if ! g++ main.cpp -o reddit_bot \
    -std=c++17 \
    ${PYTHON_INCLUDE} \
    -I./include \
    -I${NUMPY_INCLUDE} \
    ${PYTHON_LDFLAGS} \
    -lcurl; then
    error_exit ${LINENO} "Compilation failed."
fi

echo "Compilation finished successfully."
echo "Setup and compilation complete."
