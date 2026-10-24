# Web移植試作の現在の状態

以前の無修正ビルド失敗を受けて、`port/`のコピーだけを修正しました。
元のschool_git直下のソース・設定はハッシュ比較で変更なしです。

## 解消した問題

- Windows版DxLibヘッダーを使用せず、ゲームが使うAPIの一部を`web/DxLib.h`と
  `web/bridge.cpp`でCanvas・Web Audio・ブラウザー入力へ接続。
- WinMainをmainに変更。時間計測はemscripten_get_nowに置換。
- ScreenFlipでAsyncifyによる非同期待機を行い、既存ループからブラウザーに制御を返す。
- 不要なWindowsヘッダー・Windowsコンソール初期化をコピー側で除去。
- PlaySceneのデストラクターを型が完全に定義された.cpp側へ移動。
- PlayerEntity::SetIsHideの戻り値欠落をコピー側で修正。
- ソースとアセット内のCSV/TXT/JSONをUTF-8へ変換（コピーのみ）。

## 出力

`build/web/index.html`、`index.js`、`index.wasm`、`index.data`。
元プロジェクトに登録された86ファイルとWeb用バックエンドをビルド。
ゲーム素材約87 MBを含むため、初回読み込みには時間がかかります。
ブラウザーで元のタイトル画面・ステージ・プレイヤー・HP表示を確認しました。

## まだWindows版と同等とは言えない点

- フォントはブラウザー標準。DxLib完全互換ではなく、このゲームの使用範囲の実装。
- セーブはタブ内の仮想ファイルのみ。再読み込み後の永続化は未実装。
- 音声・ゲームパッド・全ステージ・戦闘全体の動作検証は未完了。
- 元の素材に存在しない`assets/images/uies/bg.png`、`kari/player.png`、
  `kari/det.png`は読めません。架空の画像で置き換えていません。
- イベントCSVの画像パスとしてL/Rが渡る元コードとデータの不整合が残っています。
- 一部の元コード由来のコンパイラ警告は残っています。
- `boss_02.cpp`は元のプロジェクト未登録なので、このビルドの対象外です。

## 再ビルド・再起動

このフォルダで：

```powershell
python build_web.py
python -m http.server 8086 --bind 127.0.0.1 --directory build/web
```

http://127.0.0.1:8086/ を開き、「ゲームを起動」をクリック。
タイトル画面ではEnter。移動は矢印またはWASD、ジャンプはSpace、
ダッシュはU、通常攻撃はI、クナイはO、メニューはM。
サーバーが既に起動していれば、二重起動せずURLを開いてください。
手動で起動したサーバーはCtrl+Cで停止できます。

試作バックエンドとスクリプトを保持すれば、保存済みsnapshot/utf8から
portの変更を再生成できます。SDK・素材・ビルド出力はGit除外対象です。
コミット・プッシュは実施していません。

## 2026-10-21 通常攻撃中の停止調査

- 実際のブラウザログでは、EnemyHPBarの破棄からfreeへ進んだ際にabortしていました。
- 再現テストで、PlayerEntity::CheckAttackHitがSceneのActor配列を参照して走査中、敵の死亡によるドロップ追加で配列が再確保され、解放済み領域を読む不具合をAddressSanitizerで確認しました。
- コピー側のCheckAttackHitは攻撃開始時の対象リストをコピーして走査し、死亡済みActorを除外するよう修正しました。build_web.pyにも再生成用の修正を記録しています。
- 配列を容量いっぱいにした状態から通常攻撃で撃破・ドロップ追加・HPバー削除を8回繰り返す検査が、修正後はAddressSanitizerでPASSしました。
- 診断ログ: logs/combat-asan-before.txt。検査コマンド: python build_web.py --regression --asan。その後、通常版はpython build_web.pyでビルドします。
- Asyncifyの検査時スタック不足を避けるため、保存用スタックを64 KiBに設定しました。
- ユーザーの長時間プレイ経路すべてを再現したわけではありません。今回再現できた不正アクセスを修正したもので、別原因の停止がないことまでは保証していません。
