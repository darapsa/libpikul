use ExtUtils::MakeMaker;
WriteMakefile(
	NAME	=> 'Pikul',
	INC	=> '`pkg-config --cflags json-c`',
	LIBS	=> ['-lpikul -lcurl -ljson-c'],
	OBJECT	=> '$(O_FILES)',
	DEFINE	=> '-DLOG_PATH=\"/var/log/pikul.log\"'
);
