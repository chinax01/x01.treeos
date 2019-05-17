## OS-0.02.2 使用 LDT

1. freedos.img 来自 bochs 官方网站。
2. b.img 可用 bximage 创建, bximage 可在终端中安装： `sudo apt install bximage` , 其他所需也可采取此种方法安装，不再重复。使用前需在 freedos 中格式化 `format B:`
3. `make` 生成 Image.com, `make copyImage` 复制到 **b.img**, `make run`运行，在 freedos 中执行命令： `B:\Image.com`