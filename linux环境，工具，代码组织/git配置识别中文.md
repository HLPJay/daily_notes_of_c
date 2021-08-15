在最近的工作中，使用git总是碰到中文不识别的问题，经过百度找到了解决方案，但是隔了一段时间后重新遇到，重新百度时结果参差不齐，好不容易找到原版解决方案，这里就做一些整理。

# 1：**问题**：

在windows环境或者linux环境使用终端命令对git仓库进行管理，发现很多中文不能识别，例如执行git status时：

![git中文乱码](..\md文档相关图片\git中文乱码.png)

# 2:解决方案：

​	很容易就能想到是编码方式的问题，这里我们需要修改配置支持utf-8编码方式。

​	参考来源：https://zhuanlan.zhihu.com/p/91741156

1：windows环境通过git bash终端 或者linux环境直接通过终端进行如下git命令的配置：

```bash
hlp@ubuntu:~/0723/daily_notes$ git config --global core.quotepath false                 # 设置 git status utf-8编码
hlp@ubuntu:~/0723/daily_notes$ git config --global gui.encoding utf-8 				    # 设置Git GUI界面utf-8编码
hlp@ubuntu:~/0723/daily_notes$ git config --global i18n.commit.encoding utf-8           # 设置commit信息utf-8编码
hlp@ubuntu:~/0723/daily_notes$ git config --global i18n.logoutputencoding utf-8         # 设置输出 log utf-8 编码

```

2：新增系统环境变量LESSCHARSET ，并赋值 utf-8

​	windows环境中，我的电脑–》属性–》高级系统设置==》环境变量，直接在弹出框中系统界面栏新增一组数据  LESSCHARSET  utf-8

​	linux环境直接配置 /etc/profile 文件，在文件末尾新增如下：

```bash
export LESSHARESET=utf-8
```

​	执行 如下命令使其生效：

```bash
hlp@ubuntu:~/0723/daily_notes$ source /etc/profile
```

