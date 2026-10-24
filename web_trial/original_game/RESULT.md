# 元ゲームのEmscriptenビルド試験（2026-10-20）

## 結果

**元ゲームをそのままブラウザーで動かすビルドは失敗。実行できるHTMLは未生成。**
四角のサンプルとは別に、実際のゲームソースを対象に試験しました。

- 試験元：作業中の `ginjyo_motion` のファイル
- 設定：Visual Studioプロジェクトの Debug|x64 に登録された86個のC++ファイル
- SDK：Emscripten 6.0.13、C++17、例外を有効化
- ゲームのソース・設定190ファイルを保存。assets（129ファイル、約87 MB）とsaveもコピー
- DxLibはインストール済み3.24fのヘッダーを使用。nlohmannのヘッダーもコピー

## 実施内容

1. 元ファイルをバイト単位でコピーし、そのままHTML出力を指定してビルド。
2. 別のコピーだけCP932/UTF-8を読み分け、UTF-8に統一して再試行。
3. プロジェクトの86ファイルと未登録2ファイルを、1ファイルずつWasmオブジェクトへコンパイル。

どちらの全体ビルドも `ArmorEnemyEntity.cpp` から読み込まれるDxLibヘッダーで停止。

```text
DxDataTypeWin.h:21:10: fatal error: 'windows.h' file not found
```

個別試験は **33ファイル成功、55ファイル失敗**。
失敗55ファイルの最初のエラーはすべて上記のWindowsヘッダー依存です。
成功した例：Actor、AnimationComponent、Camera、GravityComponent、
VelocityComponent、TransformComponent、Scene、MapData、HitboxData、SaveManager。

これはコンパイルの結果です。成功ファイルもリンク・ブラウザー動作・保存の永続化を
保証しません。また、最初のエラーの後に隠れているエラーは未調査です。

## なぜWindows SDKのパスを追加するだけでは直らないか

コンパイル対象はWindowsの実行形式ではなくWebAssemblyです。
Windows用のヘッダーとDxLibの.libを追加しても、ブラウザー内にWindows APIの
実装が用意されるわけではありません。現状ではリンク段階にも到達していません。
機能を空実装にして成功したように見せる対応は行っていません。

元コードではGameのHWND、QueryPerformanceCounter、MessageBoxA、
MainのWinMain、ProcessMessageのループなどにも対応が必要です。

## 保存場所

- `snapshot/`：元ゲームのソース・設定・素材のコピー
- `utf8/`：文字コードのみ変更したビルド試験用コピー
- `vendor/`：DxLibとnlohmannのヘッダー
- `logs/original.log`：無修正コピーでの全体ビルド結果
- `logs/utf8.log`：文字コード変換後の全体ビルド結果
- `logs/units/`：個別コンパイル結果
- `unit-results.json`：各ファイルの成否
- `manifest.json`：対象一覧・元ファイルのSHA-256・文字コード

SDK、素材コピー、生成物はGitの追加対象から除外しています。
元ソースのハッシュが変わっていないことも確認済みです。
元ゲームのブランチ切り替え、コミット、プッシュは行っていません。

## 再試行

このフォルダで：

```powershell
python try_build.py --build-only
python check_units.py
```

スナップショット作成の上書きは拒否します。
プロジェクト未登録の `boss_02.cpp` と `SkillMenuActor.cpp` は個別試験のみ対象です。
現在のブランチにない別ブランチのコードは含みません。

## 次に必要な作業

単純変換を続けるのではなく、コピー側で描画・入力・音声のWeb対応実装と
起動・ゲームループの移植を行う段階になります。
まず元ゲームのタイトル画面を実際の素材で表示するところを目標に、
DxLibの使用機能を整理して対応範囲を決める必要があります。
