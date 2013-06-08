node-oll
==============

はじめに
--------------
oll (Online-Learning Library: http://code.google.com/p/oll/) の node.js アドオンです。
ライセンスは oll に準拠します。

インストール
--------------
```
$ npm install oll
```

使い方
--------------
### アルゴリズムの選択 ###

```javascript
var oll = require('./build/Release/oll');
var Perceptron  = new oll.P();
// var ConfidenceWeighted = new oll.CW();
```

といった具合にアルゴリズムを選択することが出来ます。使用できるアルゴリズムは下記になります。

```
P   : Perceptron
AP  : Averaged Perceptron
PA  : Passive Agressive
PA1 : Passive Agressive I
PA2 : Passive Agressive II
PAK : Kernelized Passive Agressive
CW  : Confidence Weighted
```

### パラメータの指定 ###

コンストラクタに引数を渡すことで初期値を指定することが出来ます。

```javascript
var Perceptron  = new oll.P({C: 2.0, bias: 1.0});
```

C は oll_train の -C パラメータに相当し、デフォルトで 1.0、bias は -b パラメータに相当し、デフォルトは 0.0 です。

### 学習とテスト、保存と読み込み ###

次のように学習とテスト、及び学習結果の保存と読み込みをすることができます。

```javascript
var oll = require('oll');
var PA1  = new oll.PA1();

// 学習とテスト
PA1.add(true,  '0:1.0  1:2.0 2:-1.0');
PA1.add(false, '0:-0.5 1:1.0 2:-0.5');
console.log(PA1.test('0:1.0 1:1.0')); // 0.1714285910129547

// 学習結果をファイルへ保存
PA1.save('test.dat');

var PA1_2 = new oll.PA1();
PA1_2.load('test.dat');
console.log(PA1_2.test('0:1.0 1:1.0')); // 0.1714285910129547
```

また、学習のさせかたは以下の形式でも可能です。

```javascript
PA1.add('+1 0:1.0  1:2.0 2:-1.0');
PA1.add('-1 0:-0.5 1:1.0 2:-0.5');
```

詳細
--------------
その他詳細は http://d.hatena.ne.jp/hecomi/ をご参照下さい。

