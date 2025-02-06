# 1. 前言
本项目为《[Crafting Interpreters](https://craftinginterpreters.com/)》一书中`clox`的实现。我实现了执行`lox`脚本所需的词法解析器，语法解析器，虚拟机以及垃圾回收等功能。
> `lox`是一个简单且图灵完备的编程语言。没有太多的语法特性，但是它有完整的控制流和函数，且支持闭包和面向对象的特性。用它来构建一个简单的解释器再合适不过了。
# 2. 安装
`clox` 是一个C++项目（严格来说只用到了C的语法:)），因此你需要安装一些工具。
## 2.1 安装cmake
在`clox`中，我们使用`cmake`来管理整个工程。因此，你需要先安装`cmake`。可以在`cmake`的官网下载。版本要求是`3.10`。
## 2.2 安装C++编译器
`clox`使用`C++`作为主要编程语言。因此，你需要安装一个支持`C++`的编译器。在Windows平台请先安装MSVC，在Linux平台请先安装GCC。
## 2.3 建立build目录
在`clox`的根目录下，建立`build`目录。用于存放编译后的产物。
> 为了防止编译产物和源码混淆，所以在根目录下建立`build`目录。
## 2.4 编译
在`build`的目录下，执行以下命令：
```bash
cmake ..
cmake --build ./ --target clox -j 10
```
运行完成后，在`build/build/Debug`目录下，会生成`clox`的可执行文件。

<img src="./doc/img/img.png" alt="image" style="display: block; margin: 0 auto;" />

## 2.5 运行
可以在控制台运行该程序，如果以不带参数运行，默认会打开一个`prompt`，在其中输入`print "Hello, world!";`，然后回车，控制台就会输出`Hello, world!`。

<img src="./doc/img/img_1.png" alt="image" style="display: block; margin: 0 auto;" />

其他的一些输出是GC的日志。

# 3. 配置
`clox`的有些配置项目位于`include\common.h`中。
```c++
#define NAN_BOXING
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC
```
- `NAN_BOXING`：用于开启是否使用`nan boxing`来表示值。开启之后不在使用`C`的`union`来表示值，而是统一使用一个64 bit的`value`来表示数值、布尔值、`nil`以及对象等。
- `DEBUG_PRINT_CODE`：用于开启是否打印编译后的字节码。
- `DEBUG_TRACE_EXECUTION`：用于开启是否打印虚拟机执行时的调试信息。
- `DEBUG_STRESS_GC`：用于开启是否进行强制垃圾回收，不开启则会默认进行自适应垃圾回收。
- `DEBUG_LOG_GC`：用于开启是否打印垃圾回收的日志。