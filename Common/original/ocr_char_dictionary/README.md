# OCR Character Dictionary

OCR / 文字認識モデルでそのまま使える、**UTF-8・1行1文字**の文字辞書です。

外部ライブラリは不要です。必要な `.txt` を1個コピーするだけで使えます。

## 30秒で使う

全部入りなら `all.txt` を使います。

```bash
curl -L \
  -o chars.txt \
  https://raw.githubusercontent.com/tomiya7688/Open-librarys-for-Japanese/main/Common/original/ocr_char_dictionary/all.txt
```

Python:

```python
from pathlib import Path

chars = Path("chars.txt").read_text(encoding="utf-8").splitlines()

id_to_char = dict(enumerate(chars))
char_to_id = {char: i for i, char in enumerate(chars)}

print(len(chars))
print(char_to_id["あ"])
```

C++:

```cpp
#include <fstream>
#include <string>
#include <vector>

std::ifstream file("chars.txt");
std::vector<std::string> chars;

for (std::string line; std::getline(file, line); ) {
    chars.push_back(line);
}
```

C++ の `std::string::size()` は UTF-8 の**バイト数**です。
この辞書の「1行1文字」は Unicode の1文字という意味なので、漢字によって1〜4バイトになります。

## ファイル一覧

| ファイル | 文字数 | 内容 |
| --- | ---: | --- |
| `digits.txt` | 10 | 0〜9 |
| `alphabet_lower.txt` | 26 | a〜z |
| `alphabet_upper.txt` | 26 | A〜Z |
| `ascii_symbols.txt` | 32 | ASCII の半角記号（空白を除く） |
| `hiragana_gojuon.txt` | 46 | 現代仮名遣いの基本ひらがな |
| `katakana_gojuon.txt` | 46 | 現代仮名遣いの基本カタカナ |
| `hiragana.txt` | 88 | 基本 + 濁音/半濁音 + 小書き + 一部追加文字 |
| `katakana.txt` | 91 | 基本 + 濁音/半濁音 + 小書き + 追加文字 + 長音符 |
| `kanji_elementary.txt` | 1026 | 小学校の学年別漢字配当表に含まれる漢字 |
| `kanji_junior_high.txt` | 1110 | 常用漢字から小学校配当漢字を除いた集合 |
| `kanji_kanken_level2.txt` | 2136 | 漢検2級相当 = 常用漢字すべて |
| `kanji_jinmeiyo.txt` | 864 | 常用漢字外の現行人名用漢字 |
| `basic.txt` | 94 | 数字 + 英字 + ASCII半角記号 |
| `japanese.txt` | 2315 | ひらがな + カタカナ + 常用漢字 |
| `all.txt` | 2409 | `basic.txt` + `japanese.txt`（互換性維持版） |
| `japanese_extended.txt` | 3179 | かな + 常用漢字2136 + 人名用漢字864 |
| `all_extended.txt` | 3273 | `basic.txt` + `japanese_extended.txt` |
| `kanji_jis_level1.txt` | 2965 | JIS X 0208 第1水準漢字そのもの |
| `japanese_jis_level1.txt` | 3476 | `japanese_extended.txt` + JIS第1水準の不足分 |
| `all_jis_level1.txt` | 3570 | `all_extended.txt` + JIS第1水準の不足分 |
| `fullwidth_digits.txt` | 10 | 全角数字 ０〜９ |
| `fullwidth_alphabet_upper.txt` | 26 | 全角英大文字 Ａ〜Ｚ |
| `fullwidth_alphabet_lower.txt` | 26 | 全角英小文字 ａ〜ｚ |
| `fullwidth_symbols.txt` | 32 | 全角ASCII記号 |
| `fullwidth_ascii.txt` | 94 | Unicode U+FF01〜U+FF5E の全角ASCII互換文字 |
| `japanese_punctuation.txt` | 30 | 句読点・括弧・踊り字・波ダッシュ等 |
| `common_math_symbols.txt` | 37 | よく使う数学記号 |
| `common_currency_symbols.txt` | 13 | 通貨記号 |
| `common_arrows.txt` | 15 | 矢印 |
| `common_marks.txt` | 32 | 図形・注記・著作権等の一般記号 |
| `common_units.txt` | 14 | 温度・角度・単位記号 |
| `symbols_extended.txt` | 227 | 上記記号セットを重複除去して結合 |
| `all_jis_level1_symbols.txt` | 3796 | `all_jis_level1.txt` + 記号拡張 |

