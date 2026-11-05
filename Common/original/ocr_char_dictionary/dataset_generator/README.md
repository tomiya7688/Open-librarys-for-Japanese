# OCR Dataset Generator

`ocr_char_dictionary` の **1行1文字辞書**から、OCR学習用の文字画像を生成するスクリプトです。

画像データをGitに大量保存するのではなく、辞書・フォント・回転角・色パレットから同じ条件を再生成できます。

## 30秒で試す

```bash
cd Common/original/ocr_char_dictionary/dataset_generator
python -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt
```

次に `fonts/` へ利用したい `.ttf` / `.otf` / `.ttc` を置きます。

```text
fonts/
├── ゴシック系フォント.ttf
├── 明朝系フォント.otf
└── CJKフォント.ttc
```

最初は10文字だけ生成するのがおすすめです。

```bash
python generate.py --limit-chars 10
```

問題なければ全件生成します。

```bash
python generate.py
```

## デフォルト設定

- 辞書: `../all_jis_level1_symbols.txt`
- 画像: 64x64 PNG
- 回転: `-14, -10, -6, -2, 2, 6, 10, 14` 度
- 色: 32パレット
- 背景/文字色の両方を変化
- class ID: 辞書の0始まり行番号

回転は90度刻みではなく、通常の文書OCRで起こりやすい小さな傾きをデフォルトにしています。

## 複数フォント

`fonts/` 以下を再帰的に検索し、TTF / OTF / TTC を読み込みます。

TTCは内部の各フォントfaceを別フォントとして扱います。

また、フォントのUnicode cmapを確認し、収録されていない文字は**豆腐（notdef）を教師画像にせずスキップ**します。スキップした組み合わせは `skipped.csv` に記録されます。

## 出力

```text
output/
├── images/
│   ├── 000000/
│   │   ├── 000000_U+0030_f000_r00_p00.png
│   │   └── ...
│   ├── 000001/
│   └── ...
├── labels.csv
├── skipped.csv
└── summary.json
```

### labels.csv

画像とクラス、フォント、回転、色の対応です。

```csv
class_id,char,codepoint,font_id,font_name,font_file,font_index,rotation_deg,palette_id,palette_name,image_relpath
0,0,U+0030,0,Noto Sans CJK JP,NotoSansCJK-Regular.ttc,0,-14,0,black_on_white,images/000000/...
```

### skipped.csv

フォントに文字がない場合を記録します。

```csv
class_id,char,codepoint,font_id,font_name,reason
123,𠮟,U+20B9F,4,Example Font,glyph_not_in_font_cmap
```

## class IDについて

辞書の順番をそのまま使います。

```text
辞書1行目 -> class_id 0
辞書2行目 -> class_id 1
辞書3行目 -> class_id 2
```

あるフォントに文字がなく画像生成をスキップしても、class IDは詰め直しません。

これにより辞書とモデルの対応を維持できます。

## 画像数

現在の `all_jis_level1_symbols.txt` は3,796文字です。

1フォントなら:

```text
3,796文字 × 8回転 × 32パレット = 971,776枚
```

4フォントなら:

```text
3,796 × 4 × 8 × 32 = 3,887,104枚
```

実際にはフォント未収録文字がスキップされるため少し減る場合があります。

数百万枚になるため、先に `--limit-chars` で確認し、ディスク容量にも注意してください。

## 設定変更

`config.json` を編集します。

### 90度単位の8方向にする

```json
"rotations": [0, 45, 90, 135, 180, 225, 270, 315]
```

### 軽い位置ずれも追加する

```json
"position_jitter_px": 1,
"seed": 42
```

同じseed・設定なら同じ位置ずれになります。

### 別の辞書を使う

```bash
python generate.py --dictionary ../digits.txt
```

### 出力先を変更

```bash
python generate.py --output-dir ./my_dataset
```

### 生成予定数だけ確認

```bash
python generate.py --dry-run
```

## 上書き

既存の出力ディレクトリが空でない場合は停止します。

作り直す場合だけ明示的に:

```bash
python generate.py --overwrite
```

としてください。

## パレット

`palettes.json` に32種類の前景色・背景色の組み合わせがあります。

単純な32色ではなく、OCRで起こりやすい

- 黒文字/白背景
- 白文字/暗背景
- 灰色印刷
- 少し黄ばんだ紙
- 低コントラスト
- 青・赤・緑系の印刷

などを混ぜています。

## フォントのライセンス

このリポジトリにはフォントファイルを同梱しません。

利用するフォントのライセンスによっては、フォントから生成した画像の配布条件が定められている場合があります。学習・配布・商用利用を行う前に、**使用する各フォントのライセンスを利用者自身で確認してください**。

`.gitignore` で `fonts/` 内のフォントを通常はコミットしない設定にしています。

## 注意

この初期版は「1文字画像」を安定して大量生成することを優先しています。

今後追加すると有用な機能:

- ガウシアンノイズ / salt-and-pepper
- ぼかし
- JPEG劣化
- 紙・スキャン背景
- train / validation / test 自動分割
- 並列生成
