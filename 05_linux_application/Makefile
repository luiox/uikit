build.release:
	xmake f -m release && xmake build

build.debug:
	xmake f -m debug && xmake build

clean:
	xmake clean

# 定义一个伪目标，名为fmt
fmt:
	@find . \( -iname '*.h' -o -iname '*.c' -o -iname '*.cpp' -o -iname '*.hpp' \) -print0 | xargs -0 clang-format -i

.PHONY: format
