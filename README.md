# T8Y Fantastic Console

## 展览
<p align="center">
<img width="289" height="312" alt="QQ20260120-000034" src="https://github.com/user-attachments/assets/b554790b-0c79-4056-b931-3c649e963e41" />
<img width="289" height="312" alt="QQ20260120-000042" src="https://github.com/user-attachments/assets/4c66df72-c06d-47b8-a89a-b2e73b3a68c2" />
<img width="289" height="312" alt="QQ20260120-000018" src="https://github.com/user-attachments/assets/c73eaced-b9a1-4523-b3e2-f75ae8811c03" />
</p>

## 规格
```
显示    128x128 像素, 固定16色调色板
输入    8 键 Gamepad / 鼠标 / 键盘
精灵    128x128 大小图集，包含256个精灵
代码    不限定长度的Lua脚本
地图    128x128 格, 1024x1024 分辨率
```

## 基本结构
```
function init()
    ...
end

function draw()
    ...
end

function update()
    ... 每秒64次
end
```

## API
### 图像

#### 裁剪
`clip(x, y, w, h)`  
设置裁剪区域.

#### pal
`pal(c0, c1)`  
将c0颜色映射为c1(默认映射为它本身)

`pal(c) -> color`  
获取颜色c的映射

#### palt
`palt(c, t)`  
启用将c颜色值设置为透明/不透明

`palt()`  
重置所有透明色

`palt(c)`  
设置透明色
例: plat(0b0000000000000001) 将0号颜色设置为透明色

#### cls
`cls(c)`
使用颜色c清屏

#### camera
`camera(x, y)`  
设置绘制偏移, 127 >= x,y >= -127

#### pget | pset
`pget(x, y) -> color`  
获取屏幕坐标颜色

`pset(x, y, c)`  
设置屏幕坐标颜色

#### fget | fset
`fget(n)`  
获取n号精灵的标志

`fset(n, f)`  
设置n号精灵的标志

#### sget | sset
`sget(x, y) -> color`  
获取精灵图集坐标处颜色

`sset(x, y)`  
设置精灵图集坐标处颜色

#### mget | mset
`mget(x, y) -> id`  
获取地图坐标处精灵id

`mset(x, y, id)`  
设置地图坐标处精灵id

#### cric
`cric(x, y, r, color, [fill])`  
从坐标x,y处绘制半径为r，颜色为color的圆形，设置fill（布尔值）确定是否填充

#### elli
`elli(x, y, w, h, color, [fill])`  
从区域x,y,w,h处绘制颜色为color的椭圆形，设置fill（布尔值）确定是否填充

#### line
`line(x0, y0, x1, y1, color)`  
绘制从x0,y0到x1,y1，颜色为color的线条

#### rect
`rect(x, y, w, h, color, [fill])`  
从区域x，y，w，h处绘制颜色为color的矩形，设置fill（布尔值）确定是否填充

#### print
`print(text, x, y, [color], [w0], [w1])`  
从屏幕x，y处绘制文本
w0指定了charcode < 0x80的字符宽度，默认为4
w1指定了charcode >= 0x80的字符宽度，默认为8

#### spr
`spr(id, x, y, scale = 1, flip = 0, rotate = 0)`  
从x，y处绘制精灵，可按所需倍数放大，比如两倍缩放表示将 8x8 像素精灵绘制到屏幕的 16x16 区域
可以翻转精灵:
- 0 = 不翻转
- 1 = 水平翻转
- 2 = 垂直翻转
- 3 = 同时翻转
  
可以基于90度旋转精灵:
- 0 = 不旋转
- 1 = 90 度旋转
- 2 = 180 度旋转
- 3 = 270 度旋转

#### map
`map(x = 0, y = 0, w = 1, h = 1, sx = 0, sy = 0, scale = 1, layers = 0xFF)`  
地图由 8x8 像素的单元组成，每个单元都可以使用地图编辑器填充精灵。
地图的最大宽度可达 128 个单元格，最大高度可达 128 个单元格。
该函数将地图的指定区域绘制到指定的屏幕位置。
例，map(5,5,12,10,0,0) 将从屏幕的 (0,0) 绘制从地图块坐标为 (5,5)，大小为 12x10 的地图块。
如果设置了layers，则仅绘制匹配layers位的地图块

### 输入

#### btn
`btn(id) -> bool`  
检查按键是否按下

#### btnp
`btnp(id) -> bool`  
检查按键是否刚刚按下

#### key
`key(code) -> bool`  
检查键盘按键是否按下

#### keyp
`keyp(code) -> bool`  
检查键盘按键是否刚刚按下

#### mouse()
`mouse() -> x y z button`  
获取鼠标状态，包含坐标、滚轮以及按键状态，其中按键状态是 8 bit 位域

### 其它

#### log
`log(text)`  
打印文本，同时从虚拟控制台和计算机控制台上打印

#### time
`time() -> number`  
返回自卡带运行以来的tick数

#### tstamp()
返回自 1970-1-1 以来经过的秒数

#### pmem
`pmem(index, value)`  

`pmem(index) -> value`  
此功能允许您在卡带持久内存中可用的 256 个单独的 32 位插槽之一中保存和检索数据。
这对于保存高分、级别提升或成就非常有用。
数据存储为无符号 32 位整数（从 0 到 4294967295）。

## 按键 ID
```
+--------+----+----+----+----+
| ACTION | P1 | P2 | P3 | P4 |
+--------+----+----+----+----+
|     UP |  0 |  8 | 16 | 24 |
|   DOWN |  1 |  9 | 17 | 25 |
|   LEFT |  2 | 10 | 18 | 26 |
|  RIGHT |  3 | 11 | 19 | 27 |
|      A |  4 | 12 | 20 | 28 |
|      B |  5 | 13 | 21 | 29 |
|      X |  6 | 14 | 22 | 30 |
|      Y |  7 | 15 | 23 | 31 |
+--------+----+----+----+----+
另：先插入的操作杆子在后插入的操作杆之前拔出，则后插入的操作杆前移
例：P1先插入而P2后插入，在P1断开后P2变为P1
```


## 键盘 ID
参考 https://wiki.libsdl.org/SDL3/SDL_Scancode
