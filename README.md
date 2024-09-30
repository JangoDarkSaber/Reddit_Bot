# Reddit_Bot

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
  - [Clone the Repository](#clone-the-repository)
  - [Prepare `main.cpp`](#prepare-maincpp)
- [Usage](#usage)
  - [Running the Setup Script](#running-the-setup-script)
  - [Script Options](#script-options)
  - [Running the Bot](#running-the-bot)
    - [Command-Line Options](#command-line-options)
      - [Examples](#examples)
- [Error Messages and Troubleshooting](#error-messages-and-troubleshooting)
  - [General Errors](#general-errors)
  - [Configuration Errors](#configuration-errors)
  - [Network and API Errors](#network-and-api-errors)
  - [Data Processing Errors](#data-processing-errors)
  - [Visualization Errors](#visualization-errors)
- [Security Considerations](#security-considerations)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)
- [Additional Notes](#additional-notes)

---

## Introduction

This project is a Reddit bot written in C++ that analyzes a subreddit and generates a graphic displaying which users post the most. It interacts with Reddit's API to fetch submission data, processes the data to determine top posters, and uses `matplotlib-cpp` to create a visualization.

The project includes a setup and compilation script (`setup_and_compile.sh`) that automates the installation of dependencies, compiles the program, and provides options for logging and verbosity.

---

## Features

- **Reddit API Interaction**: Fetches submissions from a specified subreddit.
- **Data Analysis**: Counts the number of posts per user.
- **Visualization**: Generates a bar chart of the top posters using `matplotlib-cpp`.
- **Command-Line Options**:
  - **Verbose Mode**: Provides detailed output during execution.
  - **Rate Limiting**: Controls the delay between API requests with four options: `slow`, `default`, `fast`, and `insane`.
  - **Date Range Filtering**: Fetches submissions within a specific date range.
- **Robust Error Handling**: Handles unexpected situations gracefully.
- **Secure Credential Handling**: Manages Reddit API credentials securely.

---

## Prerequisites

- **Operating System**: Ubuntu 20.04 LTS or similar Linux distribution.
- **Hardware Requirements**:
  - Processor: A modern multi-core CPU.
  - RAM: At least 2 GB.
  - Disk Space: At least 2 GB free.
- **Software Requirements**:
  - **Git**
  - **C++ Compiler**: `g++` with support for C++11 or higher.
  - **Python 3**: Version 3.6 or higher.
  - **Pip3**
  - **Reddit Account**: With a registered Reddit application for API access.

---

## Installation

### Clone the Repository

Clone this repository to your local machine:

```bash
git clone https://github.com/yourusername/reddit-bot-cpp.git
cd reddit-bot-cpp
```

### Prepare `main.cpp`

Ensure that the `main.cpp` file is present in the project directory. If not, you can create it with the provided source code.

---

## Usage

### Running the Setup Script

The `setup_and_compile.sh` script automates the installation of dependencies, prompts for necessary credentials, and compiles the program.

**Make the script executable:**

```bash
chmod +x setup_and_compile.sh
```

**Display the help message:**

```bash
./setup_and_compile.sh -h
```

**Example Output:**

```
Usage: ./setup_and_compile.sh [options]

Options:
  -v, --verbose         Enable verbose output
  -h, --help            Display this help message and exit
  -l, --logfile FILE    Specify a log file (default: setup_and_compile.log)
```

**Run the script with default settings:**

```bash
./setup_and_compile.sh
```

**Run the script with verbose output:**

```bash
./setup_and_compile.sh -v
```

**Specify a custom log file:**

```bash
./setup_and_compile.sh -l custom_log_file.log
```

#### Script Prompts

The script will prompt you for the following information:

- **Reddit API Credentials**:
  - **Client ID**
  - **Client Secret**
  - **Reddit Username**
  - **Reddit Password** (input will be hidden)
  - **User Agent** (e.g., `YourBot/0.1 by YourUsername`)
- **Settings**:
  - **Subreddit to analyze** (default: `cpp`)
  - **Number of submissions to fetch** (default: `100`)

Ensure you have registered a Reddit application to obtain your Client ID and Client Secret. You can do this by visiting [Reddit App Preferences](https://www.reddit.com/prefs/apps).

### Script Options

- **Verbose Mode**:
  - Enable verbose output to display the progress in the terminal.
  - Use the `-v` or `--verbose` flag.
- **Help Option**:
  - Display usage information.
  - Use the `-h` or `--help` flag.
- **Log File**:
  - Specify a custom log file.
  - Use the `-l` or `--logfile` option followed by the filename.

### Running the Bot

After successful compilation, an executable named `reddit_bot` will be created in the project directory.

**Run the bot:**

```bash
./reddit_bot [options]
```

#### Command-Line Options

The bot supports several command-line options:

- **Verbose Mode**: `-v` or `--verbose`
  - Enables verbose output, providing detailed information during execution.

- **Rate Limit**: `-r [option]` or `--rate [option]`
  - Sets the rate limit delay between API requests.
  - Options:
    - `slow` (2000 ms delay)
    - `default` (1000 ms delay)
    - `fast` (500 ms delay)
    - `insane` (no delay)
  - **Note**: Using `insane` may cause rate limiting by Reddit.

- **Start Date**: `-s YYYY-MM-DD` or `--start-date YYYY-MM-DD`
  - Fetches submissions from this date onwards.
  - Date format must be `YYYY-MM-DD`.

- **End Date**: `-e YYYY-MM-DD` or `--end-date YYYY-MM-DD`
  - Fetches submissions up to this date.
  - Date format must be `YYYY-MM-DD`.

- **Help**: `-h` or `--help`
  - Displays usage information.

**Display the bot's help message:**

```bash
./reddit_bot -h
```

**Example Output:**

```
Usage: reddit_bot [options]

Options:
  -v, --verbose            Enable verbose output
  -r, --rate [option]      Set rate limit (options: slow, default, fast, insane)
  -s, --start-date [date]  Set start date (format: YYYY-MM-DD)
  -e, --end-date [date]    Set end date (format: YYYY-MM-DD)
  -h, --help               Display this help message
```

#### Examples

- **Run with Verbose Output:**

  ```bash
  ./reddit_bot -v
  ```

- **Run with Fast Rate Limit:**

  ```bash
  ./reddit_bot -r fast
  ```

- **Run for a Specific Date Range:**

  ```bash
  ./reddit_bot -s 2023-01-01 -e 2023-01-31
  ```

- **Run with Multiple Options:**

  ```bash
  ./reddit_bot -v -r fast -s 2023-01-01 -e 2023-01-31
  ```

**Viewing the Generated Graphic:**

You can view the generated image using an image viewer:

```bash
xdg-open top_posters.png
```

---

## Error Messages and Troubleshooting

The program includes robust error handling to help you diagnose and fix issues. Below are the possible error messages you may encounter, along with troubleshooting steps to remediate them.

### General Errors

#### **An error occurred: _\<error message\>_**

**Description**: A general error indicating an exception was caught during execution.

**Troubleshooting Steps**:

- Read the specific error message provided to identify the issue.
- Refer to the relevant section below for detailed troubleshooting steps.

### Configuration Errors

#### **Failed to open configuration file: config.ini**

**Description**: The program could not find or open the `config.ini` file.

**Possible Causes**:

- `config.ini` is missing or not in the correct directory.
- File permissions prevent access.

**Troubleshooting Steps**:

- Ensure `config.ini` exists in the same directory as the executable.
- Verify file permissions:

  ```bash
  ls -l config.ini
  ```

- If missing, rerun the `setup_and_compile.sh` script to generate it.

#### **Configuration file is empty or invalid.**

**Description**: The configuration file lacks required parameters or is improperly formatted.

**Possible Causes**:

- Corrupted `config.ini`.
- Incorrect manual edits.

**Troubleshooting Steps**:

- Open `config.ini` and check for required sections and keys:

  ```ini
  [reddit]
  client_id = your_client_id
  client_secret = your_client_secret
  username = your_username
  password = your_password
  user_agent = YourBot/0.1 by YourUsername

  [settings]
  subreddit_name = cpp
  submission_limit = 100
  ```

- Ensure there are no syntax errors or typos.
- Rerun the setup script to regenerate the file.

#### **submission_limit must be a positive integer.**

**Description**: The `submission_limit` value is invalid.

**Possible Causes**:

- Non-integer value.
- Negative number or zero.

**Troubleshooting Steps**:

- Open `config.ini` and set `submission_limit` to a positive integer (e.g., `100`).
- Remove any non-numeric characters or spaces.

#### **Subreddit name cannot be empty.**

**Description**: The `subreddit_name` parameter is empty.

**Troubleshooting Steps**:

- Open `config.ini` and ensure `subreddit_name` has a valid subreddit name.
- Example:

  ```ini
  subreddit_name = cpp
  ```

- Save the file after making corrections.

### Network and API Errors

#### **Failed to obtain access token.**

**Description**: The program could not get an access token from Reddit.

**Possible Causes**:

- Incorrect Reddit API credentials.
- Network connectivity issues.
- Reddit API issues.

**Troubleshooting Steps**:

- Double-check `client_id` and `client_secret` in `config.ini`.
- Verify Reddit username and password.
- Ensure your Reddit account is active and not suspended.
- Check internet connectivity:

  ```bash
  ping www.reddit.com
  ```

- Retry after some time.

#### **Error obtaining access token: _\<error message\>_**

**Description**: Reddit API returned an error during authentication.

**Possible Causes**:

- Invalid credentials.
- Account restrictions.

**Troubleshooting Steps**:

- The error message provides specifics (e.g., `invalid_grant`, `invalid_client`).
- Ensure that two-factor authentication is addressed (use an app password if necessary).
- Verify that the client ID and secret correspond to a "script" type app.

#### **Invalid rate limit option. Use one of: slow, default, fast, insane.**

**Description**: An invalid rate limit option was provided.

**Troubleshooting Steps**:

- Use one of the valid rate limit options: `slow`, `default`, `fast`, `insane`.
- Example:

  ```bash
  ./reddit_bot -r default
  ```

#### **Invalid start date format. Use YYYY-MM-DD.**

**Description**: The start date provided does not match the expected format.

**Troubleshooting Steps**:

- Ensure the date is in `YYYY-MM-DD` format.
- Example:

  ```bash
  ./reddit_bot -s 2023-01-01
  ```

#### **Invalid end date format. Use YYYY-MM-DD.**

**Description**: The end date provided does not match the expected format.

**Troubleshooting Steps**:

- Ensure the date is in `YYYY-MM-DD` format.
- Example:

  ```bash
  ./reddit_bot -e 2023-01-31
  ```

#### **Start date cannot be after end date.**

**Description**: The start date is later than the end date.

**Troubleshooting Steps**:

- Verify that the start date is before or equal to the end date.
- Correct the dates accordingly.

#### **Failed to initialize CURL for getAccessToken.**

**Description**: The CURL library failed to initialize.

**Troubleshooting Steps**:

- Ensure libcurl is installed:

  ```bash
  sudo apt-get install libcurl4-openssl-dev
  ```

- Recompile the program.

#### **Empty response received in getAccessToken.**

**Description**: No data was received during authentication.

**Troubleshooting Steps**:

- Check network connection.
- Ensure Reddit's API is operational.
- Verify that your user agent string is correctly formatted.

#### **CURL Error in getAccessToken: _\<error description\>_**

**Description**: CURL encountered an error during authentication.

**Troubleshooting Steps**:

- The error description provides details (e.g., `Failed to connect`).
- Resolve network issues.
- Ensure that SSL certificates are up-to-date.

#### **Failed to initialize CURL for httpRequest.**

**Description**: CURL failed to initialize during data fetching.

**Troubleshooting Steps**:

- Same as for `getAccessToken` initialization failure.

#### **Empty response received in httpRequest.**

**Description**: No data was received from Reddit API.

**Troubleshooting Steps**:

- Verify that the access token is valid.
- Check for network issues.
- Ensure the subreddit exists and is accessible.

#### **CURL Error in httpRequest: _\<error description\>_**

**Description**: CURL encountered an error during data fetching.

**Troubleshooting Steps**:

- Review the error description.
- Ensure that the bearer token is correctly formatted.
- Verify that the API endpoint URL is correct.

### Data Processing Errors

#### **Failed to fetch data from Reddit API.**

**Description**: The program could not retrieve data from Reddit.

**Troubleshooting Steps**:

- Confirm that the access token is valid and has not expired.
- Check internet connectivity.
- Verify that the subreddit name is correct.

#### **Failed to parse JSON response: _\<error message\>_**

**Description**: The JSON data received is malformed or unexpected.

**Possible Causes**:

- API changes or issues.
- Rate limiting causing HTML response instead of JSON.

**Troubleshooting Steps**:

- Check Reddit API status.
- Ensure that the user agent string is properly set.
- Add delays or reduce request frequency to avoid rate limiting.

#### **Unexpected JSON structure.**

**Description**: The structure of the JSON data does not match expectations.

**Possible Causes**:

- Changes in Reddit API response format.
- Invalid API endpoint.

**Troubleshooting Steps**:

- Verify the API endpoint URL.
- Consult the [Reddit API documentation](https://www.reddit.com/dev/api/) for updates.
- Update the code to handle new JSON structures if necessary.

#### **No authors found in the fetched submissions.**

**Description**: No author data was retrieved from the submissions.

**Possible Causes**:

- The subreddit has no recent posts.
- Issues with data parsing.
- Date range filters excluded all submissions.

**Troubleshooting Steps**:

- Check the subreddit to ensure it has recent activity.
- Increase `submission_limit` in `config.ini`.
- Adjust the date range to include a broader period.
- Verify that the program is correctly parsing the JSON data.

#### **No author counts to process.**

**Description**: No data is available to generate the visualization.

**Troubleshooting Steps**:

- Ensure that previous steps completed without errors.
- Check for earlier warnings or errors that might have prevented data collection.

#### **Warning: Skipping a post due to missing author information.**

**Description**: A submission lacked author data and was skipped.

**Troubleshooting Steps**:

- Typically safe to ignore unless occurring frequently.
- If frequent, verify data integrity and consider handling edge cases in the code.

### Visualization Errors

#### **Failed to generate the graphic: _\<error message\>_**

**Description**: An error occurred while creating the visualization.

**Possible Causes**:

- Issues with `matplotlib-cpp` or its dependencies.
- Empty or invalid data sets.

**Troubleshooting Steps**:

- Ensure Python packages are installed and up-to-date:

  ```bash
  pip3 install --user --upgrade matplotlib numpy
  ```

- Verify that `matplotlibcpp.h` is correctly included.
- Check that `top_authors` and `post_counts` contain data.
- Look for previous errors that might have led to empty data sets.

---

## Security Considerations

- **Protect Your Credentials**:
  - The `config.ini` file contains your Reddit API credentials.
  - It is created with permissions set to `600` (read/write for owner only).
  - Do **not** share this file or commit it to version control systems.
- **Add `config.ini` to `.gitignore`**:
  - Create a `.gitignore` file in the project directory (if it doesn't exist).
  - Add the following line to prevent `config.ini` from being tracked by Git:

    ```
    config.ini
    ```

- **Secure Storage**:
  - For production use, consider storing credentials securely using environment variables or a secrets management tool.

---

## Contributing

Contributions are welcome! If you find issues or have suggestions for improvements, please open an issue or submit a pull request.

When contributing, please:

- Follow the existing code style.
- Include documentation for any new features.
- Ensure that the script and program run without errors.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- **[PRAW (Python Reddit API Wrapper)](https://praw.readthedocs.io/)**: Inspiration for Reddit API interaction.
- **[matplotlib-cpp](https://github.com/lava/matplotlib-cpp)**: For providing a C++ interface to Matplotlib.
- **[nlohmann/json](https://github.com/nlohmann/json)**: For JSON parsing in C++.
- **[Reddit API Documentation](https://www.reddit.com/dev/api/)**: For guidelines and API reference.
