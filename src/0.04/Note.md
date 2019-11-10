# x01.os Note

- **c.img.tar.gz** 需解压到各子目录中方可使用

## 0.01 - boot
  
  1. 把引导扇区从 0x7c00 加载到 0x90000  
  2. 显示信息 “hello x01.os!"
  3. 加载 system, 进入保护模式

## 0.02 - char drive

  1. struct tty_struct
  2. strcut tty_queue

## 0.03 - kernel

  1. 可以打字了

## 0.04 - fork.c

`copy_process()`: 创建并复制数据段、代码段及环境

- 申请一页内存存放任务数据结构  
- 当前进程设为父进程，清除信号，复位统计值
- 设置 TSS：esp0 为页顶端， ss0 为数据段，保存 i387
- 设置代码数据段基址、限长，复制页目录、页表项
