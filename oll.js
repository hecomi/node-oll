var fs   = require('fs')
  , path = require('path')
  , oll
;

if ( fs.existsSync( path.join(__dirname, './build/Debug') ) ) {
	oll = require('./build/Debug/oll');
} else if ( fs.existsSync( path.join(__dirname, './build/Release') ) ) {
	oll = require('./build/Release/oll');
} else {
	throw '"oll" has not been compiled yet.';
}

module.exports = oll;
