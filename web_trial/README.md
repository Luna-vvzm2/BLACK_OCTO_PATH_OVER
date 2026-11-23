# Emscriptenの小さな試作環境

元のWindowsゲームのコードやVisual Studioプロジェクトには変更を加えません。
`main.cpp`の入力・移動処理をWebAssemblyへコンパイルし、Canvasへ描画します。
DxLib互換性の検証やゲーム本体の移植は、次の段階です。

## 最初の準備

PowerShellでこのフォルダを開き、実行します（GitとPythonが必要）。

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\setup.ps1
```

SDKは公式emsdkから、このフォルダの`tools/emsdk`内に導入します。
バージョンは6.0.13に固定。システム全体のPATHは変更しません。
SDK・生成物・ログはこのフォルダの`.gitignore`で除外しています。

## ビルドと起動

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\serve.ps1
```

ブラウザーで http://127.0.0.1:8085/ を開きます。
サーバーはこのPCだけからアクセス可能です。停止はCtrl+C。
HTMLのダブルクリックではなく、HTTP経由で開いてください。

矢印キー / WASDで移動。中央に戻すボタンでリセット。
斜め移動は速度を補正し、画面端で停止します。
フォーカスを失うと入力を解除します。

## ファイル

- `main.cpp`: C++の状態・入力・移動と描画呼び出し
- `shell.html`: ページの見た目と起動表示
- `build/index.html`, `index.js`, `index.wasm`: 生成物
- `tools/emsdk`: 試作用SDK。Gitへ追加しないでください。

これはフォルダ単位の分離で、仮想マシンではありません。
次は相談のうえ、床・重力などを一つずつ追加します。
