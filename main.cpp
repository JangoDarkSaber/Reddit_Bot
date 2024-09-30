#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <streambuf>
#include <curl/curl.h>
#include <json.hpp>            // For JSON parsing
#include "matplotlibcpp.h"     // For plotting
#include <algorithm>           // For std::sort
#include <numeric>             // For std::iota
#include <getopt.h>            // For command-line options

namespace plt = matplotlibcpp;
using json = nlohmann::json;

// Function to handle writing data received from CURL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* str = (std::string*)userp;
    str->append((char*)contents, totalSize);
    return totalSize;
}

// Function to read configuration from config.ini
bool readConfig(const std::string& filename, std::unordered_map<std::string, std::string>& configMap) {
    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        std::cerr << "Failed to open configuration file: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::string currentSection;
    while (std::getline(configFile, line)) {
        // Remove whitespace
        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;  // Skip comments and empty lines

        if (line.front() == '[' && line.back() == ']') {
            // Section header
            currentSection = line.substr(1, line.size() - 2);
        } else {
            // Key-value pair
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = currentSection + "." + line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                configMap[key] = value;
            }
        }
    }
    configFile.close();
    return true;
}

// Function to get access token from Reddit API
std::string getAccessToken(const std::unordered_map<std::string, std::string>& configMap) {
    std::string clientId = configMap.at("reddit.client_id");
    std::string clientSecret = configMap.at("reddit.client_secret");
    std::string username = configMap.at("reddit.username");
    std::string password = configMap.at("reddit.password");
    std::string userAgent = configMap.at("reddit.user_agent");

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL for getAccessToken." << std::endl;
        return "";
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.reddit.com/api/v1/access_token");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERPWD, (clientId + ":" + clientSecret).c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());

    std::string postFields = "grant_type=password&username=" + username + "&password=" + password;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL Error in getAccessToken: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return "";
    }

    curl_easy_cleanup(curl);

    if (readBuffer.empty()) {
        std::cerr << "Empty response received in getAccessToken." << std::endl;
        return "";
    }

    json jsonResponse = json::parse(readBuffer);
    if (jsonResponse.contains("access_token")) {
        return jsonResponse["access_token"];
    } else {
        std::cerr << "Error obtaining access token: " << jsonResponse.dump() << std::endl;
        return "";
    }
}

// Function to make HTTP GET requests with bearer token
std::string httpRequest(const std::string& url, const std::string& bearerToken, const std::string& userAgent) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL for httpRequest." << std::endl;
        return "";
    }

    std::string readBuffer;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + bearerToken).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL Error in httpRequest: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return "";
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (readBuffer.empty()) {
        std::cerr << "Empty response received in httpRequest." << std::endl;
        return "";
    }

    return readBuffer;
}

// Function to parse date string to UNIX timestamp
time_t parseDate(const std::string& dateStr) {
    struct tm tm = {};
    if (strptime(dateStr.c_str(), "%Y-%m-%d", &tm) == nullptr) {
        return -1;
    }
    return mktime(&tm);
}

