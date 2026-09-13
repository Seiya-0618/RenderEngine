# レンダリングエンジン開発計画

## 方針

- 動作確認できる単位で実装する
- 個人開発を前提に、過剰な抽象化を避ける
- PBR 対応に必要な範囲で既存構造を整理する
- C++14 準拠を維持する
- DirectX12 のリソース管理は単純さを優先する

## 現在の実装状況

- ✅ Win32 ウィンドウ生成とメインループ
- ✅ DirectX12 の基本初期化
- ✅ Assimp によるモデル読み込み
- ✅ 頂点バッファ / インデックスバッファ生成
- ✅ テクスチャ付き描画
- ✅ Scene による Object / Camera / Texture / Material の管理
- ✅ Object から MaterialIndex による Material 参照
- ✅ Object の LocalTransform / WorldTransform 分離
- ✅ 親子階層による WorldTransform 更新
- ✅ PipelineKey に応じた PSO 切り替え
- ✅ Scene のメインカメラを Renderer が参照
- ✅ Camera 用 ConstantBuffer の分離
- ✅ ObjectConstants / CameraConstants / LightConstants の分離
- ✅ DirectionalLight の定義と描画への接続
- ✅ BasicLighting への Lambert 成分追加
- ✅ BasicLighting への Phong 成分追加
- ✅ テクスチャ作成処理の共通化
- ✅ Diffuse 以外のマップ用フォールバックテクスチャ作成
- ✅ Material 単位の Albedo / Normal / MR テクスチャ参照
- ✅ Normal / MR のフォールバックテクスチャ割り当て
- ✅ Material 単位の 3 スロット SRV テーブル作成
- ✅ Albedo / Normal / MR の連続した DescriptorTable 配置
- ✅ Material から DescriptorTable の GPU ハンドルを Renderer へ渡す経路
- ✅ t0 / t1 / t2 のテクスチャバインド確認

## 現在の課題

- PBR 用の PSO とシェーダーが未実装
- Normal Map を使用するための Tangent 空間処理が未実装
- MR テクスチャをライティング計算へ反映していない
- 複数 Material / PSO を前提とした描画順整理は未実装

## PBR 実装計画

### 1. Material とテクスチャ参照

- [x] Material が Albedo / Normal / MR の参照名を保持する
- [x] テクスチャを `std::wstring` の名前で検索する
- [x] ResourceManager がテクスチャを読み込み、Material の各マップ名を設定する
- [x] 不足する Normal / MR にフォールバックテクスチャ名を設定する
- [x] Material はテクスチャリソースを直接保持せず、参照名を保持する

テクスチャは ResourceManager が作成し、Scene に登録する。
Material はマップ名を保持し、ResourceManager または Scene のテクスチャ管理から名前で解決する。

### 2. SRV テーブル

- [x] Albedo / Normal / MR の固定スロット順を定義する
- [x] Material ごとの SRV ディスクリプタテーブルを作成する
- [x] 3 要素の SRV descriptor range を定義する
- [x] Material から DescriptorTable の GPU ハンドルを渡す
- [x] t0 / t1 / t2 のテクスチャバインドを確認する
- [ ] PBR シェーダーで t0 / t1 / t2 を使用する

固定スロット:

| スロット |       用途        |   ResourceBinding   |
|:--:|:-------------:|:-----------------:|
| t0 | Albedo テクスチャ | `Material.Albedo` |
| t1 | Normal テクスチャ | `Material.Normal` |
| t2 | MR テクスチャ   | `Material.MR`    |

### 3. PBR シェーダー

- [ ] BasicLighting の Phong 実装結果を基準として動作確認する
- [ ] World Normal / World Position / ViewDirection を PBR 用に整理する
- [ ] Tangent を導入して Normal Map を反映する
- [ ] Roughness Map を反映する
- [ ] Metallic Map を反映する
- [ ] 簡易 Cook-Torrance BRDF を実装する
- [ ] DirectionalLight を PBR の光源計算に接続する
- [ ] 必要に応じてトーンマッピングを追加する

### 4. リソース管理上の留意点

