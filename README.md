# frida-gum-cmake

#### 介绍

frida-gum：https://github.com/frida/frida-gum

frida-gum-cmake是基于[frida-gum](https://github.com/frida/frida-gum)的CMake构建工具。

目前仅对以下系统版本做了适配：

+ [x] Android
+ [x] Windows
+ [x] Linux
+ [ ] macOS
+ [ ] iOS

#### CMake编译选项

+ `FRIDA-GUM-CMAKE_BUILD_GUMPP`：是否编译gumpp，默认为`OFF`。
+ `FRIDA-GUM-CMAKE_BUILD_GUM`：是否编译gum，默认为`ON`。
+ `FRIDA-GUM-CMAKE_BUILD_GUMJS`：是否编译gumjs，默认为`OFF`。
+ `FRIDA-GUM-CMAKE_BUILD_TEST`：是否编译测试，默认为`OFF`。

#### 使用

在父级CMakeLists.txt中添加：

```cmake
add_directory("path_to/frida-gum-cmake")

target_link_libraries(your_target frida-gum) # frida-gumpp, frida-gum, frida-gumjs
```

代码例子请看：[src/main.cc](https://github.com/Bzi-Han/frida-gum-cmake/blob/main/src/main.cc)

输出：

```log
HookListener::on_enter TestHook
  listener num_calls: 1
  arg00: "Hello, world!"

TestHook called with: "Hello, world! from hook"

HookListener::on_leave TestHook
  returnValue: false

TestHook result: true
```
