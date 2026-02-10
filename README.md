# monocodes
我的代码集中仓库



添加子项目

```bash
git submodule add <remote_url> <destination_folder>
```

移除子项目

```bash
git rm --cached morpher
```

克隆项目

```bash
git clone https://github.com/luiox/monocodes.git --recursive -submodules
```

拉取子项目的更新

```bash
git submodule update --remote
```


需要移除的submodule

[submodule "mverify"]
	path = mverify
	url = https://github.com/luiox/mverify
[submodule "libca"]
	path = libca
	url = https://github.com/luiox/libca
[submodule "morpher"]
	path = morpher
	url = https://github.com/luiox/morpher.git
[submodule "morpher-api"]
	path = morpher-api
	url = https://github.com/luiox/morpher-api.git
[submodule "mrecaf4plugin"]
	path = mrecaf4plugin
	url = https://github.com/luiox/mrecaf4plugin.git
	
[submodule "ota_demo"]
	path = ota_demo
	url = https://github.com/luiox/ota_demo

