#!/usr/bin/env bash
# 由 design/*.json 再生成 core/include/uikit/embedded_themes.h（默认主题编译期内嵌，
# 运行期不依赖包安装路径——见 PLAN.md「包装设计」）。改 design/ 后在仓库根执行：
#   ./tools/gen_embed.sh && git add core/include/uikit/embedded_themes.h
# CI 会再生成一次并 git diff --exit-code 校验二者同步。
set -euo pipefail
cd "$(dirname "$0")/.."
out=core/include/uikit/embedded_themes.h
{
  echo '#pragma once'
  echo '// 由 tools/gen_embed.sh 从 design/*.json 生成，勿手改；改 design/ 后重新生成。'
  echo 'namespace uikit::embed {'
  for name in light dark icons; do
    body="$(cat "design/${name}.json")"
    printf 'inline const char* %s_json() { return R"tk(%s\n)tk"; }\n' "$name" "$body"
  done
  echo '}  // namespace uikit::embed'
} > "$out"
