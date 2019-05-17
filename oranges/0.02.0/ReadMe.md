## OS-0.02 进入保护模式

1. freedos.img 来自 bochs 官方网站。
2. b.img 可用 bximage 创建, 使用前需在 freedos 中格式化 `format B:`
3. `make` 生成 Image.com, `make copyImage` 复制到 **b.img**, `make run`运行，在 freedos 中执行命令： `B:\Image.com`
