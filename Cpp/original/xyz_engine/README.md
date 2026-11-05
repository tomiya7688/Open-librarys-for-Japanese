# xyz_engine

**x / y / z を持つゲームオブジェクトを、すぐ作って動かせる C++17 の軽量ゲームエンジン基盤**です。

描画ライブラリには依存しません。
2Dなら `z = 0` のまま、3Dなら x / y / z をそのまま使えます。

## 30秒で使う

`include/xyz_engine.hpp` をプロジェクトへコピーします。

```cpp
#include "xyz_engine.hpp"

int main() {
    olj::game::Engine engine;

    auto& player = engine.scene().create("player");

    player.transform.position = {0.0f, 1.0f, 0.0f};
    player.velocity = {2.0f, 0.0f, 0.0f};

    engine.step(1.0f / 60.0f);
}
```

これだけで、`player` の x 座標が速度に応じて移動します。

直接コンパイル:

```bash
c++ -std=c++17 -Iinclude main.cpp -o game
```

外部依存はありません。

## 何が入っている？

- `Vec3` — x / y / z ベクトル
- `Transform` — position / rotation / scale
- `GameObject` — ゲーム内オブジェクト
- `Scene` — オブジェクトの作成・検索・削除
- `Engine` — `step(dt)` による更新
- component — 任意の型をオブジェクトに追加
- tag — オブジェクトの分類
- `BoxCollider` — 軽量なAABB衝突判定

## 座標

```cpp
player.transform.position.x = 10.0f;
player.transform.position.y = 20.0f;
player.transform.position.z = 5.0f;
```

まとめて書けます。

```cpp
player.transform.position = {10.0f, 20.0f, 5.0f};
```

2Dなら:

```cpp
player.transform.position = {100.0f, 200.0f}; // z = 0
```

## 移動

```cpp
player.velocity = {5.0f, 0.0f, 0.0f};

engine.step(1.0f);
```

1秒分進めるので、x方向へ5移動します。

加速度も使えます。

```cpp
player.acceleration = {0.0f, -9.8f, 0.0f};
```

更新順は、

1. `on_update`
2. `velocity += acceleration * dt`
3. `position += velocity * dt`

です。

## オブジェクトごとの処理

```cpp
player.on_update = [](olj::game::GameObject& self, float dt) {
    self.transform.rotation.y += 90.0f * dt;
};
```

`rotation` は度数法として扱うデータです。

このライブラリ自体は行列変換や描画をしないので、SDL / OpenGL / Vulkan / DirectX 等の描画側で必要な形式に変換してください。

## component

継承ツリーを作らず、必要なデータを合成できます。

```cpp
struct Health {
    int hp;
    explicit Health(int value) : hp(value) {}
};

auto& health = player.add_component<Health>(100);

health.hp -= 10;
```

取得:

```cpp
if (auto* health = player.get_component<Health>()) {
    health->hp -= 10;
}
```

確認:

```cpp
player.has_component<Health>();
```

削除:

```cpp
player.remove_component<Health>();
```

同じ型のcomponentは1オブジェクトにつき1つです。
再度 `add_component<T>` すると置き換えます。

## tag

```cpp
player.add_tag("player");
player.add_tag("damageable");

auto targets = engine.scene().with_tag("damageable");
```

## 名前・IDで検索

```cpp
auto* object = engine.scene().find("player");

auto id = player.id();
auto* same = engine.scene().find(id);
```

IDはScene内で自動発行されます。

## 削除

```cpp
engine.scene().destroy(player);
```

または:

```cpp
engine.scene().destroy(player.id());
```

`on_update` の途中で削除しても、そのフレームの更新終了時まで安全に遅延削除されます。

**削除後のGameObject参照・ポインタは使わないでください。**

## 衝突判定

```cpp
player.add_component<olj::game::BoxCollider>(
    olj::game::Vec3{0.5f, 1.0f, 0.5f}
);

enemy.add_component<olj::game::BoxCollider>(
    olj::game::Vec3{0.5f, 0.5f, 0.5f}
);

if (olj::game::collides_with(player, enemy)) {
    // 接触
}
```

`half_extents` は中心から各方向への半径です。

この初期版のBoxColliderは **AABB（軸平行境界ボックス）** です。
`Transform::scale` は反映しますが、`rotation` は衝突形状へ反映しません。

回転する箱が必要なら、将来的にOBB等を追加できます。

## pause / time scale

一時停止:

```cpp
engine.set_paused(true);
```

スロー:

```cpp
engine.set_time_scale(0.5f);
```

2倍速:

```cpp
engine.set_time_scale(2.0f);
```

## 描画ライブラリと組み合わせる

xyz_engine はウィンドウも描画APIも持ちません。

例えば外側のゲームループから:

```cpp
while (running) {
    poll_input();

    engine.step(delta_time);

    engine.scene().for_each([](olj::game::GameObject& object) {
        draw_object(
            object.transform.position.x,
            object.transform.position.y,
            object.transform.position.z
        );
    });
}
```

と使えます。

そのため、

- SDL
- SFML
- raylib
- OpenGL
- Vulkan
- DirectX
- 独自レンダラー

などの上に載せられます。

## CMake

このディレクトリ自体をビルド:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

別プロジェクトへコピーした場合は、単純にinclude pathを通すだけでも使えます。

## API一覧

### Vec3

- `x / y / z`
- `+ - * /`
- `length()`
- `length_squared()`
- `normalized()`
- `dot()`
- `cross()`
- `nearly_equals()`

### Transform

- `position`
- `rotation`
- `scale`
- `translate()`
- `rotate()`

### GameObject

- `id()`
- `name()`
- `set_name()`
- `transform`
- `velocity`
- `acceleration`
- `active`
- `on_update`
- `add_tag()`
- `remove_tag()`
- `has_tag()`
- `add_component<T>()`
- `get_component<T>()`
- `has_component<T>()`
- `remove_component<T>()`

### Scene

- `create()`
- `find()`
- `destroy()`
- `clear()`
- `with_tag()`
- `for_each()`
- `update()`
- `size()`
- `empty()`

### Engine

- `scene()`
- `step(dt)`
- `set_paused()`
- `set_time_scale()`
- `frame()`
- `elapsed_seconds()`
- `before_step`
- `after_step`

## なぜ描画まで入れないの？

「ゲームエンジン」と言っても、用途によって必要な描画技術が大きく異なるためです。

xyz_engine はまず、

> オブジェクトが xyz の世界に存在し、動き、データを持ち、Sceneで管理される

という共通部分だけを提供します。

描画・音声・入力を固定しないことで、小さな2Dゲームから3D実験、シミュレーションまで同じコアを利用できます。

## ファイル構成

```text
xyz_engine/
├── CMakeLists.txt
├── README.md
├── include/
│   └── xyz_engine.hpp
├── examples/
│   └── main.cpp
└── tests/
    └── test_xyz_engine.cpp
```
