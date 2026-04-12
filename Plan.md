# レンダリングエンジン開発計画

## 方針
- 個人開発を前提に、機能追加よりも基盤整理を優先する
- 当面は 親子階層による座標処理 と Material の抽出・整理 を最優先とする
- 1 機能ごとに動作確認できる単位で実装を進める

## 現在の実装状況
- ✅ Win32 ウィンドウ生成とメインループ
- ✅ DirectX12 の基本初期化
- ✅ Assimp によるモデル読み込み
- ✅ 頂点バッファ / インデックスバッファ生成
- ✅ テクスチャ付き描画の基本実装
- ✅ Scene による Object / Camera / Texture の基本管理
- ✅ Camera クラスの定義

## 現在の課題
- 親子関係は保持しているが、親の Transform が子に反映されていない
- Material 関連の処理が Init と Render に分散している
- Camera はあるが、描画時には Renderer 内の固定値を使っている
- Renderer, Scene, Object の責務分担がまだ曖昧
- Scene 内の Object 管理が `vector + id/index map` 前提になっており、削除時の再構築や責務整理が必要

## 当面の実装目標

### 1. 親子階層による座標処理
- [ ] LocalTransform と WorldTransform を分ける
- [ ] 子はそれぞれ LocalTransform を持つ
- [ ] レンダリング前に階層を上からたどり、親の WorldTransform と子の LocalTransform を行列演算して WorldTransform を更新する
- [ ] Renderer は更新済みの WorldTransform を使って描画する

### 2. Material の抽出と整理
- [ ] DXMaterial に描画設定を集約する
- [ ] ルートシグネチャと PSO を Material 側で管理する
- [ ] Object または Mesh が Material を参照する形に整理する
- [ ] まずはテクスチャ付き基本 Material を 1 つ実装する

### 3. Camera の描画系への統合
- [ ] Scene のメインカメラを Renderer が参照する
- [ ] View / Projection を Camera から取得する
- [ ] Renderer 内の固定カメラ値を削減する

### 4. 最低限の整理
- [ ] Transform 定義の整理
- [ ] Object, Scene, Renderer の責務を整理
- [ ] リソース管理の役割を明確にする
- [ ] Scene の Object 管理構造を整理する（`objectIDs` の削減、削除時の map 再構築、必要なら管理方式の見直し）

## 実装順序

### Phase 1
1. 親子階層による WorldTransform 更新
2. Renderer の Transform 更新処理の整理
3. Scene のメインカメラを描画へ接続

### Phase 2
4. DXMaterial の責務定義
5. ルートシグネチャと PSO の Material への移管
6. テクスチャ付き基本 Material の実装
7. Object / Mesh と Material の関連付け

### Phase 3
8. 基本ライティングの導入
9. Material 定数の追加
10. 複数オブジェクト描画の整理

## 技術的な注意点
- C++14 準拠を維持する
- DirectX12 のディスクリプタ管理は単純さを優先する
- まずは動く形を作り、その後に整理する
- 個人開発では過剰な抽象化を避ける
- Object 管理構造の見直しは親子階層と描画経路の整理後に着手する