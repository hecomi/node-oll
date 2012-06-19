node-oll
==============

はじめに
--------------
oll (Online-Learning Library: http://code.google.com/p/oll/) の node.js アドオンです。
ライセンス等は oll に準拠します。

インストール
--------------
git clone して次のコマンドを実行して下さい。

	$ node-waf configure build

wscript ファイル内の gcc のバージョンは適宜変更して下さい。デフォルトでは、g++-4.8 でコンパイルしています。

使い方
--------------
### アルゴリズムの選択 ###

	var oll = require('./build/Release/oll');
	var Perceptron  = new oll.P();
	// var ConfidenceWeighted = new oll.CW();

といった具合にアルゴリズムを選択することが出来ます。使用できるアルゴリズムは下記になります。

	P   : Perceptron
	AP  : Averaged Perceptron
	PA  : Passive Agressive
	PA1 : Passive Agressive I
	PA2 : Passive Agressive II
	PAK : Kernelized Passive Agressive
	CW  : Confidence Weighted

### パラメータの指定 ###

コンストラクタに引数を渡すことで初期値を指定することが出来ます。

	var Perceptron  = new oll.P({C: 2.0, bias: 1.0});

C は oll_train の -C パラメータに相当し、デフォルトで 1.0、bias は -b パラメータに相当し、デフォルトは 0.0 です。

詳細
--------------
その他詳細は http://d.hatena.ne.jp/hecomi/ をご参照下さい。

