/**
** デルタD/Aコンバータを模したステレオ音声のハイレゾ化フィルタ
** version 1.00 Copyright(c)2017 yamashiro kaname
**  注意）
** 本プログラムは無保証です。
** 著作者は本プログラムによって発生したいかなる損害について責任を持ちません。
*/

---- 概要 ----

本プログラムは、音声の 48kHz/16bit の LINE-IN　入力を、192kHz/24bit 化して、
LINE-OUT　に出力します。
 
あらかじめ、音声が 192kHz/24bit で出力できるようにする必要があります。

---- 使用法 ----

・ubuntu　の場合、はじめにコマンドラインで以下を実行し、ALSAの開発パッケージをインストールします。

apt-get install libasound2-dev

・次に以下を実行し、コンパイルします。

cc -O3 -o transST-delta1G transST_delta1G100.c -lasound

・次に音声を 192kHz/24bit で出力できるようにして、以下を実行します。

./transST-delta1G

