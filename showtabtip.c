/*
 * showtabtip - 切换 Windows 触摸键盘 (TabTip.exe)
 *
 * 运行后调用未公开但稳定的 COM 接口 ITipInvocation::Toggle:
 *   - 触摸键盘已显示 -> 隐藏
 *   - 触摸键盘未显示 -> 显示
 *
 * UIHostNoLaunch 这个 COM 类不会主动启动 TabTip.exe,
 * 所以若 TabTip 进程未运行，先拉起它，轮询等待 COM 服务器就绪后再 Toggle。
 */

#include <windows.h>
#include <shlwapi.h>
#include <initguid.h>

/* CLSID_UIHostNoLaunch: {4CE576FA-83DC-4F88-951C-9D0782B4E376} */
DEFINE_GUID(CLSID_UIHostNoLaunch,
    0x4CE576FA, 0x83DC, 0x4F88, 0x95, 0x1C, 0x9D, 0x07, 0x82, 0xB4, 0xE3, 0x76);

/* IID_ITipInvocation: {37C994E7-432B-4834-A2F7-DCE1F13B834B} */
DEFINE_GUID(IID_ITipInvocation,
    0x37C994E7, 0x432B, 0x4834, 0xA2, 0xF7, 0xDC, 0xE1, 0xF1, 0x3B, 0x83, 0x4B);

typedef struct ITipInvocationVtbl ITipInvocationVtbl;

typedef struct ITipInvocation {
    const ITipInvocationVtbl *lpVtbl;
} ITipInvocation;

struct ITipInvocationVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(ITipInvocation *, REFIID, void **);
    ULONG   (STDMETHODCALLTYPE *AddRef)(ITipInvocation *);
    ULONG   (STDMETHODCALLTYPE *Release)(ITipInvocation *);
    HRESULT (STDMETHODCALLTYPE *Toggle)(ITipInvocation *, HWND);
};

/* 尝试创建 ITipInvocation 实例，成功返回接口指针，失败返回 NULL。 */
static ITipInvocation *CreateTip(void)
{
    ITipInvocation *tip = NULL;
    HRESULT hr = CoCreateInstance(&CLSID_UIHostNoLaunch, NULL,
                                  CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER,
                                  &IID_ITipInvocation, (void **)&tip);
    return SUCCEEDED(hr) ? tip : NULL;
}

/* 启动 TabTip.exe(位于 %CommonProgramFiles%\microsoft shared\ink\)。 */
static void LaunchTabTip(void)
{
    WCHAR path[MAX_PATH];
    DWORD n = GetEnvironmentVariableW(L"CommonProgramFiles", path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        /* 回退到常见默认路径 */
        lstrcpynW(path, L"C:\\Program Files\\Common Files", MAX_PATH);
    }
    lstrcatW(path, L"\\microsoft shared\\ink\\TabTip.exe");

    /* SW_HIDE: 仅在后台注册 COM 服务器，实际显隐交给 Toggle */
    ShellExecuteW(NULL, L"open", path, NULL, NULL, SW_HIDE);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, PWSTR cmd, int show)
{
    (void)hInst; (void)hPrev; (void)cmd; (void)show;

    if (FAILED(CoInitialize(NULL)))
        return 1;

    ITipInvocation *tip = CreateTip();

    if (!tip) {
        /* TabTip 未运行：拉起进程，轮询等待 COM 服务器就绪 (最多约 3 秒) */
        LaunchTabTip();
        for (int i = 0; i < 60 && !tip; ++i) {
            Sleep(50);
            tip = CreateTip();
        }
    }

    int rc = 1;
    if (tip) {
        tip->lpVtbl->Toggle(tip, GetDesktopWindow());
        tip->lpVtbl->Release(tip);
        rc = 0;
    }

    CoUninitialize();
    return rc;
}
