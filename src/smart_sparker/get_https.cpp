#include "get_https.hpp"
#include "load_env.hpp"
#include <iostream>
#include <string>
#include <curl/curl.h>
#include "json.hpp"

using json = nlohmann::json;

// libcurl write callback
static size_t curlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int get_https(const std::string& prompt) 
{
    // Load .env
    env::load_env_file(".env");

    // Get API key
    const char* key = std::getenv("GEMINI_API_KEY");
    if (!key) {
        std::cerr << "GEMINI_API_KEY not set!\n";
        return 1;
    }

    // Build request JSON
    json payload;
    payload["contents"] = json::array();
    json part;
    part["parts"] = json::array({ { {"text", prompt} } });
    payload["contents"].push_back(part);

    std::string endpoint = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent";

    // Initialize curl
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Curl init failed!\n";
        return 1;
    }

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string keyHeader = "x-goog-api-key: ";
    keyHeader += key;  // For Gemini, API keys may be passed as Bearer
    headers = curl_slist_append(headers, keyHeader.c_str());

    std::string body = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Curl error: " << curl_easy_strerror(res) << "\n";
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Print the raw JSON response
    std::cout << "Raw response:\n" << response << "\n";

    // Parse JSON (if Gemini returns JSON)
    try {
        json resp_json = json::parse(response);
        std::cout << "\nParsed response:\n" << resp_json.dump(2) << "\n";
    } catch (...) {
        std::cerr << "Could not parse response as JSON.\n";
    }

    return 0;
}
