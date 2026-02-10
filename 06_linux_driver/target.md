开发板如果自动挂载nfs失败就手动挂载，命令如下。

```bash
mount -t nfs -o nolock,nfsvers=3 192.168.56.100:/home/canrad/linux/imx6ull/drivers /mnt/drivers
```


学习顺序


1. 内核构建、模块、dmesg、printk。调试技巧
写模块打印信息，练习交叉编译与加载。ftrace、dynamic debug、kernelshark、perf

2. 内核并发基础
spinlock/mutex/atomic

3. 中断机制
中断上下文不能睡、spin_lock_irqsave、workqueue。
小练习：IRQ 增加一个共享计数器（用 spinlock），再把工作下放到 workqueue。

4. 字符驱动模型和udev机制

5. 平台设备基础
device/driver/bus 概念、platform_driver_register、probe/remove。
小练习：把已有字符驱动改成 platform driver。

6. Device Tree 与 bindings（紧跟平台）
compatible、reg/interrupts、of_match_table、binding 文件写法。
小练习：用 DTS 描述设备，改 compatible 验证匹配机制。
资源管理与 devm（实践提升）
devm_*、clk/regulator/reset 的基本用法和错误路径处理。
小练习：在 probe 中用 devm_ 系列替换手动释放。

7. GPIO和Input子系统

8. I2C和SPI

9. PWM / ADC和Misc设备

10. Block / USB / Network














---


# 驱动学习目标与顺序清单 ✅

## 目标概述
- **最终目标**：能基于 Device Tree 在 imx6ull 上开发、调试并稳定运行 platform 驱动；能读懂相关内核源码并撰写 binding 文档与补丁。
- **学习策略**：先看别人整理好的笔记与实战示例（速成理解），再回到官方文档与源码查证原理，边做小实验边阅读（以实践驱动理解）。

---

## 优先顺序（必学项，按顺序完成）
1. C 与内核开发基础（并发/内存/模块）
2. 内核构建与模块调试（交叉编译、insmod/rmmod、dmesg、kernel config）
3. Platform device/driver 基本模型（`platform_driver_register`、`probe`/`remove`）
4. Device Tree 基本语法与绑定（`compatible`、`reg`/`interrupts`、#address-cells）
5. DT -> platform device 的生成流程（`of_platform_populate()`、`struct device_node`）
6. 资源解析与常用 API（`platform_get_resource`、`devm_ioremap_resource`、`devm_gpiod_get`）
7. 中断、workqueue、bottom halves 与并发处理
8. 时钟、复位、电源（`clk`/`reset`/`regulator`）和 devm 模式资源管理
9. 电源管理（runtime PM / system PM）与 DMA（如需要）
10. 代码阅读与补丁流程（git format-patch、邮件列表）

---

## 先读的“别人整理好的笔记”（建议按顺序）
- Bootlin Device Tree & Drivers 讲义（高质量、示例清晰）
- LDD3 的相关章节（快速回顾驱动编程基础）
- Kernel Newbies: Device Tree 指南（入门速查）
- 精选博客（带实践示例的文章，选与内核版本匹配的文章）

> 小贴士：先花 1–2 天把这些笔记通读并做里面的示例（搭配你的板子做 DT 修改与驱动加载），帮助你建立整体认知。

---

## 官方文档与源码（查原理时必看）
- `Documentation/devicetree/`（Device Tree 规范与 binding 写法）
- `Documentation/driver-model/`（driver core 设计）
- 查看内核源码：`drivers/base/`、`drivers/of/`、目标子系统下的驱动（如 `drivers/gpio/`、`drivers/platform/`）
- elixir.bootlin.com（函数实现快速索引）

---

## 每个主题的短练习（把“知道”变成“理解”）
- Platform driver：把当前字符驱动改为 platform driver，验证 bind/unbind；写一个小脚本反复绑定/解绑，检查资源是否释放。  
- Device Tree：修改节点 `compatible`（改一字），确认驱动不再 probe；添加/修改 `reg`、`interrupts` 属性并在 probe 中打印出来。  
- 资源管理：在 probe 中改用 `devm_*` 函数，模拟错误路径（部分失败）测试是否漏释放。  
- 中断与并发：实现中断处理并把重任务下放到 workqueue，压力测试看稳定性。  
- PM 与 DMA：实现 runtime PM 流程并测 suspend/resume；若设备有 DMA，练习 dma-mapping 基本流程。

---

## 验证标准（每学完一项需能做的事）
- 能在板上通过 DT 让驱动自动 probe（`dmesg` 能看到关键打印）。
- 能手动 bind/unbind 驱动并复现 edge cases（资源申请失败、probe 部分失败）。
- 能读懂并解释相关的内核函数调用链（用 elixir 查实现并能说出关键步骤）。




