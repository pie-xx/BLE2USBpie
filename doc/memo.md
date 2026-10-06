# 開発中に覚えたこと

## 電源電圧低下時のESP32のメッセージ

デバッグ中になんか起動しなくなったのでChat GPTに聞いてみた。
```Q
esp32s3を再起動すると以下のメッセージがくり返し送信され、起動できません。どういうエラーで、対処法は？

entry 0x403c88b8
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0xf (BROWNOUT_RST),boot:0xb (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce2820,len:0x10cc
load:0x403c8700,len:0xc2c
load:0x403cb700,len:0x30b0
entry 0x403c88b8
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
```
それに対する返答は
```ChatGPT
rst:0xf (BROWNOUT_RST)

これは ESP32-S3が電源電圧低下（Brownout）を検出してリセットされたという意味です。起動 → 電圧低下 → Brownoutリセット → 再起動 → また電圧低下、というループになっています。
```

ESP32-S3を接続しているUSB HUBからデスク照明用LEDの電源も取ってたのだった。

```ChatGPT
ログを見ると、
load:0x3fce2820,len=...
load:0x403c8700,len=...
load:0x403cb700,len=...
entry 0x403c88b8

までは正常にFlashからブートローダ/プログラムを読み込んでいます。
その後にBrownoutしていると考えられるので、Flashの読み込み失敗より、アプリ起動直後に電源が落ちている可能性が高いです。
```

たぶん、デスク照明用LEDのタコ足で電源が低下していたところに、アプリでBLEを立ち上げたのがトドメになってリセットがかかったのだろう。照明のケーブルを抜いたら動作が安定するようになった。USB-HUBのタコ足配線はいけません。


## ESP32はI/Oまわりの処理が重いと落ちる

打鍵を速くすると、以下のようなログを出力して落ちることがあった。
```Q
E (104878) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
E (104878) task_wdt:  - IDLE0 (CPU 0)
E (104878) task_wdt: Tasks currently running:
E (104878) task_wdt: CPU 0: nimble_host
E (104878) task_wdt: CPU 1: loopTask
E (104878) task_wdt: Aborting.
E (104878) task_wdt: Print CPU 0 (current core) backtrace

Backtrace: 0x42003f23:0x3fcc21d0 0x42003f51:0x3fcc21f0  (略)
```
ESP32はFreeRTOSの元で動作しており、周辺機器ドライバーがCPU 0タスク、loop()がCPU 1と別タスクで実行されている。CPU 0の処理が重くなって、CPU 1に処理が回らなくなると、watch dogが働いてリセットされるが、このログをそれを示している。すでに修正済だが、以前はNimbleBLEの受信イベントで親指シフト処理やら大量のログ出力などをやっていたので、打鍵量が多くなるとCPU 0が重くなって落ちていた。


## パニックログの見方

パニックログを解析するツールがある。
```Q
再起動時のログは以下のとおり。このBacktraceなどからクラッシュ位置をトレースするやり方を教えて

(略)
Backtrace: 0x42003f23:0x3fcc21d0 0x42003f51:0x3fcc21f0 0x42004048:0x3fcc2210 0x42002a95:0x3fcc2240 0x420654aa:0x3fcc2280 0x42008345:0x3fcc22a0 0x4200e9f1:0x3fcc2310 0x4200ea2d:0x3fcc2330 0x420103a8:0x3fcc2350 0x4200e609:0x3fcc23b0 0x4200c7b5:0x3fcc23e0 
```

```ChatGPT
Arduino IDEなら addr2line で調べられる
ESP32-S3なので使用するのは通常、
xtensa-esp32s3-elf-addr2line.exe

です。
Windows + Arduino IDEなら、Arduino15以下にあります。例えば概ね、
C:\Users\<ユーザー名>\AppData\Local\Arduino15\
packages\esp32\tools\esp-x32\<バージョン>\
xtensa-esp32s3-elf\bin\
xtensa-esp32s3-elf-addr2line.exe

のような場所です。ESP32 Arduino Coreのバージョンによってディレクトリ構成は変わります。
```

うちの環境ではC:\Users\ {ユーザー名}
\AppData\Local\Arduino15\packages\esp32\tools\s3-gcc\2021r2-p5\bin\xtensa-esp32s3-elf-addr2line.exe にあった。

使い方は

1. elfファイルをArduino IDEから、Sketch → Export Compiled Binaryを実行して、プロジェクトディレクトリのbuildの下に作成しておく。
2. xtensa-esp32s3-elf-addr2line.exe  -pfiaC -e {elfファイルのパス} {Backtrace:以降に表示される値リスト} で解析ツールを起動する。

オプション-pfiaC指定の意味は
- -p: アドレスも表示
- -f: 関数名表示
- -i: inline関数も表示
- -a: アドレス表示
- -C: C++名を人間が読める形にdemangle

実行結果
```
xtensa-esp32s3-elf-addr2line.exe  -pfiaC -e {elfファイル名.ino.elf} 0x42003f23:0x3fcc21d0 0x42003f51:0x3fcc21f0 0x42004048:0x3fcc2210 0x42002a95:0x3fcc2240 0x420654aa:0x3fcc2280 0x42008345:0x3fcc22a0 0x4200e9f1:0x3fcc2310 0x4200ea2d:0x3fcc2330 0x420103a8:0x3fcc2350 0x4200e609:0x3fcc23b0 0x4200c7b5:0x3fcc23e0 0x42014b5f:0x3fcc2410 0x4201348c:0x3fcc2440 0x42013497:0x3fcc2460 0x4201ab49:0x3fcc2480 0x42008913:0x3fcc24a0 0x40382d45:0x3fcc24c0

0x42003f23: StringSumHelper::~StringSumHelper() at {開発パッケージパス}\esp32\hardware\esp32\3.3.12\cores\esp32/WString.h:403
 (inlined by) addQue(KeyReport) at {プロジェクトパス}/keyboardProc.cpp:79
0x42003f51: std::vector<KeyMkBrkInfo, std::allocator<KeyMkBrkInfo> >::_M_erase(__gnu_cxx::__normal_iterator<KeyMkBrkInfo*, std::vector<KeyMkBrkInfo, std::allocator<KeyMkBrkInfo> > >) at {開発パッケージパス}/esp32/tools/esp-x32/2601/xtensa-esp-elf/include/c++/14.2.0/bits/vector.tcc:190
0x42004048: putQue(unsigned char) at {プロジェクトパス}/keyboardProc.cpp:180 (discriminator 1)
 (inlined by) putQue(unsigned char) at {プロジェクトパス}/keyboardProc.cpp:144 (discriminator 1)
0x42002a95: notifyCallback(NimBLERemoteCharacteristic*, unsigned char*, unsigned int, bool) at {プロジェクトパス}/Lticka.ino:116
(以下略)
```
という感じ。今回はメモリアクセス違反ではなく、裏タスクが重いせいで落ちたので、そのものズバリの場所ではないが、ヒントにはなるかな。
