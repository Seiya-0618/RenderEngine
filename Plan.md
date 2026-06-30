# レンダリングエンジン開発計画

## 方針
- 個人開発を前提に、機能追加よりも基盤整理を優先する
- 当面は Material 参照経路の整理、PipelineKey による描画切り替え、Camera の描画統合を優先する
- 1 機能ごとに動作確認できる単位で実装を進める
- PSO は Renderer が管理し、Material は見た目情報と PipelineKey を保持する方針とする

## 現在の実装状況
- ✅ Win32 ウィンドウ生成とメインループ
- ✅ DirectX12 の基本初期化
- ✅ Assimp によるモデル読み込み
- ✅ 頂点バッファ / インデックスバッファ生成
- ✅ テクスチャ付き描画の基本実装
- ✅ Scene による Object / Camera / Texture の基本管理
- ✅ Camera クラスの定義
- ✅ LocalTransform と WorldTransform の分離
- ✅ 親子階層をたどる WorldTransform 更新
- ✅ Renderer は更新済みの WorldTransform を使って描画
- ✅ Scene による Material 管理
- ✅ Object から MaterialIndex による Material 参照
- ✅ Render で Object -> Material -> Texture の経路で描画
- ✅ PipelineKey に応じた PSO 切り替え
- ✅ Scene のメインカメラを Renderer が参照
- ✅ View / Projection を Camera から取得して描画へ反映

## 現在の課題
- Material の質感パラメータは保持しているが、描画にはまだ十分反映されていない
- Lambert / Phong など追加 PSO は作成できるが、ライティング用パラメータ整理はこれから
- Renderer, Scene, Object, ResourceManager の責務分担がまだ曖昧
- Scene 内の Object 管理が `vector + id/index map` 前提になっており、削除時の再構築や責務整理が必要
- 複数 PSO / Material を前提にした描画順整理（バケット化）が未実装

## 当面の実装目標

### 1. 親子階層による座標処理
- [x] LocalTransform と WorldTransform を分ける
- [x] 子はそれぞれ LocalTransform を持つ
- [x] レンダリング前に階層を上からたどり、親の WorldTransform と子の LocalTransform を行列演算して WorldTransform を更新する
- [x] Renderer は更新済みの WorldTransform を使って描画する

### 2. Material の抽出と整理
- [x] Scene が Material を保持する
- [x] Object が Material を参照する形に整理する
- [x] まずはテクスチャ付き基本 Material を 1 つ実装する
- [x] Render で Object -> Material -> Texture の経路で描画する
- [x] PipelineKey に応じて Renderer が PSO を切り替える
- [ ] Material ごとの描画パラメータ反映方法を整理する
- [ ] 複数 PSO を前提にした描画順の整理を行う

### 3. Camera の描画系への統合
- [x] Scene のメインカメラを Renderer が参照する
- [x] View / Projection を Camera から取得する
- [x] Renderer 内の固定カメラ値を削減する

### 4. 最低限の整理
- [ ] Transform 定義の整理
- [ ] Object, Scene, Renderer の責務を整理
- [ ] リソース管理の役割を明確にする
- [ ] Scene の Object 管理構造を整理する（`objectIDs` の削減、削除時の map 再構築、必要なら管理方式の見直し）
- [ ] TextureName 直参照など旧経路を整理する

## 実装順序

### Phase 1
1. PipelineKey による PSO 切り替え
2. Scene のメインカメラを描画へ接続
3. Renderer の固定カメラ値を削減

### Phase 2
4. Lambert など追加 PSO の導入
5. Material パラメータの描画反映
6. RootSignature の共有範囲整理
7. Light / Material 用 ConstantBuffer の整理

### Phase 3
8. 複数オブジェクト描画の整理
9. PSO / Material 単位のバケット化
10. Scene の Object 管理構造の見直し

## 技術的な注意点
- C++14 準拠を維持する
- DirectX12 のディスクリプタ管理は単純さを優先する
- まずは動く形を作り、その後に整理する
- 個人開発では過剰な抽象化を避ける
- Object 管理構造の見直しは描画経路の整理後に着手する
- PSO の生成は描画中ではなく初期化時またはロード時に行う