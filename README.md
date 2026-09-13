# RenderEngine

C++14 と DirectX 12 を使用して開発しているリアルタイムレンダリングエンジンです。Win32 ウィンドウ上でモデル、テクスチャ、Material、ライティングを管理し、GPU へ描画コマンドを発行する最小構成の描画基盤を実装しています。

## 概要

現在は、次の描画経路を中心に構成しています。

Assimp
  ↓
ResourceManager
  ↓
Scene（Object / Texture / Material）
  ↓
Renderer
  ↓
DirectX 12

Material はテクスチャの参照名を保持し、ResourceManager がテクスチャリソースと SRV を作成します。Renderer は Material の DescriptorTa	ble を描画時にバインドします。

## 主な実装

- Win32 ウィンドウとメインループ
- DirectX 12 のデバイス、SwapChain、CommandQueue の初期化
- Assimp によるモデル読み込み
- 頂点 / インデックスバッファの作成
- Scene による Object / Camera / Texture / Material 管理
- LocalTransform / WorldTransform と親子階層更新
- PipelineKey による PSO 切り替え
- DirectionalLight を使用した BasicLighting
- Lambert / Phong ベースのライティング
- Material 単位の Albedo / Normal / MR DescriptorTable
- 不足テクスチャ用のフォールバックリソース

## テクスチャバインド

Material の DescriptorTable は次のスロット構成です。

t0: Albedo
t1: Normal
t2: MR

MR は Metallic / Roughness 用のテクスチャスロットです。現在使用している Assimp のバージョンでは Metallic / Roughness マップの取得に制限があるため、不足時はフォールバックテクスチャを使用します。

## 使用技術

- C++14
- DirectX 12
- HLSL
- Win32 API
- Assimp
- DirectXTex

## 現在のテーマ

- Material 単位のテクスチャ管理
- Albedo / Normal / MR の DescriptorTable
- Lambert / Phong ベースの BasicLighting
- PBR 用 PSO / シェーダーの実装

## 今後の実装

- PBR 用 PSO / シェーダーの追加
- Normal Map の Tangent 空間対応
- MR テクスチャのライティングへの反映
- 簡易 Cook-Torrance BRDF の実装

詳細な実装状況と開発計画は [`Plan.md`](Plan.md) を参照してください。

## 開発方針

- 動作確認できる単位で実装する
- DirectX 12 のリソース管理は単純さを優先する
- 実装を進めながら設計を見直す
- 過剰な抽象化を避ける