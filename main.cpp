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
                std::wcout << L"Информация о треке недоступна (возможно, реклама или пауза)." << std::endl;
            } else {
                std::wcout << L"Сейчас играет:" << std::endl;
                std::wcout << L"Исполнитель: " << (artist.empty() ? L"Неизвестен" : artist) << std::endl;
                std::wcout << L"Название: " << (title.empty() ? L"Без названия" : title) << std::endl;
            }
        }
    } else {
        std::wcout << L"В данный момент ничего не играет." << std::endl;
    }
}

int main() {
    winrt::init_apartment();
    _setmode(_fileno(stdout), _O_U16TEXT);
    std::wcout << L"Запрашиваю информацию о текущем треке..." << std::endl;

    try {
        GetMediaInfo().get();
    }
    catch (const winrt::hresult_error& ex) {
        std::wcerr << L"Произошла ошибка: " << ex.message().c_str() << std::endl;
    }

    return 0;
}