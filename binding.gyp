{
	"targets": [
		{
			"target_name": "pikul",
			"sources": [ "pikul.c", "pikul_wrap.cxx" ],
			"conditions": [
				['OS=="freebsd"', {
					"include_dirs": [
						"/usr/local/include",
						"/usr/local/include/json-c"
					]
				}
				],
				['OS=="linux"', {
					"include_dirs": [
						"/usr/include",
						"/usr/include/json-c"
					]
				}
				]
			]
		}
	]
}

