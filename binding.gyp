{
	'targets': [{
		'target_name'  : 'oll',
		'sources'      : [ 'src/oll.cc', 'lib/oll-0.03/oll.cpp' ],
		'cflags'       : [ '-std=c++0x' ],
		'cflags_cc!'   : [ '-fno-rtti' ],
		'include_dirs' : [ 'lib/oll-0.03' ],
		'conditions'   : [
			['OS=="mac"', {
				'xcode_settings' : {
					'OTHER_CFLAGS': [
						'-std=c++0x',
					],
					'GCC_ENABLE_CPP_RTTI': 'YES',
				},
			},],
		],
	},],
}
