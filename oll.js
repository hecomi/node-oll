var addon = require('./build/Release/oll');
var OLL  = new addon.P();

function toData(x, y) {
	return "0:" + x + " 1:" + y;
}

function inequality(x, y) {
	return y < x*x;
}

for (var i = 0; i < 1000; ++i) {
	var x = Math.random() * 10;
	var y = Math.random() * 100;
	OLL.add(inequality(x, y), toData(x, y));
}

var success = 0, fault = 0;
for (var i = 0; i < 1000; ++i) {
	var x = Math.random() * 10;
	var y = Math.random() * 100;
	if (OLL.test(toData(x, y)) > 0 == inequality(x, y))
		++success;
	else
		++fault;
	console.log("x: " + x + "; y: " + y);
	console.log(inequality(x, y));
	console.log(OLL.test(toData(x, y)));
}

console.log(success);
console.log(fault);
console.log( success/(success + fault) * 100 + "%" );

