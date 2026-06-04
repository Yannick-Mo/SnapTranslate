# SnapTranslate

A desktop screenshot translation tool built with **C++ / Qt 6 + CMake**.

## Features

- **Hotkey screenshot** — Press `Alt+Q` to capture a screen region
- **OCR recognition** — Baidu OCR (auto-fallback to EasyOCR / Tianyi OCR)
- **Translation** — Baidu Translate API, supports multiple target languages
- **Floating result window** — Tabbed view (original / translated), drag-able, copy buttons
- **Clipboard monitor** — Auto-translate copied text
- **Sticky notes** — Pin text or screenshots to desktop
- **History** — Local JSON-based translation history viewer
- **System tray** — Background running with tray menu

## Prerequisites

- Qt 6.x (Core, Gui, Widgets, Network)
- CMake 3.20+
- C++17 compiler (MSVC 2022, GCC 11+, Clang)

## Build

```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=<path-to-Qt6>/lib/cmake
cmake --build . --config Release
```

The built binary will be at `build/Release/screenshot-translator.exe`.

## Configuration

Copy `config.json.template` to `config.json` and fill in your API credentials:

- **Baidu OCR** — [Apply here](https://console.bce.baidu.com/ai/#/ai/ocr/overview/index)
- **Baidu Translate** — [Apply here](https://fanyi-api.baidu.com/api/trans/product/index)
- **EasyOCR** (optional) — [console.easyocr.org](https://console.easyocr.org)

```json
{
    "ocr": {
        "api_key": "your_baidu_ocr_api_key",
        "secret_key": "your_baidu_ocr_secret_key"
    },
    "translate": {
        "app_id": "your_baidu_translate_app_id",
        "secret_key": "your_baidu_translate_secret_key"
    },
    "general": {
        "hotkey": "Alt+Q",
        "target_language": "zh",
        "auto_copy": true,
        "show_notification": true
    }
}
```

## Project Structure

```
├── CMakeLists.txt
├── resources/
│   ├── icon.png       # Tray icon
│   ├── icon.ico
│   ├── icon.rc
│   ├── resources.qrc
│   └── style.qss
└── src/
    ├── main.cpp
    ├── MainController.h/cpp     # App controller
    ├── ScreenshotWidget.h/cpp   # Screen capture overlay
    ├── OcrEngine.h/cpp          # Baidu / EasyOCR / Tianyi OCR
    ├── TranslateEngine.h/cpp    # Baidu Translate API
    ├── ResultWidget.h/cpp       # Floating result window
    ├── TrayIcon.h/cpp           # System tray menu
    ├── HotkeyManager.h/cpp      # Global hotkey (QHotkey)
    ├── ClipboardListener.h/cpp  # Clipboard monitor
    ├── StickyNoteWidget.h/cpp   # Sticky notes
    ├── SettingsDialog.h/cpp     # Settings window
    ├── HistoryDialog.h/cpp      # Translation history
    └── Utils.h/cpp              # Utility functions
```