int main(int argc, char* argv[]) {
    try {
        // Command-line options
        bool verbose = false;
        std::string rateLimitOption = "default";
        std::string startDateStr;
        std::string endDateStr;

        const char* const short_opts = "vr:s:e:h";
        const option long_opts[] = {
            {"verbose", no_argument, nullptr, 'v'},
            {"rate", required_argument, nullptr, 'r'},
            {"start-date", required_argument, nullptr, 's'},
            {"end-date", required_argument, nullptr, 'e'},
            {"help", no_argument, nullptr, 'h'},
            {nullptr, no_argument, nullptr, 0}
        };

        while (true) {
            const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
            if (-1 == opt)
                break;

            switch (opt) {
                case 'v':
                    verbose = true;
                    break;
                case 'r':
                    rateLimitOption = optarg;
                    break;
                case 's':
                    startDateStr = optarg;
                    break;
                case 'e':
                    endDateStr = optarg;
                    break;
                case 'h':
                case '?':
                default:
                    std::cout << "Usage: " << argv[0] << " [options]\n\n"
                              << "Options:\n"
                              << "  -v, --verbose            Enable verbose output\n"
                              << "  -r, --rate [option]      Set rate limit (options: slow, default, fast, insane)\n"
                              << "  -s, --start-date [date]  Set start date (format: YYYY-MM-DD)\n"
                              << "  -e, --end-date [date]    Set end date (format: YYYY-MM-DD)\n"
                              << "  -h, --help               Display this help message\n";
                    return 0;
            }
        }

        // Set rate limit based on option
        int rateLimitMs;
        if (rateLimitOption == "slow") {
            rateLimitMs = 2000;
        } else if (rateLimitOption == "default") {
            rateLimitMs = 1000;
        } else if (rateLimitOption == "fast") {
            rateLimitMs = 500;
        } else if (rateLimitOption == "insane") {
            rateLimitMs = 0;
        } else {
            std::cerr << "Invalid rate limit option. Use one of: slow, default, fast, insane." << std::endl;
            return 1;
        }

        // Parse dates if provided
        time_t startDate = 0;
        time_t endDate = std::time(nullptr);  // Default to current time

        if (!startDateStr.empty()) {
            startDate = parseDate(startDateStr);
            if (startDate == -1) {
                std::cerr << "Invalid start date format. Use YYYY-MM-DD." << std::endl;
                return 1;
            }
        }

        if (!endDateStr.empty()) {
            endDate = parseDate(endDateStr);
            if (endDate == -1) {
                std::cerr << "Invalid end date format. Use YYYY-MM-DD." << std::endl;
                return 1;
            }
        }

        if (startDate > endDate) {
            std::cerr << "Start date cannot be after end date." << std::endl;
            return 1;
        }

        // Read configuration
        std::unordered_map<std::string, std::string> configMap;
        if (!readConfig("config.ini", configMap)) {
            return 1;
        }

        // Validate configuration
        if (configMap.empty()) {
            std::cerr << "Configuration file is empty or invalid." << std::endl;
            return 1;
        }

        std::string subreddit = configMap["settings.subreddit_name"];
        std::string userAgent = configMap["reddit.user_agent"];
        int submissionLimit = std::stoi(configMap["settings.submission_limit"]);

        if (subreddit.empty()) {
            std::cerr << "Subreddit name cannot be empty." << std::endl;
            return 1;
        }

        if (submissionLimit <= 0) {
            std::cerr << "submission_limit must be a positive integer." << std::endl;
            return 1;
        }

        // Get access token
        std::string accessToken = getAccessToken(configMap);
        if (accessToken.empty()) {
            return 1;
        }

        if (verbose) {
            std::cout << "Access token obtained successfully." << std::endl;
        }

        // Fetch submissions
        std::unordered_map<std::string, int> authorCounts;
        int afterCount = 0;
        std::string after;

        while (authorCounts.size() < static_cast<size_t>(submissionLimit)) {
            std::string url = "https://oauth.reddit.com/r/" + subreddit + "/new?limit=100";
            if (!after.empty()) {
                url += "&after=" + after;
            }

            std::string response = httpRequest(url, accessToken, userAgent);
            if (response.empty()) {
                std::cerr << "Failed to fetch data from Reddit API." << std::endl;
                return 1;
            }

            json jsonResponse = json::parse(response);
            if (!jsonResponse.contains("data") || !jsonResponse["data"].contains("children")) {
                std::cerr << "Unexpected JSON structure." << std::endl;
                return 1;
            }

            auto posts = jsonResponse["data"]["children"];
            if (posts.empty()) {
                break;  // No more posts
            }

            for (const auto& post : posts) {
                auto data = post["data"];
                if (data.contains("author") && !data["author"].is_null()) {
                    std::string author = data["author"].get<std::string>();
                    time_t postTime = data["created_utc"].get<time_t>();

                    if (startDate != 0 && postTime < startDate) {
                        continue;  // Skip posts before start date
                    }
                    if (endDate != 0 && postTime > endDate) {
                        continue;  // Skip posts after end date
                    }

                    authorCounts[author]++;
                } else {
                    if (verbose) {
                        std::cerr << "Warning: Skipping a post due to missing author information." << std::endl;
                    }
                }
            }

            if (jsonResponse["data"].contains("after") && !jsonResponse["data"]["after"].is_null()) {
                after = jsonResponse["data"]["after"].get<std::string>();
            } else {
                break;  // No more pages
            }

            afterCount += posts.size();
            if (afterCount >= submissionLimit) {
                break;
            }

            // Rate limiting
            std::this_thread::sleep_for(std::chrono::milliseconds(rateLimitMs));
        }

        if (authorCounts.empty()) {
            std::cerr << "No authors found in the fetched submissions." << std::endl;
            return 1;
        }

        // Convert the unordered_map to a vector of pairs
        std::vector<std::pair<std::string, int>> authorPostCounts(authorCounts.begin(), authorCounts.end());

        // Sort the vector by post counts in descending order
        std::sort(authorPostCounts.begin(), authorPostCounts.end(),
                  [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                      return a.second > b.second;
                  });

        // Get the top N authors (e.g., top 10)
        size_t topN = 10;
        if (authorPostCounts.size() < topN) {
            topN = authorPostCounts.size();
        }

        std::vector<std::string> topAuthors;
        std::vector<int> postCounts;

        for (size_t i = 0; i < topN; ++i) {
            topAuthors.push_back(authorPostCounts[i].first);
            postCounts.push_back(authorPostCounts[i].second);
        }

        // Generate numerical positions for x-axis
        std::vector<int> xPositions(topN);
        std::iota(xPositions.begin(), xPositions.end(), 0);

        // Plot the bar chart
        plt::bar(xPositions, postCounts);

        // Set x-ticks and labels
        plt::xticks(xPositions, topAuthors);
        plt::xlabel("Authors");
        plt::ylabel("Number of Posts");
        plt::title("Top Reddit Posters in r/" + subreddit);

        // Rotate x-axis labels
        plt::xticks(xPositions, topAuthors);
        plt::xticks(xPositions, topAuthors, {{"rotation", "45"}});

        // Adjust layout to prevent label cutoff
        plt::tight_layout();

        // Save the plot
        plt::save("top_posters.png");

        if (verbose) {
            std::cout << "Plot saved as top_posters.png" << std::endl;
        }

        return 0;

    } catch (const std::exception& ex) {
        std::cerr << "An error occurred: " << ex.what() << std::endl;
        return 1;
    }
}
