set dir=%CD%
cd C:\"Program Files <x86>\Google\Chrome\Application"
IF EXIST chrome.exe (
   chrome.exe --kiosk --allow-file-access-from-files file://%dir%\viewer.html?game=%1
)
ELSE (
   cd C:\"Program Files <x86>\Mozilla Firefox"
   firefox.exe --kiosk --allow-file-access-from-files file://%dir%\viewer.html?game=%1
)
cd %dir%