- [ ] ResourceManager は Scene の内部コンテナを直接操作せず、Scene の登録 API を使用する
- [ ] ResourceManager と Scene の所有権を重複させない
- [ ] ResourceManager が Scene に直接登録する現在のデータフローは維持する
- [ ] 上記ルールに反する該当箇所があれば修正する

### 5. 今後の描画整理

- [ ] 複数 Material / PSO を前提に描画単位を整理する
- [ ] 必要になった段階で PSO / Material 単位のバケット化を行う
- [ ] Object の責務整理は、現状で問題が発生した場合に実施する

## 実装順序

1. Material の PBR テクスチャスロット整理
2. 固定スロット順の定義
3. SRV テーブルと RootSignature の構築
4. PBR 用シェーダーの実装
5. Normal Map 対応
6. PBR の動作確認
7. 必要に応じた描画順・Object 管理の整理

## 技術的な注意点

- PBR テクスチャが存在しない場合もフォールバックで常に有効な SRV を提供する
- テクスチャの有無で描画経路を分岐させない
- PSO は描画中ではなく初期化時またはロード時に作成する
- Descriptor のスロット順を固定する
- まずは DirectionalLight 1 個の最小構成で実装する
- IBL、複数ライト、シャドウは基本 PBR の動作確認後に追加する

---

## Old の位置づけ

初期計画は実装前の見積もりをもとに作成した。
実装を進めることで作業内容の解像度が上がり、当初の計画と実態に差が生じたため、
完了した内容を反映した新しい計画を再構成している。
旧計画は、計画の変遷を確認できるよう `Old` として残す。

# Old

## 方針

- 個人開発を前提に、機能追加よりも基盤整理を優先する
- Material 参照経路の整理、PipelineKey による描画切り替え、Camera の描画統合を優先する
- 1 機能ごとに動作確認できる単位で実装を進める
- PSO は Renderer が管理し、Material は見た目情報と PipelineKey を保持する

## 実装目標

### 1. 親子階層による座標処理

- [x] LocalTransform と WorldTransform を分ける
- [x] 子はそれぞれ LocalTransform を持つ
- [x] 親の WorldTransform と子の LocalTransform から WorldTransform を更新する
- [x] Renderer は更新済みの WorldTransform を使って描画する

### 2. Material の抽出と整理

- [x] Scene が Material を保持する
- [x] Object が Material を参照する
- [x] テクスチャ付き基本 Material を実装する
- [x] Object → Material → Texture の経路で描画する
- [x] PipelineKey に応じて Renderer が PSO を切り替える
- [ ] Material ごとの描画パラメータ反映方法を整理する
- [ ] 複数 PSO を前提にした描画順を整理する

### 3. Camera の描画系への統合

- [x] Scene のメインカメラを Renderer が参照する
- [x] View / Projection を Camera から取得する
- [x] Renderer 内の固定カメラ値を削減する
- [x] Camera 用 ConstantBuffer を分離する

### 4. Light の導入

- [x] DirectionalLight を定義する
- [x] Scene に Light を保持する
- [x] Light 用 ConstantBuffer を作成する
- [x] BasicLighting で Light を使用する

### 5. 最低限の整理

- [ ] Transform 定義を整理する
- [ ] Object / Scene / Renderer の責務を整理する
- [ ] リソース管理の役割を明確にする
- [ ] Scene の Object 管理構造を見直す
- [ ] TextureName 直参照など旧経路を整理する

## 旧実装順序

### Phase 1

1. PipelineKey による PSO 切り替え
2. Scene のメインカメラを描画へ接続
3. Renderer の固定カメラ値を削減
4. Camera 用 ConstantBuffer の分離

### Phase 2

5. BasicLighting 系 PSO の導入
6. Light / Material 用 ConstantBuffer の整理
7. BasicLighting へのライト反映
8. Material パラメータの描画反映
9. RootSignature の共有範囲整理

### Phase 3

10. 複数オブジェクト描画の整理
11. PSO / Material 単位のバケット化
12. Scene の Object 管理構造の見直し