## 「50音」が46文字なのはなぜ？

現代の五十音表には空いているマスがあるため、基本文字として実際に並べると46文字です。

そのため、

- `hiragana_gojuon.txt`: 46文字
- `katakana_gojuon.txt`: 46文字

としています。

OCRでは濁音や小書き文字も必要になることが多いため、実用向けに `hiragana.txt` と `katakana.txt` も用意しています。

## 漢字

### 小学校

`kanji_elementary.txt` は、現行の小学校学習指導要領の学年別漢字配当表に対応する **1,026字**です。

### 中学校

中学校の学習指導要領では、第3学年までに「その他の常用漢字の大体を読むこと」とされています。

この辞書ではOCRの文字集合として明確に扱えるよう、

```text
常用漢字 2,136字 - 小学校 1,026字 = 1,110字
```

を `kanji_junior_high.txt` としています。

これは「中学校で全1,110字を必ず書けるようにする」という意味ではなく、**OCR用の文字集合として常用漢字を小学校分とそれ以外に分割したもの**です。

常用漢字の `𠮟` (U+20B9F) もそのまま収録しています。

## 半角記号

`ascii_symbols.txt` は ASCII の表示可能文字のうち、

- 数字
- A-Z / a-z
- 空白

を除いた32文字です。

空白は見た目では「空行」と区別できず、OCR方式によって扱いも異なるため収録していません。

## CTC blank / PAD / UNK について

この辞書には以下の予約トークンを含めません。

- CTC blank
- PAD
- UNK
- BOS / EOS
- 空白クラス

フレームワークやモデルごとに必要な位置が異なるため、利用側で追加してください。

例えばCTCなら、

```python
chars = Path("all.txt").read_text(encoding="utf-8").splitlines()

blank_id = len(chars)
num_classes = len(chars) + 1
```

のように、辞書末尾の次を blank にできます。


## 漢検2級・人名用漢字まで使いたい場合

`kanji_kanken_level2.txt` は、日本漢字能力検定の2級相当として扱いやすいよう、常用漢字2,136字を1ファイルにまとめたものです。

漢検2級の対象は「すべての常用漢字 2,136字」なので、文字集合としては従来の

```text
kanji_elementary.txt 1,026字
+
kanji_junior_high.txt 1,110字
=
2,136字
```

と同じです。

さらに、2026年6月26日の戸籍法施行規則改正で「勒」が追加され、現行の人名用漢字は864字です。

`kanji_jinmeiyo.txt` にはこの864字を収録しています。常用漢字2,136字と合わせると、名前に使える漢字はちょうど3,000字になります。

氏名、住所録、申込書、名簿、請求書などをOCRするなら、まず `all_extended.txt` を使うのがおすすめです。

### 既存モデルとの互換性

`all.txt` と `japanese.txt` は既存の class ID を壊さないため変更していません。

新しく学習するモデルで文字範囲を広げたい場合だけ、

- `japanese_extended.txt`
- `all_extended.txt`

を選んでください。

### 公的な基準

- 漢検2級: 日本漢字能力検定協会「各級の概要」
  https://www.kanken.or.jp/kanken/grades/overview/
- 子の名に使える漢字: 法務省
  https://www.moj.go.jp/MINJI/minji86.html
- 戸籍法施行規則: e-Gov法令検索
  https://laws.e-gov.go.jp/law/322M40000010094/



## JIS第1水準まで使いたい場合

JIS X 0208 の第1水準漢字 **2,965字**を `kanji_jis_level1.txt` に収録しています。

JIS第1水準は区点の16区〜47区に配置された漢字集合です。この辞書では、その区点範囲を基準に2,965字であることを検証しています。

### 上位互換版

JIS第1水準は、常用漢字や人名用漢字と完全な包含関係ではありません。

そのため `japanese_jis_level1.txt` / `all_jis_level1.txt` は、既存の拡張版を削るのではなく、

```text
既存の拡張辞書
+
JIS第1水準のうち未収録の漢字
```

として作っています。

