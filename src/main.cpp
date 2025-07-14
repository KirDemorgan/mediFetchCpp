#include "pch.h"
#include <fstream>
#include <string>
#include <sstream>
#include <locale>

using namespace winrt;
using namespace Windows::Media::Control;

// Function to replace special characters in JSON
std::wstring escape_json(const std::wstring& s) {
    std::wstringstream ss;
    for (auto c : s) {
        switch (c) {
            case L'\"': ss << L"\\\""; break;
            case L'\\': ss << L"\\\\"; break;
            case L'\b': ss << L"\\b"; break;
            case L'\f': ss << L"\\f"; break;
            case L'\n': ss << L"\\n"; break;
            case L'\r': ss << L"\\r"; break;
            case L'\t': ss << L"\\t"; break;
            default: ss << c; break;
        }
    }
    return ss.str();
}

// Create or update the HTML file for OBS
void CreateHtmlFile() {
    std::wofstream file("../../track.html");
    if (file.is_open()) {
        file << L"<!DOCTYPE html>"
             << L"<html><head><meta charset=\"UTF-8\">"
             << L"<title>Now Playing</title>"
             << L"<style>"
             << L"@import url('https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap');"
             << L"body { background-color: rgba(0, 0, 0, 0); margin: 0; padding: 0; font-family: 'Roboto', sans-serif; color: white; text-shadow: 2px 2px 4px #000000; }"
             << L".container { position: fixed; bottom: 20px; left: 20px; background-color: rgba(0, 0, 0, 0.5); padding: 15px; border-radius: 10px; width: 350px; }"
             << L"#title { font-size: 24px; font-weight: 700; }"
             << L"#artist { font-size: 18px; font-weight: 400; }"
             << L"</style>"
             << L"<script>"
             << L"function updateTrack() { fetch('track.json?t=' + new Date().getTime()).then(response => response.json()).then(data => { document.getElementById('title').innerText = data.title; document.getElementById('artist').innerText = data.artist; }).catch(err => console.error(err)); }"
             << L"setInterval(updateTrack, 1000);"
             << L"window.onload = updateTrack;"
             << L"</script>"
             << L"</head><body>"
             << L"<div class=\"container\">"
             << L"<div id=\"title\">...</div>"
             << L"<div id=\"artist\">...</div>"
             << L"</div>"
             << L"</body></html>";
    }
}

// Create or update the JSON file with track info
void CreateJsonFile(const std::wstring& title, const std::wstring& artist) {
    std::wofstream file("../../track.json");
    if (file.is_open()) {
        file.imbue(std::locale("", std::locale::ctype));
        file << L"{ \"title\": \"" << escape_json(title) << L"\", \"artist\": \"" << escape_json(artist) << L"\" }";
    }
}

// Main function
int main()
{
    init_apartment();
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());
    std::wcerr.imbue(std::locale());

    GlobalSystemMediaTransportControlsSessionManager gsmtc = nullptr;
    try {
        gsmtc = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
    } catch (const hresult_error& ex) {
        std::wcerr << L"[FATAL] Failed to get GSMTC manager: " << ex.message().c_str() << std::endl;
        return 1;
    }

    std::wstring currentTitle = L"";
    std::wstring currentArtist = L"";
    
    CreateHtmlFile();
    CreateJsonFile(L"Nothing is playing", L"...");

    while (true) {
        std::wcout << L"[LOG] Loop start" << std::endl;
        try {
            auto session = gsmtc.GetCurrentSession();
            if (session) {
                auto info = session.TryGetMediaPropertiesAsync().get();
                if (currentTitle != info.Title() || currentArtist != info.Artist()) {
                    currentTitle = info.Title();
                    currentArtist = info.Artist();

                    std::wcout << L"[LOG] Now Playing: " << currentTitle << L" - " << currentArtist << std::endl;
                    CreateJsonFile(currentTitle, currentArtist);
                }
            } else {
                 if (currentTitle != L"Nothing is playing") {
                    currentTitle = L"Nothing is playing";
                    currentArtist = L"...";
                    std::wcout << L"[LOG] Nothing is playing" << std::endl;
                    CreateJsonFile(currentTitle, currentArtist);
                 }
            }
        } catch (const hresult_error& ex) {
            std::wcerr << L"[ERROR] Error updating track info: " << ex.message().c_str() << std::endl;
            if (currentTitle != L"Error") {
                currentTitle = L"Error";
                currentArtist = L"Could not get track info";
                CreateJsonFile(currentTitle, currentArtist);
            }
        }
        std::wcout << L"[LOG] Loop end, sleeping..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}