#include "get_https.hpp"
#include "load_env.hpp"
#include "json.hpp"
#include <iostream>
#include <curl/curl.h>

using json = nlohmann::json;

// libcurl write callback
static size_t curlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string get_https(const json& processList)
{
    env::load_env_file(".env");

    const char* key = std::getenv("GEMINI_API_KEY");
    if (!key) {
        std::cerr << "GEMINI_API_KEY not set!\n";
        return "";
    }

    const std::string fixedPrompt =
        "You are a resource optimization AI.\n"
        "Analyze the following processes based on:\n"
        "- CPU usage\n"
        "- Memory usage\n"
        "- Network usage\n"
        "- Process name significance\n\n"
        "Return ONLY the PID (just the number) of the single process that is the best candidate to kill.\n"
        "No explanation. No extra text. Only return the PID.\n"
        "Avoid killing critical system processes.";

    json payload;
    payload["contents"] = json::array();

    json part;
    part["parts"] = json::array({
        {
            {"text", fixedPrompt + "\n\nProcesses:\n" + processList.dump()}
        }
    });

    payload["contents"].push_back(part);

    std::string endpoint =
        "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent";

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Curl init failed!\n";
        return "";
    }

    std::string response;
    struct curl_slist* headers = nullptr;

    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string keyHeader = "x-goog-api-key: ";
    keyHeader += key;
    headers = curl_slist_append(headers, keyHeader.c_str());

    std::string body = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Curl error: " << curl_easy_strerror(res) << "\n";
        return "";
    }

    try {
        json resp_json = json::parse(response);
        std::string pid_to_kill =
            resp_json["candidates"][0]["content"]["parts"][0]["text"];
        return pid_to_kill;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse Gemini response: " << e.what() << "\n";
        return "";
    }
}
