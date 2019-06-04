# Alt Tab Royal (Work in progress)

This is not easily usable yet. But it works.

TODO: Details to be written here...

Currently the requirement is to have Virtual Desktop Accessories DLL in the location `C:\\Source\\CandCPP\\VirtualDesktopAccessor\\x64\\Debug\\VirtualDesktopAccessor.dll`

License MIT, Copyright 2019 Jari Pennanen, see LICENSE.txt for copying.

## Todo

- Remove raw pointers
- Remove assignment and copy constructors
- Get grips with moving constuctor


## Proof of concept AHK

```ahk
#SingleInstance, force

OnExit, ExitClean

Run, "C:\Source\CandCPP\AltTabRoyalCpp\x64\Release\AltTabRoyal.exe", C:\Source\CandCPP\AltTabRoyalCpp\x64\Release
Sleep, 100

DetectHiddenWindows, On
WinGet, hwnd, ID, ahk_exe AltTabRoyal.exe
if (!hwnd) {
    MsgBox, Could not start alt tab royal
    ExitApp
    return
}

!Tab::
    AltTabber(1)
    return

!+Tab::
    AltTabber(-1)
    return

return

AltTabber(dir) {
    global hwnd
    Critical, 500
    SetTimer, ListenAltKeyUp, 30
    if (dir == 1) {
        PostMessage, 0x400+1,0,0,, ahk_id %hwnd%
        PostMessage, 0x400+4,0,0,, ahk_id %hwnd%
    } else {
        PostMessage, 0x400+1,0,0,, ahk_id %hwnd%
        PostMessage, 0x400+5,0,0,, ahk_id %hwnd%
    }
}

ListenAltKeyUp:
    global hwnd
    if (!GetKeyState("Alt", "P")) {
        PostMessage, 0x400+6,0,0,, ahk_id %hwnd%
        PostMessage, 0x400+2,0,0,, ahk_id %hwnd%
        SetTimer, ListenAltKeyUp, Off
    }
    return

ExitClean:
    Process,Close,AltTabRoyal.exe
    ExitApp
    return
```