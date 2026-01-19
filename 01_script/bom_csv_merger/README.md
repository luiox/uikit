python 包管理器 uv使用教程

## 3️⃣ 标准项目工作流

这是最常用的场景：从创建项目到运行、测试、打包。

### 3.1 初始化项目

```
# 创建一个新项目目录
uv init my-app
cd my-app
```

生成的文件结构：

```
my-app/
├── pyproject.toml   # 项目配置（类似 package.json）
├── uv.lock          # 锁定依赖版本（类似 package-lock.json）
├── .python-version  # 指定 Python 版本
├── src/
│   └── my_app/
│       └── __init__.py
└── README.md
```

### 3.2 管理 Python 版本

`uv` 可以自动下载和管理不同版本的 Python：

```
# 查看当前使用的 Python
uv python pin 3.12

# 或者创建虚拟环境时指定版本
uv venv --python 3.11
```

### 3.3 添加依赖

```
# 添加运行时依赖
uv add requests pandas

# 添加开发依赖（如测试工具）
uv add --dev pytest ruff

# 指定版本
uv add "numpy>=1.24,<2.0"
```

查看 `pyproject.toml`，依赖已被自动写入：

```
[project]
dependencies = [
    "requests>=2.32.0",
    "pandas>=2.2.0",
]

[project.optional-dependencies]
dev = [
    "pytest>=8.0.0",
    "ruff>=0.6.0",
]
```

### 3.4 运行代码

```
# 在当前虚拟环境中运行
uv run python main.py

# 或者直接运行模块
uv run python -m my_app
```

### 3.5 运行测试

```
# 运行 pytest（自动使用项目虚拟环境）
uv run pytest
```

### 3.6 代码检查与格式化（Ruff 集成）

```
# 检查代码
uv run ruff check .

# 自动修复
uv run ruff check --fix .

# 格式化代码
uv run ruff format .
```

## 4️⃣ 锁定依赖（uv.lock）

`uv.lock` 文件确保团队协作时依赖版本完全一致。

```
# 生成/更新锁文件（通常 add 时自动执行）
uv lock

# 根据锁文件精确安装依赖（用于 CI/CD 或生产环境）
uv sync
```

## 5️⃣ 脚本模式（类似 pipx）

直接运行第三方工具，无需安装到系统或手动管理环境：

```
# 使用 uvx 直接运行工具（自动安装并缓存）
uvx ruff check .
uvx pycowsay "Hello uv!"
uvx httpie https://api.github.com
```

## 6️⃣ 常用命令速查表

| 功能                 | 传统工具                                  | uv 命令              |
| :------------------- | :---------------------------------------- | :------------------- |
| **初始化项目**       | 手动建文件夹 + `touch pyproject.toml`     | `uv init <name>`     |
| **创建虚拟环境**     | `python -m venv .venv`                    | `uv venv`            |
| **添加依赖**         | `pip install xxx`                         | `uv add xxx`         |
| **添加开发依赖**     | `pip install xxx`                         | `uv add --dev xxx`   |
| **移除依赖**         | `pip uninstall xxx`                       | `uv remove xxx`      |
| **更新依赖**         | `pip install --upgrade xxx`               | `uv add xxx@latest`  |
| **锁定依赖**         | `pip-compile requirements.in`             | `uv lock`            |
| **安装锁定的依赖**   | `pip install -r requirements.txt`         | `uv sync`            |
| **运行脚本**         | `source .venv/bin/activate && python xxx` | `uv run python xxx`  |
| **运行临时工具**     | `pipx run xxx`                            | `uvx xxx`            |
| **管理 Python 版本** | `pyenv install 3.11`                      | `uv python pin 3.11` |
