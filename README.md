<!--
原始 README（初始需求记录，保留备查）：

# Show Tabtip

> 文本提到的触摸键盘指的是`tabtip.exe`不是`osk.exe`

因为在 Windows 环境下使用数位屏时点击任务栏的触摸键盘的按钮不太方便（特别是还开了自动隐藏任务栏）
且 Windows 没有提供打开触摸键盘的快捷键
所以我希望能有一个单文件的程序，运行后自动打开触摸键盘，如果触摸键盘已经打开则关闭触摸键盘
当前电脑中没有任何编程语言的开发环境，请你可以告诉我有哪些开发工具链的硬盘占用和对系统环境的污染比较小，我来抉择
-->

# Show Tabtip

> 文本提到的触摸键盘指的是 `tabtip.exe`，不是 `osk.exe`

一个单文件、零依赖的 Windows 小程序：运行后**切换**触摸键盘——已打开则关闭，未打开则打开。

在使用数位屏时点击任务栏的触摸键盘按钮不太方便（特别是开了自动隐藏任务栏），而 Windows 又没有提供打开触摸键盘的快捷键。本程序把这个操作变成一次可执行文件调用，可绑定到数位屏驱动的实体按键、快捷方式或任意启动器上。

## 原理

程序调用未公开但稳定的 COM 接口 `ITipInvocation::Toggle`（`CLSID_UIHostNoLaunch`）。该接口本身就是**切换**语义，无需手动检测键盘窗口状态，比查找窗口类更可靠。

由于 `UIHostNoLaunch` 这个 COM 类不会主动启动 `TabTip.exe`，若其进程未运行，程序会先静默拉起它，轮询等待 COM 服务器就绪（最多约 3 秒）后再切换。

## 使用

直接运行 `showtabtip.exe` 即可切换触摸键盘。产物约 16.5 KB，静态链接、零运行时依赖，可拷贝到任意 Windows 10/11 x64 机器运行。

建议将其绑定到数位屏驱动的实体按键上，实现一键呼出/收起触摸键盘。

## 编译

工具链使用 [w64devkit](https://github.com/skeeto/w64devkit)（绿色便携，解压即用，不写注册表、不改系统 PATH），已放在 `tools/w64devkit/`。

```bash
tools/w64devkit/bin/gcc.exe -O2 -s -mwindows showtabtip.c -o showtabtip.exe -lole32 -lshlwapi
```

- `-mwindows`：生成无控制台窗口的 GUI 程序
- `-O2 -s`：优化并去除符号表，压小体积
- `-lole32 -lshlwapi`：链接 COM 与 Shell 辅助库

## 项目结构

```text
showtabtip/
├── showtabtip.c              源码
├── showtabtip.exe            编译产物（单文件，可分发）
├── README.md
├── .vscode/
│   └── c_cpp_properties.json IntelliSense 配置（指向 w64devkit gcc）
└── tools/
    └── w64devkit/            绿色工具链，删除即卸载
```

> `.vscode/c_cpp_properties.json` 仅用于让编辑器的 C/C++ 插件按 GCC 规则解析代码，消除 COM 宏相关的 IntelliSense 误报，不影响编译。

## 许可证

本项目采用 [GNU General Public License v3.0](LICENSE) 授权，完整条款见 [LICENSE](LICENSE) 文件。