- `japanese_jis_level1.txt`: 3476文字
- `all_jis_level1.txt`: 3570文字
- 既存の `japanese_extended.txt` に対してJIS第1水準から追加された文字: 297字

既存モデルとの互換性を守るため、`japanese_extended.txt` と `all_extended.txt` 自体は変更していません。

日本語OCRを新しく学習するなら、広めの実用セットとして `all_jis_level1.txt` を使うのが簡単です。



## 全角文字・日本語記号まで使いたい場合

日本語OCRでは、漢字やかなだけでなく、全角の英数字・記号、括弧、句読点、数学記号、通貨記号などが頻繁に現れます。

`fullwidth_ascii.txt` には Unicode の **U+FF01〜U+FF5E** をそのまま並べた94文字を収録しています。これは ASCII の `!`〜`~` に対応する全角互換文字です。

内訳:

- 全角数字: 10文字
- 全角英大文字: 26文字
- 全角英小文字: 26文字
- 全角記号: 32文字

さらに日本語文書向けに、

- 句読点・括弧・踊り字
- 数学記号
- 通貨記号
- 矢印
- 図形・注記記号
- 温度・単位記号

をカテゴリ別ファイルで用意しています。

全部まとめて使いたい場合は `symbols_extended.txt`、JIS第1水準まで含む現在の最大セットを使いたい場合は **`all_jis_level1_symbols.txt`** を使ってください。

### 全角空白について

U+3000 IDEOGRAPHIC SPACE（全角空白）はこの辞書には自動で入れていません。

理由は、見た目が空行と区別しづらく、ローダー側で `strip()` されると消え、CTC等では空白クラスの扱いもモデルごとに異なるためです。

必要な場合は利用側で明示的にクラスへ追加してください。

### Unicodeの基準

- Halfwidth and Fullwidth Forms
  https://www.unicode.org/charts/PDF/UFF00.pdf
- CJK Symbols and Punctuation
  https://www.unicode.org/charts/PDF/U3000.pdf



## OCR学習用画像データセットを生成する

辞書と同じclass IDで文字画像を作りたい場合は、[dataset_generator](./dataset_generator/README.md) を使えます。

- 複数フォント
- 8段階の小回転
- 32色パレット
- 64x64 PNG
- labels.csv 自動生成
- フォント未収録文字の自動スキップ

画像そのものをGitから取得するのではなく、必要なフォントを用意してローカルで再現可能に生成する方式です。


## 重要: 学習開始後は並び順を変えない

OCRモデルでは一般に、

```text
行番号 = class ID
```

として扱います。

例えば辞書の3行目が `2` なら、そのモデルではその位置が `2` のクラスになります。

**学習済みモデルを使い続ける場合、辞書の並び替え・途中挿入は互換性を壊します。**

文字を増やしたい場合は、既存順序を維持したまま末尾追加するなど、モデル側の仕様に合わせてください。

## 形式

- UTF-8
- BOMなし
- LF改行
- 空行なし
- 1データ行 = Unicode 1文字
- 各ファイル内に重複なし

## 並び順

- 数字: 0 → 9
- 英字: a → z / A → Z
- ASCII記号: ASCIIコード順
- 五十音: 五十音順
- 漢字: 常用漢字表側の掲載順を基準に固定
- 結合済み辞書: READMEのファイル説明順に結合

## 出典・基準

漢字の文字集合は、以下の公的資料を基準にしています。

- 文部科学省「小学校学習指導要領（平成29年告示）」学年別漢字配当表  
  https://www.mext.go.jp/content/20230120-mxt_kyoiku02-100002604_01.pdf
- 文部科学省「中学校学習指導要領（平成29年告示）」  
  https://www.mext.go.jp/content/20230120-mxt_kyoiku02-100002604_02.pdf
- 文化庁「常用漢字表の音訓索引」  
  https://www.bunka.go.jp/kokugo_nihongo/sisaku/joho/joho/kijun/naikaku/kanji/joyokanjisakuin/index.html

検証日: 2026-11-04

## こんな用途に

- CRNN + CTC の文字辞書
- OCR Transformer の vocabulary
- 合成文字画像生成
- 文字分類器
- OCRの評価用ラベル変換
- 日本語文字認識の試作
