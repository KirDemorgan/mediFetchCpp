#include <iostream>
#include <string>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <fcntl.h>
#include <io.h>
#include <Windows.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Media::Control;

IAsyncAction GetMediaInfo() {
    auto sessionManager = co_await GlobalSystemMediaTransportControlsSessionManager::RequestAsync();
    auto currentSession = sessionManager.GetCurrentSession();

    if (currentSession) {
        auto mediaProperties = co_await currentSession.TryGetMediaPropertiesAsync();

        if (mediaProperties) {
            std::wstring artist = mediaProperties.Artist().c_str();
            std::wstring title = mediaProperties.Title().c_str();

            if (artist.empty() && title.empty()) {
                std::wcout << L"���������� � ����� ���������� (��������, ������� ��� �����)." << std::endl;
            } else {
                std::wcout << L"������ ������:" << std::endl;
                std::wcout << L"�����������: " << (artist.empty() ? L"����������" : artist) << std::endl;
                std::wcout << L"��������: " << (title.empty() ? L"��� ��������" : title) << std::endl;
            }
        }
    } else {
        std::wcout << L"� ������ ������ ������ �� ������." << std::endl;
    }
}

int main() {
    winrt::init_apartment();
    _setmode(_fileno(stdout), _O_U16TEXT);
    std::wcout << L"���������� ���������� � ������� �����..." << std::endl;

    try {
        GetMediaInfo().get();
    }
    catch (const winrt::hresult_error& ex) {
        std::wcerr << L"��������� ������: " << ex.message().c_str() << std::endl;
    }

    return 0;
}