# fmt 日本語ガイド

`fmt` は C++ の文字列フォーマットと出力を扱うオープンソースライブラリです。
C の `printf` 系や C++ の iostream の代わりとして利用でき、`std::format` / `std::print` に近い書き方ができます。

このディレクトリは **external** です。fmt 本体のソースコードはこのリポジトリには含めません。

## 公式リンク

- 公式リポジトリ: https://github.com/fmtlib/fmt
- 公式ドキュメント: https://fmt.dev/
- Getting Started: https://fmt.dev/latest/get-started/
- ライセンス: MIT
- 確認時点の最新リリース: 12.2.0（2026-10-02 確認）

## まず試す

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install libfmt-dev
```

### macOS + Homebrew

```bash
brew install fmt
```

### vcpkg

```bash
vcpkg install fmt
```

### Conan

```bash
conan install -r conancenter --requires="fmt/[*]" --build=missing
```

## 最小コード

`main.cpp`:

```cpp
#include <fmt/core.h>

int main() {
    fmt::print("Hello, {}!\n", "C++");
}
```

fmt がシステムにインストール済みで、pkg-config が利用できる環境なら一例として次のようにビルドできます。

```bash
c++ -std=c++17 main.cpp -lfmt -o app
./app
```

出力:

```text
Hello, C++!
```

## CMake: インストール済みの fmt を使う

構成:

```text
my-project/
├── CMakeLists.txt
└── main.cpp
```

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(fmt_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt REQUIRED)

add_executable(fmt_example main.cpp)
target_link_libraries(fmt_example PRIVATE fmt::fmt)
```

ビルド:

```bash
cmake -S . -B build
cmake --build build
```

## CMake: FetchContent で取得する

プロジェクト側で依存ライブラリを取得したい場合は CMake の `FetchContent` が使えます。

```cmake
cmake_minimum_required(VERSION 3.16)
project(fmt_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.2.0
)

FetchContent_MakeAvailable(fmt)

add_executable(fmt_example main.cpp)
target_link_libraries(fmt_example PRIVATE fmt::fmt)
```

バージョンを固定しておくと、ある日 upstream の変更で突然ビルドが変わる問題を避けやすくなります。

## fmt 自体をソースからビルドする

C++ らしく、自分でビルドしてインストールすることもできます。

```bash
git clone https://github.com/fmtlib/fmt.git
cd fmt
git checkout 12.2.0

cmake -S . -B build -DFMT_TEST=OFF
cmake --build build --config Release
```

Unix 系でシステムへインストールする場合:

```bash
sudo cmake --install build
```

インストール先を自分で決めたい場合:

```bash
cmake -S . -B build \
  -DFMT_TEST=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local"

cmake --build build
cmake --install build
```

CMake が fmt を見つけられない場合は、必要に応じて `CMAKE_PREFIX_PATH` を指定します。

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$HOME/.local"
```

## 基本: fmt::print

```cpp
#include <fmt/core.h>

int main() {
    const char* name = "Taro";
    int age = 20;

    fmt::print("{} is {} years old.\n", name, age);
}
```

`{}` の位置へ後ろの引数が順番に入ります。

## 文字列を作る: fmt::format

画面へ直接出力せず、`std::string` として受け取りたい場合は `fmt::format` を使います。

```cpp
#include <fmt/format.h>
#include <string>

int main() {
    std::string message = fmt::format("value = {}", 42);
}
```

## 桁数・幅を指定する

```cpp
#include <fmt/format.h>

auto a = fmt::format("{:08}", 42);
auto b = fmt::format("{:.2f}", 3.1415926);
auto c = fmt::format("{:>10}", "hello");
```

イメージ:

```text
00000042
3.14
     hello
```

フォーマット指定の詳細は公式の Format String Syntax を参照してください。

https://fmt.dev/latest/syntax/

## vector などを出力する

`fmt/ranges.h` を読み込むと、コンテナをそのままフォーマットできます。

```cpp
#include <fmt/ranges.h>
#include <vector>

int main() {
    std::vector<int> values{1, 2, 3, 4};
    fmt::print("{}\n", values);
}
```

出力例:

```text
[1, 2, 3, 4]
```

## 日付・時刻

`fmt/chrono.h` で chrono 型を扱えます。

```cpp
#include <chrono>
#include <fmt/chrono.h>

int main() {
    auto now = std::chrono::system_clock::now();
    fmt::print("now: {}\n", now);
}
```

## Header-only で使う

CMake では `fmt::fmt-header-only` ターゲットも用意されています。

```cmake
target_link_libraries(fmt_example PRIVATE fmt::fmt-header-only)
```

ただし公式ドキュメントでは、ビルド時間の観点から通常のコンパイル済みライブラリ `fmt::fmt` またはモジュール版の利用が推奨されています。

## よくある問題

### undefined reference が出る

ヘッダーを include しただけで通常版 fmt を使っている場合、リンクが必要です。

直接コンパイルするなら:

```bash
c++ main.cpp -lfmt -o app
```

CMake なら:

```cmake
target_link_libraries(your_target PRIVATE fmt::fmt)
```

### CMake が fmt を見つけない

インストール先が標準パスではない可能性があります。

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/prefix
```

または FetchContent を使う方法もあります。

### fmt と std::format のどちらを使うべき？

C++20 以降では標準ライブラリに `std::format` がありますが、コンパイラ・標準ライブラリの対応状況や、fmt 独自の機能を使いたい場合には fmt が選択肢になります。

既存の fmt 利用コードを無理に `std::format` へ置き換える必要はありません。プロジェクトの対象コンパイラや依存方針に合わせて選びます。

## CMake ターゲット

公式に主に次のターゲットが用意されています。

| ターゲット | 用途 |
| --- | --- |
| `fmt::fmt` | 通常のコンパイル済みライブラリ |
| `fmt::fmt-header-only` | Header-only 版 |
| `fmt::fmt-module` | C++20 module 版（対応環境・設定が必要） |

まずは `fmt::fmt` を使うのが分かりやすいです。

## ライセンスについて

fmt は MIT License で公開されています。

このページは fmt のコードを再配布するものではなく、日本語の導入・利用ガイドです。
実際に fmt を利用・再配布する場合は、必ず公式リポジトリにある最新のライセンス本文を確認してください。

## 参考

- https://github.com/fmtlib/fmt
- https://fmt.dev/
- https://fmt.dev/latest/get-started/
- https://fmt.dev/latest/api/
- https://fmt.dev/latest/syntax/
