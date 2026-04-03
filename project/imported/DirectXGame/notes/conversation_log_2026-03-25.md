# Conversation Log 2026-03-25

このファイルは、2026-03-25 時点の作業会話を後から追跡できるように、主要な依頼、判断、実施内容、ブランチ情報を時系列でまとめたログです。
UI 上の完全な生ログではなく、実作業の再現に必要な情報を残すことを目的とします。

## 1. GitHub Actions 確認と修正

- `master` ブランチの Debug / Release build の GitHub Actions を確認。
- `windows-2026` が runner として不適切で、queued / timeout の原因になっていることを確認。
- `.github/workflows/DebugBuild.yml`
- `.github/workflows/ReleaseBuid.yml`
  - `runs-on: windows-2026` を `windows-2025` に修正。
- `master` へ push 済みの状態まで確認。

## 2. モデル生成負荷の軽減

- 同一 OBJ を個体ごとに毎回ロードしていた経路を見直し。
- `ModelCache` を導入して、敵、弾、ドローン、EXP オーブ、各種パーティクルでモデル共有を実施。
- 音声や白テクスチャも共有ロードへ寄せて、生成スパイクを低減。

主な対象:

- `core/ModelCache.h`
- `enemy/Enemy.cpp`
- `player/weapons/NormalBullet.cpp`
- `player/weapons/OrbitBullet.cpp`
- `items/ExpOrb.cpp`

## 3. FPS 表示の追加と修正

- 最初は `GameScene` から ImGui を直接呼び出して FPS を表示。
- その後 `imgui.cpp line 7528` 付近で例外が発生。
- 原因は `KamataEngine` 側の ImGui フレーム管理と競合していた可能性が高いと判断。
- 参考コードを確認し、最終的に `main.cpp` の `imguiManager->Begin()` / `End()` の間へ FPS 表示を移設。

## 4. 音声まわりの整理

- 現在ついている音声と、追加すべき音声を一覧化。
- UI 操作、ゲーム開始、レベルアップ、被弾、死亡などへ音を追加。
- 一部の `wav` が実際には MP3 で、`pSourceVoice == nullptr` になる問題を確認。
- `bgm_title.wav` を正しい PCM WAV に修正。
- UI 操作時のクラッシュ要因を除去。

## 5. ファイル構成整理

- ソースコードの物理配置を整理。

整理後の主な構成:

- `core`
- `effects`
- `items`
- `enemy`
- `world`
- `scene/core`
- `scene/game`
- `scene/title`
- `scene/result`
- `player/core`
- `player/weapons`
- `ui/common`
- `ui/gauge`
- `ui/hud`

- `DirectXGame.vcxproj`
- `DirectXGame.vcxproj.filters`
  も物理構成に合わせて更新。

関連ブランチ:

- `ファイル構成整理と音声修正`

## 6. Resources 整理

- `Resources` 配下の整理を実施。
- ただし `CreateFromOBJ` の都合でモデルだけは `Resources` 直下配置が必要と判明。
- `octopus` など一部モデルは旧配置に戻した。

## 7. GameScene の肥大化対応

- `GameScene` の責務を分解。
- 開始待ち、ポーズ、レベルアップなどを controller 化。

追加した主なクラス:

- `scene/game/GameStartController`
- `scene/game/GamePauseController`
- `scene/game/GameLevelUpController`

- 既存 `Pause` クラスの責務は `GamePauseController` へ統合。
- `ui/Pause.*` は削除。

## 8. ポーズからリザルト遷移しない不具合

- `Finish` 選択後にカーテンは閉じるが、リザルト遷移完了判定が止まる問題を修正。
- `GameScene::Update()` 冒頭で毎フレーム遷移完了を確認する形へ変更。

## 9. グリッドプレーンの改善

- 原点固定の1枚板では一定範囲外で未描画領域が発生。
- プレイヤー追従方式を試したが、スナップが見えて不自然。
- 最終的に 3x3 タイル方式へ変更。
- さらにタイル境界で UV 位相が切れて見える問題を修正し、つなぎ目を自然に調整。
- シェーダーによる無限グリッド化も試したが、このプロジェクトでは shader 反映経路が実行系と一致していなかったため撤回。

対象:

- `world/GridPlane.h`
- `world/GridPlane.cpp`

## 10. コードレビュー観点での整理

チェックリストに基づいて以下を実施。

- `EnemyManager` の更新処理と当たり判定を関数分割。
- `GameData` と `IScene::sceneNo_` によるグローバル状態を撤去。
- `GameSessionContext` を導入して、シーン間共有データを集約。
- `Score`, `Timer`, `ExpGauge` の重複数字表示処理を `DigitSpriteUtil` に共通化。
- public API コメントを主要クラスへ追加。
- コメントアウトされた不要コードや未使用メンバを整理。

新規追加:

- `core/GameSessionContext.h`
- `ui/common/DigitSpriteUtil.h`
- `ui/common/DigitSpriteUtil.cpp`

削除:

- `core/GameData.h`
- `core/GameData.cpp`

## 11. namespace の統一

- 一部ファイルで使われていた `EngineLayer` を廃止。
- 自作クラス、列挙、共有データ、補助関数を `DirectXGame` namespace 配下へ統一。
- header 内での `using namespace` は使用しない方針を維持。
- `DigitSpriteUtil` は `DirectXGame::DigitSpriteUtil` に変更。

主な対象:

- `scene/core/IScene.*`
- `scene/core/SceneManager.*`
- `scene/game/*`
- `scene/title/*`
- `scene/result/*`
- `player/core/*`
- `player/weapons/*`
- `enemy/*`
- `effects/*`
- `world/*`
- `items/*`
- `ui/*`
- `core/GameSessionContext.h`
- `core/ModelCache.h`
- `core/main.cpp`

ビルド確認:

- Debug / x64 / `v143` 指定でビルド成功

## 12. ブランチと commit

現在のローカルブランチ:

- `namespace統一と構成整理`

現在のローカル commit:

- `2d795e2`
- メッセージ: `namespace統一と構成整理`

備考:

- push は試行したが、GitHub への権限不足により `403` で失敗。
- エラー内容: `Permission to OKUBO-0/MyGame.git denied`

## 13. 一時ファイル整理

不要と判断して削除:

- `.codex-build`
- `.tmp_pdfinspect`
- `docs`

これらはビルド一時成果物、PDF 確認用一時フォルダ、補助資料であり、ゲーム本体には不要。

## 14. 今後の再開用メモ

この会話の続きとして再開するなら、最低限以下を参照すれば流れを復元しやすい。

- 作業ブランチ: `namespace統一と構成整理`
- commit: `2d795e2`
- 主要変更:
  - ファイル構成整理
  - `GameSessionContext` 導入
  - `DigitSpriteUtil` 共通化
  - `GamePauseController` 統合
  - `GridPlane` 3x3 化
  - `DirectXGame` namespace 統一

