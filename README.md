# JuceMinimum

CMake を使用した **JUCE の最小構成アプリケーション**です。

## 🚀 動作環境
- **OS**: Arch Linux (C++20 対応コンパイラ)
- **ビルドシステム**: CMake (3.22以上)
- **フレームワーク**: JUCE

## 🛠️ ビルドと実行方法

リポジトリをクローンした後、以下のコマンドでビルドおよび実行が可能です。

```bash
# 1. ビルド用フォルダの作成と移動
mkdir build && cd build

# 2. CMakeの構成
cmake ..

# 3. コンパイル
cmake --build .

# 4. アプリの実行
./JuceMinimum_artefacts/JuceMinimum
```

## 📝 ライセンス
このプロジェクトは [MIT License](LICENSE) のもとで公開されています。
