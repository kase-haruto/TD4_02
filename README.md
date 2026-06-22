## ビルドと実行

このリポジトリにはゲームプロジェクトのみが含まれています。
エンジンのバイナリと SDK ファイルは、`.calyxproj` の `engineVersion` に一致する CalyxEngine の GitHub Release から、CalyxLauncher によってインストールされます。

1. Visual Studio で `TD4_02.sln` を開きます。
2. エンジンパッケージをインストールまたは更新したい場合は、`CalyxLauncher` をビルドします。
3. `TD4_02` をスタートアッププロジェクトに設定し、Debug、Develop、または Release で実行します。

`TD4_02` は、ゲームコードを DLL として `Generated/Outputs/<Configuration>` にビルドします。
`CalyxGame` はエンジン側のホスト実行ファイルであり、`.calyxproj` に保存されているパスからその DLL を読み込みます。

デフォルトの SDK パスは次のとおりです。

`%LOCALAPPDATA%\\CalyxEngine\\Engines\\v1.0.8\\SDK`


