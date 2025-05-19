#define TRAY_WINAPI 1

#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include <Powerbase.h>
extern "C" {
    #include <Powrprof.h>
#include <string>
}
#pragma comment(lib, "Powrprof.lib")


static GUID const CLSID_RadioManagementAPI = { 0x581333f6, 0x28db, 0x41be, { 0xbc, 0x7a, 0xff, 0x20, 0x1f, 0x12, 0xf3, 0xf6 } };
static GUID const CID_IRadioManager = { 0xdb3afbfb, 0x08e6, 0x46c6, { 0xaa, 0x70, 0xbf, 0x9a, 0x34, 0xc3, 0x0a, 0xb7 } };

typedef IUnknown IUIRadioInstanceCollection;
typedef DWORD _RADIO_CHANGE_REASON;

typedef struct IRadioManagerVtbl IRadioManagerVtbl;
typedef struct IRadioManager {
    IRadioManagerVtbl* lpVtbl;
} IRadioManager;
struct IRadioManagerVtbl {
    /* IUnknown */
    HRESULT(STDMETHODCALLTYPE* QueryInterface)(IRadioManager* This, GUID const* riid, LPVOID* ppvObj);
    ULONG(STDMETHODCALLTYPE* AddRef)(IRadioManager* This);
    ULONG(STDMETHODCALLTYPE* Release)(IRadioManager* This);
    /* IRadioManager (aka. `CUIRadioManager') */
    HRESULT(STDMETHODCALLTYPE* IsRMSupported)(IRadioManager* This, DWORD* pdwState);
    HRESULT(STDMETHODCALLTYPE* GetUIRadioInstances)(IRadioManager* This, IUIRadioInstanceCollection** param_1);
    HRESULT(STDMETHODCALLTYPE* GetSystemRadioState)(IRadioManager* This, int* pbEnabled, int* param_2, _RADIO_CHANGE_REASON* param_3);
    HRESULT(STDMETHODCALLTYPE* SetSystemRadioState)(IRadioManager* This, int bEnabled);
    HRESULT(STDMETHODCALLTYPE* Refresh)(IRadioManager* This);
    HRESULT(STDMETHODCALLTYPE* OnHardwareSliderChange)(IRadioManager* This, int param_1, int param_2);
};

// Switch AirPlane Mode
void SetAirPlaneMode(bool isEnable) {
    HRESULT hr;
    IRadioManager* irm;
    hr = CoInitialize(NULL);
    assert(!FAILED(hr));

    irm = NULL;
    hr = CoCreateInstance(CLSID_RadioManagementAPI, NULL, 4,
        CID_IRadioManager, (void**)&irm);
    assert(!FAILED(hr) && irm);

    /* Set flight mode to the opposite state. */
    hr = irm->lpVtbl->SetSystemRadioState(irm, isEnable);
    assert(!FAILED(hr));
    irm->lpVtbl->Release(irm);
    CoUninitialize();
}

// Callback
ULONG CALLBACK DeviceNotifyCallbackRoutine(
    _In_opt_ PVOID Context,
    ULONG          Type,
    PVOID          Setting
) {
    if (Type == PBT_APMRESUMEAUTOMATIC) {
        // Resume
        printf("AirPlane Disabled\n");
        SetAirPlaneMode(true);
    }
    if (Type == PBT_APMSUSPEND) {
        // Suspend
        printf("AirPlane Enabled\n");
        SetAirPlaneMode(false);
    }

    return 0;
}

void RegisterNotification() {
    DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS parameters = { DeviceNotifyCallbackRoutine, nullptr };

    HPOWERNOTIFY notify;
    PowerRegisterSuspendResumeNotification(DEVICE_NOTIFY_CALLBACK, &parameters, &notify);
}

int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        std::string str = argv[i];
        if (str == "h") {
            FreeConsole();
        }
    }


    RegisterNotification();

    printf("Started\n");
    while (true) {
        Sleep(10);
    }
}