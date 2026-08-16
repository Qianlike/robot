"""pytest 公共配置。

不向 sys.path 插入源码树：测试必须在安装后的环境运行
（本地 `pip install -e tools/python-build/python`，或 cibuildwheel 容器内的 wheel 安装）。
"""
