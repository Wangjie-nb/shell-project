# My Shell

一个基于Linux系统调用实现的简单Shell命令解释器，支持管道、重定向、后台运行。

## 功能特性
- 基本命令执行（ls、pwd、echo等外部命令）
- 内置命令（cd、exit、help）
- 管道支持（`cmd1 | cmd2`）
- 输入输出重定向（`<`、`>`）
- 后台运行（`&`）

## 技术栈
C++ / Linux系统调用 / fork/exec/wait / 管道IPC

## 目录结构
```
shell-project/
├── src/
│   ├── shell.h/cpp    # Shell核心实现
│   └── main.cpp       # 程序入口
├── docs/
├── CMakeLists.txt
└── README.md
```

## 编译运行
```bash
mkdir build && cd build
cmake ..
make
cd ..
./bin/myshell
```

## 支持的命令示例
```bash
# 基本命令
ls -l
pwd
echo hello world

# 内置命令
cd ..
help
exit

# 管道
ls | grep .cpp
cat file.txt | wc -l

# 重定向
ls > file.txt
wc -l < file.txt

# 后台运行
sleep 10 &
```

## 设计思路
1. **命令解析**：按管道符`|`分割成多个命令，每个命令再按空格分割成参数
2. **进程创建**：每个命令用fork创建子进程，父进程wait等待子进程结束
3. **管道实现**：调用pipe创建管道，前一个命令的标准输出重定向到管道写端，后一个命令的标准输入从管道读端读
4. **重定向**：用open打开文件，dup2把标准输入/输出重定向到文件描述符
5. **内置命令**：cd等命令直接在父进程执行，不需要fork子进程
