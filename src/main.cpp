#include "pch.h"
#include <fstream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>

using namespace winrt;
using namespace Windows::Media::Control;

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

void CreateJsonFile(const std::wstring& title, const std::wstring& artist) {
    std::wofstream file("../../track.json");
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));
    if (file.is_open()) {
        file << L"{ \"title\": \"" << escape_json(title) << L"\", \"artist\": \"" << escape_json(artist) << L"\" }";
    }
}

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
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}