# GitHub 上传说明

目标仓库：<https://github.com/JiaLiu-S/STM32F103C8T6>

该仓库已经有一个 README 初始提交，直接通过网页上传本目录内容最简单，也不会产生 Git 历史冲突。

## 上传源码

1. 打开目标仓库，点击绿色“代码”按钮左侧的 `+`，或点击 `添加文件`。
2. 选择 `上传文件`。
3. 打开本机的 `VillaSecurity_GitHub` 目录，选择目录内的全部文件和文件夹，不要只上传外层文件夹或 ZIP。
4. 将内容拖到 GitHub 上传区域。新的 `README.md` 会替换当前只有标题的 README。
5. 提交说明填写 `Publish VillaSecurity 1.0.0 source`。
6. 选择直接提交到 `main`，点击提交。
7. 上传完成后检查首页是否显示功能、接线、构建和许可证章节。

## 发布可执行文件

1. 打开仓库右侧的 `Releases`，点击 `Create a new release`。
2. 新建标签 `v1.0.0`，标题填写 `VillaSecurity 1.0.0`。
3. 说明中写明正式固件、诊断固件、Windows 监控程序和实物联调边界。
4. 上传 `VillaSecurity_Release_v1.0.0` 目录中的 HEX、EXE、源码 ZIP 和 SHA-256 清单。
5. 点击发布，随后从 README 的 Releases 链接检查下载是否正常。

不要把 GitHub 密码、访问令牌或浏览器登录信息写进任何文件。
