# 64位操作系统学习代码

## 对应章节

- 0: chapter 3
- 1: chapter 4
- 2: chapter 5
- 3: chapter 6
- 4: chapter 8
- 5: chapter 9
- 6: chapter 10
- 7: chapter 15

## bochs debug

- x /nuf address => n:数量， u:大小， f:格式 （示例： x /128hx 0x8000)

## VBE  

``` 获取设置 VBE
; int 10h (ax=0x4f00)
; 输入：
;    ax=0x4f00 => 获取 VBE 控制器信息
;    es:di => 指向信息块结构的起始地址
; 输出：
;    ax => VBE 返回状态

; int 10h (ax=0x4f01)
; 输入：
;    ax=0x4f01 => 获取 VBE 模式信息
;    cx => 模式号
;    es:di => 指向模式结构起始地址
; 输出：
;    ax => VBE 返回状态

; int 10h (ax = 0x4f02)
; 输入：
;    ax=0x4f02 => 设置 VBE 显示模式
;    bx => D0-8(VBE模式号）， D9-10（保留，必须为0）
;          D11=0(使用当前刷新率），D11=1(使用CRTC刷新率定制值）
;          D12-13(VBE/AF 保留使用，必须为0）
;          D14=0(使用窗口帧缓存模式）， D14=1(使用线性帧缓存模式）
;          D15=0(清空显示内存数据），D15=1(保留显示内存数据）
;    es:di => 显示模式结果起始地址
; 输出：
;    ax => VBE 返回状态
```