#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <getopt.h> // For command-line argument parsing

// Include matplotlib-cpp header
#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;
using json = nlohmann::json;

// Function prototypes
std::string getAccessToken(const std::string& client_id, const std::string& client_secret,
                           const std::string& username, const std::string& password, const std::string& user_agent, bool verbose);
std::string httpRequest(const std::string& url, const std::string& bearer_token, const std::string& user_agent, bool verbose);
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
std::map<std::string, std::string> readConfig(const std::string& filename);
void printUsage();

int main(int argc, char* argv[]) {
    // Default settings
    bool verbose = false;
    std::string rate_limit_option = "default";
    std::string start_date_str;
    std::string end_date_str;

    // Command-line argument parsing
    int opt;
    // Option struct for getopt_long
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"rate", required_argument, 0, 'r'},
        {"start-date", required_argument, 0, 's'},
        {"end-date", required_argument, 0, 'e'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "vr:s:e:h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 'r':
                rate_limit_option = optarg;
                break;
            case 's':
                start_date_str = optarg;
                break;
            case 'e':
                end_date_str = optarg;
                break;
            case 'h':
            default:
                printUsage();
                return EXIT_SUCCESS;
        }
    }

    // Validate rate limit option
    std::map<std::string, int> rate_limits = {
        {"slow", 2000},
        {"default", 1000},
        {"fast", 500},
        {"insane", 0}
    };

    if (rate_limits.find(rate_limit_option) == rate_limits.end()) {
        std::cerr << "Invalid rate limit option. Use one of: slow, default, fast, insane." << std::endl;
        return EXIT_FAILURE;
    }

    // Convert date strings to timestamps
    time_t start_timestamp = 0;
    time_t end_timestamp = 0;
    struct tm tm = {};

    if (!start_date_str.empty()) {
        if (strptime(start_date_str.c_str(), "%Y-%m-%d", &tm)) {
            start_timestamp = mktime(&tm);
        } else {
            std::cerr << "Invalid start date format. Use YYYY-MM-DD." << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (!end_date_str.empty()) {
        if (strptime(end_date_str.c_str(), "%Y-%m-%d", &tm)) {
            end_timestamp = mktime(&tm);
        } else {
            std::cerr << "Invalid end date format. Use YYYY-MM-DD." << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (start_timestamp > end_timestamp && end_timestamp != 0) {
        std::cerr << "Start date cannot be after end date." << std::endl;
        return EXIT_FAILURE;
    }

    try {
        // Read configuration
        auto config = readConfig("config.ini");

        // Reddit API credentials
        std::string client_id = config.at("reddit.client_id");
        std::string client_secret = config.at("reddit.client_secret");
        std::string username = config.at("reddit.username");
        std::string password = config.at("reddit.password");
        std::string user_agent = config.at("reddit.user_agent");

        // Settings
        std::string subreddit_name = config.at("settings.subreddit_name");
        int submission_limit = std::stoi(config.at("settings.submission_limit"));

        // Validate submission_limit
        if (submission_limit <= 0) {
            throw std::invalid_argument("submission_limit must be a positive integer.");
        }

        // Obtain access token
        std::string access_token = getAccessToken(client_id, client_secret, username, password, user_agent, verbose);
        if (access_token.empty()) {
            throw std::runtime_error("Failed to obtain access token.");
        }

        // Subreddit to analyze
        if (subreddit_name.empty()) {
            throw std::invalid_argument("Subreddit name cannot be empty.");
        }

        int max_submissions_per_request = 100; // Reddit API limit per request
        int total_submissions_fetched = 0;
        std::vector<std::string> authors;

        if (verbose) {
            std::cout << "Collecting data from r/" << subreddit_name << "..." << std::endl;
        }

        // Initialize pagination
        std::string after;

        // Rate limit delay in milliseconds
        int rate_delay_ms = rate_limits[rate_limit_option];

        // Fetch submissions in batches
        while (total_submissions_fetched < submission_limit) {
            int submissions_to_fetch = std::min(max_submissions_per_request, submission_limit - total_submissions_fetched);
            std::string url = "https://oauth.reddit.com/r/" + subreddit_name + "/new?limit=" + std::to_string(submissions_to_fetch);
            if (!after.empty()) {
                url += "&after=" + after;
            }

            // Fetch data
            std::string response = httpRequest(url, access_token, user_agent, verbose);
            if (response.empty()) {
                throw std::runtime_error("Failed to fetch data from Reddit API.");
            }

            // Parse JSON response
            json data;
            try {
                data = json::parse(response);
            } catch (const json::parse_error& e) {
                throw std::runtime_error("Failed to parse JSON response: " + std::string(e.what()));
            }

            if (!data.contains("data") || !data["data"].contains("children")) {
                throw std::runtime_error("Unexpected JSON structure.");
            }

            for (const auto& post : data["data"]["children"]) {
                if (post.contains("data") && post["data"].contains("author")) {
                    std::string author = post["data"]["author"].get<std::string>();
                    int created_utc = post["data"]["created_utc"].get<int>();

                    // Check date range
                    if ((start_timestamp == 0 || created_utc >= start_timestamp) &&
                        (end_timestamp == 0 || created_utc <= end_timestamp)) {
                        if (!author.empty()) {
                            authors.push_back(author);
                        }
                    }
                } else {
                    // Handle missing data
                    if (verbose) {
                        std::cerr << "Warning: Skipping a post due to missing author information." << std::endl;
                    }
                }
            }

            total_submissions_fetched += submissions_to_fetch;

            // Check if there are more submissions to fetch
            if (!data["data"].contains("after") || data["data"]["after"].is_null()) {
                break; // No more submissions available
            }

            after = data["data"]["after"].get<std::string>();

            // Rate limiting
            if (rate_delay_ms > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(rate_delay_ms));
            }
        }

        if (verbose) {
            std::cout << "Data collection complete. Fetched " << authors.size() << " submissions." << std::endl;
        }

        if (authors.empty()) {
            throw std::runtime_error("No authors found in the fetched submissions.");
        }

        // Count posts per author
        std::map<std::string, int> author_counts;
        for (const auto& author : authors) {
            author_counts[author]++;
        }

        if (author_counts.empty()) {
            throw std::runtime_error("No author counts to process.");
        }

        // Sort authors by post count
        std::vector<std::pair<std::string, int>> author_vector(author_counts.begin(), author_counts.end());
        std::sort(author_vector.begin(), author_vector.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });

        // Get top N authors
        int top_n = 10;
        if (author_vector.size() < static_cast<size_t>(top_n)) {
            top_n = author_vector.size();
        }

        std::vector<std::string> top_authors;
        std::vector<int> post_counts;

        for (int i = 0; i < top_n; ++i) {
            top_authors.push_back(author_vector[i].first);
            post_counts.push_back(author_vector[i].second);
        }

        // Generate the graphic
        try {
            plt::figure_size(800, 600);
            plt::bar(top_authors, post_counts);
            plt::xlabel("Users");
            plt::ylabel("Number of Posts");
            plt::title("Top " + std::to_string(top_n) + " Posters in r/" + subreddit_name);
            plt::xticks(top_authors);
            plt::xtickangle(45);
            plt::tight_layout();
            plt::save("top_posters.png");
            if (verbose) {
                plt::show();
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to generate the graphic: " + std::string(e.what()));
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Function to obtain the access token
std::string getAccessToken(const std::string& client_id, const std::string& client_secret,
                           const std::string& username, const std::string& password, const std::string& user_agent, bool verbose) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if (curl) {
        std::string credentials = client_id + ":" + client_secret;
        std::string auth_header = "Authorization: Basic " + credentials;

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, auth_header.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        std::string post_fields = "grant_type=password&username=" + username +
                                  "&password=" + password;

        curl_easy_setopt(curl, CURLOPT_URL, "https://www.reddit.com/api/v1/access_token");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        if (verbose) {
            std::cout << "Requesting access token..." << std::endl;
        }

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "CURL Error in getAccessToken: " << curl_easy_strerror(res) << std::endl;
            readBuffer.clear();
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        throw std::runtime_error("Failed to initialize CURL for getAccessToken.");
    }

    if (readBuffer.empty()) {
        throw std::runtime_error("Empty response received in getAccessToken.");
    }

    json token_json;
    try {
        token_json = json::parse(readBuffer);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON in getAccessToken: " + std::string(e.what()));
    }

    if (token_json.contains("access_token")) {
        return token_json["access_token"];
    } else {
        if (token_json.contains("error")) {
            throw std::runtime_error("Error obtaining access token: " + token_json["error"].get<std::string>());
        } else {
            throw std::runtime_error("Unknown error obtaining access token.");
        }
    }
}

// Function to make authenticated HTTP requests
std::string httpRequest(const std::string& url, const std::string& bearer_token, const std::string& user_agent, bool verbose) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if (curl) {
        std::string auth_header = "Authorization: bearer " + bearer_token;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, auth_header.c_str());
        headers = curl_slist_append(headers, ("User-Agent: " + user_agent).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Set timeout options
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L); // 10 seconds to connect
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);        // 30 seconds total timeout

        if (verbose) {
            std::cout << "Requesting URL: " << url << std::endl;
        }

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "CURL Error in httpRequest: " << curl_easy_strerror(res) << std::endl;
            readBuffer.clear();
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        throw std::runtime_error("Failed to initialize CURL for httpRequest.");
    }

    if (readBuffer.empty()) {
        throw std::runtime_error("Empty response received in httpRequest.");
    }

    return readBuffer;
}

// Callback function for libcurl
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    try {
        ((std::string*)userp)->append((char*)contents, totalSize);
    } catch (const std::bad_alloc& e) {
        // Handle memory allocation errors
        std::cerr << "Memory allocation error in writeCallback: " << e.what() << std::endl;
        return 0; // Returning 0 will cause curl to abort
    }
    return totalSize;
}

// Function to read config file
std::map<std::string, std::string> readConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open configuration file: " + filename);
    }

    std::map<std::string, std::string> config;
    std::string line;
    std::string current_section;

    while (std::getline(file, line)) {
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;

        if (line.front() == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }

        auto delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            // Remove whitespace around key and value
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            config[current_section + "." + key] = value;
        }
    }

    if (config.empty()) {
        throw std::runtime_error("Configuration file is empty or invalid.");
    }

    return config;
}

// Function to print usage information
void printUsage() {
    std::cout << "Usage: reddit_bot [options]\n\n"
              << "Options:\n"
              << "  -v, --verbose            Enable verbose output\n"
              << "  -r, --rate [option]      Set rate limit (options: slow, default, fast, insane)\n"
              << "  -s, --start-date [date]  Set start date (format: YYYY-MM-DD)\n"
              << "  -e, --end-date [date]    Set end date (format: YYYY-MM-DD)\n"
              << "  -h, --help               Display this help message\n";
}
