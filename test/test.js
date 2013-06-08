var oll  = require('../oll')
  , PA1  = new oll.PA1()
  , exec = require('child_process').exec;

// 学習とテスト
PA1.add('+1 0:1.0  1:2.0 2:-1.0');
PA1.add('-1 0:-0.5 1:1.0 2:-0.5');
console.log(PA1.test('0:1.0 1:1.0')); // 0.1714285910129547

// 学習結果をファイルへ保存
PA1.save('test.dat');

var PA1_2 = new oll.PA1();
PA1_2.load('test.dat');
console.log(PA1_2.test('0:1.0 1:1.0')); // 0.1714285910129547

exec('rm test.dat', function(err) {
	if (err) throw err;
});

