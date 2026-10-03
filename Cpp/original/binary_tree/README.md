# binary_tree

C++17 で使えるシンプルな **二分探索木（Binary Search Tree / BST）** です。

このライブラリは Open-librarys-for-Japanese の `original` として、このリポジトリ内で実装しています。

## できること

- 値の追加: `insert`
- 値の検索: `contains`
- 値の削除: `erase`
- 要素数: `size`
- 空判定: `empty`
- 全削除: `clear`
- 中間順走査: `inorder`
- 行きがけ順走査: `preorder`
- 帰りがけ順走査: `postorder`
- ツリーのコピー / ムーブ
- 比較関数の差し替え

重複する値は保存しません。すでに存在する値を `insert` すると `false` を返します。

## 必要環境

- C++17 以上
- 標準ライブラリのみ
- 外部依存なし

## 一番簡単な使い方

`include/binary_tree.hpp` を自分のプロジェクトへ置き、include します。

```cpp
#include "binary_tree.hpp"

int main() {
    olj::BinarySearchTree<int> tree;

    tree.insert(8);
    tree.insert(3);
    tree.insert(10);

    if (tree.contains(3)) {
        // 3 は存在する
    }
}
```

直接コンパイルする場合:

```bash
c++ -std=c++17 -Iinclude main.cpp -o app
```

## CMake でビルドする

このディレクトリ自体をビルドする場合:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

サンプル実行ファイルは `binary_tree_example`、テストは `binary_tree_test` です。

## insert

```cpp
olj::BinarySearchTree<int> tree;

bool first = tree.insert(10);   // true
bool second = tree.insert(10);  // false
```

同じ値は1つだけ保持します。

## contains

```cpp
tree.insert(42);

bool a = tree.contains(42);  // true
bool b = tree.contains(7);   // false
```

## erase

```cpp
tree.insert(8);
tree.insert(3);
tree.insert(10);

bool removed = tree.erase(8);  // true
bool again = tree.erase(8);    // false
```

葉、子が1つのノード、子が2つのノードを削除できます。

## inorder

中間順走査では、

```text
左 → 自分 → 右
```

の順に走査します。

通常の `std::less<T>` を使った二分探索木なら、昇順の値を取得できます。

```cpp
olj::BinarySearchTree<int> tree;

for (int value : {8, 3, 10, 1, 6}) {
    tree.insert(value);
}

for (int value : tree.inorder()) {
    // 1, 3, 6, 8, 10
}
```

## preorder / postorder

```cpp
auto pre = tree.preorder();
auto post = tree.postorder();
```

- `preorder`: 自分 → 左 → 右
- `postorder`: 左 → 右 → 自分

## size / empty / clear

```cpp
tree.size();
tree.empty();

tree.clear();
```

## 文字列でも使える

テンプレートなので、比較可能な型なら `int` 以外にも使えます。

```cpp
#include <string>

olj::BinarySearchTree<std::string> tree;

tree.insert("banana");
tree.insert("apple");
tree.insert("orange");
```

## 比較方法を変更する

第2テンプレート引数で比較関数を指定できます。

```cpp
#include <functional>

olj::BinarySearchTree<int, std::greater<int>> tree;

tree.insert(1);
tree.insert(2);
tree.insert(3);
```

この場合、木の並び方も `std::greater<int>` に従います。

## 計算量

この実装は AVL Tree や Red-Black Tree のような自己平衡木ではありません。

木の高さを `h` とすると:

| 操作 | 計算量 |
| --- | --- |
| insert | O(h) |
| contains | O(h) |
| erase | O(h) |
| inorder / preorder / postorder | O(n) |
| size / empty | O(1) |

木がある程度均等なら `h ≈ log n` ですが、例えば

```text
1, 2, 3, 4, 5, 6, ...
```

のような順番で追加すると片側へ偏り、最悪の場合 `h = n` になります。

その場合、検索・追加・削除も最悪 O(n) です。

## なぜ std::unique_ptr を使っているのか

各ノードが子ノードを所有する構造を、

```cpp
std::unique_ptr<Node> left;
std::unique_ptr<Node> right;
```

で表現しています。

これによりノードを `new` / `delete` で手動管理する必要がなく、ツリーが破棄されると子ノードも自動的に破棄されます。

## コピー

```cpp
olj::BinarySearchTree<int> a;
a.insert(10);
a.insert(20);

auto b = a;

b.insert(30);
```

`b` は独立したツリーなので、`b` の変更は `a` に影響しません。

## 注意点

このライブラリは「一般的な二分木」ではなく、値の大小関係を使って配置する **二分探索木** です。

また自己平衡化は行いません。大量のデータを扱い、常に O(log n) に近い性能が必要なら、AVL Tree / Red-Black Tree や標準ライブラリの `std::set` / `std::map` も検討してください。

走査メソッドは `std::vector<T>` を返すため、走査結果を取得する場合は `T` がコピー可能である必要があります。

## ファイル構成

```text
binary_tree/
├── CMakeLists.txt
├── README.md
├── include/
│   └── binary_tree.hpp
├── examples/
│   └── main.cpp
└── tests/
    └── test_binary_tree.cpp
```
